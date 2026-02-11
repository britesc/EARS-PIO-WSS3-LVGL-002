/**
 * @file MAIN_core0TasksLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 0 UI Task implementation with animation support
 * @details Manages Core 0 UI task - LVGL processing at 200Hz + Animation updates
 * @version 1.1.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_core0TasksLib.h"
#include "EARS_systemDef.h"
#include "MAIN_animationLib.h"  // NEW! Animation support
#include <lvgl.h>

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_developmentFeaturesLib.h"
#endif

/******************************************************************************
 * External Global Variables
 *****************************************************************************/

// Animation object created in main.cpp
extern lv_obj_t* g_animation_img;

/******************************************************************************
 * Core 0 UI Task Function
 *****************************************************************************/

/**
 * @brief Core 0 UI Task (runs on Core 0)
 * @param parameter Task parameter (unused)
 * @details Handles LVGL UI processing at 200Hz + Animation frame updates
 *
 * This task is responsible for:
 * - Running LVGL timer handler (updates widgets, animations)
 * - Processing UI events
 * - Updating animation frames (marching soldier)
 * - Maintaining smooth display updates
 * - Future: Touch input processing, transitions
 */
void MAIN_core0_ui_task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
    if (g_animation_img != NULL)
    {
        Serial.println("[CORE0] Animation enabled - soldier will march!");
    }
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

        // Update animation frame if animation object exists
        if (g_animation_img != NULL)
        {
            MAIN_update_animation_frame(g_animation_img);
        }

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
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_Core0Tasks_getLibraryName() {
    return MAIN_Core0Tasks::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Core0Tasks_getVersionEncoded() {
    return VERS_ENCODE(MAIN_Core0Tasks::VERSION_MAJOR, 
                       MAIN_Core0Tasks::VERSION_MINOR, 
                       MAIN_Core0Tasks::VERSION_PATCH);
}

// Get version date
const char* MAIN_Core0Tasks_getVersionDate() {
    return MAIN_Core0Tasks::VERSION_DATE;
}

// Format version as string
void MAIN_Core0Tasks_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_Core0Tasks_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/******************************************************************************
 * End of MAIN_core0TasksLib.cpp
 *****************************************************************************/
