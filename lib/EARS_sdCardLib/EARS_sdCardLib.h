/**
 * @file EARS_sdCardLib.h
 * @author JTB & Claude Sonnet 4.2
 * @brief SD Card library for ESP32-S3 using SD_MMC (SDIO 1-bit mode)
 * @version 2.3.0
 * @date 20260204
 *
 * @details
 * This library uses SD_MMC for SD card access (SDIO interface)
 * Compatible with Waveshare ESP32-S3 Touch LCD 3.5"
 * Pin definitions verified from Waveshare wiki schematic
 *
 * PIN CONFIGURATION (SD_MMC 1-bit mode):
 * - CLK: GPIO 11 (SD clock)
 * - CMD: GPIO 10 (SD command)
 * - D0:  GPIO 9  (SD data 0)
 *
 * NOTE: These pins do NOT conflict with display (which uses GPIOs 1,2,3,5,6)
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __EARS_SDCARD_LIB_H__
#define __EARS_SDCARD_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <SD_MMC.h>

/******************************************************************************
 * SD Card Pin Definitions (SD_MMC mode - verified from schematic)
 *****************************************************************************/
#define SDMMC_CLK 11 // Clock (IO11)
#define SDMMC_CMD 10 // Command (IO10)
#define SDMMC_D0 9   // Data 0 (IO9)

/******************************************************************************
 * SD Card State Enum
 *****************************************************************************/
enum SDCardState
{
    SD_NOT_INITIALIZED = 0,
    SD_INIT_FAILED,
    SD_NO_CARD,
    SD_CARD_READY
};

/******************************************************************************
 * SD Card Initialization Result Structure (DEBLOAT Step 5)
 *****************************************************************************/
/**
 * @struct SDCardInitResult
 * @brief Structure to hold SD card initialization results
 *
 * @details
 * Used by performFullInitialization() to return detailed initialization
 * status to the caller. Contains card state, type, capacity, and
 * directory creation status.
 */
struct SDCardInitResult
{
    SDCardState state;       // Current SD card state
    String cardType;         // Card type (MMC, SDSC, SDHC)
    uint64_t cardSizeMB;     // Total card size in MB
    uint64_t freeMB;         // Free space in MB
    uint64_t usedMB;         // Used space in MB
    bool directoriesCreated; // Essential directories created successfully

    SDCardInitResult() : state(SD_NOT_INITIALIZED),
                         cardType(""),
                         cardSizeMB(0),
                         freeMB(0),
                         usedMB(0),
                         directoriesCreated(false) {}
};

/******************************************************************************
 * EARS_sdCard Class
 *****************************************************************************/
class EARS_sdCard
{
public:
    EARS_sdCard();
    ~EARS_sdCard();

    /**
     * @brief Initialize the SD card (SD_MMC 1-bit mode)
     * @return true if SD card initialized successfully
     * @return false if SD card initialization failed
     */
    bool begin();

    bool isAvailable() const;
    SDCardState getState() const;
    String getCardType() const;
    uint64_t getCardSizeMB() const;
    uint64_t getFreeSpaceMB() const;
    uint64_t getUsedSpaceMB() const;

    bool createDirectory(const char *path);
    bool fileExists(const char *path);
    bool directoryExists(const char *path);
    bool removeFile(const char *path);
    bool removeDirectory(const char *path);
    void listDirectory(const char *path, uint8_t indent = 0);

    String readFile(const char *path);
    bool writeFile(const char *path, const String &content);
    bool appendFile(const char *path, const String &content);

    /**
     * @brief Perform complete SD card initialization sequence (DEBLOAT Step 5)
     * @return SDCardInitResult Detailed result of initialization
     *
     * @details
     * This function orchestrates the complete SD card initialization:
     * 1. Initialize SD_MMC interface via begin()
     * 2. Retrieve card information (type, size, free space)
     * 3. Create essential directories (/logs, /config, /images)
     * 4. Return detailed status for caller interpretation
     *
     * This function was extracted from main.cpp during debloat Step 5.
     * It encapsulates all SD card initialization logic in one place.
     *
     * @note The caller should interpret the result to set LED patterns
     *       and update application state accordingly.
     */
    SDCardInitResult performFullInitialization();

private:
    SDCardState _state;
    uint8_t _cardType;
};

/**
 * @brief Get reference to global SD Card instance (Singleton pattern)
 */
EARS_sdCard &using_sdcard();

#endif // __EARS_SDCARD_LIB_H__