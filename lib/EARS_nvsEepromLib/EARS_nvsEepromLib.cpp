/**
 * @file EARS_nvsEepromLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief NVS EEPROM wrapper class header
 * @version 2.0.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes Information
 *****************************************************************************/
#include "EARS_nvsEepromLib.h"

// NVS Version constant definition
const uint16_t EARS_nvsEeprom::CURRENT_VERSION;

// NVS Namespace - using definition from EARS_systemDef.h
const char *EARS_nvsEeprom::NAMESPACE = EARS_NAMESPACE;

// Standard NVS Keys - using definitions from EARS_systemDef.h
const char *EARS_nvsEeprom::KEY_VERSION = EARS_VERSION_CODE;
const char *EARS_nvsEeprom::KEY_ZAPNUMBER = EARS_ZAPCODE;
const char *EARS_nvsEeprom::KEY_PASSWORD_HASH = EARS_PASSWORD_HASH;
const char *EARS_nvsEeprom::KEY_BACKLIGHT = EARS_BACKLIGHT_VALUE;
const char *EARS_nvsEeprom::KEY_NVS_CRC = EARS_CRC32;

// NVS Constructor
EARS_nvsEeprom::EARS_nvsEeprom()
{
}

// NVS Destructor
EARS_nvsEeprom::~EARS_nvsEeprom()
{
}

/**
 * @brief Begin NVS
 * @return true
 * @return false
 */
bool EARS_nvsEeprom::begin()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    return (err == ESP_OK);
}

/**
 * @brief Get Hash from NVS
 *
 * @param key
 * @param defaultValue
 * @return Hash String
 */
String EARS_nvsEeprom::getHash(const char *key, const String &defaultValue)
{
    // Open namespace in read-only mode
    if (!Preferences::begin(NAMESPACE, true))
    {
        return defaultValue;
    }

    String hash = getString(key, defaultValue);
    end();

    return hash;
}

/**
 * @brief Put Hash into NVS
 *
 * @param key
 * @param value
 * @return true
 * @return false
 */
bool EARS_nvsEeprom::putHash(const char *key, const String &value)
{
    // Open namespace in read-write mode
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    size_t result = putString(key, value);
    end();

    return (result > 0);
}

/**
 * @brief Calculate CRC32 checksum.
 *
 * @param data
 * @param length
 * @return uint32_t
 */
uint32_t EARS_nvsEeprom::calculateCRC32(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }

    return ~crc;
}

/**
 * @brief Generate CRC32 hash from data.
 *
 * @param data
 * @return String
 */
String EARS_nvsEeprom::makeHash(const String &data)
{
    uint32_t crc = calculateCRC32((const uint8_t *)data.c_str(), data.length());

    // Convert to 8-character hex string
    char hashStr[9];
    sprintf(hashStr, "%08X", crc);

    return String(hashStr);
}

/**
 * @brief Compare data against stored hash.
 *
 * @param data
 * @param storedHash
 * @return true
 * @return false
 */
bool EARS_nvsEeprom::compareHash(const String &data, const String &storedHash)
{
    String computedHash = makeHash(data);
    return (computedHash.equals(storedHash));
}

/**
 * @brief Get version number from NVS.
 *
 * @param key
 * @param defaultVersion
 * @return uint16_t Version number (0-65535)
 */
uint16_t EARS_nvsEeprom::getVersion(const char *key, uint16_t defaultVersion)
{
    // Open namespace in read-only mode
    if (!Preferences::begin(NAMESPACE, true))
    {
        return defaultVersion;
    }

    uint16_t version = getUShort(key, defaultVersion);
    end();

    return version;
}

/**
 * @brief Put version number into NVS.
 *
 * @param key
 * @param version Version number (0-65535)
 * @return true Success
 * @return false Failed
 */
bool EARS_nvsEeprom::putVersion(const char *key, uint16_t version)
{
    // Open namespace in read-write mode
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    size_t result = putUShort(key, version);
    end();

    return (result > 0);
}

/**
 * @brief Validate ZapNumber format (AANNNN)
 *
 * @param zapNumber String to validate
 * @return true Valid format
 * @return false Invalid format
 */
bool EARS_nvsEeprom::isValidZapNumber(const String &zapNumber)
{
    // Must be exactly 6 characters
    if (zapNumber.length() != 6)
    {
        return false;
    }

    // First two characters must be letters (A-Z)
    if (!isAlpha(zapNumber[0]) || !isAlpha(zapNumber[1]))
    {
        return false;
    }

    // Last four characters must be digits (0-9)
    for (int i = 2; i < 6; i++)
    {
        if (!isDigit(zapNumber[i]))
        {
            return false;
        }
    }

    return true;
}

/**
 * @brief Calculate CRC32 for entire NVS contents (excluding the CRC itself)
 *
 * @return uint32_t CRC32 value
 */
uint32_t EARS_nvsEeprom::calculateNVSCRC()
{
    String dataToHash = "";

    // Open namespace in read-only mode
    if (!Preferences::begin(NAMESPACE, true))
    {
        return 0;
    }

    // Concatenate all critical data in a specific order
    // Version
    uint16_t version = getUShort(KEY_VERSION, 0);
    dataToHash += String(version);
    dataToHash += "|";

    // ZapNumber
    String zapNum = getString(KEY_ZAPNUMBER, "");
    dataToHash += zapNum;
    dataToHash += "|";

    // Password Hash
    String pwdHash = getString(KEY_PASSWORD_HASH, "");
    dataToHash += pwdHash;

    end();

    // Calculate CRC32 of concatenated data
    return calculateCRC32((const uint8_t *)dataToHash.c_str(), dataToHash.length());
}

/**
 * @brief Update the stored NVS CRC32 value
 *
 * @return true Success
 * @return false Failed
 */
bool EARS_nvsEeprom::updateNVSCRC()
{
    uint32_t crc = calculateNVSCRC();

    // Open namespace in read-write mode
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    size_t result = putUInt(KEY_NVS_CRC, crc);
    end();

    return (result > 0);
}

/**
 * @brief Upgrade NVS from one version to another
 *
 * @param fromVersion Current version
 * @param toVersion Target version
 * @return true Upgrade successful
 * @return false Upgrade failed
 */
bool EARS_nvsEeprom::upgradeNVS(uint16_t fromVersion, uint16_t toVersion)
{
    // Prevent downgrade
    if (toVersion <= fromVersion)
    {
        return false;
    }

    // Prevent upgrade beyond CURRENT_VERSION
    if (toVersion > CURRENT_VERSION)
    {
        return false;
    }

    // Open namespace in read-write mode
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    // Future: Add version-specific upgrade logic here
    // For now, just update the version number

    // Example upgrade paths:
    // if (fromVersion == 0 && toVersion >= 1) {
    //     // Upgrade from version 0 to 1
    //     // Add new keys, migrate data, etc.
    // }

    // Update version
    size_t result = putUShort(KEY_VERSION, toVersion);
    end();

    if (result == 0)
    {
        return false;
    }

    // Recalculate and update CRC after upgrade
    return updateNVSCRC();
}

/**
 * @brief Validate entire NVS storage
 *
 * This function checks:
 * 1. Version matches or can be upgraded
 * 2. ZapNumber exists and is valid format
 * 3. Password hash exists
 * 4. Overall CRC32 is valid (no tampering)
 *
 * @return NVSValidationResult Structure containing validation results
 */
NVSValidationResult EARS_nvsEeprom::validateNVS()
{
    NVSValidationResult result;
    result.expectedVersion = CURRENT_VERSION;

    // Step 1: Check NVS initialization
    if (!Preferences::begin(NAMESPACE, true))
    {
        result.status = NVSStatus::INITIALIZATION_FAILED;
        end();
        return result;
    }

    // Step 2: Get and check version
    result.currentVersion = getUShort(KEY_VERSION, 0);

    // Check if upgrade is needed
    if (result.currentVersion < CURRENT_VERSION)
    {
        end(); // Close read-only session

        // Attempt upgrade
        if (upgradeNVS(result.currentVersion, CURRENT_VERSION))
        {
            result.wasUpgraded = true;
            result.currentVersion = CURRENT_VERSION;
        }
        else
        {
            result.status = NVSStatus::INVALID_VERSION;
            return result;
        }

        // Reopen for continued validation
        if (!Preferences::begin(NAMESPACE, true))
        {
            result.status = NVSStatus::INITIALIZATION_FAILED;
            return result;
        }
    }
    else if (result.currentVersion > CURRENT_VERSION)
    {
        // Version from future - cannot handle
        result.status = NVSStatus::INVALID_VERSION;
        end();
        return result;
    }

    // Step 3: Check ZapNumber
    String zapNum = getString(KEY_ZAPNUMBER, "");
    if (zapNum.length() == 0 || !isValidZapNumber(zapNum))
    {
        result.zapNumberValid = false;
        result.status = NVSStatus::MISSING_ZAPNUMBER;
        end();
        return result;
    }
    result.zapNumberValid = true;
    zapNum.toCharArray(result.zapNumber, 7);

    // Step 4: Check password hash
    String pwdHash = getString(KEY_PASSWORD_HASH, "");
    if (pwdHash.length() == 0)
    {
        result.passwordHashValid = false;
        result.status = NVSStatus::MISSING_PASSWORD;
        end();
        return result;
    }
    result.passwordHashValid = true;

    // Step 5: Check overall CRC32
    uint32_t storedCRC = getUInt(KEY_NVS_CRC, 0);
    end(); // Close before calculating new CRC

    uint32_t calculatedCRC = calculateNVSCRC();
    result.calculatedCRC = calculatedCRC;

    if (storedCRC != calculatedCRC)
    {
        result.crcValid = false;
        result.status = NVSStatus::CRC_FAILED;
        return result;
    }
    result.crcValid = true;

    // Step 6: All checks passed
    if (result.wasUpgraded)
    {
        result.status = NVSStatus::UPGRADED;
    }
    else
    {
        result.status = NVSStatus::VALID;
    }

    return result;
}

/**
 * @brief Get ZapNumber from NVS
 *
 * @return String The stored ZapNumber or empty string if not found
 */
String EARS_nvsEeprom::getZapNumber()
{
    // Open namespace in read-only mode
    if (!Preferences::begin(NAMESPACE, true))
    {
        return "";
    }

    String zapNumber = getString(KEY_ZAPNUMBER, "");
    end();

    return zapNumber;
}

/**
 * @brief Set ZapNumber in NVS
 *
 * @param zapNumber The ZapNumber to store (format: AANNNN)
 * @return true if successful
 * @return false if invalid format or write failed
 */
bool EARS_nvsEeprom::setZapNumber(const String &zapNumber)
{
    // Validate format first
    if (!isValidZapNumber(zapNumber))
    {
        return false;
    }

    // Open namespace in read-write mode
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    // Store in NVS
    size_t result = putString(KEY_ZAPNUMBER, zapNumber);
    end();

    if (result == 0)
    {
        return false;
    }

    // Update CRC after successful write
    return updateNVSCRC();
}

/******************************************************************************
 * NEW FUNCTIONS - Version Code Management
 *****************************************************************************/

/**
 * @brief Get NVS version as 2-digit hexadecimal string
 * @return String Version code (e.g., "01", "0A", "FF")
 */
String EARS_nvsEeprom::getNVSVersionString()
{
    if (!Preferences::begin(NAMESPACE, true))
    {
        return "00";
    }

    String versionStr = getString(KEY_VERSION, "00");
    end();

    return versionStr;
}

/**
 * @brief Get NVS version as integer
 * @return uint16_t Version number (0-255)
 */
uint16_t EARS_nvsEeprom::getNVSVersionInt()
{
    String versionStr = getNVSVersionString();

    // Convert hex string to integer
    return (uint16_t)strtol(versionStr.c_str(), NULL, 16);
}

/**
 * @brief Set NVS version from integer (converts to 2-digit hex string)
 * @param version Version number (0-255)
 * @return true if successful
 */
bool EARS_nvsEeprom::setNVSVersion(uint16_t version)
{
    // Limit to 0-255 (0x00-0xFF)
    if (version > 255)
    {
        return false;
    }

    // Convert to 2-digit hex string (e.g., 1 â†’ "01", 15 â†’ "0F", 255 â†’ "FF")
    char hexStr[3];
    sprintf(hexStr, "%02X", version);

    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    size_t result = putString(KEY_VERSION, String(hexStr));
    end();

    if (result == 0)
    {
        return false;
    }

    // Update CRC after successful write
    return updateNVSCRC();
}

/******************************************************************************
 * Password Hash Management
 *****************************************************************************/

/**
 * @brief Get stored password hash
 * @return String CRC32 hash of password (8 hex characters)
 */
String EARS_nvsEeprom::getPasswordHash()
{
    return getHash(KEY_PASSWORD_HASH, "");
}

/**
 * @brief Set password (converts to CRC32 hash and stores)
 * @param password Plain text password
 * @return true if successful
 */
bool EARS_nvsEeprom::setPassword(const String &password)
{
    if (password.length() == 0)
    {
        return false;
    }

    // Generate CRC32 hash of password
    String passwordHash = makeHash(password);

    // Store hash
    bool result = putHash(KEY_PASSWORD_HASH, passwordHash);

    if (result)
    {
        // Update overall CRC
        return updateNVSCRC();
    }

    return false;
}

/**
 * @brief Verify password against stored hash
 * @param password Plain text password to verify
 * @return true if password matches stored hash
 */
bool EARS_nvsEeprom::verifyPassword(const String &password)
{
    String storedHash = getPasswordHash();

    if (storedHash.length() == 0)
    {
        return false; // No password set
    }

    return compareHash(password, storedHash);
}

/**
 * @brief Check if password exists in NVS
 * @return true if password hash is stored
 */
bool EARS_nvsEeprom::hasPassword()
{
    String hash = getPasswordHash();
    return (hash.length() > 0);
}

/******************************************************************************
 * Backlight Management
 *****************************************************************************/

/**
 * @brief Get backlight value (0-100)
 * @return uint8_t Backlight brightness (0-100)
 */
uint8_t EARS_nvsEeprom::getBacklightValue()
{
    if (!Preferences::begin(NAMESPACE, true))
    {
        return 100; // Default to full brightness
    }

    uint8_t value = getUChar(KEY_BACKLIGHT, 100);
    end();

    // Ensure in valid range
    if (value > 100)
    {
        value = 100;
    }

    return value;
}

/**
 * @brief Set backlight value (0-100)
 * @param value Brightness level (0-100)
 * @return true if successful
 */
bool EARS_nvsEeprom::setBacklightValue(uint8_t value)
{
    // Constrain to valid range
    if (value > 100)
    {
        value = 100;
    }

    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    size_t result = putUChar(KEY_BACKLIGHT, value);
    end();

    if (result == 0)
    {
        return false;
    }

    // Update CRC after successful write
    return updateNVSCRC();
}

/**
 * @brief Get backlight value mapped for PWM (0-255)
 * @return uint8_t PWM value (0-255)
 */
uint8_t EARS_nvsEeprom::getBacklightPWM()
{
    uint8_t brightness = getBacklightValue();

    // Map 0-100 to 0-255
    return (brightness * 255) / 100;
}

/******************************************************************************
 * Complete NVS Management
 *****************************************************************************/

/**
 * @brief Initialize NVS with default values (first-time setup)
 * @return true if successful
 */
bool EARS_nvsEeprom::initializeNVS()
{
    // Set default version
    if (!setNVSVersion(CURRENT_VERSION))
    {
        return false;
    }

    // Set default backlight (100%)
    if (!setBacklightValue(100))
    {
        return false;
    }

    // Calculate and store initial CRC (with no ZapNumber or Password)
    return updateNVSCRC();
}

/**
 * @brief Check if NVS has been initialized
 * @return true if NVS contains valid version code
 */
bool EARS_nvsEeprom::isInitialized()
{
    if (!Preferences::begin(NAMESPACE, true))
    {
        return false;
    }

    bool exists = isKey(KEY_VERSION);
    end();

    return exists;
}

/**
 * @brief Erase all NVS data (factory reset)
 * @return true if successful
 */
bool EARS_nvsEeprom::factoryReset()
{
    if (!Preferences::begin(NAMESPACE, false))
    {
        return false;
    }

    bool result = clear();
    end();

    return result;
}

/******************************************************************************
 * High-Level Initialization Orchestration (DEBLOAT Step 4)
 *****************************************************************************/

/**
 * @brief Perform complete NVS initialization sequence
 * @return NVSValidationResult Detailed result of initialization
 *
 * @details
 * This function orchestrates the complete 5-step NVS initialization:
 * 1. Initialize NVS flash
 * 2. Check if NVS is initialized (first boot detection)
 * 3. Validate ZapNumber exists and is valid format
 * 4. Check if password exists
 * 5. Run full CRC validation
 *
 * This function was extracted from main.cpp during debloat Step 4.
 * It encapsulates all NVS initialization logic in one place.
 *
 * @note The caller should interpret the result to set LED patterns
 *       and update application state accordingly.
 */
NVSValidationResult EARS_nvsEeprom::performFullInitialization()
{
    NVSValidationResult result;
    result.expectedVersion = CURRENT_VERSION;

    // ========================================================================
    // STEP 1: Initialize NVS flash
    // ========================================================================
    if (!begin())
    {
        result.status = NVSStatus::INITIALIZATION_FAILED;
        return result;
    }

    // ========================================================================
    // STEP 2: Check if NVS is initialized (first boot?)
    // ========================================================================
    if (!isInitialized())
    {
        // First boot detected - initialize with defaults
        if (initializeNVS())
        {
            // Successfully initialized with defaults
            // Status: Needs configuration (ZapNumber and Password)
            result.status = NVSStatus::MISSING_ZAPNUMBER;
            result.currentVersion = CURRENT_VERSION;
            result.zapNumberValid = false;
            result.passwordHashValid = false;
            return result;
        }
        else
        {
            // Failed to initialize
            result.status = NVSStatus::INITIALIZATION_FAILED;
            return result;
        }
    }

    // ========================================================================
    // STEP 3: Check ZapNumber
    // ========================================================================
    String zapNumber = getZapNumber();
    if (zapNumber.length() == 0 || !isValidZapNumber(zapNumber))
    {
        result.status = NVSStatus::MISSING_ZAPNUMBER;
        result.zapNumberValid = false;
        result.currentVersion = getNVSVersionInt();
        return result;
    }

    // ZapNumber is valid - store it in result
    result.zapNumberValid = true;
    zapNumber.toCharArray(result.zapNumber, 7);

    // ========================================================================
    // STEP 4: Check password
    // ========================================================================
    if (!hasPassword())
    {
        result.status = NVSStatus::MISSING_PASSWORD;
        result.passwordHashValid = false;
        result.currentVersion = getNVSVersionInt();
        return result;
    }

    result.passwordHashValid = true;

    // ========================================================================
    // STEP 5: Full validation (including CRC check)
    // ========================================================================
    result = validateNVS();

    // If validation passed or was upgraded, we're ready
    if (result.status == NVSStatus::VALID || result.status == NVSStatus::UPGRADED)
    {
        // Successfully validated
        return result;
    }

    // Validation failed (CRC error, version error, etc.)
    return result;
}

/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char *EARS_nvsEeprom::getLibraryName()
{
    return EARS_NVSEeprom::LIB_NAME;
}

// Get encoded version as integer
uint32_t EARS_nvsEeprom::getVersionEncoded()
{
    return VERS_ENCODE(EARS_NVSEeprom::VERSION_MAJOR,
                       EARS_NVSEeprom::VERSION_MINOR,
                       EARS_NVSEeprom::VERSION_PATCH);
}

// Get version date
const char *EARS_nvsEeprom::getVersionDate()
{
    return EARS_NVSEeprom::VERSION_DATE;
}

// Format version as string
void EARS_nvsEeprom::getVersionString(char *buffer)
{
    uint32_t encoded = getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/**
 * @brief Get reference to global NVS EEPROM instance (Singleton pattern)
 *
 * This function implements the singleton pattern to ensure only one
 * NVS EEPROM instance exists throughout the application lifecycle.
 *
 * @return EARS_nvsEeprom& Reference to the global NVS EEPROM instance
 */
EARS_nvsEeprom &using_nvseeprom()
{
    static EARS_nvsEeprom instance;
    return instance;
}

/******************************************************************************
 * End of EARS_nvsEepromLib.cpp
 ******************************************************************************/