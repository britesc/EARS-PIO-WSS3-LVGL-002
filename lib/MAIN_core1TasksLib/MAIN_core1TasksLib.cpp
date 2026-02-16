/**
 * @file MAIN_core1TasksLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 1 Background Task implementation
 * @details Manages Core 1 background task - Config checking → Screen routing → Monitoring
 * @version 1.2.0
 * @date 20260215
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_core1TasksLib.h"
#include "EARS_systemDef.h"
#include "MAIN_animationGfxLib.h"   // For signaling screen ID to Core0
#include "MAIN_initializationLib.h" // For MAIN_initialise_nvs() and MAIN_initialise_sd()
#include "EARS_nvsEepromLib.h"      // For checking ZapNumber/Password

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_ledLib.h"
#include "MAIN_developmentFeaturesLib.h"
#endif

/******************************************************************************
 * Private Variables
 *****************************************************************************/
static bool configCheckComplete = false;
static uint8_t determinedScreenID = 0;

/******************************************************************************
 * Private Helper Functions
 *****************************************************************************/

/**
 * @brief Check device configuration status
 * @return Screen ID to display (0=config, 1=main menu, 2=error)
 */
static uint8_t checkDeviceConfiguration()
{
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Checking device configuration...");
#endif

    // Check NVS for ZapNumber
    bool hasZapNumber = false;
    // TODO: Implement actual NVS check
    // hasZapNumber = EARS_NVS::hasKey("ZapNumber");
#if EARS_DEBUG == 1
    Serial.printf("[CORE1]   ZapNumber: %s\n", hasZapNumber ? "FOUND" : "NOT FOUND");
#endif

    // Check NVS for Password
    bool hasPassword = false;
    // TODO: Implement actual NVS check
    // hasPassword = EARS_NVS::hasKey("Password");
#if EARS_DEBUG == 1
    Serial.printf("[CORE1]   Password: %s\n", hasPassword ? "FOUND" : "NOT FOUND");
#endif

    // Check SD Card status
    bool sdCardReady = false;
    // TODO: Implement actual SD card check
    // sdCardReady = EARS_SDCard::isReady();
#if EARS_DEBUG == 1
    Serial.printf("[CORE1]   SD Card: %s\n", sdCardReady ? "READY" : "NOT READY");
#endif

    // Determine which screen to show
    if (!hasZapNumber || !hasPassword)
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE1] → Device requires configuration");
        Serial.println("[CORE1] → Will display Configuration Screen (ID 0)");
#endif
        return 0; // Configuration screen
    }
    else if (!sdCardReady)
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE1] → SD Card issue detected");
        Serial.println("[CORE1] → Will display Error/Warning Screen (ID 2)");
#endif
        return 2; // Error/Warning screen
    }
    else
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE1] → Device fully configured and ready");
        Serial.println("[CORE1] → Will display Main Menu Screen (ID 1)");
#endif
        return 1; // Main menu screen
    }
}

/******************************************************************************
 * Core 1 Background Task Function
 *****************************************************************************/

/**
 * @brief Core 1 Background Task (runs on Core 1)
 * @param parameter Task parameter (unused)
 * @details Handles configuration checking → screen routing → background monitoring
 *
 * STARTUP PHASE (one-time):
 * - Initialize NVS
 * - Initialize SD Card
 * - Check device configuration (ZapNumber, Password, SD Card)
 * - Determine appropriate screen to display
 * - Signal Core0 to show that screen (via MAIN_AnimationGfxLib::stop)
 *
 * MONITORING PHASE (continuous):
 * - Monitor system health
 * - Update LED heartbeat (development mode)
 * - Future: WiFi/BLE, sensors, data logging
 */
void MAIN_core1_background_task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Background Task started");
#endif

    // ========================================================================
    // DEFERRED INITIALIZATION (moved from setup() for faster animation start)
    // ========================================================================

    // Initialize LEDs (moved from setup() to not delay animation)
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Initializing development LEDs...");
    MAIN_led_init();
    MAIN_led_test_sequence(200);
    Serial.println("[CORE1] [OK] LEDs initialized");

    // Print detailed system info (moved from setup() to not delay animation)
    Serial.println("[CORE1] Printing system information...");
    DEV_print_system_info();
    Serial.println("[CORE1] [OK] System info printed\n");
#endif

    // ========================================================================
    // STARTUP PHASE - Configuration Check and Screen Routing
    // ========================================================================

#if EARS_DEBUG == 1
    Serial.println("[CORE1] ========================================");
    Serial.println("[CORE1] Starting configuration check...");
    Serial.println("[CORE1] ========================================");
#endif

    // Record start time for animation duration enforcement
    uint32_t startTime = millis();

    // Small delay to allow animation to start on Core0
    vTaskDelay(pdMS_TO_TICKS(100));

    // Initialize NVS
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Initializing NVS...");
#endif
    MAIN_initialise_nvs();

    // Initialize SD Card
#if EARS_DEBUG == 1
    Serial.println("[CORE1] Initializing SD Card...");
#endif
    MAIN_initialise_sd();

    // Check device configuration and determine screen
    determinedScreenID = checkDeviceConfiguration();

    // Calculate elapsed time and wait for minimum animation duration
    uint32_t elapsedTime = millis() - startTime;
    const uint32_t MIN_ANIMATION_DURATION_MS = 3000; // 3 seconds minimum

    if (elapsedTime < MIN_ANIMATION_DURATION_MS)
    {
        uint32_t remainingTime = MIN_ANIMATION_DURATION_MS - elapsedTime;
#if EARS_DEBUG == 1
        Serial.printf("[CORE1] Config check complete in %d ms\n", elapsedTime);
        Serial.printf("[CORE1] Waiting %d ms for minimum animation duration...\n", remainingTime);
#endif
        vTaskDelay(pdMS_TO_TICKS(remainingTime));
    }

    // Signal Core0 which screen to display (via animation library)
#if EARS_DEBUG == 1
    Serial.println("[CORE1] ========================================");
    Serial.printf("[CORE1] Signaling Core0 to load Screen ID %d\n", determinedScreenID);
    Serial.println("[CORE1] ========================================");
#endif

    MAIN_AnimationGfxLib::stop(determinedScreenID);
    configCheckComplete = true;

#if EARS_DEBUG == 1
    Serial.println("[CORE1] Configuration check complete");
    Serial.println("[CORE1] Entering monitoring mode...\n");
#endif

    // ========================================================================
    // MONITORING PHASE - Continuous Background Operations
    // ========================================================================

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
        // - Handle WiFi/BLE communications
        // - Log data to SD card
        // - Update system status displays

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
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char *MAIN_Core1Tasks_getLibraryName()
{
    return MAIN_Core1Tasks::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Core1Tasks_getVersionEncoded()
{
    return VERS_ENCODE(MAIN_Core1Tasks::VERSION_MAJOR,
                       MAIN_Core1Tasks::VERSION_MINOR,
                       MAIN_Core1Tasks::VERSION_PATCH);
}

// Get version date
const char *MAIN_Core1Tasks_getVersionDate()
{
    return MAIN_Core1Tasks::VERSION_DATE;
}

// Format version as string
void MAIN_Core1Tasks_getVersionString(char *buffer)
{
    uint32_t encoded = MAIN_Core1Tasks_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/******************************************************************************
 * End of MAIN_core1TasksLib.cpp
 *****************************************************************************/