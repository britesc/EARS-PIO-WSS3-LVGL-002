/**
 * @file main.cpp - MILESTONE v0.3.2 - LED + SysInfo Integration
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Main Application - Development with Hardware LED Indicators + System Info
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * NEW IN v0.3.2:
 * - Hardware LED debugging on GPIO40 (Red), GPIO41 (Yellow), GPIO42 (Green)
 * - LED patterns for initialization status
 * - Green heartbeat on Core 1
 * - Error indicators for failed initializations
 * - System information library (MAIN_sysinfoLib)
 * - Complete system report on boot
 *
 * @version 0.3.2
 * @date 20260128
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 *
 * ============================================================================
 * DEVELOPMENT ROADMAP - NEXT STEPS:
 * ============================================================================
 *
 * ✅ COMPLETED (Steps 1-4.2):
 *    - Display hardware working (ST7796, Arduino GFX 1.5.5)
 *    - FreeRTOS dual-core (Core0=UI @1Hz, Core1=BG @10Hz)
 *    - EARS colour definitions (RGB565)
 *    - MAIN_drawingLib (rectangle functions)
 *    - Live development screen with heartbeat counters
 *    - Display mutex for thread safety
 *    - MAIN_ledLib (hardware LED indicators)
 *    - MAIN_sysinfoLib (ESP32-S3 system information)
 *
 * 📋 TODO - STEP 5: Initialize NVS on Core 1
 *    Location: Core1_Background_Task() - run once at startup
 *    Purpose: Initialize Non-Volatile Storage for settings
 *    Dependencies: EARS_nvsEepromLib
 *    Tasks:
 *      - Call using_nvseeprom().begin()
 *      - Validate NVS (check version, CRC)
 *      - Handle first-time setup vs existing data
 *      - Store result in global flag for other cores to check
 *    Note: This fixes the backlight manager NVS dependency issue
 *
 * 📋 TODO - STEP 6: Initialize SD Card on Core 1
 *    Location: Core1_Background_Task() - run after NVS init
 *    Purpose: Mount SD card for logging, config, images
 *    Dependencies: EARS_sdCardLib
 *    Tasks:
 *      - Call using_sdcard().begin()
 *      - Create required directories (/logs, /config, /images)
 *      - Initialize EARS_logger with SD card reference
 *      - Log system startup info
 *
 * 📋 TODO - STEP 7: Initialize LVGL on Core 0
 *    Location: Core0_UI_Task() - run once at startup
 *    Purpose: Start LVGL graphics engine for UI
 *    Dependencies: LVGL 9.3.0, lv_conf.h
 *    Tasks:
 *      - Call lv_init()
 *      - Create display buffer (in PSRAM)
 *      - Register display driver with Arduino_GFX
 *      - Set up tick timer for LVGL
 *      - Load initial screen (can keep dev screen as fallback)
 *    Note: Keep development screen as fallback until EEZ UI ready
 *
 * 📋 TODO - STEP 8: Integrate Touch Input on Core 0
 *    Location: Core0_UI_Task() - in main loop
 *    Purpose: Handle touch screen input for LVGL
 *    Dependencies: Touch I2C driver, LVGL input driver
 *    Tasks:
 *      - Initialize touch controller (I2C)
 *      - Register LVGL input driver
 *      - Add touch polling to UI task loop
 *
 * 📋 TODO - STEP 9: Add EEZ Studio Generated UI
 *    Location: After LVGL initialization
 *    Purpose: Load and display EEZ Studio designed screens
 *    Dependencies: EEZ Studio Flow generated code
 *    Tasks:
 *      - Include ui/screens.h and ui/ui.h
 *      - Call ui_init() after LVGL init
 *      - Load default screen
 *      - Replace dev screen with real UI
 *
 * 📋 TODO - STEP 10: Enable Backlight Manager (after NVS ready)
 *    Location: initialise_display()
 *    Purpose: Replace direct GPIO with proper backlight management
 *    Dependencies: EARS_backLightManagerLib, working NVS
 *    Tasks:
 *      - Replace pinMode/digitalWrite with using_backlightmanager().begin()
 *      - Load saved brightness from NVS
 *      - Integrate with screensaver
 *
 * ============================================================================
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

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#endif

// Display driver
#include <Arduino_GFX_Library.h>

// TODO STEP 7: Uncomment when ready for LVGL
// #include <lvgl.h>
// #include "lv_conf.h"

// TODO STEP 9: Uncomment when EEZ UI is ready
// #include "ui/screens.h"
// #include "ui/ui.h"

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
// FREERTOS CONFIGURATION
// ============================================================================
TaskHandle_t Core0_Task_Handle = NULL; // UI and Display
TaskHandle_t Core1_Task_Handle = NULL; // Background Processing
SemaphoreHandle_t xDisplayMutex = NULL;

#define CORE0_STACK_SIZE 8192 // UI Task (LVGL, Display, Touch)
#define CORE1_STACK_SIZE 4096 // Background Task (Data Processing)
#define CORE0_PRIORITY 2      // Higher priority for UI responsiveness
#define CORE1_PRIORITY 1      // Lower priority for background tasks

// ============================================================================
// Development Mode Variables (remove when production ready)
// ============================================================================
volatile uint32_t core0_heartbeat = 0;
volatile uint32_t core1_heartbeat = 0;
volatile uint32_t display_updates = 0;

// TODO STEP 5: Add NVS status flag
// volatile bool nvs_ready = false;

// TODO STEP 6: Add SD card status flag
// volatile bool sdcard_ready = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_serial();
void initialise_display();
void draw_development_screen();
void update_development_screen();

// TODO STEP 5: Add NVS init function
// void initialise_nvs();

// TODO STEP 6: Add SD card init function
// void initialise_sd_card();

// TODO STEP 7: Add LVGL init function
// void initialise_lvgl();

// TODO STEP 8: Add touch init function
// void initialise_touch();

// ============================================================================
// ARDUINO SETUP - Runs once on Core 1
// ============================================================================
void setup()
{
    initialise_serial();

    DEBUG_PRINTLN("\n\n");
    DEBUG_PRINTLN("╔════════════════════════════════════════════════════════════╗");
    DEBUG_PRINTLN("║  EARS - Equipment & Ammunition Reporting System           ║");
    DEBUG_PRINTLN("╚════════════════════════════════════════════════════════════╝");
    DEBUG_PRINTF("  Version:    %s.%s.%s %s\n",
                 EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR,
                 EARS_APP_VERSION_PATCH, EARS_STATUS);
    DEBUG_PRINTF("  Compiler:   %s\n", EARS_XTENSA_COMPILER_VERSION);
    DEBUG_PRINTF("  Platform:   %s\n", EARS_ESPRESSIF_PLATFORM_VERSION);
    DEBUG_PRINTLN("╚════════════════════════════════════════════════════════════╝");
    DEBUG_PRINTLN();

#if EARS_DEBUG == 1
    // Print complete system information report
    MAIN_sysinfo_print_all();
#endif

    // ------------------------------------------------------------------------
    // Create synchronization primitives
    // ------------------------------------------------------------------------
    DEBUG_PRINTLN("[INIT] Creating synchronisation primitives...");
    xDisplayMutex = xSemaphoreCreateMutex();
    if (xDisplayMutex == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create display mutex!");
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Synchronisation primitives created");

    // ------------------------------------------------------------------------
    // Initialize development LEDs (debug mode only)
    // ------------------------------------------------------------------------
#if EARS_DEBUG == 1
    DEBUG_PRINTLN("[INIT] Initialising development LEDs...");
    MAIN_led_init();
    MAIN_led_test_sequence(200); // Power-on self-test
    DEBUG_PRINTLN("[OK] LEDs initialized");
#endif

    // ------------------------------------------------------------------------
    // Initialize display (before tasks for visual feedback)
    // ------------------------------------------------------------------------
    initialise_display();

    // ------------------------------------------------------------------------
    // Create FreeRTOS tasks
    // ------------------------------------------------------------------------
    DEBUG_PRINTLN("[INIT] Creating FreeRTOS tasks...");

    xTaskCreatePinnedToCore(Core0_UI_Task, "Core0_UI", CORE0_STACK_SIZE,
                            NULL, CORE0_PRIORITY, &Core0_Task_Handle, 0);
    if (Core0_Task_Handle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create Core 0 task!");
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Core 0 UI task created");

    xTaskCreatePinnedToCore(Core1_Background_Task, "Core1_Background",
                            CORE1_STACK_SIZE, NULL, CORE1_PRIORITY,
                            &Core1_Task_Handle, 1);
    if (Core1_Task_Handle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create Core 1 task!");
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Core 1 background task created");

    DEBUG_PRINTLN("[INIT] System initialisation complete\n");
}

// ============================================================================
// ARDUINO LOOP - Runs on Core 1 (kept minimal as tasks handle work)
// ============================================================================
void loop()
{
    delay(1000); // Prevent tight loop
}

// ============================================================================
// CORE 0 TASK - UI, Display, and Touch Processing
// ============================================================================
void Core0_UI_Task(void *parameter)
{
    DEBUG_PRINTLN("[CORE0] UI Task started");

    // TODO STEP 7: Initialize LVGL here (once at startup)
    // initialise_lvgl();

    // TODO STEP 8: Initialize touch input here
    // initialise_touch();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second for dev screen

    // TODO STEP 7: Change to 5ms (200Hz) when LVGL active:
    // const TickType_t xFrequency = pdMS_TO_TICKS(5);

    while (1)
    {
        core0_heartbeat++;

        // Current: Update development screen
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        {
            update_development_screen();
            xSemaphoreGive(xDisplayMutex);
        }

        // TODO STEP 7: Replace above with LVGL handler:
        // if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        // {
        //     lv_task_handler();
        //     xSemaphoreGive(xDisplayMutex);
        // }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// CORE 1 TASK - Background Processing
// ============================================================================
void Core1_Background_Task(void *parameter)
{
    DEBUG_PRINTLN("[CORE1] Background Task started");

    // TODO STEP 5: Initialize NVS (once at startup)
    // initialise_nvs();
    // nvs_ready = true;

    // TODO STEP 6: Initialize SD card (after NVS)
    // initialise_sd_card();
    // sdcard_ready = true;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms = 10Hz

    while (1)
    {
        core1_heartbeat++;

#if EARS_DEBUG == 1
        // Toggle green LED every 500ms (every 5th iteration at 10Hz)
        if (core1_heartbeat % 5 == 0)
        {
            MAIN_led_green_toggle();
        }
#endif

        // TODO STEP 5+: Add background processing here:
        // - Monitor system health
        // - Process data queues
        // - Handle SD card logging
        // - Update system statistics

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// INITIALISATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize serial communication for debugging
 */
void initialise_serial()
{
#if EARS_DEBUG == 1
    Serial.begin(EARS_DEBUG_BAUD_RATE);
    delay(500);
    uint32_t timeout = millis();
    while (!Serial && (millis() - timeout < 2000))
        delay(10);
#endif
}

/**
 * @brief Initialize display hardware
 * @details Uses direct GPIO backlight control (simple and reliable)
 *
 * TODO STEP 10: Replace with backlight manager after NVS is ready:
 * if (!using_backlightmanager().begin(GFX_BL, 0, 5000, 8)) { ... }
 */
void initialise_display()
{
    DEBUG_PRINTLN("[INIT] Initialising display...");

    // Direct backlight control (WORKS RELIABLY)
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
    DEBUG_PRINTLN("[OK] Backlight ON");

    if (!gfx->begin())
    {
        DEBUG_PRINTLN("[ERROR] Display init failed!");
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(10); // Flash red 10 times
        MAIN_led_red_on();          // Leave red on
#endif
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Display initialized");

#if EARS_DEBUG == 1
    MAIN_led_success_pattern(); // Quick green double-blink
#endif

    // Color test pattern
    DEBUG_PRINTLN("[TEST] Drawing colour bars...");
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    uint16_t barWidth = TFT_WIDTH / 8;
    MAIN_draw_filled_rect(gfx, 0 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_RED);
    MAIN_draw_filled_rect(gfx, 1 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GREEN);
    MAIN_draw_filled_rect(gfx, 2 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_BLUE);
    MAIN_draw_filled_rect(gfx, 3 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_YELLOW);
    MAIN_draw_filled_rect(gfx, 4 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_CYAN);
    MAIN_draw_filled_rect(gfx, 5 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_MAGENTA);
    MAIN_draw_filled_rect(gfx, 6 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_WHITE);
    MAIN_draw_filled_rect(gfx, 7 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GRAY);

    DEBUG_PRINTLN("[OK] Colour bars drawn");
    delay(2000);

    draw_development_screen();
    DEBUG_PRINTLN("[OK] Development screen displayed");
}

// ============================================================================
// DEVELOPMENT MODE DISPLAY FUNCTIONS
// (Keep these until LVGL/EEZ UI is ready - useful for debugging!)
// ============================================================================

/**
 * @brief Draw initial development screen layout
 */
void draw_development_screen()
{
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    // Title bar
    MAIN_draw_filled_rect(gfx, 0, 0, TFT_WIDTH, 40, EARS_RGB565_RS_PRIMARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 12);
    gfx->print("EARS - DEVELOPMENT MODE");

    // Version box
    MAIN_draw_rounded_rect(gfx, 10, 50, 220, 80, 5, EARS_RGB565_CS_PRIMARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(20, 60);
    gfx->print("Version:");
    gfx->setTextSize(2);
    gfx->setCursor(20, 75);
    gfx->printf("%s.%s.%s", EARS_APP_VERSION_MAJOR,
                EARS_APP_VERSION_MINOR, EARS_APP_VERSION_PATCH);
    gfx->setTextSize(1);
    gfx->setCursor(20, 105);
    gfx->print(EARS_STATUS);

    // System Info box
    MAIN_draw_rounded_rect(gfx, 240, 50, 230, 80, 5, EARS_RGB565_CS_SECONDARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(250, 60);
    gfx->print("Platform:");
    gfx->setCursor(250, 75);
    gfx->printf("%s @ %dMHz", MAIN_sysinfo_get_chip_model().c_str(),
                MAIN_sysinfo_get_cpu_freq_mhz());
    gfx->setCursor(250, 90);
    gfx->printf("Heap: %s free", MAIN_sysinfo_format_bytes(MAIN_sysinfo_get_free_heap()).c_str());
    gfx->setCursor(250, 105);
    gfx->printf("PSRAM: %s free", MAIN_sysinfo_format_bytes(MAIN_sysinfo_get_free_psram()).c_str());

    // Status labels
    gfx->setTextColor(EARS_RGB565_CS_TEXT);
    gfx->setTextSize(1);
    gfx->setCursor(10, 150);
    gfx->print("Core 0 (UI):");
    gfx->setCursor(10, 180);
    gfx->print("Core 1 (BG):");
    gfx->setCursor(10, 210);
    gfx->print("Uptime:");
    gfx->setCursor(10, 240);
    gfx->print("Display:");

    // Footer
    gfx->setTextColor(EARS_RGB565_GRAY);
    gfx->setCursor(10, 300);
    gfx->print("LED Heartbeat: GPIO42 (Green)");
}

/**
 * @brief Update live statistics on development screen
 */
void update_development_screen()
{
    display_updates++;
    uint32_t uptime_sec = millis() / 1000;

    // Clear update area
    MAIN_draw_filled_rect(gfx, 120, 145, 350, 100, EARS_RGB565_BLACK);

    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);

    // Core 0 heartbeat
    gfx->setCursor(120, 150);
    gfx->printf("Running (%lu beats)", core0_heartbeat);

    // Core 1 heartbeat
    gfx->setCursor(120, 180);
    gfx->printf("Running (%lu beats)", core1_heartbeat);

    // Uptime
    gfx->setCursor(120, 210);
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    gfx->printf("%02lu:%02lu:%02lu", hours, minutes, seconds);

    // Display updates
    gfx->setCursor(120, 240);
    gfx->printf("%lu updates", display_updates);

    // Heartbeat indicator (flashing dot)
    uint16_t color = (core0_heartbeat % 2) ? EARS_RGB565_GREEN : EARS_RGB565_DARKGRAY;
    gfx->fillCircle(450, 155, 8, color);
}

// ============================================================================
// END OF FILE - v0.3.2 with LED + SysInfo Integration
// ============================================================================