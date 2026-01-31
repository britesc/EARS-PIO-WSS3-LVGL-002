/**
 * @file main.cpp - MILESTONE v0.4.1 - NVS Keys Aligned
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Main Application - Production-Ready Architecture
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * NEW IN v0.4.1:
 * - NVS library keys now use EARS_systemDef.h definitions
 * - All NVS keys properly aligned with system standards
 * - KEY_VERSION: "nvsVersion" → "EARS_VC"
 * - KEY_ZAPNUMBER: "zapNumber" → "EARS_ZC"
 * - KEY_PASSWORD_HASH: "pwdHash" → "EARS_PW"
 * - KEY_NVS_CRC: "nvsCRC" → "EARS_32"
 *
 * COMPLETED STEPS:
 * - Step 1-4.3: Display, FreeRTOS, Libraries, Development Features ✓
 * - Step 5.0: NVS Initialization ✓
 * - Step 5.1: NVS Keys Aligned with systemDef.h ✓
 *
 * DEVELOPMENT BUILD (EARS_DEBUG=1):
 * - Hardware LED debugging
 * - Development screen with live stats
 * - System information display
 * - Boot banner and diagnostics
 * - NVS validation status reporting
 *
 * PRODUCTION BUILD (EARS_DEBUG=0):
 * - Zero development overhead
 * - LVGL/EEZ UI only
 * - Minimal binary size
 * - Professional end-user experience
 *
 * @version 0.4.1
 * @date 20260130
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 *
 * ============================================================================
 * DEVELOPMENT ROADMAP - NEXT STEPS:
 * ============================================================================
 *
 * ✅ COMPLETED (Steps 1-5.1):
 *    - Display hardware working (ST7796, Arduino GFX 1.5.5)
 *    - FreeRTOS dual-core (Core0=UI @1Hz, Core1=BG @10Hz)
 *    - EARS colour definitions (RGB565)
 *    - MAIN_drawingLib (rectangle functions)
 *    - MAIN_developmentFeaturesLib (dev screen, heartbeats)
 *    - MAIN_ledLib (hardware LED indicators)
 *    - MAIN_sysinfoLib (system information)
 *    - Clean architecture with compile-time dev feature removal
 *    - NVS initialization with validation and LED feedback
 *    - NVS keys aligned with EARS_systemDef.h
 *
 * 📋 TODO - STEP 6: Initialize SD Card on Core 1
 *    Location: Core1_Background_Task() - run after NVS init
 *    Purpose: Mount SD card for logging, config, images
 *    Dependencies: EARS_sdCardLib
 *
 * 📋 TODO - STEP 7: Initialize LVGL on Core 0
 *    Location: Core0_UI_Task() - run once at startup
 *    Purpose: Start LVGL graphics engine for UI
 *    Dependencies: LVGL 9.3.0, lv_conf.h
 *
 * 📋 TODO - STEP 8: Integrate Touch Input on Core 0
 * 📋 TODO - STEP 9: Add EEZ Studio Generated UI
 * 📋 TODO - STEP 10: Enable Backlight Manager (after NVS ready)
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
#include "MAIN_sysinfoLib.h" // Keep for production "About" screen

// EARS Libraries
#include "EARS_nvsEepromLib.h"

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
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
// Status Flags
// ============================================================================
// STEP 5: NVS status flag
volatile bool nvs_ready = false;

// TODO STEP 6: Add SD card status flag
// volatile bool sdcard_ready = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_display();

// STEP 5: NVS init function
void initialise_nvs();

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
#if EARS_DEBUG == 1
    // Initialize Serial for development debugging
    Serial.begin(EARS_DEBUG_BAUD_RATE);
    delay(500);
    uint32_t timeout = millis();
    while (!Serial && (millis() - timeout < 2000))
        delay(10);

    // Print boot banner and system info
    DEV_print_boot_banner();
    DEV_print_system_info();

    // Initialize development LEDs
    Serial.println("[INIT] Initialising development LEDs...");
    MAIN_led_init();
    MAIN_led_test_sequence(200);
    Serial.println("[OK] LEDs initialized");
#endif

    // ------------------------------------------------------------------------
    // Create synchronization primitives
    // ------------------------------------------------------------------------
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

    // ------------------------------------------------------------------------
    // Initialize display (before tasks for visual feedback)
    // ------------------------------------------------------------------------
    initialise_display();

    // ------------------------------------------------------------------------
    // Create FreeRTOS tasks
    // ------------------------------------------------------------------------
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
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
#endif

    // TODO STEP 7: Initialize LVGL here (once at startup)
    // initialise_lvgl();

    // TODO STEP 8: Initialize touch input here
    // initialise_touch();

    TickType_t xLastWakeTime = xTaskGetTickCount();

#if EARS_DEBUG == 1
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second for dev screen
#else
    // TODO STEP 7: Production LVGL rate
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz for LVGL
#endif

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core0_heartbeat();

        // Update development screen
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        {
            DEV_update_screen(gfx);
            xSemaphoreGive(xDisplayMutex);
        }
#else
        // TODO STEP 7: Production LVGL handler
        // if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        // {
        //     lv_task_handler();
        //     xSemaphoreGive(xDisplayMutex);
        // }
#endif

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

    // STEP 5: Initialize NVS (once at startup)
    initialise_nvs();

    // TODO STEP 6: Initialize SD card (after NVS)
    // initialise_sd_card();
    // sdcard_ready = true;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms = 10Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core1_heartbeat();

        // Toggle green LED every 500ms (every 5th iteration at 10Hz)
        if (DEV_get_core1_heartbeat() % 5 == 0)
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

// ============================================================================
// STEP 5: NVS INITIALIZATION
// ============================================================================
/**
 * @brief Initialize Non-Volatile Storage
 * @details Called once at startup from Core1_Background_Task
 *
 * Initializes NVS flash, validates existing data, and sets nvs_ready flag.
 * Provides comprehensive LED feedback:
 * - Green double-blink: NVS valid and ready
 * - Yellow slow blinks: NVS needs first-time configuration
 * - Red fast blinks: NVS initialization failed
 *
 * NVS Keys (from EARS_systemDef.h):
 * - EARS_VERSION_CODE "EARS_VC" - Version (2 hex digits)
 * - EARS_ZAPCODE "EARS_ZC" - ZapNumber (2 letters + 4 digits)
 * - EARS_PASSWORD_HASH "EARS_PW" - Password CRC32 hash
 * - EARS_CRC32 "EARS_32" - Overall NVS CRC32
 */
void initialise_nvs()
{
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising NVS...");
#endif

    // Initialize NVS flash
    if (!using_nvseeprom().begin())
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS initialization failed!");
        MAIN_led_error_pattern(3);
        MAIN_led_red_on();
#endif
        nvs_ready = false;
        return;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] NVS flash initialized");
#endif

    // Validate NVS contents
    NVSValidationResult validation = using_nvseeprom().validateNVS();

#if EARS_DEBUG == 1
    Serial.print("[NVS] Status: ");
    switch (validation.status)
    {
    case NVSStatus::VALID:
        Serial.println("VALID");
        break;
    case NVSStatus::UPGRADED:
        Serial.println("UPGRADED");
        break;
    case NVSStatus::INVALID_VERSION:
        Serial.println("INVALID VERSION");
        break;
    case NVSStatus::MISSING_ZAPNUMBER:
        Serial.println("MISSING ZAPNUMBER");
        break;
    case NVSStatus::MISSING_PASSWORD:
        Serial.println("MISSING PASSWORD");
        break;
    case NVSStatus::CRC_FAILED:
        Serial.println("CRC FAILED");
        break;
    case NVSStatus::INITIALIZATION_FAILED:
        Serial.println("INITIALIZATION FAILED");
        break;
    default:
        Serial.println("UNKNOWN");
        break;
    }

    Serial.printf("[NVS] Current Version: %d\n", validation.currentVersion);
    Serial.printf("[NVS] Expected Version: %d\n", validation.expectedVersion);
    Serial.printf("[NVS] ZapNumber Valid: %s\n", validation.zapNumberValid ? "YES" : "NO");
    Serial.printf("[NVS] Password Valid: %s\n", validation.passwordHashValid ? "YES" : "NO");
    Serial.printf("[NVS] CRC Valid: %s\n", validation.crcValid ? "YES" : "NO");
#endif

    // Set ready flag based on validation
    if (validation.status == NVSStatus::VALID ||
        validation.status == NVSStatus::UPGRADED)
    {
        nvs_ready = true;
#if EARS_DEBUG == 1
        Serial.println("[OK] NVS ready for use");
        MAIN_led_success_pattern();
#endif
    }
    else
    {
        nvs_ready = false;
#if EARS_DEBUG == 1
        Serial.println("[WARN] NVS needs configuration");
        MAIN_led_warning_pattern(2);
        MAIN_led_yellow_on();
#endif
    }
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
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initialising display...");
#endif

    // Direct backlight control (WORKS RELIABLY)
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

    // Color test pattern
#if EARS_DEBUG == 1
    Serial.println("[TEST] Drawing colour bars...");
#endif

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

#if EARS_DEBUG == 1
    Serial.println("[OK] Colour bars drawn");
#endif

    delay(2000);

#if EARS_DEBUG == 1
    // Draw development screen
    DEV_draw_screen(gfx);
    Serial.println("[OK] Development screen displayed");
#else
    // TODO STEP 7: Production - show LVGL loading screen or blank
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);
#endif
}

// ============================================================================
// END OF FILE - v0.4.1 NVS Keys Aligned with EARS_systemDef.h
// ============================================================================