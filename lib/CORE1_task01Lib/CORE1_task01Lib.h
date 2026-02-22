/**
 * @file CORE1_task01Lib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 1 Background Task management for EARS
 * @details Manages Core 1 background task - Config checking → Screen routing → Monitoring
 * @version 1.2.0
 * @date 20260215
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_CORE1_TASKS_LIB_H__
#define __MAIN_CORE1_TASKS_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "EARS_versionDef.h"

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace MAIN_Core1Tasks
{
    constexpr const char *LIB_NAME = "MAIN_Core1Tasks";
    constexpr const char *VERSION_MAJOR = "1";
    constexpr const char *VERSION_MINOR = "2";
    constexpr const char *VERSION_PATCH = "0";
    constexpr const char *VERSION_DATE = "2026-02-14";
}

/******************************************************************************
 * Core 1 Configuration
 *****************************************************************************/

// Stack size (in words, not bytes)
#define CORE1_STACK_SIZE 4096

// Task priority
#define CORE1_PRIORITY 1

// Task update frequency
#define CORE1_FREQUENCY_HZ 10 // 10Hz for background tasks

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Create Core 1 Background Task
 * @param taskHandle Pointer to store task handle
 * @return true if task created successfully
 * @return false if task creation failed
 */
bool MAIN_create_core1_task(TaskHandle_t *taskHandle);

/**
 * @brief Core 1 Background Task function (runs on Core 1)
 * @param parameter Task parameter (unused)
 * @details Handles configuration checking → screen routing → background monitoring
 *
 * Two-phase operation:
 *
 * STARTUP PHASE (one-time):
 * - Initialize NVS (non-volatile storage)
 * - Initialize SD Card
 * - Check for ZapNumber in NVS
 * - Check for Password in NVS
 * - Check SD Card readiness
 * - Determine appropriate screen to display:
 *   → 0: Configuration screen (no ZapNumber or Password)
 *   → 1: Main menu (device fully configured)
 *   → 2: Error/Warning screen (SD card issues)
 * - Signal Core0 via MAIN_AnimationLib::stop(screenID)
 *
 * MONITORING PHASE (continuous):
 * - Monitor system health at 10Hz
 * - LED heartbeat indication (development mode)
 * - Future: WiFi/BLE, sensors, data logging
 */
void MAIN_core1_background_task(void *parameter);

/******************************************************************************
 * Version Information Getters
 *****************************************************************************/
const char *MAIN_Core1Tasks_getLibraryName();
uint32_t MAIN_Core1Tasks_getVersionEncoded();
const char *MAIN_Core1Tasks_getVersionDate();
void MAIN_Core1Tasks_getVersionString(char *buffer);

#endif // __MAIN_CORE1_TASKS_LIB_H__

/******************************************************************************
 * End of CORE1_task01Lib.h
 *****************************************************************************/
