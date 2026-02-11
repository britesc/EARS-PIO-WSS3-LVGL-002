/**
 * @file MAIN_initializationLib.cpp
 * @author JTB & Claude Sonnet 4.5
 * @brief Centralized initialization functions for EARS subsystems
 * @version 0.1.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#include "MAIN_initializationLib.h"

/******************************************************************************
 * Global State Variables
 *****************************************************************************/
volatile TouchState touch_state = TOUCH_NOT_INITIALIZED;
volatile bool touch_initialized = false;
volatile NVSInitState nvs_state = NVS_NOT_INITIALIZED;
volatile SDCardState sd_card_state = SD_NOT_INITIALIZED;

/******************************************************************************
 * Touch Controller Initialization
 *****************************************************************************/

/**
 * @brief Initialize Touch Controller
 * @details Uses EARS_touchLib::performFullInitialization()
 */
void MAIN_initialise_touch()
{
    // Guard against duplicate initialization
    if (touch_state != TOUCH_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[TOUCH] Already initialized, skipping");
#endif
        return;
    }

    TouchInitResult result = using_touch().performFullInitialization(TOUCH_SDA, TOUCH_SCL);
    touch_state = result.state;

    switch (result.state)
    {
    case TOUCH_INIT_FAILED:
#if EARS_DEBUG == 1
        Serial.println("[WARNING] Touch initialization failed");
        Serial.println("          System will continue without touch input");
        MAIN_led_warning_pattern(3);
#endif
        touch_initialized = false;
        break;

    case TOUCH_READY:
#if EARS_DEBUG == 1
        Serial.printf("[OK] Touch ready: %s\n", result.modelName.c_str());
        Serial.printf("     I2C: 0x%02X @ SDA=%d, SCL=%d\n",
                      result.i2cAddress, result.sdaPin, result.sclPin);
        Serial.printf("     Max Touch Points: %d\n", result.maxTouchPoints);
        MAIN_led_success_pattern();
#endif
        touch_initialized = true;
        break;

    default:
        break;
    }
}

/******************************************************************************
 * NVS (Non-Volatile Storage) Initialization
 *****************************************************************************/

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 * @details Uses EARS_nvsEepromLib::performFullInitialization()
 */
void MAIN_initialise_nvs()
{
    // Guard against duplicate initialization
    if (nvs_state != NVS_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[NVS] Already initialized, skipping");
#endif
        return;
    }

#if EARS_DEBUG == 1
    Serial.println("[INIT] Initializing NVS...");
#endif

    NVSValidationResult result = using_nvseeprom().performFullInitialization();

    switch (result.status)
    {
    case NVSStatus::INITIALIZATION_FAILED:
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS flash initialization failed!");
        MAIN_led_error_pattern(10);
        MAIN_led_red_on();
#endif
        nvs_state = NVS_NOT_INITIALIZED;
        break;

    case NVSStatus::MISSING_ZAPNUMBER:
        if (result.currentVersion == result.expectedVersion &&
            !result.zapNumberValid && !result.passwordHashValid)
        {
#if EARS_DEBUG == 1
            Serial.println("[INFO] First boot - NVS initialized with defaults");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            nvs_state = NVS_INITIALIZED_EMPTY;
        }
        else
        {
#if EARS_DEBUG == 1
            Serial.println("[INFO] NVS needs ZapNumber");
            MAIN_led_warning_pattern(3);
            MAIN_led_yellow_on();
#endif
            nvs_state = NVS_NEEDS_ZAPNUMBER;
        }
        break;

    case NVSStatus::MISSING_PASSWORD:
#if EARS_DEBUG == 1
        Serial.printf("[OK] ZapNumber valid: %s\n", result.zapNumber);
        Serial.println("[INFO] NVS needs Password");
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_NEEDS_PASSWORD;
        break;

    case NVSStatus::VALID:
#if EARS_DEBUG == 1
        Serial.println("[OK] NVS fully validated and ready");
        MAIN_led_success_pattern();
#endif
        nvs_state = NVS_READY;
        break;

    case NVSStatus::UPGRADED:
#if EARS_DEBUG == 1
        Serial.printf("[INFO] NVS upgraded from v%d to v%d\n",
                      result.currentVersion, result.expectedVersion);
        Serial.println("[OK] NVS fully validated and ready");
        MAIN_led_success_pattern();
#endif
        nvs_state = NVS_READY;
        break;

    case NVSStatus::INVALID_VERSION:
    case NVSStatus::CRC_FAILED:
    default:
#if EARS_DEBUG == 1
        Serial.println("[ERROR] NVS validation failed");
        MAIN_led_error_pattern(5);
        MAIN_led_yellow_on();
#endif
        nvs_state = NVS_INITIALIZED_EMPTY;
        break;
    }
}

/******************************************************************************
 * SD Card Initialization
 *****************************************************************************/

/**
 * @brief Initialize SD Card (SD_MMC mode)
 * @details Uses EARS_sdCardLib::performFullInitialization()
 */
void MAIN_initialise_sd()
{
    // Guard against duplicate initialization
    if (sd_card_state != SD_NOT_INITIALIZED)
    {
#if EARS_DEBUG == 1
        Serial.println("[SD] Already initialized, skipping");
#endif
        return;
    }

    SDCardInitResult result = using_sdcard().performFullInitialization();
    sd_card_state = result.state;

    switch (result.state)
    {
    case SD_INIT_FAILED:
#if EARS_DEBUG == 1
        MAIN_led_error_pattern(3);
        MAIN_led_red_on();
#endif
        break;

    case SD_NO_CARD:
#if EARS_DEBUG == 1
        MAIN_led_warning_pattern(3);
        MAIN_led_yellow_on();
#endif
        break;

    case SD_CARD_READY:
#if EARS_DEBUG == 1
        MAIN_led_success_pattern();
#endif
        break;

    default:
        break;
    }
}

/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_Initialization_getLibraryName() {
    return MAIN_Initialization::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Initialization_getVersionEncoded() {
    return VERS_ENCODE(MAIN_Initialization::VERSION_MAJOR, 
                       MAIN_Initialization::VERSION_MINOR, 
                       MAIN_Initialization::VERSION_PATCH);
}

// Get version date
const char* MAIN_Initialization_getVersionDate() {
    return MAIN_Initialization::VERSION_DATE;
}

// Format version as string
void MAIN_Initialization_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_Initialization_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/******************************************************************************
 * End of MAIN_initializationLib.cpp
 *****************************************************************************/
