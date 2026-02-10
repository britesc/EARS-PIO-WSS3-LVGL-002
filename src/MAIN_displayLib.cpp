/**
 * @file MAIN_displayLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Display initialisation and management for EARS
 * @details Handles Arduino GFX library initialisation for Waveshare 3.5" LCD
 * @version 1.0.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_displayLib.h"
#include "MAIN_drawingLib.h" // â† Need this for MAIN_clear_screen()
#include "EARS_systemDef.h"

// Only include LED lib if debug mode enabled
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#endif

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

    // Step 1: Backlight OFF during initialisation (prevents flicker/garbage)
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, LOW);
    DEBUG_PRINTLN("[OK] Backlight OFF (init phase)");

    // Step 2: Small delay to ensure display power stable
    delay(100);

    // Step 3: Initialise the display hardware
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

    // Step 4: Set display rotation (1 = landscape, USB port on left)
    gfx->setRotation(1);
    DEBUG_PRINTLN("[OK] Display rotation set to landscape");

    // Step 5: CRITICAL - Fill screen BLACK multiple times to clear ST7796 framebuffer
    DEBUG_PRINTLN("[INFO] Clearing display framebuffer...");
    for (int i = 0; i < 3; i++)
    {
        MAIN_clear_screen(gfx, EARS_RGB565_BLACK);
        delay(10); // Small delay between clears
    }
    DEBUG_PRINTLN("[OK] Display framebuffer cleared");

    // Step 6: Turn backlight ON
    digitalWrite(GFX_BL, HIGH);
    DEBUG_PRINTLN("[OK] Backlight ON");

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
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_Display_getLibraryName() {
    return MAIN_Display::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Display_getVersionEncoded() {
    return VERS_ENCODE(MAIN_Display::VERSION_MAJOR, 
                       MAIN_Display::VERSION_MINOR, 
                       MAIN_Display::VERSION_PATCH);
}

// Get version date
const char* MAIN_Display_getVersionDate() {
    return MAIN_Display::VERSION_DATE;
}

// Format version as string
void MAIN_Display_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_Display_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}


/******************************************************************************
 * End of MAIN_displayLib.cpp
 ******************************************************************************/