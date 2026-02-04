/**
 * @file MAIN_lvglLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief LVGL 9.3.0 initialization and management for EARS (extracted from main.cpp)
 * @details Handles LVGL display setup, buffers, and callbacks
 * @version 0.1.0
 * @date 20260204
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_LVGL_LIB_H__
#define __MAIN_LVGL_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <freertos/semphr.h>
#include "EARS_ws35tlcdPins.h"

/******************************************************************************
 * LVGL Configuration
 *****************************************************************************/

// Buffer configuration
#define LVGL_BUFFER_LINES 60 // Number of lines per buffer (60 lines = ~57KB per buffer)

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialize LVGL 9.3.0 display system
 * @param gfx Pointer to Arduino_GFX display object
 * @param displayMutex FreeRTOS mutex for display access synchronization
 * @param screenWidth Display width in pixels
 * @param screenHeight Display height in pixels
 * @return true if initialization successful
 * @return false if initialization failed
 */
bool MAIN_initialise_lvgl(Arduino_GFX *gfx, SemaphoreHandle_t displayMutex,
                          uint32_t screenWidth, uint32_t screenHeight);

/**
 * @brief Get LVGL display object
 * @return lv_display_t* Pointer to LVGL display (NULL if not initialized)
 */
lv_display_t *MAIN_get_lvgl_display(void);

/**
 * @brief Force clear LVGL display buffers
 * @details Zeros out both draw buffers to prevent old content showing
 */
void MAIN_clear_lvgl_buffers(void);

/**
 * @brief Create a simple test UI panel
 * @param message Text to display on the test panel
 */
void MAIN_create_test_ui(const char *message);

/******************************************************************************
 * LVGL Callback Prototypes (internal use)
 *****************************************************************************/

/**
 * @brief LVGL display flush callback
 * @note Called automatically by LVGL when rendering is complete
 */
void MAIN_lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**
 * @brief LVGL tick callback
 * @note Provides millisecond timing to LVGL
 * @return uint32_t Current milliseconds since boot
 */
uint32_t MAIN_lvgl_tick_cb(void);

#endif // __MAIN_LVGL_LIB_H__

/******************************************************************************
 * End of MAIN_lvglLib.h
 ******************************************************************************/