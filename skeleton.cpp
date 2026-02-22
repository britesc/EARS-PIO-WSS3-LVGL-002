/**
 * @file main.cpp - v0.22.5
 * @author Julian 51fiftyone51fiftyone_at_gmail.com
 * @brief EARS Main Application - FAST Animation Startup
 *
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.22.5 - 20260216
 * ============================================================================
 * CHANGES FROM v0.21.0:
 * - OPTIMIZED STARTUP: Animation now starts within ~100ms of power-on
 * - Removed LED test sequence from startup (moved to background task)
 * - Streamlined display initialization (minimal steps before animation)
 * - Backlight turns ON immediately before animation
 * - Serial/debug output moved after animation starts
 *
 * STARTUP TIMELINE (v0.22.5):
 * - 0ms:    Power on
 * - ~50ms:  Display hardware ready
 * - ~60ms:  Backlight ON
 * - ~100ms: ANIMATION STARTS (user sees soldier!)
 * - 3000ms: Animation completes, LVGL loads
 *
 * @version 0.22.5
 * @date 20260217
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
#include "EARS_rgb888ColoursDef.h" // Colour palette definitions

// 4. EARS LIBRARY HEADERS (alphabetical within group)
#include "EARS_backLightManagerLib.h"
#include "EARS_hapticLib.h"
#include "EARS_nvsEepromLib.h"
#include "EARS_screenSaverLib.h"
#include "EARS_sdCardLib.h"
#include "EARS_touchLib.h"

// 5. MAIN LIBRARY HEADERS (alphabetical)
#include "MAIN_animationGfxLib.h" // Startup animation (Arduino GFX)
#include "CORE0_task01Lib.h"
#include "CORE1_task01Lib.h"
#include "MAIN_displayLib.h"
#include "MAIN_drawingLib.h"
#include "MAIN_initializationLib.h"
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

/**
 * @brief ULTRA-FAST setup - Animation visible within ~100ms
 * @details This version is OPTIMIZED FOR SPEED. Everything that can be deferred
 *          has been moved to after the animation starts or to background tasks.
 *
 *          CRITICAL PATH (must be fast):
 *          1. Create display mutex
 *          2. Initialize display hardware (gfx->begin())
 *          3. Clear screen once
 *          4. Turn backlight ON
 *          5. Start animation ← USER SEES THIS FAST!
 *          6. Create tasks
 *
 *          DEFERRED (happens in background):
 *          - Serial/debug output (after animation running)
 *          - LED initialization (in Core1 task)
 *          - System info printing (in Core1 task)
 *          - All other initialization
 *
 *          Result: Animation appears within ~100ms of power-on!
 */
void setup()
{
    // ========================================================================
    // CRITICAL PATH - Keep this section MINIMAL for fast animation startup
    // ========================================================================

    // STEP 1: Create display mutex (required for tasks)
    xDisplayMutex = xSemaphoreCreateMutex();
    if (xDisplayMutex == NULL)
    {
        // Fatal error - halt
        while (1)
            delay(1000);
    }

    // STEP 2: Initialize display hardware (minimal - just get it working)
    gfx->begin();

    // STEP 3: Clear screen once (black background for animation)
    gfx->fillScreen(BLACK);

    // STEP 4: Turn backlight ON (use existing function - fast enough)
    MAIN_initialise_display(gfx);

    // STEP 5: Initialize and START animation library
    if (!MAIN_AnimationGfxLib::initialise(gfx))
    {
        // Failed to initialize animation - halt
        while (1)
            delay(1000);
    }

    if (!MAIN_AnimationGfxLib::start())
    {
        // Failed to start animation - halt
        while (1)
            delay(1000);
    }

    // ========================================================================
    // ANIMATION NOW VISIBLE! User sees marching soldier within ~100ms!
    // Everything below happens while animation is running
    // ========================================================================

    // STEP 6: Start serial (now that animation is running)
#if EARS_DEBUG == 1
    Serial.begin(EARS_DEBUG_BAUD_RATE);
    delay(100); // Short delay for serial to stabilize

    // Print minimal startup message
    Serial.println("\n\n================================================================");
    Serial.println("  EARS - Equipment & Ammunition Reporting System");
    Serial.println("================================================================");
    Serial.printf("  Version:    %s.%s.%s Development\n",
                  EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR, EARS_APP_VERSION_PATCH);
    Serial.println("================================================================\n");

    Serial.println("[OK] Animation started - soldier marching!");
    Serial.println("[INFO] Full system info will print in background...\n");
#endif

    // STEP 7: Create FreeRTOS tasks
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating FreeRTOS tasks...");
#endif

    // Create Core 0 UI Task
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating Core 0 UI task...");
#endif

    if (!MAIN_create_core0_task(&Core0_Task_Handle))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 0 task!");
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 0 UI task created");
#endif

    // Create Core 1 Background Task
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating Core 1 background task...");
#endif

    if (!MAIN_create_core1_task(&Core1_Task_Handle))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 1 task!");
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 1 background task created");
    Serial.println("[OK] All tasks created");
    Serial.println("[INIT] Setup complete - animation running on Core0");
    Serial.println("[INIT] Configuration check running on Core1");
    Serial.println("=====================================\n");
#endif
}

// ============================================================================
// ARDUINO LOOP - Empty (all work done in FreeRTOS tasks)
// ============================================================================

/**
 * @brief Empty loop function
 * @details All application logic runs in Core0 and Core1 FreeRTOS tasks.
 *          This loop runs on Core 1 but does nothing as the Core1 background
 *          task handles all Core 1 responsibilities.
 */
void loop()
{
    // Nothing here - all work done in FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}