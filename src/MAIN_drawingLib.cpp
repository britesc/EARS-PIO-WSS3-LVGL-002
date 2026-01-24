/**
 * @file MAIN_drawingLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Drawing functions for EARS - rectangles, shapes, etc.
 * @details Functions callable from main code and EEZ Studio Flow
 * @version 0.1.0
 * @date 20260124
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_drawingLib.h"
#include "EARS_systemDef.h"

/******************************************************************************
 * Rectangle Drawing Functions
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
void MAIN_draw_filled_rect(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    gfx->fillRect(x, y, w, h, colour);

    DEBUG_PRINTF("[DRAW] Filled rect at (%d,%d) size %dx%d colour 0x%04X\n", x, y, w, h, colour);
}

/**
 * @brief Draw a rectangle outline
 * @param gfx Pointer to Arduino_GFX object
 * @param x X coordinate (top-left corner)
 * @param y Y coordinate (top-left corner)
 * @param w Width in pixels
 * @param h Height in pixels
 * @param colour RGB565 colour value
 */
void MAIN_draw_rect_outline(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t colour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    gfx->drawRect(x, y, w, h, colour);

    DEBUG_PRINTF("[DRAW] Rect outline at (%d,%d) size %dx%d colour 0x%04X\n", x, y, w, h, colour);
}

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
void MAIN_draw_rounded_rect(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t colour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    gfx->fillRoundRect(x, y, w, h, r, colour);

    DEBUG_PRINTF("[DRAW] Rounded rect at (%d,%d) size %dx%d radius %d colour 0x%04X\n",
                 x, y, w, h, r, colour);
}

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
void MAIN_draw_rounded_rect_outline(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t colour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    gfx->drawRoundRect(x, y, w, h, r, colour);

    DEBUG_PRINTF("[DRAW] Rounded rect outline at (%d,%d) size %dx%d radius %d colour 0x%04X\n",
                 x, y, w, h, r, colour);
}

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
                           uint16_t fillColour, uint16_t borderColour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    // Draw filled rectangle
    gfx->fillRect(x, y, w, h, fillColour);

    // Draw border
    gfx->drawRect(x, y, w, h, borderColour);

    DEBUG_PRINTF("[DRAW] Button rect at (%d,%d) size %dx%d fill 0x%04X border 0x%04X\n",
                 x, y, w, h, fillColour, borderColour);
}

/**
 * @brief Clear screen to specified colour
 * @param gfx Pointer to Arduino_GFX object
 * @param colour RGB565 colour value (default: black)
 */
void MAIN_clear_screen(Arduino_GFX *gfx, uint16_t colour)
{
    if (gfx == nullptr)
    {
        DEBUG_PRINTLN("[ERROR] GFX object is null");
        return;
    }

    gfx->fillScreen(colour);

    DEBUG_PRINTF("[DRAW] Screen cleared to colour 0x%04X\n", colour);
}

/******************************************************************************
 * End of MAIN_drawingLib.cpp
 ******************************************************************************/