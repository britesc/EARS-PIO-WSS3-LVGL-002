/**
 * @file MAIN_core1TasksLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 1 Background Task management for EARS (extracted from main.cpp)
 * @details Manages Core 1 background task - System initialization and monitoring
 * @version 0.1.0
 * @date 20260204
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
 * @details Handles system initialization and background monitoring at 10Hz
 *
 * Responsibilities:
 * - NVS initialization (once at startup)
 * - SD card initialization (once at startup)
 * - System monitoring (continuous)
 * - LED heartbeat (continuous)
 * - Future: WiFi, BLE, sensors, logging
 */
void MAIN_core1_background_task(void *parameter);

#endif // __MAIN_CORE1_TASKS_LIB_H__

/******************************************************************************
 * End of MAIN_core1TasksLib.h
 *****************************************************************************/