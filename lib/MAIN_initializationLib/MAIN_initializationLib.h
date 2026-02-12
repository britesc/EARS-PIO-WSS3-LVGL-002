/**
 * @file MAIN_initializationLib.h
 * @author JTB & Claude Sonnet 4.5
 * @brief Centralized initialization functions for EARS subsystems
 * @version 0.1.0
 * @date 20260210
 *
 * @details
 * This library consolidates initialization functions for Touch, NVS, and SD Card
 * subsystems, reducing clutter in main.cpp and providing a clean interface for
 * system startup sequencing.
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_INITIALIZATION_LIB_H__
#define __MAIN_INITIALIZATION_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include "EARS_versionDef.h"
#include "EARS_systemDef.h"
#include "EARS_ws35tlcdPins.h" // Hardware pin definitions (TOUCH_SDA, TOUCH_SCL)
#include "EARS_touchLib.h"
#include "EARS_nvsEepromLib.h"
#include "EARS_sdCardLib.h"

// LED library for status indication (debug builds only)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#endif

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace MAIN_Initialization
{
    constexpr const char *LIB_NAME = "MAIN_Initialization";
    constexpr const char *VERSION_MAJOR = "1";
    constexpr const char *VERSION_MINOR = "0";
    constexpr const char *VERSION_PATCH = "0";
    constexpr const char *VERSION_DATE = "2026-02-10";
}

/******************************************************************************
 * NVS State Machine
 *****************************************************************************/
/**
 * @enum NVSInitState
 * @brief State machine for NVS initialization tracking
 */
enum NVSInitState
{
    NVS_NOT_INITIALIZED,   ///< NVS not yet initialized
    NVS_INITIALIZED_EMPTY, ///< NVS initialized but empty (first boot)
    NVS_NEEDS_ZAPNUMBER,   ///< NVS requires ZapNumber entry
    NVS_NEEDS_PASSWORD,    ///< NVS requires Password entry
    NVS_READY              ///< NVS fully validated and ready
};

/******************************************************************************
 * Global State Variables
 *****************************************************************************/
extern volatile TouchState touch_state;
extern volatile bool touch_initialized;
extern volatile NVSInitState nvs_state;
extern volatile SDCardState sd_card_state;

/******************************************************************************
 * Initialization Functions
 *****************************************************************************/

/**
 * @brief Initialize Touch Controller
 * @details Uses EARS_touchLib::performFullInitialization()
 *          Updates touch_state and touch_initialized globals
 *          Provides LED feedback in debug builds
 */
void MAIN_initialise_touch();

/**
 * @brief Initialize NVS (Non-Volatile Storage)
 * @details Uses EARS_nvsEepromLib::performFullInitialization()
 *          Updates nvs_state global
 *          Handles first boot, upgrades, and validation
 *          Provides LED feedback in debug builds
 */
void MAIN_initialise_nvs();

/**
 * @brief Initialize SD Card (SD_MMC mode)
 * @details Uses EARS_sdCardLib::performFullInitialization()
 *          Updates sd_card_state global
 *          Provides LED feedback in debug builds
 */
void MAIN_initialise_sd();

/******************************************************************************
 * Version Information Getters
 *****************************************************************************/
const char *MAIN_Initialization_getLibraryName();
uint32_t MAIN_Initialization_getVersionEncoded();
const char *MAIN_Initialization_getVersionDate();
void MAIN_Initialization_getVersionString(char *buffer);

#endif // __MAIN_INITIALIZATION_LIB_H__

/******************************************************************************
 * End of MAIN_initializationLib.h
 *****************************************************************************/