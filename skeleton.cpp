/**
 * @file main.cpp - v0.11.0 TOUCH CONTROLLER INTEGRATION
 * @author JTB
 * @brief EARS Main Application - LVGL 9.3.0 + Touch + Fully Modular Architecture
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.11.0 - 20260207
 * NEW FEATURES:
 * - FT6236U/FT3267 touch controller integration (STEP 7)
 * - I2C pins corrected: SDA=8, SCL=7 (verified via I2C scanner)
 * - LVGL touch input device driver
 * - Touch library based on Waveshare TouchDrvFT6X36
 * - EARS_touchLib modular library following established EARS pattern
 * - performFullInitialization() method matching NVS/SD pattern
 * ============================================================================
 *
 * @version 0.24.0
 * @date 20260210
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
#include "EARS_versionDef.h" // Project version definitions - must come first.
#include "EARS_systemDef.h"  // System-wide definitions and enums - must come second.

// EARS Modular Definitions
#include "EARS_toolsVersionDef.h"
#include "EARS_ws35tlcdPins.h"
#include "EARS_rgb565ColoursDef.h"

// MAIN Modular Libraries
#include "MAIN_drawingLib.h"
#include "MAIN_displayLib.h"
#include "MAIN_lvglLib.h"
#include "MAIN_core0TasksLib.h"
#include "MAIN_core1TasksLib.h"
#include "MAIN_sysinfoLib.h"

// EARS Modular Libraries
#include "EARS_nvsEepromLib.h"
#include "EARS_sdCardLib.h"
#include "EARS_backLightManagerLib.h"
#include "EARS_touchLib.h"
#include "EARS_hapticLib.h"
#include "EARS_screenSaverLib.h"

//  #include "EARS_touchDemoUI.h"         // STEP 7: Touch demo UI - NEW!

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
// TOUCH CONTROLLER STATE MACHINE
// ============================================================================
volatile TouchState touch_state = TOUCH_NOT_INITIALIZED;
volatile bool touch_initialized = false;

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
void initialise_touch();
//  void create_touch_demo_ui(); // Touch demo UI

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

    // STEP 7: Initialize Touch Controller - NEW!
    initialise_touch();

    // STEP 4: Initialize NVS (BEFORE creating tasks)
    initialise_nvs();

    // STEP 5: Initialize SD Card (BEFORE creating tasks)
    initialise_sd();

    // // Create touch demo UI (replaces simple test UI)
    // create_touch_demo_ui();

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
// Could these be moved to a their appropraite fils? main.cpp is getting crowded with all the new features! Thank you.
// ============================================================================

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize Touch Controller
 * @details Uses EARS_touchLib::performFullInitialization()
 */
void initialise_touch()
{
    // Guard against duplicate initialization
    if (touch_state != TOUCH_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[TOUCH] Already initialized, skipping");
#endif
        return;
    }

    TouchInitResult result = using_touch().performFullInitialization(TOUCH_SDA, TOUCH_SCL);
    touch_state = result.state;

    switch (result.state)
    {
    case TOUCH_INIT_FAILED:
#if EARS_DEBUG == 1
        Serial.println("[WARNING] Touch initialization failed");
        Serial.println("          System will continue without touch input");
        MAIN_led_warning_pattern(3);
#endif
        touch_initialized = false;
        break;

    case TOUCH_READY:
#if EARS_DEBUG == 1
        Serial.printf("[OK] Touch ready: %s\n", result.modelName.c_str());
        Serial.printf("     I2C: 0x%02X @ SDA=%d, SCL=%d\n",
                      result.i2cAddress, result.sdaPin, result.sclPin);
        Serial.printf("     Max Touch Points: %d\n", result.maxTouchPoints);
        MAIN_led_success_pattern();
#endif
        touch_initialized = true;
        break;

    default:
        break;
    }
}

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 * @details Uses EARS_nvsEepromLib::performFullInitialization()
 */
void initialise_nvs()
{
    // Guard against duplicate initialization
    if (nvs_state != NVS_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[NVS] Already initialized, skipping");
#endif
        return;
    }

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
    // Guard against duplicate initialization
    if (sd_card_state != SD_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[SD] Already initialized, skipping");
#endif
        return;
    }

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
// End of file main.cpp
// ============================================================================