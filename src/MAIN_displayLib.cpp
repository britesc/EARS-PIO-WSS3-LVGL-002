/**
 * @file MAIN_displayLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Display initialisation and management for EARS
 * @details Handles Arduino GFX library initialisation for Waveshare 3.5" LCD
 * @version 0.3.0
 * @date 20260204
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_displayLib.h"
#include "MAIN_drawingLib.h"
#include "EARS_systemDef.h"
#include "EARS_backLightManagerLib.h" // STEP 6: PWM backlight control

// Only include LED lib if debug mode enabled
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#endif

/******************************************************************************
 * Display Initialisation
 *****************************************************************************/

/**
 * @brief Initialise the display hardware with PWM backlight control
 * @param gfx Pointer to Arduino_GFX object
 * @return true if successful, false if failed
 */
bool MAIN_initialise_display(Arduino_GFX *gfx)
{
    DEBUG_PRINTLN("[INIT] Initialising display...");

    // ========================================================================
    // STEP 1: Initialize PWM backlight manager (STEP 6 - PWM MIGRATION)
    // ========================================================================
    DEBUG_PRINTLN("[INIT] Initialising PWM backlight manager...");

    // Initialize backlight manager with:
    // - Pin: GFX_BL (GPIO 6)
    // - PWM Channel: 0
    // - PWM Frequency: 5000 Hz
    // - PWM Resolution: 8-bit (0-255)
    if (!using_backlightmanager().begin(GFX_BL, 0, 5000, 8))
    {
        DEBUG_PRINTLN("[ERROR] Backlight manager initialization failed!");
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        return false;
    }

    // Turn backlight OFF during display initialization (prevents flicker)
    using_backlightmanager().setBrightness(0);
    DEBUG_PRINTLN("[OK] Backlight manager initialized (brightness: 0%)");

    // ========================================================================
    // STEP 2: Small delay to ensure display power stable
    // ========================================================================
    delay(100);

    // ========================================================================
    // STEP 3: Initialise the display hardware
    // ========================================================================
    if (!gfx->begin())
    {
        DEBUG_PRINTLN("[ERROR] Display initialisation failed!");
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        return false;
    }

    DEBUG_PRINTLN("[OK] Display hardware initialised");

    // ========================================================================
    // STEP 4: Set display rotation (1 = landscape, USB port on left)
    // ========================================================================
    gfx->setRotation(1);
    DEBUG_PRINTLN("[OK] Display rotation set to landscape");

    // ========================================================================
    // STEP 5: CRITICAL - Fill screen BLACK multiple times to clear framebuffer
    // ========================================================================
    DEBUG_PRINTLN("[INFO] Clearing display framebuffer...");
    for (int i = 0; i < 3; i++)
    {
        MAIN_clear_screen(gfx, EARS_RGB565_BLACK);
        delay(10);
    }
    DEBUG_PRINTLN("[OK] Display framebuffer cleared");

    // ========================================================================
    // STEP 6: Fade backlight ON smoothly (STEP 6 - PWM MIGRATION)
    // ========================================================================
    DEBUG_PRINTLN("[INFO] Fading backlight on...");

    // Check if this is initial device configuration
    if (using_backlightmanager().isInitialConfig())
    {
        // First boot - use 100% brightness
        DEBUG_PRINTLN("[INFO] Initial config detected - using 100% brightness");
        using_backlightmanager().fadeToBrightness(100, 500);
        using_backlightmanager().completeInitialConfig();
    }
    else
    {
        // Load saved brightness from NVS
        if (using_backlightmanager().loadBrightness())
        {
            uint8_t brightness = using_backlightmanager().getBrightness();
            DEBUG_PRINTF("[INFO] Loaded brightness: %d%%\n", brightness);
            using_backlightmanager().fadeToBrightness(brightness, 500);
        }
        else
        {
            // Default to 75% if load fails
            DEBUG_PRINTLN("[INFO] Using default brightness: 75%");
            using_backlightmanager().fadeToBrightness(75, 500);
        }
    }

    DEBUG_PRINTLN("[OK] Backlight faded on");

    DEBUG_PRINTLN("[OK] Display initialisation complete");

#if EARS_DEBUG == 1
    MAIN_led_success_pattern();
#endif

    return true;
}

/**
 * @brief Test display with colour bars and text
 * @param gfx Pointer to Arduino_GFX object
 */
void MAIN_display_test_pattern(Arduino_GFX *gfx)
{
    DEBUG_PRINTLN("[TEST] Drawing test pattern...");

    // Clear screen
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    // Draw colour bars (vertical stripes)
    uint16_t barWidth = TFT_WIDTH / 8;

    gfx->fillRect(0 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_RED);
    gfx->fillRect(1 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GREEN);
    gfx->fillRect(2 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_BLUE);
    gfx->fillRect(3 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_YELLOW);
    gfx->fillRect(4 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_CYAN);
    gfx->fillRect(5 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_MAGENTA);
    gfx->fillRect(6 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_WHITE);
    gfx->fillRect(7 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GRAY);

    // Draw text overlay
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println("EARS Display Test");

    gfx->setCursor(10, 40);
    gfx->print("Resolution: ");
    gfx->print(TFT_WIDTH);
    gfx->print("x");
    gfx->println(TFT_HEIGHT);

    DEBUG_PRINTLN("[OK] Test pattern drawn");
}

/******************************************************************************
 * End of MAIN_displayLib.cpp
 ******************************************************************************/