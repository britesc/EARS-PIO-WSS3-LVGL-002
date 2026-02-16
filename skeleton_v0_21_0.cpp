/**
 * @file main.cpp - v0.21.0
 * @author Julian 51fiftyone51fiftyone_at_gmail.com
 * @brief EARS Main Application - Animation + LVGL 9.3.0 + Touch
 *
 * @details Equipment & Ammunition Reporting System
 *          Dual-core ESP32-S3 implementation using FreeRTOS
 *
 * ============================================================================
 * VERSION v0.21.0 - 20260214
 * ============================================================================
 * CHANGES FROM v0.20.0:
 * - Integrated MAIN_animationLib for startup animation
 * - Moved ALL initialization to Core0/Core1 tasks (except display hardware)
 * - setup() now minimal - starts animation and creates tasks only
 * - Core0 handles animation → LVGL transition
 * - Core1 handles config checks and screen routing
 *
 * @version 0.21.0
 * @date 20260214
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
#include "MAIN_animationGfxLib.h"  // NEW: Startup animation (Arduino GFX)
#include "MAIN_core0TasksLib.h"
#include "MAIN_core1TasksLib.h"
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
 * @brief Minimal setup - Start animation and create tasks
 * @details This function is kept as lean as possible. All heavy initialization
 *          is delegated to Core0 and Core1 tasks:
 *
 *          SETUP RESPONSIBILITIES:
 *          1. Debug/LED initialization (development only)
 *          2. Create synchronization primitives (mutex)
 *          3. Initialize display hardware (gfx->begin())
 *          4. Initialize and start animation library
 *          5. Create Core0 and Core1 FreeRTOS tasks
 *
 *          CORE0 TASK RESPONSIBILITIES:
 *          - Update animation until complete
 *          - Initialize LVGL when animation finishes
 *          - Initialize touch controller
 *          - Load appropriate ESF screen based on Core1 signal
 *          - Run LVGL UI updates (lv_timer_handler)
 *
 *          CORE1 TASK RESPONSIBILITIES:
 *          - Initialize NVS (check for ZapNumber/Password)
 *          - Initialize SD Card
 *          - Determine device configuration state
 *          - Signal Core0 which screen to display (via MAIN_AnimationGfxLib::stop)
 *          - Continue background monitoring
 *
 *          This architecture ensures the animation starts IMMEDIATELY on power-up
 *          while all other initialization happens in parallel.
 */
void setup()
{
    // ========================================================================
    // STEP 1: Development/Debug Setup (compile out in production)
    // ========================================================================
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

    // ========================================================================
    // STEP 2: Create Synchronization Primitives
    // ========================================================================
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating synchronization primitives...");
#endif

    xDisplayMutex = xSemaphoreCreateMutex();
    if (xDisplayMutex == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create display mutex!");
        MAIN_led_red_on();
#endif
        while (1)
            delay(1000);
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Synchronization primitives created");
#endif

    // ========================================================================
    // STEP 3: Initialize Display Hardware (Minimal - Just Hardware)
    // ========================================================================
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initializing display hardware...");
#endif

    gfx->begin();
    gfx->fillScreen(BLACK); // Clear to black immediately

#if EARS_DEBUG == 1
    Serial.println("[OK] Display hardware initialized");
#endif

    // ========================================================================
    // STEP 4: Initialize and Start Animation (CRITICAL - Must be early!)
    // ========================================================================
#if EARS_DEBUG == 1
    Serial.println("[INIT] Initializing animation library...");
#endif

    if (!MAIN_AnimationGfxLib::initialise(gfx))
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Animation library initialization failed!");
        MAIN_led_red_on();
#endif
        // Continue anyway - display will be black
    }
    else
    {
        if (!MAIN_AnimationGfxLib::start())
        {
#if EARS_DEBUG == 1
            Serial.println("[ERROR] Animation start failed!");
            MAIN_led_red_on();
#endif
        }
        else
        {
#if EARS_DEBUG == 1
            Serial.println("[OK] Animation started - soldier marching!");
#endif
        }
    }

    // ========================================================================
    // STEP 5: Create FreeRTOS Tasks
    // ========================================================================
#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating FreeRTOS tasks...");
#endif

    // Create Core0 Task (UI + Animation)
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

    // Create Core1 Task (Background + Config Check)
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
    Serial.println("[INIT] Setup complete - animation running on Core0");
    Serial.println("[INIT] Configuration check running on Core1");
    Serial.println("=====================================\n");
#endif
}

// ============================================================================
// ARDUINO LOOP - Runs on Core 1 (Empty - All work done in tasks)
// ============================================================================
void loop()
{
    // All processing now handled by FreeRTOS tasks:
    // - Core0: Animation updates → LVGL UI
    // - Core1: Background monitoring and config checks
    delay(1000);
}

// ============================================================================
// END OF FILE - main.cpp v0.21.0
// ============================================================================
