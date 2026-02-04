/**
 * @file MAIN_core1TasksLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 1 Background Task implementation (extracted from main.cpp)
 * @details Manages Core 1 background task - System initialization and monitoring
 * @version 0.1.0
 * @date 20260204
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_core1TasksLib.h"
#include "EARS_systemDef.h"

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

/******************************************************************************
 * External Function Prototypes
 *****************************************************************************/

// These functions are defined in main.cpp (will be moved to libraries in Steps 4 & 5)
extern void initialise_nvs();
extern void initialise_sd();

/******************************************************************************
 * Core 1 Background Task Function
 *****************************************************************************/

/**
 * @brief Core 1 Background Task (runs on Core 1)
 * @param parameter Task parameter (unused)
 * @details Handles system initialization and background monitoring at 10Hz
 *
 * This task is responsible for:
 * - One-time initialization (NVS, SD card)
 * - System monitoring and maintenance
 * - LED heartbeat indication
 * - Future: WiFi, BLE, sensor polling, data logging
 */
void MAIN_core1_background_task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Background Task started");
#endif

    // One-time initialization tasks
    initialise_nvs();
    initialise_sd();

    // Continuous background loop
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / CORE1_FREQUENCY_HZ); // 100ms for 10Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core1_heartbeat();

        // Toggle green LED every 500ms (heartbeat at 1Hz)
        if (DEV_get_core1_heartbeat() % 5 == 0)
        {
            MAIN_led_green_toggle();
        }
#endif

        // Future: Add background monitoring tasks here
        // - Check system health
        // - Monitor temperatures
        // - Handle WiFi/BLE
        // - Log data to SD card

        // Wait for next cycle
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/******************************************************************************
 * Task Creation Function
 *****************************************************************************/

/**
 * @brief Create Core 1 Background Task
 * @param taskHandle Pointer to store task handle
 * @return true if task created successfully
 * @return false if task creation failed
 */
bool MAIN_create_core1_task(TaskHandle_t *taskHandle)
{
    if (taskHandle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Invalid Core 1 task handle pointer");
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[INIT] Creating Core 1 background task...");
#endif

    // Create Core 1 Background Task
    BaseType_t result = xTaskCreatePinnedToCore(
        MAIN_core1_background_task,
        "Core1_Background",
        CORE1_STACK_SIZE,
        NULL,
        CORE1_PRIORITY,
        taskHandle,
        1 // Pin to Core 1
    );

    if (result != pdPASS || *taskHandle == NULL)
    {
#if EARS_DEBUG == 1
        Serial.println("[ERROR] Failed to create Core 1 background task!");
#endif
        return false;
    }

#if EARS_DEBUG == 1
    Serial.println("[OK] Core 1 background task created");
#endif

    return true;
}

/******************************************************************************
 * End of MAIN_core1TasksLib.cpp
 *****************************************************************************/