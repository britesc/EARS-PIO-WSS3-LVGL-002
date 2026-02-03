/**
 * @file main.cpp - v0.8.1 COMPLETE - NVS/SD Validation Restored
 * @author Julian (51fiftyone51fiftyone_at_gmail.com)
 * @brief EARS Main Application - LVGL 9.3.0 Working + Full System Validation
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.8.1 COMPLETE - LVGL 9.3.0 + NVS/SD VALIDATION RESTORED
 * ============================================================================
 *
 * ✅ WHAT'S WORKING:
 * - LVGL 9.3.0 display (RGB565, 16-bit color)
 * - Display: Red panel with white text rendering correctly
 * - 60-line double buffering in regular RAM (115KB total)
 * - NVS: Full 5-step validation with LED patterns
 * - SD Card: Full initialization with LED patterns
 * - FreeRTOS: Dual-core operation (Core0=UI, Core1=Background)
 * - Development LEDs: Red/Yellow/Green status indicators
 *
 * ✅ CRITICAL FIX - lv_conf.h Location:
 * - lv_conf.h MUST be in src/ directory (not just include/)
 * - NEVER manually include lv_conf.h (violates LVGL guidelines)
 * - ALWAYS let LVGL find it: #include <lvgl.h>
 *
 * ✅ LED PATTERNS RESTORED:
 * - RED ON = Critical error (NVS/SD/Display init failed)
 * - YELLOW ON = Warning (needs ZapNumber/Password, no SD card)
 * - GREEN blink = Heartbeat (system running)
 * - GREEN double-blink = Success (NVS/SD ready)
 *
 * ============================================================================
 *
 * @version 0.8.1
 * @date 20260203
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 */

// ============================================================================
// INCLUDES
// ============================================================================
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Project Headers
#include "EARS_versionDef.h"
#include "EARS_systemDef.h"
#include "EARS_toolsVersionDef.h"
#include "EARS_ws35tlcdPins.h"
#include "EARS_rgb565ColoursDef.h"
#include "MAIN_drawingLib.h"
#include "MAIN_sysinfoLib.h"

// NVS and SD Card Libraries
#include "EARS_nvsEepromLib.h"
#include "EARS_sdCardLib.h"

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

// Display driver
#include <Arduino_GFX_Library.h>

// LVGL 9.3.0 - CRITICAL: Only include lvgl.h, NOT lv_conf.h!
#include <lvgl.h>

// ============================================================================
// Display Settings
// ============================================================================
static const uint32_t screenWidth = TFT_WIDTH;
static const uint32_t screenHeight = TFT_HEIGHT;

// ============================================================================
// Arduino GFX display object
// ============================================================================
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, SPI_SCLK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7796(bus, LCD_RST, 1, true, TFT_HEIGHT, TFT_WIDTH);

// ============================================================================
// LVGL Display Variables
// ============================================================================
lv_display_t *lvgl_disp = NULL;
static lv_color_t *disp_draw_buf1 = NULL;
static lv_color_t *disp_draw_buf2 = NULL;

// ============================================================================
// FREERTOS CONFIGURATION
// ============================================================================
TaskHandle_t Core0_Task_Handle = NULL;
TaskHandle_t Core1_Task_Handle = NULL;
SemaphoreHandle_t xDisplayMutex = NULL;

#define CORE0_STACK_SIZE 8192
#define CORE1_STACK_SIZE 4096
#define CORE0_PRIORITY 2
#define CORE1_PRIORITY 1

// ============================================================================
// NVS STATE MACHINE (STEP 5) - RESTORED
// ============================================================================
enum NVSInitState
{
    NVS_NOT_INITIALIZED,   // Flash not initialized
    NVS_INITIALIZED_EMPTY, // Initialized but no user data
    NVS_NEEDS_ZAPNUMBER,   // Version set, needs ZapNumber
    NVS_NEEDS_PASSWORD,    // ZapNumber set, needs Password
    NVS_READY              // Fully configured and validated
};

volatile NVSInitState nvs_state = NVS_NOT_INITIALIZED;

// ============================================================================
// SD CARD STATE MACHINE (STEP 6A) - RESTORED
// ============================================================================
volatile SDCardState sd_card_state = SD_NOT_INITIALIZED;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_display();
void initialise_lvgl();
void initialise_nvs();
void initialise_sd();

// LVGL Callbacks
void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static uint32_t lvgl_tick_cb(void);

// ============================================================================
// ARDUINO SETUP - Runs once on Core 1
// ============================================================================
void setup()
{
#if EARS_DEBUG == 1
    Serial.begin(EARS_DEBUG_BAUD_RATE);
    delay(500);
    uint32_t timeout = millis();
    while (!Serial && (millis() - timeout < 2000))
        delay(10);

    DEV_print_boot_banner();
    DEV_print_system_info();

    Serial.println("[INIT] Initialising development LEDs...");
    MAIN_led_init();
    MAIN_led_test_sequence(200);
    Serial.println("[OK] LEDs initialized");
#endif

    // Create synchronization primitives
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating synchronisation primitives...");
#endif

    xDisplayMutex = xSemaphoreCreateMutex();
    if (xDisplayMutex == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create display mutex!");
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Synchronisation primitives created");
#endif

    // Initialize display (Arduino GFX)
    initialise_display();

    // Initialize LVGL
    initialise_lvgl();

    // Create FreeRTOS tasks
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating FreeRTOS tasks...");
#endif

    xTaskCreatePinnedToCore(Core0_UI_Task, "Core0_UI", CORE0_STACK_SIZE,
                            NULL, CORE0_PRIORITY, &Core0_Task_Handle, 0);
    if (Core0_Task_Handle == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 0 task!");
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 0 UI task created");
#endif

    xTaskCreatePinnedToCore(Core1_Background_Task, "Core1_Background",
                            CORE1_STACK_SIZE, NULL, CORE1_PRIORITY,
                            &Core1_Task_Handle, 1);
    if (Core1_Task_Handle == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 1 task!");
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 1 background task created");
    Serial.println("[INIT] System initialisation complete\n");
#endif
}

// ============================================================================
// ARDUINO LOOP - Runs on Core 1
// ============================================================================
void loop()
{
    delay(1000);
}

// ============================================================================
// CORE 0 TASK - UI, Display, and LVGL Processing
// ============================================================================
void Core0_UI_Task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
#endif

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz for LVGL

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core0_heartbeat();
#endif

        // Run LVGL task handler
        lv_timer_handler();

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// CORE 1 TASK - Background Processing
// ============================================================================
void Core1_Background_Task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Background Task started");
#endif

    // STEP 5: Initialize NVS (RESTORED FULL VALIDATION)
    initialise_nvs();

    // STEP 6A: Initialize SD card (RESTORED FULL VALIDATION)
    initialise_sd();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core1_heartbeat();

        // Toggle green LED every 500ms (heartbeat)
        if (DEV_get_core1_heartbeat() % 5 == 0)
        {
            MAIN_led_green_toggle();
        }
#endif

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// LVGL FLUSH CALLBACK
// ============================================================================
void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
    {
        gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
        xSemaphoreGive(xDisplayMutex);
    }

    lv_display_flush_ready(disp);
}

// ============================================================================
// LVGL TICK CALLBACK
// ============================================================================
static uint32_t lvgl_tick_cb(void)
{
    return millis();
}

// ============================================================================
// INITIALISATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize display hardware (Arduino GFX)
 */
void initialise_display()
{
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising display...");
#endif

    // Direct backlight control
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);

#if EARS_DEBUG == 1
    Serial.println("[OK] Backlight ON");
#endif

    if (!gfx->begin())
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Display init failed!");
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Display initialized");
    MAIN_led_success_pattern();
#endif

    // Clear to black
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);
}

/**
 * @brief Initialize LVGL 9.3.0 display system
 */
void initialise_lvgl()
{
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

    // Initialize LVGL
    lv_init();

    // Calculate buffer size (60 lines)
    uint32_t bufSize = screenWidth * 60;

#if EARS_DEBUG == 1
    Serial.print("[INFO] Buffer size: ");
    Serial.print(bufSize);
    Serial.println(" pixels");
    Serial.print("[INFO] Bytes per buffer (hardcoded): ");
    Serial.print(bufSize * 2);
    Serial.println(" bytes");
    Serial.print("[INFO] Total allocation: ");
    Serial.print(bufSize * 2 * 2);
    Serial.println(" bytes");
#endif

    // Allocate display buffers (hardcoded * 2 for RGB565)
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
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Buffers allocated");
#endif

    // Create LVGL display
    lvgl_disp = lv_display_create(screenWidth, screenHeight);
    if (lvgl_disp == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] lv_display_create failed!");
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] LVGL display created");
#endif

    // Set display buffers (size in BYTES = bufSize * 2)
    lv_display_set_buffers(lvgl_disp, disp_draw_buf1, disp_draw_buf2,
                           bufSize * 2, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Set flush callback
    lv_display_set_flush_cb(lvgl_disp, lvgl_flush_cb);

    // Set tick callback
    lv_tick_set_cb(lvgl_tick_cb);

#if EARS_DEBUG == 1
    Serial.println("[OK] LVGL initialization complete!");
#endif

    // Create test UI
    lv_obj_t *panel = lv_obj_create(lv_screen_active());
    lv_obj_set_size(panel, 400, 200);
    lv_obj_center(panel);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFF0000), 0);

    lv_obj_t *label = lv_label_create(panel);
    lv_label_set_text(label, "LVGL 9.3.0 WORKING!\nv0.8.1 COMPLETE\nNVS/SD Restored");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(label);

#if EARS_DEBUG == 1
    Serial.println("[OK] Test UI created");
#endif
}

/**
 * @brief Initialize NVS (Non-Volatile Storage) - STEP 5
 * @details RESTORED FULL 5-STEP VALIDATION
 *
 * 5-step initialization process:
 * 1. Initialize NVS flash
 * 2. Check if NVS is initialized (has version key)
 * 3. Validate ZapNumber exists and is valid format
 * 4. Check if password exists
 * 5. Run full CRC validation
 *
 * Sets nvs_state and LED patterns based on result:
 * - Red ON = Flash initialization failed (critical)
 * - Yellow ON = Needs configuration (ZapNumber or Password missing)
 * - Green double-blink = Fully configured and validated
 */
void initialise_nvs()
{
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising NVS...");
#endif

    // Step 1: Initialize NVS flash
    if (!using_nvseeprom().begin())
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS flash initialization failed!");
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        nvs_state = NVS_NOT_INITIALIZED;
        return;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] NVS flash initialized");
#endif

    // Step 2: Check if NVS is initialized (first boot?)
    if (!using_nvseeprom().isInitialized())
    {
#if EARS_DEBUG == 1
        Serial.println("[INFO] First boot detected - initializing NVS...");
#endif

        if (using_nvseeprom().initializeNVS())
        {
#if EARS_DEBUG == 1
            Serial.println("[OK] NVS initialized with defaults (Version=01, Backlight=100)");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            nvs_state = NVS_INITIALIZED_EMPTY;
            return;
        }
        else
        {
#if EARS_DEBUG == 1
            Serial.println("[ERROR] Failed to initialize NVS!");
            MAIN_led_error_pattern(5);
            MAIN_led_red_on();
#endif
            nvs_state = NVS_NOT_INITIALIZED;
            return;
        }
    }

    // Step 3: Check ZapNumber
    String zapNumber = using_nvseeprom().getZapNumber();
    if (zapNumber.length() == 0 || !using_nvseeprom().isValidZapNumber(zapNumber))
    {
#if EARS_DEBUG == 1
        Serial.println("[INFO] NVS needs ZapNumber");
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_NEEDS_ZAPNUMBER;
        return;
    }

#if EARS_DEBUG == 1
    Serial.printf("[OK] ZapNumber valid: %s\n", zapNumber.c_str());
#endif

    // Step 4: Check password
    if (!using_nvseeprom().hasPassword())
    {
#if EARS_DEBUG == 1
        Serial.println("[INFO] NVS needs Password");
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_NEEDS_PASSWORD;
        return;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Password hash exists");
#endif

    // Step 5: Full validation (including CRC check)
    NVSValidationResult result = using_nvseeprom().validateNVS();

    if (result.status == NVSStatus::VALID || result.status == NVSStatus::UPGRADED)
    {
#if EARS_DEBUG == 1
        Serial.println("[OK] NVS fully validated and ready");
        if (result.status == NVSStatus::UPGRADED)
        {
            Serial.printf("[INFO] NVS upgraded from v%d to v%d\n",
                          result.currentVersion, result.expectedVersion);
        }
        MAIN_led_success_pattern();
#endif
        nvs_state = NVS_READY;
    }
    else
    {
#if EARS_DEBUG == 1
        Serial.printf("[ERROR] NVS validation failed: ");
        switch (result.status)
        {
        case NVSStatus::INVALID_VERSION:
            Serial.println("Invalid version");
            break;
        case NVSStatus::CRC_FAILED:
            Serial.println("CRC check failed - possible tampering");
            break;
        default:
            Serial.println("Unknown error");
            break;
        }
        MAIN_led_error_pattern(5);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_INITIALIZED_EMPTY;
    }
}

/**
 * @brief Initialize SD Card (SD_MMC mode) - STEP 6A
 * @details RESTORED FULL VALIDATION WITH LED PATTERNS
 *
 * Initializes SD card using SD_MMC interface (1-bit SDIO mode)
 * Pins: CLK=11, CMD=10, D0=9 (verified from Waveshare schematic)
 *
 * Sets sd_card_state based on result:
 * - SD_NOT_INITIALIZED → Hasn't been tried yet
 * - SD_INIT_FAILED → Pin setup or begin() failed
 * - SD_NO_CARD → No card detected
 * - SD_CARD_READY → Card mounted and ready
 *
 * LED Patterns:
 * - Red blink = SD init failed (critical for logging)
 * - Yellow blink = No card detected (warning)
 * - Green double-blink = SD ready!
 */
void initialise_sd()
{
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising SD card...");
#endif

    // Try to initialize SD card
    if (!using_sdcard().begin())
    {
        sd_card_state = using_sdcard().getState();

        if (sd_card_state == SD_INIT_FAILED)
        {
#if EARS_DEBUG == 1
            Serial.println("[ERROR] SD card initialization failed!");
            MAIN_led_error_pattern(3);
            MAIN_led_red_on();
#endif
            return;
        }
        else if (sd_card_state == SD_NO_CARD)
        {
#if EARS_DEBUG == 1
            Serial.println("[WARNING] No SD card detected");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            return;
        }
    }

    // SD card ready!
    sd_card_state = SD_CARD_READY;

#if EARS_DEBUG == 1
    Serial.println("[OK] SD card initialized successfully");
    Serial.printf("[INFO] Card type: %s\n", using_sdcard().getCardType().c_str());
    Serial.printf("[INFO] Card size: %llu MB\n", using_sdcard().getCardSizeMB());
    Serial.printf("[INFO] Free space: %llu MB\n", using_sdcard().getFreeSpaceMB());

    // Create essential directories
    Serial.println("[INFO] Creating essential directories...");
    using_sdcard().createDirectory("/logs");
    using_sdcard().createDirectory("/config");
    using_sdcard().createDirectory("/images");

    MAIN_led_success_pattern();
#endif
}

// ============================================================================
// END OF FILE - v0.8.1 COMPLETE - NVS/SD Validation Restored
// ============================================================================