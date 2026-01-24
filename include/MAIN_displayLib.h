/**
 * @file MAIN_displayLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Display initialisation and management for EARS
 * @details Handles Arduino GFX library initialisation for Waveshare 3.5" LCD
 * @version 0.1.0
 * @date 20260124
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_DISPLAY_LIB_H__
#define __MAIN_DISPLAY_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "EARS_ws35tlcdPins.h"
#include "EARS_rgb565ColoursDef.h"
#include "EARS_backLightManagerLib.h"

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialise the display hardware and backlight
 * @param gfx Pointer to Arduino_GFX object
 * @return true if successful, false if failed
 */
bool MAIN_initialise_display(Arduino_GFX *gfx);

/**
 * @brief Test display with colour bars
 * @param gfx Pointer to Arduino_GFX object
 */
void MAIN_display_test_pattern(Arduino_GFX *gfx);

#endif // __MAIN_DISPLAY_LIB_H__

/******************************************************************************
 * End of MAIN_displayLib.h
 ******************************************************************************/