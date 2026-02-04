/**
 * @file main.cpp - v0.8.4 DEBLOAT STEP 3 - Separated Core Task Libraries
 * @author Julian (51fiftyone51fiftyone_at_gmail.com)
 * @brief EARS Main Application - LVGL 9.3.0 Working + Separated Core Tasks
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.8.4 DEBLOAT STEP 3 - SEPARATED CORE TASK LIBRARIES
 * ============================================================================
 *
 * ✅ CHANGES IN THIS VERSION:
 * - Removed Core0_UI_Task() function (~20 lines)
 * - Removed Core1_Background_Task() function (~30 lines)
 * - Removed CORE0/CORE1 configuration defines (~4 lines)
 * - Now uses MAIN_core0TasksLib for Core 0 UI task
 * - Now uses MAIN_core1TasksLib for Core 1 background task
 * - Added #include "MAIN_core0TasksLib.h"
 * - Added #include "MAIN_core1TasksLib.h"
 * - Simplified task creation to library calls
 *
 * ✅ WHAT'S WORKING:
 * - LVGL 9.3.0 display (RGB565, 16-bit color)
 * - Display: Red panel with white text rendering correctly
 * - 60-line double buffering in regular RAM (115KB total)
 * - NVS: Full 5-step validation with LED patterns
 * - SD Card: Full initialization with LED patterns
 * - FreeRTOS: Dual-core operation (Core0=UI, Core1=Background)
 * - Development LEDs: Red/Yellow/Green status indicators
 * - Display initialization modularized (Step 1)
 * - LVGL initialization modularized (Step 2)
 * - Core task management modularized (Step 3) - SEPARATED LIBRARIES!
 *
 * ============================================================================
 *
 * @version 0.8.4
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
#include "MAIN_displayLib.h"    // STEP 1: Display library
#include "MAIN_lvglLib.h"       // STEP 2: LVGL library
#include "MAIN_core0TasksLib.h" // STEP 3a: Core 0 UI task
#include "MAIN_core1TasksLib.h" // STEP 3b: Core 1 background task
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

// NOTE: Stack sizes and priorities now defined in MAIN_core0TasksLib.h and MAIN_core1TasksLib.h

// ============================================================================
// NVS STATE MACHINE (STEP 5)
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
// SD CARD STATE MACHINE (STEP 6A)
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

    // STEP 1: Initialize display using library (Arduino GFX)
    if (!MAIN_initialise_display(gfx))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Display initialization failed!");
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

    // STEP 2: Initialize LVGL using library
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
    MAIN_create_test_ui("LVGL 9.3.0 WORKING!\nv0.8.4 STEP 3\nCore Tasks Separated!");

    // STEP 3: Create FreeRTOS tasks using separated libraries
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating FreeRTOS tasks...");
#endif

    // Create Core 0 UI Task
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

    // Create Core 1 Background Task
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

// STEP 1: Display initialization moved to MAIN_displayLib
// STEP 2: LVGL initialization moved to MAIN_lvglLib
// STEP 3: Core task management moved to MAIN_core0TasksLib and MAIN_core1TasksLib

/**
 * @brief Initialize NVS (Non-Volatile Storage) - STEP 5
 * @details Full 5-step validation
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
    Serial.println("[INIT] Initializing NVS...");
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
 * @details Full validation with LED patterns
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
    Serial.println("[INIT] Initializing SD card...");
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
// END OF FILE - v0.8.4 STEP 3 - Separated Core Task Libraries Complete
// ============================================================================