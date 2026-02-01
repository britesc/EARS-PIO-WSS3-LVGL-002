/**
 * @file skeleton.cpp - FROZEN MILESTONE v0.6.0 - SD Card Integration Complete
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Main Application - NVS + SD Card Storage Ready
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * FROZEN MILESTONE v0.6.0 - STEP 6A COMPLETE (20260131)
 * ============================================================================
 *
 * ✅ WHAT'S WORKING:
 * - Display: Waveshare 3.5" ESP32-S3 LCD (480x320, ST7796 driver)
 * - Backlight: GPIO 6 (no conflicts!)
 * - NVS Storage: Version control, ZapNumber, Password, CRC validation
 * - SD Card: 14.9GB SDHC detected via SD_MMC (pins 11,10,9)
 * - Directories: /logs, /config, /images auto-created
 * - FreeRTOS: Dual-core operation (Core0=UI, Core1=Background)
 * - Development screen: Live stats, heartbeat, system info
 *
 * ✅ VERIFIED HARDWARE:
 * - Display pins: MOSI=1, MISO=2, SCLK=5, DC=3, BL=6
 * - SD_MMC pins: CLK=11, CMD=10, D0=9 (verified from Waveshare schematic)
 * - LED pins: Red=40, Yellow=41, Green=42
 * - No pin conflicts between display and SD card!
 *
 * ✅ KEY LESSONS LEARNED:
 * - Waveshare changed pinout between revisions
 * - SD_MMC requires build flag: -D SOC_SDMMC_HOST_SUPPORTED
 * - Pin 6 (backlight) was wrongly documented as SD_SCK in old docs
 * - SD_MMC works in 1-bit mode with just CLK, CMD, D0
 *
 * ============================================================================
 * 🎯 NEXT STEP: STEP 6B or STEP 7
 * ============================================================================
 *
 * OPTION A - STEP 6B: Design Loading Screen in EEZ Studio Flow (ESF)
 * ────────────────────────────────────────────────────────────────────
 * Purpose: Create splash/loading animation for startup
 * Tool: EEZ Studio Flow 0.23.2 (0.24.0 has colour bug)
 * Status: Design ONLY - test in ESF simulator, don't integrate yet
 * Why wait: Need LVGL initialized first (STEP 7) before ESF code works
 *
 * What to design:
 * - Splash screen with EARS logo/text
 * - Progress bar showing: NVS init → SD init → Ready
 * - State machine to show Core1 initialization progress
 * - Export UI code but DON'T integrate yet
 *
 * OPTION B - STEP 7: Initialize LVGL on Core 0 (RECOMMENDED)
 * ────────────────────────────────────────────────────────────────────
 * Purpose: Start LVGL graphics engine for UI
 * Location: Core0_UI_Task() - run once at startup
 * Dependencies: LVGL 9.3.0, lv_conf.h (already configured)
 *
 * Key tasks:
 * 1. Initialize LVGL display driver
 * 2. Create display buffers in PSRAM
 * 3. Set up LVGL tick handler
 * 4. Test with simple LVGL label/button
 * 5. Then integrate ESF generated code (STEP 6C)
 *
 * RECOMMENDATION: Do STEP 7 first, then STEP 6B, then STEP 6C
 * This avoids designing ESF screens that can't be tested until LVGL works
 *
 * ============================================================================
 * PREVIOUS MILESTONES:
 * ============================================================================
 *
 * v0.5.0 - STEP 5: NVS (Non-Volatile Storage)
 *    - NVS library v1.8.0 complete
 *    - State machine: NOT_INITIALIZED → INITIALIZED_EMPTY →
 *                     NEEDS_ZAPNUMBER → NEEDS_PASSWORD → READY
 *    - LED patterns for each state
 *    - First boot creates NVS with version "01", backlight 100
 *    - Data persists across firmware uploads
 *    - Complete API: version, ZapNumber, password, backlight, CRC validation
 *
 * v0.6.0 - STEP 6A: SD Card Initialization (THIS MILESTONE)
 *    - SD library v2.2.0 using SD_MMC (pins 11, 10, 9)
 *    - State machine: NOT_INITIALIZED → INIT_FAILED / NO_CARD / READY
 *    - LED patterns for each state
 *    - Auto-creates /logs, /config, /images directories
 *    - Ready for logger, config files, image storage
 *    - Corrected Waveshare pin definitions from schematic
 *
 * v0.3.3 - Clean architecture with development features library
 * v0.2.x - FreeRTOS dual-core implementation
 * v0.1.x - Display hardware working
 *
 * ============================================================================
 * DEVELOPMENT vs PRODUCTION BUILDS:
 * ============================================================================
 *
 * DEVELOPMENT BUILD (EARS_DEBUG=1) - Current Configuration:
 * - Hardware LED debugging (Red, Yellow, Green)
 * - Development screen with live stats
 * - NVS + SD card state displayed on screen
 * - Boot banner and system diagnostics via Serial
 * - Heartbeat visible on Green LED
 *
 * PRODUCTION BUILD (EARS_DEBUG=0) - Future:
 * - LVGL/EEZ UI only
 * - Minimal binary size
 * - Professional end-user experience
 * - No serial output, no LEDs
 *
 * ============================================================================
 * CRITICAL BUILD CONFIGURATION:
 * ============================================================================
 *
 * platformio.ini MUST include:
 * - lib_ldf_mode = chain+  (NOT deep+)
 * - lib_compat_mode = soft
 * - Build flag: -D SOC_SDMMC_HOST_SUPPORTED (ESSENTIAL for SD_MMC!)
 *
 * Without SOC_SDMMC_HOST_SUPPORTED, you'll get: "FS.h: No such file"
 *
 * ============================================================================
 *
 * @version 0.6.0
 * @date 20260131
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

// STEP 5: NVS Library
#include "EARS_nvsEepromLib.h"

// STEP 6A: SD Card Library (SD_MMC mode)
#include "EARS_sdCardLib.h"

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

// Display driver
#include <Arduino_GFX_Library.h>

// ============================================================================
// TODO STEP 7: Uncomment when initializing LVGL
// ============================================================================
// #include <lvgl.h>
// #include "lv_conf.h"

// ============================================================================
// TODO STEP 9: Uncomment when EEZ UI is ready (after STEP 7 + 6B + 6C)
// ============================================================================
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
// SD CARD STATE MACHINE (STEP 6A)
// ============================================================================
volatile SDCardState sd_card_state = SD_NOT_INITIALIZED;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_display();
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

    // ========================================================================
    // 🎯 NEXT STEP 7: Initialize LVGL here
    // ========================================================================
    // 1. Create LVGL display buffers in PSRAM
    // 2. Initialize LVGL display driver
    // 3. Set up LVGL tick handler
    // 4. Test with simple label: lv_label_create(lv_scr_act())
    // 5. Replace DEV_update_screen() with lv_timer_handler()
    // ========================================================================

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
        // ====================================================================
        // 🎯 STEP 7: Replace this with LVGL handler
        // ====================================================================
        // lv_timer_handler();
        // ====================================================================
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

    // STEP 6A: Initialize SD card
    initialise_sd();

    // ========================================================================
    // 🎯 STEP 8+: Add additional background tasks here
    // ========================================================================
    // Examples:
    // - Logger initialization (needs SD card)
    // - WiFi/BLE setup
    // - Sensor polling
    // - Data sync
    // ========================================================================

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

        // Background processing goes here

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

/**
 * @brief Initialize SD Card (SD_MMC mode) - STEP 6A
 *
 * @details
 * Initializes SD card using SD_MMC interface (1-bit SDIO mode)
 * Correct pins verified from Waveshare schematic: CLK=11, CMD=10, D0=9
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
 *
 * Auto-creates essential directories:
 * - /logs - For system logging
 * - /config - For configuration files
 * - /images - For UI assets and screenshots
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

    // List root directory
    Serial.println("[INFO] Root directory contents:");
    using_sdcard().listDirectory("/", 0);

    // Create essential directories
    Serial.println("[INFO] Creating essential directories...");
    using_sdcard().createDirectory("/logs");
    using_sdcard().createDirectory("/config");
    using_sdcard().createDirectory("/images");

    MAIN_led_success_pattern();
#endif
}

// ============================================================================
// END OF FILE - FROZEN MILESTONE v0.6.0 - STEP 6A COMPLETE
// ============================================================================
//
// 🎯 NEXT STEPS:
//
// OPTION A: STEP 6B - Design loading screen in EEZ Studio (design only)
// OPTION B: STEP 7 - Initialize LVGL (recommended first)
//
// Both are valid next steps. STEP 7 is recommended because:
// - You can test LVGL works before designing ESF screens
// - ESF screens need LVGL to run anyway
// - Easier to debug if LVGL is working first
//
// See header comments for detailed next step instructions!
//
// ============================================================================