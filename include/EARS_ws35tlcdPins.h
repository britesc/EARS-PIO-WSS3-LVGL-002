/**
 * @file EARS_ws35tlcdPins.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Defines pin assignments for the Waveshare 3.5" ESP32-S3 LCD display.
 * @details Pin definitions verified from Waveshare wiki schematic
 * @version 0.2
 * @date 20260131
 */
#pragma once
#ifndef __EARS_WS35TLCD_PINS_H_
#define __EARS_WS35TLCD_PINS_H_

// Waveshare 3.5" ESP32-S3 Display Pin Definitions (Arduino GFX)

// TFT Display Pins (SPI)
#define GFX_BL 6   // Backlight
#define SPI_MISO 2 // Display MISO
#define SPI_MOSI 1 // Display MOSI
#define SPI_SCLK 5 // Display SCLK
#define LCD_CS -1  // Chip Select (not used)
#define LCD_DC 3   // Data/Command
#define LCD_RST -1 // Reset (not used)

// SD Card Pins (SD_MMC - SDIO 1-bit mode)
// CORRECT pins verified from Waveshare schematic
#define SDMMC_CLK 11 // SD Clock (was wrongly defined as 6)
#define SDMMC_CMD 10 // SD Command (was wrongly defined as 7)
#define SDMMC_D0 9   // SD Data 0 (was wrongly defined as 8)
// Note: SD_CS (EXIO3) not used in SD_MMC mode

// Touch Screen Pins (I2C) - if needed later
#define TOUCH_SDA 38
#define TOUCH_SCL 39
#define TOUCH_INT 18
#define TOUCH_RST -1

// Display Specifications
#define TFT_WIDTH 480
#define TFT_HEIGHT 320

#endif // __EARS_WS35TLCD_PINS_H_