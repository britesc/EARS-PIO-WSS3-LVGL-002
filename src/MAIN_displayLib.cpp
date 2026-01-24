/**
 * @file MAIN_displayLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Display initialisation and management for EARS
 * @details Handles Arduino GFX library initialisation for Waveshare 3.5" LCD
 * @version 0.1.0
 * @date 20260124
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_displayLib.h"
#include "EARS_systemDef.h"

/******************************************************************************
 * Display Initialisation
 *****************************************************************************/

/**
 * @brief Initialise the display hardware
 * @param gfx Pointer to Arduino_GFX object
 * @return true if successful, false if failed
 */
bool MAIN_initialise_display(Arduino_GFX *gfx)
{
    DEBUG_PRINTLN("[INIT] Initialising display...");

    // Step 1: Initialise the backlight manager
    if (!using_backlightmanager().begin(GFX_BL, 0, 5000, 8))
    {
        DEBUG_PRINTLN("[ERROR] Backlight manager initialisation failed!");
        return false;
    }
    DEBUG_PRINTLN("[OK] Backlight manager initialised");

    // Step 2: Initialise the display hardware
    if (!gfx->begin())
    {
        DEBUG_PRINTLN("[ERROR] Display initialisation failed!");
        return false;
    }
    DEBUG_PRINTLN("[OK] Display hardware initialised");

    // Step 3: Set display rotation (1 = landscape, USB port on left)
    gfx->setRotation(1);
    DEBUG_PRINTLN("[OK] Display rotation set to landscape");

    // Step 4: Fill screen with black to clear any garbage
    gfx->fillScreen(EARS_RGB565_BLACK);
    DEBUG_PRINTLN("[OK] Screen cleared to black");

    // Note: Backlight brightness is already set by the manager
    // It will be 100% on first boot, then saved preference on subsequent boots

    DEBUG_PRINTLN("[OK] Display initialisation complete");
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
    gfx->fillScreen(EARS_RGB565_BLACK);

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