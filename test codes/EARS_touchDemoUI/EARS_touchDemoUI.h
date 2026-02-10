/**
 * @file EARS_touchDemoUI.h
 * @author JTB
 * @brief Touch demonstration UI with interactive widgets
 * @version 1.0.0
 * @date 20260207
 *
 * @details
 * Simple demonstration UI to verify touch functionality
 * Features:
 * - ON/OFF switch
 * - Slider (0-100)
 * - Visual feedback for touch interaction
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __EARS_TOUCH_DEMO_UI_H__
#define __EARS_TOUCH_DEMO_UI_H__

#include <lvgl.h>

/**
 * @brief Create touch demonstration UI
 * @details Creates an interactive screen with switch and slider to test touch
 */
void create_touch_demo_ui();

#endif // __EARS_TOUCH_DEMO_UI_H__