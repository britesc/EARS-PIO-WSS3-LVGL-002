/**
 * @file main.cpp - DEVELOPMENT STEP 6: SD Card & Logger Initialization
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief EARS Main Application - Adding SD Card and Logger Support
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * CURRENT STEP: Initialize SD Card and Logger on Core 1 with timing
 * DATE: 20260127
 *
 * @version 0.3.2
 * @date 20260127
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 *
 * ============================================================================
 * DEVELOPMENT ROADMAP - NEXT STEPS:
 * ============================================================================
 *
 * ✅ COMPLETED (Steps 1-6):
 *    - Display hardware working (ST7796, Arduino GFX 1.5.5)
 *    - FreeRTOS dual-core (Core0=UI @1Hz, Core1=BG @10Hz)
 *    - EARS colour definitions (RGB565)
 *    - MAIN_drawingLib (rectangle functions)
 *    - Live development screen with heartbeat counters
 *    - Display mutex for thread safety
 *    - NVS initialization and validation
 *    - SD Card initialization and directory creation
 *    - Logger system initialization
 *    - Core1 setup timing instrumentation
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

// STEP 5: NVS Library
#include "EARS_nvsEepromLib.h"

// STEP 6: SD Card and Logger Libraries
#include "EARS_sdCardLib.h"
#include "EARS_loggerLib.h"

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

// STEP 5: NVS status flags
volatile bool nvs_ready = false;
volatile bool nvs_first_time = false;
String nvs_status_text = "Initialising...";

// STEP 6: SD Card status flags
volatile bool sdcard_ready = false;
String sdcard_status_text = "Initialising...";

// STEP 6: Logger status flags
volatile bool logger_ready = false;
String logger_status_text = "Waiting for SD...";

// STEP 6: Core1 setup timing
volatile uint32_t core1_setup_time_ms = 0;
volatile bool core1_setup_complete = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_serial();
void initialise_display();
void draw_development_screen();
void update_development_screen();

// STEP 5: NVS init function
void initialise_nvs();

// STEP 6: SD card and logger init functions
void initialise_sd_card();
void initialise_logger();

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
    DEBUG_PRINTLN("  EARS - Equipment & Ammunition Reporting System");
    DEBUG_PRINTLN("╚════════════════════════════════════════════════════════════╝");
    DEBUG_PRINTF("  Version:    %s.%s.%s %s\n",
                 EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR,
                 EARS_APP_VERSION_PATCH, EARS_STATUS);
    DEBUG_PRINTF("  Compiler:   %s\n", EARS_XTENSA_COMPILER_VERSION);
    DEBUG_PRINTF("  Platform:   %s\n", EARS_ESPRESSIF_PLATFORM_VERSION);
    DEBUG_PRINTLN("╔════════════════════════════════════════════════════════════╗");
    DEBUG_PRINTLN();

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

    // STEP 6: Start timing the setup process
    uint32_t setup_start_time = millis();
    DEBUG_PRINTLN("\n[CORE1] ========== SETUP SEQUENCE START ==========");

    // STEP 5: Initialize NVS (once at startup)
    uint32_t nvs_start = millis();
    initialise_nvs();
    uint32_t nvs_duration = millis() - nvs_start;
    DEBUG_PRINTF("[TIMING] NVS initialization: %lu ms\n", nvs_duration);

    // STEP 6: Initialize SD card (after NVS)
    uint32_t sd_start = millis();
    initialise_sd_card();
    uint32_t sd_duration = millis() - sd_start;
    DEBUG_PRINTF("[TIMING] SD Card initialization: %lu ms\n", sd_duration);

    // STEP 6: Initialize Logger (after SD card)
    uint32_t logger_start = millis();
    initialise_logger();
    uint32_t logger_duration = millis() - logger_start;
    DEBUG_PRINTF("[TIMING] Logger initialization: %lu ms\n", logger_duration);

    // STEP 6: Calculate total setup time
    core1_setup_time_ms = millis() - setup_start_time;
    core1_setup_complete = true;

    DEBUG_PRINTLN("[CORE1] ========================================");
    DEBUG_PRINTF("[CORE1] TOTAL SETUP TIME: %lu ms\n", core1_setup_time_ms);
    DEBUG_PRINTLN("[CORE1] ========== SETUP SEQUENCE COMPLETE ==========\n");

    // Log the startup to the logger if it's ready
    if (logger_ready)
    {
        EARS_logger::getInstance().info("=== EARS System Started ===");
        EARS_logger::getInstance().infof("Version: %s.%s.%s",
                                         EARS_APP_VERSION_MAJOR,
                                         EARS_APP_VERSION_MINOR,
                                         EARS_APP_VERSION_PATCH);
        EARS_logger::getInstance().infof("Core1 setup time: %lu ms", core1_setup_time_ms);
        EARS_logger::getInstance().infof("NVS: %s", nvs_status_text.c_str());
        EARS_logger::getInstance().infof("SD Card: %s", sdcard_status_text.c_str());
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms = 10Hz

    while (1)
    {
        core1_heartbeat++;

        // TODO STEP 6+: Add background processing here:
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
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Display initialized");

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
// STEP 5: NVS INITIALIZATION
// ============================================================================

/**
 * @brief Initialize Non-Volatile Storage (NVS)
 * @details This function:
 *          1. Initializes NVS flash
 *          2. Validates existing data (or creates defaults)
 *          3. Sets global flags for other cores
 *          4. Reports status via serial
 */
void initialise_nvs()
{
    DEBUG_PRINTLN("\n[INIT] ========== NVS Initialization ==========");

    // Step 1: Initialize NVS flash
    DEBUG_PRINTLN("[NVS] Initializing NVS flash...");
    if (!using_nvseeprom().begin())
    {
        DEBUG_PRINTLN("[ERROR] NVS initialization failed!");
        nvs_status_text = "FAILED - Flash Error";
        nvs_ready = false;
        return;
    }
    DEBUG_PRINTLN("[OK] NVS flash initialized");

    // Step 2: Validate NVS data
    DEBUG_PRINTLN("[NVS] Validating stored data...");
    NVSValidationResult validation = using_nvseeprom().validateNVS();

    // Step 3: Process validation results
    switch (validation.status)
    {
    case NVSStatus::VALID:
        DEBUG_PRINTLN("[OK] NVS data valid");
        DEBUG_PRINTF("[NVS] Version: %d (current: %d)\n",
                     validation.currentVersion, validation.expectedVersion);
        DEBUG_PRINTF("[NVS] ZapNumber: %s\n",
                     validation.zapNumberValid ? validation.zapNumber : "NOT SET");
        nvs_status_text = "Ready (Valid)";
        nvs_first_time = false;
        nvs_ready = true;
        break;

    case NVSStatus::UPGRADED:
        DEBUG_PRINTLN("[OK] NVS data upgraded");
        DEBUG_PRINTF("[NVS] Upgraded from version %d to %d\n",
                     validation.currentVersion, validation.expectedVersion);
        nvs_status_text = "Ready (Upgraded)";
        nvs_first_time = false;
        nvs_ready = true;
        break;

    case NVSStatus::MISSING_ZAPNUMBER:
        DEBUG_PRINTLN("[WARN] First-time setup detected (no ZapNumber)");
        nvs_status_text = "First-time Setup";
        nvs_first_time = true;
        nvs_ready = true; // Ready, but needs user setup
        break;

    case NVSStatus::MISSING_PASSWORD:
        DEBUG_PRINTLN("[WARN] Password not set");
        nvs_status_text = "Password Required";
        nvs_first_time = true;
        nvs_ready = true; // Ready, but needs user setup
        break;

    case NVSStatus::INVALID_VERSION:
        DEBUG_PRINTLN("[ERROR] Invalid NVS version");
        DEBUG_PRINTF("[NVS] Found version %d, expected %d\n",
                     validation.currentVersion, validation.expectedVersion);
        nvs_status_text = "FAILED - Version";
        nvs_ready = false;
        break;

    case NVSStatus::CRC_FAILED:
        DEBUG_PRINTLN("[ERROR] NVS CRC check failed (data corrupted)");
        DEBUG_PRINTF("[NVS] Expected CRC: 0x%08X\n", validation.calculatedCRC);
        nvs_status_text = "FAILED - Corrupted";
        nvs_ready = false;
        break;

    case NVSStatus::INITIALIZATION_FAILED:
        DEBUG_PRINTLN("[ERROR] NVS could not be accessed");
        nvs_status_text = "FAILED - Access";
        nvs_ready = false;
        break;

    default:
        DEBUG_PRINTLN("[ERROR] Unknown NVS status");
        nvs_status_text = "FAILED - Unknown";
        nvs_ready = false;
        break;
    }

    // Step 4: Report final status
    DEBUG_PRINTLN("[NVS] ========================================");
    DEBUG_PRINTF("[NVS] Status: %s\n", nvs_status_text.c_str());
    DEBUG_PRINTF("[NVS] Ready: %s\n", nvs_ready ? "YES" : "NO");
    DEBUG_PRINTF("[NVS] First-time: %s\n", nvs_first_time ? "YES" : "NO");
    DEBUG_PRINTLN("[NVS] ========================================\n");

    // Step 5: If first-time, we could set default values here
    if (nvs_first_time && nvs_ready)
    {
        DEBUG_PRINTLN("[NVS] First-time setup - initializing defaults...");
        // Example: Set default version
        using_nvseeprom().putVersion(EARS_nvsEeprom::KEY_VERSION,
                                     EARS_nvsEeprom::CURRENT_VERSION);
        using_nvseeprom().updateNVSCRC();
        DEBUG_PRINTLN("[OK] Defaults initialized");
    }
}

// ============================================================================
// STEP 6: SD CARD INITIALIZATION
// ============================================================================

/**
 * @brief Initialize SD Card
 * @details This function:
 *          1. Mounts the SD card
 *          2. Creates required directories
 *          3. Reports card information
 *          4. Sets global flags
 */
void initialise_sd_card()
{
    DEBUG_PRINTLN("\n[INIT] ========== SD Card Initialization ==========");

    // Step 1: Initialize SD card
    DEBUG_PRINTLN("[SD] Mounting SD card...");
    if (!using_sdcard().begin())
    {
        DEBUG_PRINTLN("[ERROR] SD card initialization failed!");
        sdcard_status_text = "FAILED - No Card";
        sdcard_ready = false;
        return;
    }
    DEBUG_PRINTLN("[OK] SD card mounted");

    // Step 2: Display card information
    DEBUG_PRINTF("[SD] Type: %s\n", using_sdcard().getCardType().c_str());
    DEBUG_PRINTF("[SD] Size: %llu MB\n", using_sdcard().getCardSizeMB());
    DEBUG_PRINTF("[SD] Free: %llu MB\n", using_sdcard().getFreeSpaceMB());

    // Step 3: Create required directories
    DEBUG_PRINTLN("[SD] Creating directory structure...");

    bool dirs_ok = true;
    if (!using_sdcard().createDirectory("/logs"))
    {
        DEBUG_PRINTLN("[WARN] Failed to create /logs directory");
        dirs_ok = false;
    }
    else
    {
        DEBUG_PRINTLN("[OK] /logs directory ready");
    }

    if (!using_sdcard().createDirectory("/config"))
    {
        DEBUG_PRINTLN("[WARN] Failed to create /config directory");
        dirs_ok = false;
    }
    else
    {
        DEBUG_PRINTLN("[OK] /config directory ready");
    }

    if (!using_sdcard().createDirectory("/images"))
    {
        DEBUG_PRINTLN("[WARN] Failed to create /images directory");
        dirs_ok = false;
    }
    else
    {
        DEBUG_PRINTLN("[OK] /images directory ready");
    }

    // Step 4: Set status
    if (dirs_ok)
    {
        sdcard_status_text = "Ready";
        sdcard_ready = true;
    }
    else
    {
        sdcard_status_text = "Ready (Dir Warnings)";
        sdcard_ready = true; // Still usable
    }

    DEBUG_PRINTLN("[SD] ========================================");
    DEBUG_PRINTF("[SD] Status: %s\n", sdcard_status_text.c_str());
    DEBUG_PRINTF("[SD] Ready: %s\n", sdcard_ready ? "YES" : "NO");
    DEBUG_PRINTLN("[SD] ========================================\n");
}

// ============================================================================
// STEP 6: LOGGER INITIALIZATION
// ============================================================================

/**
 * @brief Initialize Logger System
 * @details This function:
 *          1. Initializes the logger with SD card reference
 *          2. Sets log level
 *          3. Creates log file
 *          4. Sets global flags
 */
void initialise_logger()
{
    DEBUG_PRINTLN("\n[INIT] ========== Logger Initialization ==========");

    // Step 1: Check SD card is ready
    if (!sdcard_ready)
    {
        DEBUG_PRINTLN("[ERROR] Logger requires SD card - skipping");
        logger_status_text = "FAILED - No SD";
        logger_ready = false;
        return;
    }

    // Step 2: Initialize logger
    DEBUG_PRINTLN("[LOG] Initializing logger system...");
    if (!EARS_logger::getInstance().begin("/logs/debug.log", "/config/ears.config", &using_sdcard()))
    {
        DEBUG_PRINTLN("[ERROR] Logger initialization failed!");
        logger_status_text = "FAILED - Init Error";
        logger_ready = false;
        return;
    }
    DEBUG_PRINTLN("[OK] Logger initialized");

    // Step 3: Set log level (DEBUG for development)
    EARS_logger::getInstance().setLogLevel(LogLevel::DEBUG);
    DEBUG_PRINTLN("[OK] Log level set to DEBUG");

    // Step 4: Test logging
    EARS_logger::getInstance().info("Logger system started");
    DEBUG_PRINTLN("[OK] Test log entry written");

    // Step 5: Set status
    logger_status_text = "Ready (DEBUG)";
    logger_ready = true;

    DEBUG_PRINTLN("[LOG] ========================================");
    DEBUG_PRINTF("[LOG] Status: %s\n", logger_status_text.c_str());
    DEBUG_PRINTF("[LOG] Ready: %s\n", logger_ready ? "YES" : "NO");
    DEBUG_PRINTLN("[LOG] ========================================\n");
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
    gfx->print("ESP32-S3 @ 240MHz");
    gfx->setCursor(250, 90);
    gfx->printf("Compiler: %s", EARS_XTENSA_COMPILER_VERSION);
    gfx->setCursor(250, 105);
    gfx->printf("Platform: %s", EARS_ESPRESSIF_PLATFORM_VERSION);

    // Size Reference Panel - Visual guide for UI elements
    gfx->setTextColor(EARS_RGB565_CS_TEXT);
    gfx->setTextSize(1);
    gfx->setCursor(10, 145);
    gfx->print("Size Reference (pixels):");

    // Draw size reference rectangles with labels
    int16_t x_start = 10;
    int16_t y_base = 160;

    // 12x12
    MAIN_draw_filled_rect(gfx, x_start, y_base, 12, 12, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start, y_base + 15);
    gfx->print("12");

    // 24x24
    MAIN_draw_filled_rect(gfx, x_start + 40, y_base, 24, 24, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start + 40, y_base + 27);
    gfx->print("24");

    // 36x36
    MAIN_draw_filled_rect(gfx, x_start + 90, y_base, 36, 36, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start + 90, y_base + 39);
    gfx->print("36");

    // 48x48
    MAIN_draw_filled_rect(gfx, x_start + 155, y_base, 48, 48, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start + 155, y_base + 51);
    gfx->print("48");

    // 64x64
    MAIN_draw_filled_rect(gfx, x_start + 230, y_base, 64, 64, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start + 230, y_base + 67);
    gfx->print("64");

    // 96x96
    MAIN_draw_filled_rect(gfx, x_start + 320, y_base - 10, 96, 96, EARS_RGB565_CS_PRIMARY);
    gfx->setCursor(x_start + 320, y_base + 89);
    gfx->print("96");

    // Status labels
    gfx->setTextColor(EARS_RGB565_CS_TEXT);
    gfx->setTextSize(1);
    gfx->setCursor(10, 230);
    gfx->print("Core 0 (UI):");
    gfx->setCursor(10, 245);
    gfx->print("Core 1 (BG):");
    gfx->setCursor(10, 260);
    gfx->print("Setup Time:");
    gfx->setCursor(10, 275);
    gfx->print("NVS:");
    gfx->setCursor(10, 290);
    gfx->print("SD Card:");
    gfx->setCursor(10, 305);
    gfx->print("Logger:");

    // Footer
    gfx->setTextColor(EARS_RGB565_GRAY);
    gfx->setCursor(10, 315);
    gfx->print("Step 6: SD Card & Logger Init");
}

/**
 * @brief Update live statistics on development screen
 */
void update_development_screen()
{
    display_updates++;
    uint32_t uptime_sec = millis() / 1000;

    // Clear update area
    MAIN_draw_filled_rect(gfx, 120, 225, 350, 100, EARS_RGB565_BLACK);

    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);

    // Core 0 heartbeat
    gfx->setCursor(120, 230);
    gfx->printf("Running (%lu beats)", core0_heartbeat);

    // Core 1 heartbeat
    gfx->setCursor(120, 245);
    gfx->printf("Running (%lu beats)", core1_heartbeat);

    // Setup time
    gfx->setCursor(120, 260);
    if (core1_setup_complete)
    {
        gfx->setTextColor(EARS_RGB565_GREEN);
        gfx->printf("%lu ms (Complete)", core1_setup_time_ms);
    }
    else
    {
        gfx->setTextColor(EARS_RGB565_YELLOW);
        gfx->print("In progress...");
    }

    // NVS status with colour coding
    gfx->setCursor(120, 275);
    if (nvs_ready)
    {
        if (nvs_first_time)
        {
            gfx->setTextColor(EARS_RGB565_YELLOW); // Warning - needs setup
        }
        else
        {
            gfx->setTextColor(EARS_RGB565_GREEN); // OK - ready
        }
    }
    else
    {
        gfx->setTextColor(EARS_RGB565_RED); // Error
    }
    gfx->print(nvs_status_text);

    // SD Card status with colour coding
    gfx->setCursor(120, 290);
    if (sdcard_ready)
    {
        gfx->setTextColor(EARS_RGB565_GREEN);
    }
    else
    {
        gfx->setTextColor(EARS_RGB565_RED);
    }
    gfx->print(sdcard_status_text);

    // Logger status with colour coding
    gfx->setCursor(120, 305);
    if (logger_ready)
    {
        gfx->setTextColor(EARS_RGB565_GREEN);
    }
    else
    {
        gfx->setTextColor(EARS_RGB565_RED);
    }
    gfx->print(logger_status_text);

    // Heartbeat indicator (flashing dot)
    uint16_t color = (core0_heartbeat % 2) ? EARS_RGB565_GREEN : EARS_RGB565_DARKGRAY;
    gfx->fillCircle(450, 235, 8, color);

    // Setup complete indicator
    uint16_t setup_color = core1_setup_complete ? EARS_RGB565_GREEN : EARS_RGB565_YELLOW;
    gfx->fillCircle(450, 265, 8, setup_color);

    // NVS status indicator
    uint16_t nvs_color = nvs_ready ? EARS_RGB565_GREEN : EARS_RGB565_RED;
    if (nvs_ready && nvs_first_time)
    {
        nvs_color = EARS_RGB565_YELLOW;
    }
    gfx->fillCircle(450, 280, 8, nvs_color);

    // SD Card status indicator
    uint16_t sd_color = sdcard_ready ? EARS_RGB565_GREEN : EARS_RGB565_RED;
    gfx->fillCircle(450, 295, 8, sd_color);

    // Logger status indicator
    uint16_t logger_color = logger_ready ? EARS_RGB565_GREEN : EARS_RGB565_RED;
    gfx->fillCircle(450, 310, 8, logger_color);
}

// ============================================================================
// END OF FILE - STEP 6: SD Card & Logger Initialization Complete
// ============================================================================