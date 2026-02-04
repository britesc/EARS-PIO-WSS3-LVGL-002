/**
 * @file MAIN_core0TasksLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 0 UI Task management for EARS (extracted from main.cpp)
 * @details Manages Core 0 UI task - LVGL processing at 200Hz
 * @version 0.1.0
 * @date 20260204
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

/******************************************************************************
 * Core 0 Configuration
 *****************************************************************************/

// Stack size (in words, not bytes)
#define CORE0_STACK_SIZE 8192

// Task priority
#define CORE0_PRIORITY 2

// Task update frequency
#define CORE0_FREQUENCY_HZ 200 // 200Hz for smooth LVGL

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
 * @details Handles LVGL UI processing at 200Hz
 *
 * Responsibilities:
 * - LVGL timer handler (lv_timer_handler)
 * - UI updates and animations
 * - Touch input processing (future)
 * - Display rendering
 */
void MAIN_core0_ui_task(void *parameter);

#endif // __MAIN_CORE0_TASKS_LIB_H__

/******************************************************************************
 * End of MAIN_core0TasksLib.h
 *****************************************************************************/