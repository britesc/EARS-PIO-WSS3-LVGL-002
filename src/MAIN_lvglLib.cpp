/**
 * @file MAIN_lvglLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief LVGL 9.3.0 initialization and management (extracted from main.cpp)
 * @details Handles LVGL display setup, buffers, and callbacks
 * @version 1.0.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_lvglLib.h"
#include "EARS_systemDef.h"

// Only include LED lib if debug mode enabled
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#endif

/******************************************************************************
 * Static Variables (internal to library)
 *****************************************************************************/

// LVGL display object
static lv_display_t *lvgl_disp = NULL;

// Display buffers
static lv_color_t *disp_draw_buf1 = NULL;
static lv_color_t *disp_draw_buf2 = NULL;

// Display dimensions (stored during init)
static uint32_t display_width = 0;
static uint32_t display_height = 0;

// Arduino GFX pointer (needed for flush callback)
static Arduino_GFX *display_gfx = NULL;

// FreeRTOS mutex for display access
static SemaphoreHandle_t display_mutex = NULL;

/******************************************************************************
 * LVGL Callback Functions
 *****************************************************************************/

/**
 * @brief LVGL display flush callback
 * @details Called by LVGL when a region needs to be drawn to the display
 */
void MAIN_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    if (display_mutex != NULL && display_gfx != NULL)
    {
        if (xSemaphoreTake(display_mutex, portMAX_DELAY) == pdTRUE)
        {
            display_gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
            xSemaphoreGive(display_mutex);
        }
    }

    lv_display_flush_ready(disp);
}

/**
 * @brief LVGL tick callback
 * @details Provides millisecond timing to LVGL
 */
uint32_t MAIN_lvgl_tick_cb(void)
{
    return millis();
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**
 * @brief Force clear LVGL display buffers
 */
void MAIN_clear_lvgl_buffers(void)
{
    uint32_t bufSize = display_width * LVGL_BUFFER_LINES * 2; // *2 for RGB565

    if (disp_draw_buf1 != NULL)
    {
        memset(disp_draw_buf1, 0x00, bufSize);
    }

    if (disp_draw_buf2 != NULL)
    {
        memset(disp_draw_buf2, 0x00, bufSize);
    }

    DEBUG_PRINTLN("[OK] LVGL buffers cleared");
}

/**
 * @brief Get LVGL display object
 */
lv_display_t *MAIN_get_lvgl_display(void)
{
    return lvgl_disp;
}

/**
 * @brief Initialize LVGL 9.3.0 display system
 */
bool MAIN_initialise_lvgl(Arduino_GFX *gfx, SemaphoreHandle_t displayMutex,
                          uint32_t screenWidth, uint32_t screenHeight)
{
    // Validate parameters
    if (gfx == NULL || displayMutex == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Invalid parameters to MAIN_initialise_lvgl");
        return false;
    }

    // Store parameters for callbacks
    display_gfx = gfx;
    display_mutex = displayMutex;
    display_width = screenWidth;
    display_height = screenHeight;

#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising LVGL 9.3.0...");

    // DIAGNOSTIC: Check if lv_conf.h is being read correctly
    Serial.print("[DIAG] sizeof(lv_color_t) = ");
    Serial.print(sizeof(lv_color_t));
    Serial.println(" bytes (should be 2!)");

    Serial.print("[DIAG] LV_COLOR_DEPTH = ");
    Serial.print(LV_COLOR_DEPTH);
    Serial.println(" (should be 16!)");
#endif

    // Initialize LVGL core
    lv_init();

    // Calculate buffer size (LVGL_BUFFER_LINES lines)
    uint32_t bufSize = screenWidth * LVGL_BUFFER_LINES;

#if EARS_DEBUG == 1
    Serial.print("[INFO] Buffer size: ");
    Serial.print(bufSize);
    Serial.println(" pixels");
    Serial.print("[INFO] Bytes per buffer: ");
    Serial.print(bufSize * 2);
    Serial.println(" bytes");
    Serial.print("[INFO] Total allocation: ");
    Serial.print(bufSize * 2 * 2);
    Serial.println(" bytes");
#endif

    // Allocate display buffers (bufSize * 2 for RGB565)
    disp_draw_buf1 = (lv_color_t *)malloc(bufSize * 2);
    disp_draw_buf2 = (lv_color_t *)malloc(bufSize * 2);

#if EARS_DEBUG == 1
    if (disp_draw_buf1)
    {
        Serial.println("[OK] Buffer 1 allocated");
    }
    if (disp_draw_buf2)
    {
        Serial.println("[OK] Buffer 2 allocated");
    }
#endif

    if (!disp_draw_buf1 || !disp_draw_buf2)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Buffer allocation failed!");
        if (!disp_draw_buf1)
            Serial.println("  buf1 is NULL");
        if (!disp_draw_buf2)
            Serial.println("  buf2 is NULL");
        MAIN_led_red_on();
#endif
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Buffers allocated");
#endif

    // Clear buffers immediately to prevent old content
    MAIN_clear_lvgl_buffers();

    // Create LVGL display
    lvgl_disp = lv_display_create(screenWidth, screenHeight);
    if (lvgl_disp == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] lv_display_create failed!");
        MAIN_led_red_on();
#endif
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] LVGL display created");
#endif

    // Set display buffers (size in BYTES = bufSize * 2)
    lv_display_set_buffers(lvgl_disp, disp_draw_buf1, disp_draw_buf2,
                           bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Set flush callback
    lv_display_set_flush_cb(lvgl_disp, MAIN_lvgl_flush_cb);

    // Set tick callback
    lv_tick_set_cb(MAIN_lvgl_tick_cb);

#if EARS_DEBUG == 1
    Serial.println("[OK] LVGL initialisation complete!");
#endif

    return true;
}

/**
 * @brief Create a simple test UI panel
 */
void MAIN_create_test_ui(const char *message)
{
    if (lvgl_disp == NULL)
    {
        DEBUG_PRINTLN("[ERROR] LVGL not initialised - cannot create test UI");
        return;
    }

    // Create red panel
    lv_obj_t *panel = lv_obj_create(lv_screen_active());
    lv_obj_set_size(panel, 400, 200);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFF0000), 0);

    // Create white text label
    lv_obj_t *label = lv_label_create(panel);
    lv_label_set_text(label, message);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

    DEBUG_PRINTLN("[OK] Test UI created");
}


/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_LVGL_getLibraryName() {
    return MAIN_LVGL::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_LVGL_getVersionEncoded() {
    return VERS_ENCODE(MAIN_LVGL::VERSION_MAJOR, 
                       MAIN_LVGL::VERSION_MINOR, 
                       MAIN_LVGL::VERSION_PATCH);
}

// Get version date
const char* MAIN_LVGL_getVersionDate() {
    return MAIN_LVGL::VERSION_DATE;
}

// Format version as string
void MAIN_LVGL_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_LVGL_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}


/******************************************************************************
 * End of MAIN_lvglLib.cpp
 ******************************************************************************/