/**
 * @file EARS_ws35tlcdPins.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Defines pin assignments for the Waveshare 3.5" ESP32-S3 LCD display.
 * @details Pin definitions verified from Waveshare wiki schematic and I2C scanner
 * @version 0.3
 * @date 20260207
 *
 * CHANGE LOG:
 * v0.3 - CORRECTED Touch I2C pins from scanner results:
 *        Was: SDA=38, SCL=39 (INCORRECT)
 *        Now: SDA=8, SCL=7 (VERIFIED via I2C scanner)
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

// Touch Screen Pins (I2C) - CORRECTED via I2C scanner 2026-02-07
// Chip detected: FT6236U/FT3267 (Chip ID: 0x64) at address 0x38
#define TOUCH_SDA 8  // I2C Data (CORRECTED from 38)
#define TOUCH_SCL 7  // I2C Clock (CORRECTED from 39)
#define TOUCH_INT 18 // Touch interrupt pin
#define TOUCH_RST -1 // Touch reset (controlled via TCA9554 GPIO expander)

// Display Specifications
#define TFT_WIDTH 480
#define TFT_HEIGHT 320

#endif // __EARS_WS35TLCD_PINS_H_