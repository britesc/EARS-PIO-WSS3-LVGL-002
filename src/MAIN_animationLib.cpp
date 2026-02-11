/**
 * @file MAIN_animationLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Startup animation implementation for EARS
 * @version 0.1.0
 * @date 20260209
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_animationLib.h"
#include "EARS_systemDef.h"

// Declare the 3 embedded animation frames (104×104 pixels, RGB565)
LV_IMAGE_DECLARE(MAIN_animFrame1);
LV_IMAGE_DECLARE(MAIN_animFrame2);
LV_IMAGE_DECLARE(MAIN_animFrame3);

/******************************************************************************
 * Private Variables
 *****************************************************************************/
static uint8_t current_frame = 0;       // Current frame index (0, 1, 2)
static uint32_t last_frame_time = 0;    // Last frame update timestamp

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**
 * @brief Create startup animation screen
 * @return lv_obj_t* Pointer to animation image object (NULL on error)
 */
lv_obj_t* MAIN_create_startup_animation(void)
{
    DEBUG_PRINTLN("[ANIM] Creating startup animation");
    
    // Create fullscreen image object on active screen
    lv_obj_t *anim_img = lv_image_create(lv_screen_active());
    if (anim_img == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create animation image object");
        return NULL;
    }
    
    // Centre the image on screen (104×104 on 480×320 display)
    lv_obj_center(anim_img);
    
    // Set first frame
    lv_image_set_src(anim_img, &MAIN_animFrame1);
    
    // Initialize animation state
    current_frame = 0;
    last_frame_time = millis();
    
    DEBUG_PRINTLN("[ANIM] Startup animation created - Frame 1 displayed");
    
    return anim_img;
}

/**
 * @brief Update animation frame (call from Core0 task)
 * @param anim_img Pointer to animation image object
 */
void MAIN_update_animation_frame(lv_obj_t* anim_img)
{
    // Guard: Check valid object
    if (anim_img == NULL)
    {
        return;
    }
    
    uint32_t now = millis();
    
    // Check if it's time to advance frame
    if (now - last_frame_time >= ANIM_FRAME_DELAY_MS)
    {
        // Advance to next frame (loop: 0 → 1 → 2 → 0 → 1 → 2...)
        current_frame++;
        if (current_frame > 2)
        {
            current_frame = 0;
        }
        
        // Update image source based on current frame
        switch (current_frame)
        {
            case 0:
                lv_image_set_src(anim_img, &MAIN_animFrame1);
                DEBUG_PRINTLN("[ANIM] Frame 1");
                break;
                
            case 1:
                lv_image_set_src(anim_img, &MAIN_animFrame2);
                DEBUG_PRINTLN("[ANIM] Frame 2");
                break;
                
            case 2:
                lv_image_set_src(anim_img, &MAIN_animFrame3);
                DEBUG_PRINTLN("[ANIM] Frame 3");
                break;
                
            default:
                // Should never reach here
                current_frame = 0;
                lv_image_set_src(anim_img, &MAIN_animFrame1);
                break;
        }
        
        // Update last frame time
        last_frame_time = now;
    }
}

/**
 * @brief Get current animation frame number
 * @return uint8_t Current frame (0, 1, or 2)
 */
uint8_t MAIN_get_current_animation_frame(void)
{
    return current_frame;
}

/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_Animation_getLibraryName() {
    return MAIN_Animation::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_Animation_getVersionEncoded() {
    return VERS_ENCODE(MAIN_Animation::VERSION_MAJOR, 
                       MAIN_Animation::VERSION_MINOR, 
                       MAIN_Animation::VERSION_PATCH);
}

// Get version date
const char* MAIN_Animation_getVersionDate() {
    return MAIN_Animation::VERSION_DATE;
}

// Format version as string
void MAIN_Animation_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_Animation_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}

/******************************************************************************
 * End of MAIN_animationLib.cpp
 ******************************************************************************/
