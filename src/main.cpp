/**
 * @file main.cpp - STEP 2
 * @brief Add MAIN_drawingLib functions
 */

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "EARS_ws35tlcdPins.h"
#include "EARS_rgb565ColoursDef.h"
#include "MAIN_drawingLib.h"

// Create display
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, SPI_SCLK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7796(bus, LCD_RST, 1, true, TFT_HEIGHT, TFT_WIDTH);

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n════════════════════════════════════════");
    Serial.println("  EARS - Step 2: Drawing Functions");
    Serial.println("════════════════════════════════════════");

    // Turn on backlight
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
    Serial.println("[OK] Backlight ON");

    // Initialize display
    if (!gfx->begin())
    {
        Serial.println("[ERROR] Display init failed!");
        while (1)
            delay(1000);
    }
    Serial.println("[OK] Display initialized");

    // Test pattern - color bars using drawing functions
    Serial.println("[TEST] Drawing color bars with MAIN_clear_screen()...");
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    uint16_t barWidth = TFT_WIDTH / 8;
    MAIN_draw_filled_rect(gfx, 0 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_RED);
    MAIN_draw_filled_rect(gfx, 1 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GREEN);
    MAIN_draw_filled_rect(gfx, 2 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_BLUE);
    MAIN_draw_filled_rect(gfx, 3 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_YELLOW);
    MAIN_draw_filled_rect(gfx, 4 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_CYAN);
    MAIN_draw_filled_rect(gfx, 5 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_MAGENTA);
    MAIN_draw_filled_rect(gfx, 6 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_WHITE);
    MAIN_draw_filled_rect(gfx, 7 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GRAY);

    // Text overlay (still using gfx directly for text)
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println("EARS Display Test");
    gfx->setCursor(10, 40);
    gfx->print("Step 2: Draw Lib");

    Serial.println("[OK] Color bars drawn with MAIN functions");

    delay(2000);

    // Clear and draw various rectangles
    Serial.println("[TEST] Testing all drawing functions...");
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    // 1. Filled rectangle
    MAIN_draw_filled_rect(gfx, 10, 10, 150, 80, EARS_RGB565_RED);

    // 2. Rectangle outline
    MAIN_draw_rect_outline(gfx, 170, 10, 150, 80, EARS_RGB565_GREEN);

    // 3. Rounded rectangle
    MAIN_draw_rounded_rect(gfx, 330, 10, 140, 80, 10, EARS_RGB565_BLUE);

    // 4. Rounded outline
    MAIN_draw_rounded_rect_outline(gfx, 10, 110, 150, 80, 10, EARS_RGB565_YELLOW);

    // 5. Button (filled with border)
    MAIN_draw_button_rect(gfx, 170, 110, 150, 80,
                          EARS_RGB565_CS_PRIMARY, // Green fill
                          EARS_RGB565_CS_TEXT);   // Tan border

    // 6. Another button with red swatch
    MAIN_draw_button_rect(gfx, 330, 110, 140, 80,
                          EARS_RGB565_RS_PRIMARY, // Red fill
                          EARS_RGB565_WHITE);     // White border

    // Add labels
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(10, 200);
    gfx->println("All drawing functions tested!");

    Serial.println("[OK] All drawing functions working!");
    Serial.println("════════════════════════════════════════");
    Serial.println("Step 2 complete - Drawing lib working!");
    Serial.println("════════════════════════════════════════\n");
}

void loop()
{
    delay(1000);
}