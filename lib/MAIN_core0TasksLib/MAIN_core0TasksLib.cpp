/**
 * @file MAIN_core0TasksLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Core 0 UI Task implementation with animation support
 * @details Manages Core 0 UI task - Animation updates → LVGL initialization → UI processing
 * @version 1.2.0
 * @date 20260215
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_core0TasksLib.h"
#include "EARS_systemDef.h"
#include "EARS_ws35tlcdPins.h"      // For TFT_WIDTH and TFT_HEIGHT
#include "EARS_rgb565ColoursDef.h"  // RGB565 color definitions
#include "EARS_rgb888ColoursDef.h"  // RGB888 color definitions
#include "MAIN_animationGfxLib.h"   // Animation support (Arduino GFX)
#include "MAIN_lvglLib.h"           // LVGL initialization
#include "MAIN_displayLib.h"        // Display initialization (PWM backlight)
#include "MAIN_initializationLib.h" // Touch initialization
#include <lvgl.h>
#include "ui/ui.h" // ESF generated UI

// Development tools (compile out in production)
#if EARS_DEBUG == 1
#include "MAIN_developmentFeaturesLib.h"
#endif

/******************************************************************************
 * External References
 *****************************************************************************/
extern Arduino_GFX *gfx;                // Display object from main.cpp
extern SemaphoreHandle_t xDisplayMutex; // Display mutex from main.cpp

/******************************************************************************
 * Private Variables
 *****************************************************************************/
static bool lvglInitialised = false;

/******************************************************************************
 * Private Helper Functions
 *****************************************************************************/

/**
 * @brief Load appropriate ESF screen based on target screen ID
 * @param screenID Screen ID from Core1 (via animation library)
 */
static void loadESFScreen(uint8_t screenID)
{
#if EARS_DEBUG == 1
    Serial.printf("[CORE0] Loading ESF Screen ID: %d\n", screenID);
#endif

    // Get active screen
    lv_obj_t *screen = lv_screen_active();

    // Set background to TRUE_BLACK
    lv_obj_set_style_bg_color(screen, lv_color_hex(EARS_RGB888_TRUE_BLACK), LV_PART_MAIN);

    // TEMPORARY PLACEHOLDER LABELS (until ESF files copied to src/ui/)
    // Create visible test labels to verify LVGL is working

    // Center label with system info
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "EARS v0.21.0\n\nSystem Initialized\n\nReady for ESF Screens");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Bottom label with screen ID
    lv_obj_t *screenLabel = lv_label_create(screen);
    char screenText[64];
    snprintf(screenText, sizeof(screenText), "Screen ID: %d", screenID);
    lv_label_set_text(screenLabel, screenText);
    lv_obj_set_style_text_color(screenLabel, lv_color_make(255, 255, 0), LV_PART_MAIN);
    lv_obj_align(screenLabel, LV_ALIGN_BOTTOM_MID, 0, -20);

    // TODO: When ESF ui/ files are in src/ui/, uncomment this section
    // and comment out the placeholder labels above
    // Screen ID mapping: 0=Start/Error, 1=Config, 2=Main
    /*
    switch (screenID)
    {
    case 0:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Start/Error Screen");
#endif
        loadScreen(SCREEN_ID_SCREEN_START);  // ESF ID 1
        break;
    }
    case 1:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Configuration Screen");
#endif
        loadScreen(SCREEN_ID_SCREEN_CONFIG);  // ESF ID 2
        break;
    }
    case 2:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Main Menu Screen");
#endif
        loadScreen(SCREEN_ID_SCREEN_MAIN);  // ESF ID 3
        break;
    }
    default:
    {
#if EARS_DEBUG == 1
        Serial.printf("[CORE0]   -> Unknown Screen ID %d, defaulting to Config\n", screenID);
#endif
        loadScreen(SCREEN_ID_SCREEN_CONFIG);  // Default to config
        break;
    }
    }
    */

    // Temporary screen-specific indicators (until ESF integrated)
    // Screen ID mapping: 0=Start/Error, 1=Config, 2=Main
    switch (screenID)
    {
    case 0:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Start/Error Screen (placeholder)");
#endif
        lv_obj_t *errorLabel = lv_label_create(screen);
        lv_label_set_text(errorLabel, "Start/Error Screen");
        lv_obj_set_style_text_color(errorLabel, lv_color_make(255, 0, 0), LV_PART_MAIN);
        lv_obj_align(errorLabel, LV_ALIGN_TOP_MID, 0, 20);
        break;
    }

    case 1:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Configuration Screen (placeholder)");
#endif
        lv_obj_t *configLabel = lv_label_create(screen);
        lv_label_set_text(configLabel, "Configuration Required");
        lv_obj_set_style_text_color(configLabel, lv_color_make(255, 128, 0), LV_PART_MAIN);
        lv_obj_align(configLabel, LV_ALIGN_TOP_MID, 0, 20);
        break;
    }

    case 2:
    {
#if EARS_DEBUG == 1
        Serial.println("[CORE0]   -> Main Menu Screen (placeholder)");
#endif
        lv_obj_t *menuLabel = lv_label_create(screen);
        lv_label_set_text(menuLabel, "Main Menu");
        lv_obj_set_style_text_color(menuLabel, lv_color_make(0, 255, 0), LV_PART_MAIN);
        lv_obj_align(menuLabel, LV_ALIGN_TOP_MID, 0, 20);
        break;
    }

    default:
    {
#if EARS_DEBUG == 1
        Serial.printf("[CORE0]   -> Unknown Screen ID %d (placeholder)\n", screenID);
#endif
        lv_obj_t *unknownLabel = lv_label_create(screen);
        lv_label_set_text(unknownLabel, "Unknown Screen");
        lv_obj_set_style_text_color(unknownLabel, lv_color_make(255, 0, 0), LV_PART_MAIN);
        lv_obj_align(unknownLabel, LV_ALIGN_TOP_MID, 0, 20);
        break;
    }
    }

#if EARS_DEBUG == 1
    Serial.println("[CORE0] [OK] Screen loaded (placeholder)");
#endif
}

/******************************************************************************
 * Core 0 UI Task Function
 *****************************************************************************/

/**
 * @brief Core 0 UI Task (runs on Core 0)
 * @param parameter Task parameter (unused)
 * @details Handles animation updates → LVGL initialization → UI processing
 *
 * PHASE 1: ANIMATION (runs until animation completes)
 * - Update animation frames (200ms per frame, 3 seconds minimum)
 * - Monitor animation completion status
 *
 * PHASE 2: TRANSITION (one-time when animation completes)
 * - Initialize LVGL subsystem
 * - Initialize PWM backlight
 * - Initialize touch controller
 * - Get target screen ID from animation library (set by Core1)
 * - Load appropriate ESF screen
 *
 * PHASE 3: UI OPERATION (continuous after initialization)
 * - Run LVGL timer handler at 200Hz
 * - Process touch input
 * - Update UI widgets and animations
 * - Handle user interactions
 */
void MAIN_core0_ui_task(void *parameter)
{
#if EARS_DEBUG == 1
    Serial.println("[CORE0] UI Task started");
    Serial.println("[CORE0] Phase 1: Animation mode");
#endif

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000 / CORE0_FREQUENCY_HZ); // 5ms for 200Hz

    while (1)
    {
#if EARS_DEBUG == 1
        DEV_increment_core0_heartbeat();
#endif

        // ====================================================================
        // PHASE 1 + 2: ANIMATION AND TRANSITION
        // ====================================================================
        if (!lvglInitialised)
        {
            // Update animation
            MAIN_AnimationGfxLib::update();

            // Check if Core1 has signaled to stop (NOT just time-based completion)
            // This ensures we wait for Core1's configuration check to complete
            if (MAIN_AnimationGfxLib::animationStopRequested)
            {
#if EARS_DEBUG == 1
                Serial.println("\n[CORE0] ========================================");
                Serial.println("[CORE0] Animation Complete!");
                Serial.println("[CORE0] Phase 2: Transition to LVGL");
                Serial.println("[CORE0] ========================================");

                // Show animation statistics
                Serial.printf("[CORE0] Animation ran for: %d ms\n",
                              (MAIN_AnimationGfxLib::getProgress() * 30)); // Approximate
                Serial.printf("[CORE0] Total frames displayed: %d\n",
                              MAIN_AnimationGfxLib::getFrameCount() * 5); // 5 cycles
#endif

                // Get target screen from animation library (set by Core1)
                uint8_t targetScreen = MAIN_AnimationGfxLib::getTargetScreenID();

#if EARS_DEBUG == 1
                Serial.printf("[CORE0] Target Screen ID: %d\n", targetScreen);
                Serial.println("[CORE0] Initializing LVGL subsystem...");
#endif

                // Initialize LVGL
                if (!MAIN_initialise_lvgl(gfx, xDisplayMutex, TFT_WIDTH, TFT_HEIGHT))
                {
#if EARS_DEBUG == 1
                    Serial.println("[CORE0] [ERROR] LVGL initialization failed!");
                    MAIN_led_red_on();
#endif
                    while (1)
                        delay(1000); // Halt on critical error
                }

#if EARS_DEBUG == 1
                Serial.println("[CORE0] [OK] LVGL initialized");
                // TODO: Uncomment when ESF files copied to src/ui/
                // Serial.println("[CORE0] Initializing ESF UI system...");
#endif

                // TODO: Uncomment when ESF files copied to src/ui/
                // ui_init();
                // Serial.println("[CORE0] [OK] ESF UI initialized");

#if EARS_DEBUG == 1
                Serial.println("[CORE0] Initializing touch controller...");
#endif

                // Initialize touch controller
                MAIN_initialise_touch();

#if EARS_DEBUG == 1
                Serial.println("[CORE0] [OK] Touch controller initialized");
                Serial.println("[CORE0] Loading ESF screen...");
#endif

                // Load appropriate screen
                loadESFScreen(targetScreen);

#if EARS_DEBUG == 1
                Serial.println("[CORE0] [OK] ESF screen loaded");
                Serial.println("[CORE0] ========================================");
                Serial.println("[CORE0] Phase 3: UI Operation mode");
                Serial.println("[CORE0] Running LVGL at 200Hz");
                Serial.println("[CORE0] ========================================\n");
#endif

                lvglInitialised = true;
            }
        }

        // ====================================================================
        // PHASE 3: UI OPERATION
        // ====================================================================
        else
        {
            // Run LVGL task handler (processes timers, animations, redraws)
            lv_timer_handler();

            // TODO: Uncomment when ESF files copied to src/ui/
            // ui_tick();  // Run ESF UI tick (updates screens, handles events)

            // Future: Touch input processing, gesture detection, etc.
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
const char *MAIN_Core0Tasks_getLibraryName()
{
    return MAIN_Core0Tasks::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Core0Tasks_getVersionEncoded()
{
    return VERS_ENCODE(MAIN_Core0Tasks::VERSION_MAJOR,
                       MAIN_Core0Tasks::VERSION_MINOR,
                       MAIN_Core0Tasks::VERSION_PATCH);
}

// Get version date
const char *MAIN_Core0Tasks_getVersionDate()
{
    return MAIN_Core0Tasks::VERSION_DATE;
}

// Format version as string
void MAIN_Core0Tasks_getVersionString(char *buffer)
{
    uint32_t encoded = MAIN_Core0Tasks_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/******************************************************************************
 * End of MAIN_core0TasksLib.cpp
 *****************************************************************************/