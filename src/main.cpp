/**
 * @file main.cpp - v0.12.0 MODULAR INITIALIZATION
 * @author JTB
 * @brief EARS Main Application - LVGL 9.3.0 + Touch + Fully Modular Architecture
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.12.0 - 20260210
 * NEW FEATURES:
 * - MAIN_initializationLib created - initialization functions extracted
 * - Cleaner main.cpp structure - only setup() and loop()
 * - Proper include organization following best practices
 * - All state machines and init functions in dedicated library
 * ============================================================================
 *
 * @version 0.12.0
 * @date 20260210
 * @copyright Copyright (c) 2026 JTB All Rights Reserved
 */

// ============================================================================
// INCLUDES - Organized by category for clarity and maintainability
// ============================================================================

// 1. SYSTEM HEADERS (Arduino framework)
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// 2. THIRD-PARTY LIBRARIES
#include <Arduino_GFX_Library.h>

// 3. PROJECT DEFINITION HEADERS (system-wide)
#include "EARS_versionDef.h"       // Project version definitions - must come first
#include "EARS_systemDef.h"        // System-wide definitions and enums - must come second
#include "EARS_toolsVersionDef.h"  // Build tools version tracking
#include "EARS_ws35tlcdPins.h"     // Hardware pin definitions
#include "EARS_rgb565ColoursDef.h" // Colour palette definitions

// 4. EARS LIBRARY HEADERS (alphabetical within group)
#include "EARS_backLightManagerLib.h"
#include "EARS_hapticLib.h"
#include "EARS_nvsEepromLib.h"
#include "EARS_screenSaverLib.h"
#include "EARS_sdCardLib.h"
#include "EARS_touchLib.h"

// 5. MAIN LIBRARY HEADERS (alphabetical)
#include "MAIN_core0TasksLib.h"
#include "MAIN_core1TasksLib.h"
#include "MAIN_displayLib.h"
#include "MAIN_drawingLib.h"
#include "MAIN_initializationLib.h" // NEW! Centralized initialization
#include "MAIN_lvglLib.h"
#include "MAIN_sysinfoLib.h"

// 6. DEVELOPMENT TOOLS (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

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

    // STEP 7: Initialize Touch Controller (via MAIN_initializationLib)
    MAIN_initialise_touch();

    // STEP 4: Initialize NVS (via MAIN_initializationLib)
    MAIN_initialise_nvs();

    // STEP 5: Initialize SD Card (via MAIN_initializationLib)
    MAIN_initialise_sd();

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
// END OF FILE - v0.12.0 MODULAR INITIALIZATION! 🎯
// ============================================================================
