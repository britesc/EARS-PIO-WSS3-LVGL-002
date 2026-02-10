/**
 * @file MAIN_developmentFeaturesLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Development features library for EARS
 * @details Contains all development/debugging features that compile out in production
 * @version 1.0.0
 * @date 20260210
 *
 * PURPOSE:
 * - Development screen display
 * - System heartbeat tracking
 * - Live statistics display
 * - Boot information
 *
 * PRODUCTION BUILD:
 * Set EARS_DEBUG=0 in platformio.ini to completely remove all development features
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_DEVELOPMENT_FEATURES_LIB_H__
#define __MAIN_DEVELOPMENT_FEATURES_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include "EARS_versionDef.h"
#include <Arduino_GFX_Library.h>

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace MAIN_DevFeatures
{
    constexpr const char* LIB_NAME = "MAIN_DevelopmentFeatures";
    constexpr const char* VERSION_MAJOR = "1";
    constexpr const char* VERSION_MINOR = "0";
    constexpr const char* VERSION_PATCH = "0";
    constexpr const char* VERSION_DATE = "2026-02-10";
}


// Version information getters
const char* MAIN_DevelopmentFeatures_getLibraryName();
uint32_t MAIN_DevelopmentFeatures_getVersionEncoded();
const char* MAIN_DevelopmentFeatures_getVersionDate();
void MAIN_DevelopmentFeatures_getVersionString(char* buffer);

/******************************************************************************
 * Development Mode Variables
 *****************************************************************************/

// Core heartbeat counters (volatile for cross-core access)
extern volatile uint32_t dev_core0_heartbeat;
extern volatile uint32_t dev_core1_heartbeat;
extern volatile uint32_t dev_display_updates;

/******************************************************************************
 * Boot Banner Functions
 *****************************************************************************/

/**
 * @brief Print EARS boot banner
 * @details Displays version, compiler, platform info
 * @return void
 */
void DEV_print_boot_banner(void);

/**
 * @brief Print complete system information report
 * @details Calls MAIN_sysinfo_print_all() if available
 * @return void
 */
void DEV_print_system_info(void);

/******************************************************************************
 * Development Screen Functions
 *****************************************************************************/

/**
 * @brief Draw the initial development screen layout
 * @param gfx Pointer to Arduino_GFX display object
 * @return void
 */
void DEV_draw_screen(Arduino_GFX *gfx);

/**
 * @brief Update live statistics on development screen
 * @param gfx Pointer to Arduino_GFX display object
 * @return void
 */
void DEV_update_screen(Arduino_GFX *gfx);

/******************************************************************************
 * Heartbeat Management Functions
 *****************************************************************************/

/**
 * @brief Increment Core 0 heartbeat counter
 * @return void
 */
void DEV_increment_core0_heartbeat(void);

/**
 * @brief Increment Core 1 heartbeat counter
 * @return void
 */
void DEV_increment_core1_heartbeat(void);

/**
 * @brief Increment display update counter
 * @return void
 */
void DEV_increment_display_updates(void);

/**
 * @brief Get Core 0 heartbeat count
 * @return uint32_t Current heartbeat count
 */
uint32_t DEV_get_core0_heartbeat(void);

/**
 * @brief Get Core 1 heartbeat count
 * @return uint32_t Current heartbeat count
 */
uint32_t DEV_get_core1_heartbeat(void);

/**
 * @brief Get display update count
 * @return uint32_t Current update count
 */
uint32_t DEV_get_display_updates(void);

#endif // __MAIN_DEVELOPMENT_FEATURES_LIB_H__

/******************************************************************************
 * End of MAIN_developmentFeaturesLib.h
 ******************************************************************************/