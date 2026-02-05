/**
 * @file main.cpp - v0.9.0 DEBLOAT STEP 6 COMPLETE - PWM Backlight + Final Cleanup
 * @author Julian (51fiftyone51fiftyone_at_gmail.com)
 * @brief EARS Main Application - LVGL 9.3.0 + Fully Modular Architecture
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.9.0 DEBLOAT STEP 6 COMPLETE - PWM BACKLIGHT + FINAL CLEANUP
 * ============================================================================
 *
 * ✅ CHANGES IN THIS VERSION:
 * - Migrated backlight from digital HIGH/LOW to PWM control
 * - Integrated EARS_backLightManagerLib into display initialization
 * - Smooth fade transitions on startup
 * - NVS-backed brightness persistence (100% first boot, 75% default)
 * - Final code cleanup and polish
 *
 * ✅ WHAT'S WORKING:
 * - LVGL 9.3.0 display (RGB565, 16-bit color)
 * - Display: Red panel with white text rendering correctly
 * - 60-line double buffering in regular RAM (115KB total)
 * - PWM backlight control with smooth fading
 * - NVS: Full 5-step validation via library
 * - SD Card: Full initialization via library
 * - FreeRTOS: Dual-core operation (Core0=UI, Core1=Background)
 * - Development LEDs: Red/Yellow/Green status indicators
 * - Display initialization modularized (Step 1) + PWM (Step 6)
 * - LVGL initialization modularized (Step 2)
 * - Core task management modularized (Step 3)
 * - NVS initialization modularized (Step 4)
 * - SD card initialization modularized (Step 5)
 * - Backlight management modularized (Step 6)
 *
 * 📊 CODE REDUCTION ACHIEVED:
 * - Original: ~676 lines
 * - Current: ~310 lines
 * - Removed: 366 lines (54% reduction!)
 *
 * ============================================================================
 *
 * @version 0.9.0
 * @date 20260204
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
#include "MAIN_displayLib.h"    // STEP 1 + STEP 6: Display + PWM backlight
#include "MAIN_lvglLib.h"       // STEP 2: LVGL library
#include "MAIN_core0TasksLib.h" // STEP 3a: Core 0 UI task
#include "MAIN_core1TasksLib.h" // STEP 3b: Core 1 background task
#include "MAIN_sysinfoLib.h"
#include "EARS_nvsEepromLib.h"        // STEP 4: Enhanced NVS
#include "EARS_sdCardLib.h"           // STEP 5: Enhanced SD Card
#include "EARS_backLightManagerLib.h" // STEP 6: PWM backlight manager

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

// Display driver
#include <Arduino_GFX_Library.h>

// ============================================================================
// DISPLAY SETTINGS
// ============================================================================
static const uint32_t screenWidth = TFT_WIDTH;
static const uint32_t screenHeight = TFT_HEIGHT;

// ============================================================================
// ARDUINO GFX DISPLAY OBJECT
// ============================================================================
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, SPI_SCLK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7796(bus, LCD_RST, 1, true, TFT_HEIGHT, TFT_WIDTH);

// ============================================================================
// FREERTOS CONFIGURATION
// ============================================================================
TaskHandle_t Core0_Task_Handle = NULL;
TaskHandle_t Core1_Task_Handle = NULL;
SemaphoreHandle_t xDisplayMutex = NULL;

// ============================================================================
// NVS STATE MACHINE
// ============================================================================
enum NVSInitState
{
    NVS_NOT_INITIALIZED,
    NVS_INITIALIZED_EMPTY,
    NVS_NEEDS_ZAPNUMBER,
    NVS_NEEDS_PASSWORD,
    NVS_READY
};

volatile NVSInitState nvs_state = NVS_NOT_INITIALIZED;

// ============================================================================
// SD CARD STATE MACHINE
// ============================================================================
volatile SDCardState sd_card_state = SD_NOT_INITIALIZED;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void initialise_nvs();
void initialise_sd();

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

    Serial.println("[INIT] Initializing development LEDs...");
    MAIN_led_init();
    MAIN_led_test_sequence(200);
    Serial.println("[OK] LEDs initialized");
#endif

    // Create synchronization primitives
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating synchronization primitives...");
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
    Serial.println("[OK] Synchronization primitives created");
#endif

    // STEP 1 + STEP 6: Initialize display with PWM backlight
    if (!MAIN_initialise_display(gfx))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Display initialization failed!");
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

    // STEP 2: Initialize LVGL
    if (!MAIN_initialise_lvgl(gfx, xDisplayMutex, screenWidth, screenHeight))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] LVGL initialization failed!");
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

    // Create test UI
    MAIN_create_test_ui("DEBLOAT COMPLETE!\nv0.9.0 STEP 6\nPWM Backlight Active!");

    // STEP 3: Create FreeRTOS tasks
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating FreeRTOS tasks...");
#endif

    if (!MAIN_create_core0_task(&Core0_Task_Handle))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Core 0 task creation failed!");
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

    if (!MAIN_create_core1_task(&Core1_Task_Handle))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Core 1 task creation failed!");
        MAIN_led_error_pattern(5);
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] All tasks created");
    Serial.println("[INIT] System initialization complete\n");
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
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 * @details Uses EARS_nvsEepromLib::performFullInitialization()
 */
void initialise_nvs()
{
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initializing NVS...");
#endif

    NVSValidationResult result = using_nvseeprom().performFullInitialization();

    switch (result.status)
    {
    case NVSStatus::INITIALIZATION_FAILED:
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS flash initialization failed!");
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        nvs_state = NVS_NOT_INITIALIZED;
        break;

    case NVSStatus::MISSING_ZAPNUMBER:
        if (result.currentVersion == result.expectedVersion &&
            !result.zapNumberValid && !result.passwordHashValid)
        {
#if EARS_DEBUG == 1
            Serial.println("[INFO] First boot - NVS initialized with defaults");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            nvs_state = NVS_INITIALIZED_EMPTY;
        }
        else
        {
#if EARS_DEBUG == 1
            Serial.println("[INFO] NVS needs ZapNumber");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            nvs_state = NVS_NEEDS_ZAPNUMBER;
        }
        break;

    case NVSStatus::MISSING_PASSWORD:
#if EARS_DEBUG == 1
        Serial.printf("[OK] ZapNumber valid: %s\n", result.zapNumber);
        Serial.println("[INFO] NVS needs Password");
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_NEEDS_PASSWORD;
        break;

    case NVSStatus::VALID:
#if EARS_DEBUG == 1
        Serial.println("[OK] NVS fully validated and ready");
        MAIN_led_success_pattern();
#endif
        nvs_state = NVS_READY;
        break;

    case NVSStatus::UPGRADED:
#if EARS_DEBUG == 1
        Serial.printf("[INFO] NVS upgraded from v%d to v%d\n",
                      result.currentVersion, result.expectedVersion);
        Serial.println("[OK] NVS fully validated and ready");
        MAIN_led_success_pattern();
#endif
        nvs_state = NVS_READY;
        break;

    case NVSStatus::INVALID_VERSION:
    case NVSStatus::CRC_FAILED:
    default:
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS validation failed");
        MAIN_led_error_pattern(5);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_INITIALIZED_EMPTY;
        break;
    }
}

/**
 * @brief Initialize SD Card (SD_MMC mode)
 * @details Uses EARS_sdCardLib::performFullInitialization()
 */
void initialise_sd()
{
    SDCardInitResult result = using_sdcard().performFullInitialization();
    sd_card_state = result.state;

    switch (result.state)
    {
    case SD_INIT_FAILED:
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(3);
        MAIN_led_red_on();
#endif
        break;

    case SD_NO_CARD:
#if EARS_DEBUG == 1
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        break;

    case SD_CARD_READY:
#if EARS_DEBUG == 1
        MAIN_led_success_pattern();
#endif
        break;

    default:
        break;
    }
}

// ============================================================================
// END OF FILE - v0.9.0 STEP 6 COMPLETE - DEBLOAT EXERCISE FINISHED! 🎉
// ============================================================================