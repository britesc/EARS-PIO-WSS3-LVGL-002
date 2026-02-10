/**
 * @file EARS_nvsEepromLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief NVS EEPROM wrapper class header
 * @version 2.0.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __EARS_NVSEEPROM_LIB_H__
#define __EARS_NVSEEPROM_LIB_H__

/******************************************************************************
 * Includes Information
 *****************************************************************************/
#include <Preferences.h>
#include "EARS_versionDef.h"
#include <Arduino.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "EARS_systemDef.h"

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace EARS_NVSEeprom
{
    constexpr const char* LIB_NAME = "EARS_nvsEeprom";
    constexpr const char* VERSION_MAJOR = "2";
    constexpr const char* VERSION_MINOR = "0";
    constexpr const char* VERSION_PATCH = "0";
    constexpr const char* VERSION_DATE = "2026-02-10";
}


/******************************************************************************
 * Validation Status Enum
 *****************************************************************************/
/**
 * @enum NVSStatus
 * @brief Enumeration for NVS validation status.
 */
enum class NVSStatus : uint8_t
{
    NOT_CHECKED = 0,
    VALID = 1,
    INVALID_VERSION = 2,
    MISSING_ZAPNUMBER = 3,
    MISSING_PASSWORD = 4,
    CRC_FAILED = 5,
    UPGRADED = 6,
    INITIALIZATION_FAILED = 7
};

/******************************************************************************
 * Core0/Core1 Communication Struct
 *****************************************************************************/
/**
 * @struct NVSValidationResult
 * @brief Structure to hold NVS validation results.
 */
struct NVSValidationResult
{
    NVSStatus status;
    uint16_t currentVersion;
    uint16_t expectedVersion;
    bool zapNumberValid;
    bool passwordHashValid;
    bool crcValid;
    bool wasUpgraded;
    uint32_t calculatedCRC;
    char zapNumber[7];

    NVSValidationResult() : status(NVSStatus::NOT_CHECKED),
                            currentVersion(0),
                            expectedVersion(0),
                            zapNumberValid(false),
                            passwordHashValid(false),
                            crcValid(false),
                            wasUpgraded(false),
                            calculatedCRC(0)
    {
        zapNumber[0] = '\0';
    }
};

/******************************************************************************
 * NVS EEPROM Wrapper Class
 *****************************************************************************/
/**
 * @brief NVS EEPROM wrapper class.
 *
 * @details
 * All NVS key names are defined in EARS_systemDef.h
 */
class EARS_nvsEeprom : public Preferences
{
public:
    // NVS Version - increment when NVS structure changes
    static const uint16_t CURRENT_VERSION = 1;

    // NVS Namespace
    static const char *NAMESPACE;

    // Standard NVS keys
    static const char *KEY_VERSION;
    static const char *KEY_ZAPNUMBER;
    static const char *KEY_PASSWORD_HASH;
    static const char *KEY_BACKLIGHT;
    static const char *KEY_NVS_CRC;

    // Constructor and Destructor
    EARS_nvsEeprom();
    ~EARS_nvsEeprom();

    // Version information getters
    static const char* getLibraryName();
    static uint32_t getVersionEncoded();
    static const char* getVersionDate();
    static void getVersionString(char* buffer);

    // Initialize NVS - call this in setup()
    bool begin();

    // Hash functions
    String getHash(const char *key, const String &defaultValue = "");
    bool putHash(const char *key, const String &value);
    String makeHash(const String &data);
    bool compareHash(const String &data, const String &storedHash);

    // Version management
    uint16_t getVersion(const char *key, uint16_t defaultVersion = 0);
    bool putVersion(const char *key, uint16_t version);

    // NVS Version functions
    String getNVSVersionString();
    uint16_t getNVSVersionInt();
    bool setNVSVersion(uint16_t version);

    // ZapNumber management
    bool isValidZapNumber(const String &zapNumber);
    String getZapNumber();
    bool setZapNumber(const String &zapNumber);

    // Password management
    String getPasswordHash();
    bool setPassword(const String &password);
    bool verifyPassword(const String &password);
    bool hasPassword();

    // Backlight management
    uint8_t getBacklightValue();
    bool setBacklightValue(uint8_t value);
    uint8_t getBacklightPWM();

    // NVS validation and CRC
    NVSValidationResult validateNVS();
    bool updateNVSCRC();
    uint32_t calculateNVSCRC();

    // Complete NVS management
    bool initializeNVS();
    bool isInitialized();
    bool factoryReset();

    // High-level initialization orchestration - DEBLOAT Step 4
    NVSValidationResult performFullInitialization();

private:
    uint32_t calculateCRC32(const uint8_t *data, size_t length);
    bool upgradeNVS(uint16_t fromVersion, uint16_t toVersion);
};

// Global instance access function (Singleton pattern)
EARS_nvsEeprom &using_nvseeprom();

#endif // __EARS_NVSEEPROM_LIB_H__