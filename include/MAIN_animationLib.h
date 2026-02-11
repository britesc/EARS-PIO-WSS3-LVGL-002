/**
 * @file MAIN_animationLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Startup animation functions for EARS
 * @version 0.1.0
 * @date 20260209
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_ANIMATION_LIB_H__
#define __MAIN_ANIMATION_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <lvgl.h>
#include "EARS_versionDef.h"

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace MAIN_Animation
{
    constexpr const char* LIB_NAME = "MAIN_Animation";
    constexpr const char* VERSION_MAJOR = "1";
    constexpr const char* VERSION_MINOR = "0";
    constexpr const char* VERSION_PATCH = "0";
    constexpr const char* VERSION_DATE = "2026-02-09";
}

/******************************************************************************
 * Animation Configuration
 *****************************************************************************/

/**
 * @brief Frame delay in milliseconds
 * @details Tested in ESF with 550ms total duration for 3 frames
 *          = 183ms per frame for proper marching pace
 */
#define ANIM_FRAME_DELAY_MS 183

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Create startup animation screen
 * @return lv_obj_t* Pointer to animation image object (NULL on error)
 * @note Must be called after LVGL initialization
 */
lv_obj_t* MAIN_create_startup_animation(void);

/**
 * @brief Update animation frame (call from Core0 task)
 * @param anim_img Pointer to animation image object
 * @note Automatically handles frame timing and looping
 */
void MAIN_update_animation_frame(lv_obj_t* anim_img);

/**
 * @brief Get current animation frame number
 * @return uint8_t Current frame (0, 1, or 2)
 */
uint8_t MAIN_get_current_animation_frame(void);

/******************************************************************************
 * Version Information Getters
 *****************************************************************************/
const char* MAIN_Animation_getLibraryName();
uint32_t MAIN_Animation_getVersionEncoded();
const char* MAIN_Animation_getVersionDate();
void MAIN_Animation_getVersionString(char* buffer);

#endif // __MAIN_ANIMATION_LIB_H__

/******************************************************************************
 * End of MAIN_animationLib.h
 ******************************************************************************/
