/**
 * @file EARS_sdCardLib.cpp
 * @author JTB & Claude Sonnet 4.2
 * @brief SD Card library implementation for ESP32-S3 using SD_MMC
 * @version 2.2.0
 * @date 20260131
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#include "EARS_sdCardLib.h"

EARS_sdCard::EARS_sdCard() : _state(SD_NOT_INITIALIZED), _cardType(CARD_NONE)
{
}

EARS_sdCard::~EARS_sdCard()
{
    SD_MMC.end();
}

bool EARS_sdCard::begin()
{
    Serial.println("[SD] Initializing SD card (SD_MMC mode)...");

    // Step 1: Set pins for SD_MMC (verified from Waveshare schematic)
    if (!SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_D0))
    {
        Serial.println("[SD] ERROR: Failed to set SD_MMC pins!");
        _state = SD_INIT_FAILED;
        return false;
    }
    Serial.println("[SD] Pins configured (CLK=11, CMD=10, D0=9)");

    // Step 2: Begin SD_MMC in 1-bit mode
    // Parameters: mountpoint, mode1bit, format_if_mount_failed
    if (!SD_MMC.begin("/sdcard", true, false))
    {
        Serial.println("[SD] ERROR: SD_MMC.begin() failed!");
        _state = SD_INIT_FAILED;
        return false;
    }
    Serial.println("[SD] SD_MMC interface initialized");

    // Step 3: Check card type
    _cardType = SD_MMC.cardType();

    if (_cardType == CARD_NONE)
    {
        Serial.println("[SD] ERROR: No SD card detected!");
        _state = SD_NO_CARD;
        SD_MMC.end();
        return false;
    }

    // Step 4: Display card info
    Serial.print("[SD] Card Type: ");
    Serial.println(getCardType());

    uint64_t cardSizeMB = getCardSizeMB();
    Serial.print("[SD] Card Size: ");
    Serial.print(cardSizeMB);
    Serial.println(" MB");

    uint64_t usedMB = getUsedSpaceMB();
    uint64_t freeMB = getFreeSpaceMB();
    Serial.print("[SD] Used: ");
    Serial.print(usedMB);
    Serial.print(" MB, Free: ");
    Serial.print(freeMB);
    Serial.println(" MB");

    _state = SD_CARD_READY;
    Serial.println("[SD] ✅ SD card ready!");

    return true;
}

bool EARS_sdCard::isAvailable() const
{
    return (_state == SD_CARD_READY);
}

SDCardState EARS_sdCard::getState() const
{
    return _state;
}

String EARS_sdCard::getCardType() const
{
    if (!isAvailable())
        return "NONE";

    switch (_cardType)
    {
    case CARD_MMC:
        return "MMC";
    case CARD_SD:
        return "SDSC";
    case CARD_SDHC:
        return "SDHC";
    default:
        return "UNKNOWN";
    }
}

uint64_t EARS_sdCard::getCardSizeMB() const
{
    if (!isAvailable())
        return 0;
    return SD_MMC.cardSize() / (1024 * 1024);
}

uint64_t EARS_sdCard::getFreeSpaceMB() const
{
    if (!isAvailable())
        return 0;
    uint64_t totalBytes = SD_MMC.totalBytes();
    uint64_t usedBytes = SD_MMC.usedBytes();
    return (totalBytes - usedBytes) / (1024 * 1024);
}

uint64_t EARS_sdCard::getUsedSpaceMB() const
{
    if (!isAvailable())
        return 0;
    return SD_MMC.usedBytes() / (1024 * 1024);
}

bool EARS_sdCard::createDirectory(const char *path)
{
    if (!isAvailable())
        return false;

    if (SD_MMC.mkdir(path))
    {
        Serial.print("[SD] Directory created: ");
        Serial.println(path);
        return true;
    }
    else
    {
        if (directoryExists(path))
            return true;
        Serial.print("[SD] Failed to create directory: ");
        Serial.println(path);
        return false;
    }
}

bool EARS_sdCard::fileExists(const char *path)
{
    if (!isAvailable())
        return false;

    File file = SD_MMC.open(path);
    if (file)
    {
        bool isFile = !file.isDirectory();
        file.close();
        return isFile;
    }
    return false;
}

bool EARS_sdCard::directoryExists(const char *path)
{
    if (!isAvailable())
        return false;

    File dir = SD_MMC.open(path);
    if (dir)
    {
        bool isDir = dir.isDirectory();
        dir.close();
        return isDir;
    }
    return false;
}

bool EARS_sdCard::removeFile(const char *path)
{
    if (!isAvailable())
        return false;

    if (SD_MMC.remove(path))
    {
        Serial.print("[SD] File removed: ");
        Serial.println(path);
        return true;
    }
    Serial.print("[SD] Failed to remove file: ");
    Serial.println(path);
    return false;
}

bool EARS_sdCard::removeDirectory(const char *path)
{
    if (!isAvailable())
        return false;

    if (SD_MMC.rmdir(path))
    {
        Serial.print("[SD] Directory removed: ");
        Serial.println(path);
        return true;
    }
    Serial.print("[SD] Failed to remove directory: ");
    Serial.println(path);
    return false;
}

void EARS_sdCard::listDirectory(const char *path, uint8_t indent)
{
    if (!isAvailable())
        return;

    File dir = SD_MMC.open(path);
    if (!dir)
    {
        Serial.print("[SD] Failed to open directory: ");
        Serial.println(path);
        return;
    }

    if (!dir.isDirectory())
    {
        Serial.println("[SD] Not a directory");
        dir.close();
        return;
    }

    File file = dir.openNextFile();
    while (file)
    {
        for (uint8_t i = 0; i < indent; i++)
            Serial.print("  ");

        Serial.print(file.name());

        if (file.isDirectory())
        {
            Serial.println("/");
            String subPath = String(path) + "/" + String(file.name());
            listDirectory(subPath.c_str(), indent + 1);
        }
        else
        {
            Serial.print(" - ");
            Serial.print(file.size());
            Serial.println(" bytes");
        }

        file.close();
        file = dir.openNextFile();
    }

    dir.close();
}

String EARS_sdCard::readFile(const char *path)
{
    if (!isAvailable())
        return "";

    File file = SD_MMC.open(path, FILE_READ);
    if (!file)
    {
        Serial.print("[SD] Failed to open file for reading: ");
        Serial.println(path);
        return "";
    }

    String content = "";
    while (file.available())
        content += char(file.read());

    file.close();
    return content;
}

bool EARS_sdCard::writeFile(const char *path, const String &content)
{
    if (!isAvailable())
        return false;

    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file)
    {
        Serial.print("[SD] Failed to open file for writing: ");
        Serial.println(path);
        return false;
    }

    size_t written = file.print(content);
    file.close();

    if (written == content.length())
        return true;

    Serial.print("[SD] Write failed: ");
    Serial.println(path);
    return false;
}

bool EARS_sdCard::appendFile(const char *path, const String &content)
{
    if (!isAvailable())
        return false;

    File file = SD_MMC.open(path, FILE_APPEND);
    if (!file)
    {
        Serial.print("[SD] Failed to open file for appending: ");
        Serial.println(path);
        return false;
    }

    size_t written = file.print(content);
    file.close();

    if (written == content.length())
        return true;

    Serial.print("[SD] Append failed: ");
    Serial.println(path);
    return false;
}

EARS_sdCard &using_sdcard()
{
    static EARS_sdCard instance;
    return instance;
}

/******************************************************************************
 * End of EARS_sdCardLib.cpp
 *****************************************************************************/