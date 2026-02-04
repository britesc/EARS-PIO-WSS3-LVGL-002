/**
 * @file MAIN_core0TasksLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 0 UI Task implementation (extracted from main.cpp)
 * @details Manages Core 0 UI task - LVGL processing at 200Hz
 * @version 0.1.0
 * @date 20260204
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_core0TasksLib.h"
#include "EARS_systemDef.h"
#include <lvgl.h>

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_developmentFeaturesLib.h"
#endif

/******************************************************************************
 * Core 0 UI Task Function
 *****************************************************************************/

/**
 * @brief Core 0 UI Task (runs on Core 0)
 * @param parameter Task parameter (unused)
 * @details Handles LVGL UI processing at 200Hz
 *
 * This task is responsible for:
 * - Running LVGL timer handler (updates widgets, animations)
 * - Processing UI events
 * - Maintaining smooth display updates
 * - Future: Touch input processing
 */
void MAIN_core0_ui_task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
#endif

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / CORE0_FREQUENCY_HZ); // 5ms for 200Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core0_heartbeat();
#endif

        // Run LVGL task handler (processes timers, animations, redraws)
        lv_timer_handler();

        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/******************************************************************************
 * Task Creation Function
 *****************************************************************************/

/**
 * @brief Create Core 0 UI Task
 * @param taskHandle Pointer to store task handle
 * @return true if task created successfully
 * @return false if task creation failed
 */
bool MAIN_create_core0_task(TaskHandle_t *taskHandle)
{
    if (taskHandle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Invalid Core 0 task handle pointer");
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating Core 0 UI task...");
#endif

    // Create Core 0 UI Task
    BaseType_t result = xTaskCreatePinnedToCore(
        MAIN_core0_ui_task,
        "Core0_UI",
        CORE0_STACK_SIZE,
        NULL,
        CORE0_PRIORITY,
        taskHandle,
        0 // Pin to Core 0
    );

    if (result != pdPASS || *taskHandle == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 0 UI task!");
#endif
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 0 UI task created");
#endif

    return true;
}

/******************************************************************************
 * End of MAIN_core0TasksLib.cpp
 *****************************************************************************/