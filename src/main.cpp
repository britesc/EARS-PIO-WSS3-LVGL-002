/**
 * @file main.cpp - MILESTONE v0.5.0 - NVS Integration Complete
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Main Application - NVS Storage Ready
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * NEW IN v0.5.0 - STEP 5 COMPLETE:
 * - NVS (Non-Volatile Storage) fully integrated
 * - NVS state machine implemented (5 states)
 * - LED feedback for NVS status
 * - Complete NVS API (version, ZapNumber, password, backlight, CRC)
 * - constexpr system definitions for type safety
 * - All NVS keys properly mapped from EARS_systemDef.h
 *
 * PREVIOUS MILESTONES:
 * v0.3.3 - Clean architecture with development features library
 * v0.2.x - FreeRTOS dual-core implementation
 * v0.1.x - Display hardware working
 *
 * DEVELOPMENT BUILD (EARS_DEBUG=1):
 * - Hardware LED debugging
 * - Development screen with live stats
 * - NVS state displayed on screen
 * - Boot banner and diagnostics
 *
 * PRODUCTION BUILD (EARS_DEBUG=0):
 * - LVGL/EEZ UI only
 * - Minimal binary size
 * - Professional end-user experience
 *
 * @version 0.5.0
 * @date 20260130
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 *
 * ============================================================================
 * DEVELOPMENT ROADMAP:
 * ============================================================================
 *
 * ✅ COMPLETED - STEP 5: NVS Initialization
 *    - NVS library v1.7.0 complete
 *    - State machine: NOT_INITIALIZED → INITIALIZED_EMPTY →
 *                     NEEDS_ZAPNUMBER → NEEDS_PASSWORD → READY
 *    - LED patterns for each state
 *    - First boot creates NVS with version "01", backlight 100
 *    - Data persists across firmware uploads
 *    - Complete API: version, ZapNumber, password, backlight, CRC validation
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
 * 📋 TODO - STEP 10: Enable Backlight Manager (uses NVS backlight value)
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

// STEP 5: NVS Library
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
TaskHandle_t Core0_Task_Handle = NULL;
TaskHandle_t Core1_Task_Handle = NULL;
SemaphoreHandle_t xDisplayMutex = NULL;

#define CORE0_STACK_SIZE 8192
#define CORE1_STACK_SIZE 4096
#define CORE0_PRIORITY 2
#define CORE1_PRIORITY 1

// ============================================================================
// NVS STATE MACHINE (STEP 5)
// ============================================================================
enum NVSInitState
{
    NVS_NOT_INITIALIZED,   // Flash not initialized
    NVS_INITIALIZED_EMPTY, // Initialized but no user data (needs ZapNumber + Password)
    NVS_NEEDS_ZAPNUMBER,   // Version set, needs ZapNumber
    NVS_NEEDS_PASSWORD,    // ZapNumber set, needs Password
    NVS_READY              // Fully configured and validated
};

volatile NVSInitState nvs_state = NVS_NOT_INITIALIZED;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_display();
void initialise_nvs();

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

    // Initialize display
    initialise_display();

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
// CORE 0 TASK - UI, Display, and Touch Processing
// ============================================================================
void Core0_UI_Task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
#endif

    // TODO STEP 7: Initialize LVGL here

    TickType_t xLastWakeTime = xTaskGetTickCount();

#if EARS_DEBUG == 1
    const TickType_t xFrequency = pdMS_TO_TICKS(1000); // 1 second for dev screen
#else
    const TickType_t xFrequency = pdMS_TO_TICKS(5); // 200Hz for LVGL
#endif

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core0_heartbeat();

        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        {
            DEV_update_screen(gfx);
            xSemaphoreGive(xDisplayMutex);
        }
#else
        // TODO STEP 7: Production LVGL handler
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

    // STEP 5: Initialize NVS
    initialise_nvs();

    // TODO STEP 6: Initialize SD card after NVS

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 10Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core1_heartbeat();

        // Toggle green LED every 500ms
        if (DEV_get_core1_heartbeat() % 5 == 0)
        {
            MAIN_led_green_toggle();
        }
#endif

        // TODO STEP 6+: Background tasks here

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

// ============================================================================
// INITIALISATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize display hardware
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

    // Color test pattern
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

    delay(2000);

#if EARS_DEBUG == 1
    DEV_draw_screen(gfx);
    Serial.println("[OK] Development screen displayed");
#else
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);
#endif
}

/**
 * @brief Initialize NVS (Non-Volatile Storage) - STEP 5
 *
 * @details
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

// ============================================================================
// END OF FILE - v0.5.0 STEP 5 COMPLETE - NVS READY
// ============================================================================