/**
 * @file CORE0_task01Lib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 0 UI Task management for EARS
 * @details Manages Core 0 UI task - Animation → LVGL transition → UI processing at 200Hz
 * @version 1.2.0
 * @date 20260215
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_CORE0_TASKS_LIB_H__
#define __MAIN_CORE0_TASKS_LIB_H__

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
namespace MAIN_Core0Tasks
{
    constexpr const char *LIB_NAME = "MAIN_Core0Tasks";
    constexpr const char *VERSION_MAJOR = "1";
    constexpr const char *VERSION_MINOR = "2";
    constexpr const char *VERSION_PATCH = "0";
    constexpr const char *VERSION_DATE = "2026-02-14";
}

/******************************************************************************
 * Core 0 Configuration
 *****************************************************************************/

// Stack size (in words, not bytes)
#define CORE0_STACK_SIZE 8192

// Task priority
#define CORE0_PRIORITY 2

// Task update frequency
#define CORE0_FREQUENCY_HZ 200 // 200Hz for smooth LVGL and animation updates

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Create Core 0 UI Task
 * @param taskHandle Pointer to store task handle
 * @return true if task created successfully
 * @return false if task creation failed
 */
bool MAIN_create_core0_task(TaskHandle_t *taskHandle);

/**
 * @brief Core 0 UI Task function (runs on Core 0)
 * @param parameter Task parameter (unused)
 * @details Handles animation updates → LVGL initialization → UI processing
 *
 * Three-phase operation:
 *
 * PHASE 1 - ANIMATION MODE:
 * - Update animation frames (200ms per frame)
 * - Monitor completion status
 * - Runs until animation finishes (3 seconds minimum)
 *
 * PHASE 2 - TRANSITION MODE (one-time):
 * - Initialize LVGL subsystem
 * - Initialize PWM backlight
 * - Initialize touch controller
 * - Get target screen ID from Core1 (via animation library)
 * - Load appropriate ESF screen
 *
 * PHASE 3 - UI OPERATION MODE (continuous):
 * - Run LVGL timer handler at 200Hz
 * - Process touch input
 * - Update UI widgets and animations
 * - Handle user interactions
 */
void MAIN_core0_ui_task(void *parameter);

/******************************************************************************
 * Version Information Getters
 *****************************************************************************/
const char *MAIN_Core0Tasks_getLibraryName();
uint32_t MAIN_Core0Tasks_getVersionEncoded();
const char *MAIN_Core0Tasks_getVersionDate();
void MAIN_Core0Tasks_getVersionString(char *buffer);

#endif // __MAIN_CORE0_TASKS_LIB_H__

/******************************************************************************
 * End of CORE0_task01Lib.h
 *****************************************************************************/
