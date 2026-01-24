/**
 * @file MAIN_drawingLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Drawing functions for EARS - rectangles, shapes, etc.
 * @details Functions callable from main code and EEZ Studio Flow
 * @version 0.1.0
 * @date 20260124
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_DRAWING_LIB_H__
#define __MAIN_DRAWING_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "EARS_rgb565ColoursDef.h"

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Draw a filled rectangle
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param colour RGB565 colour value
 */
void MAIN_draw_filled_rect(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour);

/**
 * @brief Draw a rectangle outline
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param colour RGB565 colour value
 */
void MAIN_draw_rect_outline(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour);

/**
 * @brief Draw a rounded rectangle (filled)
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param r Corner radius in pixels
 * @param colour RGB565 colour value
 */
void MAIN_draw_rounded_rect(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t colour);

/**
 * @brief Draw a rounded rectangle outline
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param r Corner radius in pixels
 * @param colour RGB565 colour value
 */
void MAIN_draw_rounded_rect_outline(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t colour);

/**
 * @brief Draw a button-style rectangle (filled with border)
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param fillColour RGB565 fill colour
 * @param borderColour RGB565 border colour
 */
void MAIN_draw_button_rect(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h,
                           uint16_t fillColour, uint16_t borderColour);

/**
 * @brief Clear screen to specified colour
 * @param gfx Pointer to Arduino_GFX object
 * @param colour RGB565 colour value (default: black)
 */
void MAIN_clear_screen(Arduino_GFX *gfx, uint16_t colour = EARS_RGB565_BLACK);

#endif // __MAIN_DRAWING_LIB_H__

/******************************************************************************
 * End of MAIN_drawingLib.h
 ******************************************************************************/