/*
 * MAIN_animationGfxLib.h
 * 
 * Animation Library for EARS (Equipment & Ammunition Reporting System)
 * Handles startup animation display using Arduino GFX Library
 * 
 * Features:
 * - Displays sequential frame animation on startup
 * - Monitors control variables for stop command
 * - Transitions to ESF screens when complete
 * - Minimum 3 second animation duration
 * 
 * Author: EARS Development Team
 * Language: British English
 * Platform: ESP32-S3 (Waveshare 3.5" Touch LCD)
 * 
 * Copyright (c) 2025 EARS Project
 */

#ifndef MAIN_ANIMATIONGFXLIB_H
#define MAIN_ANIMATIONGFXLIB_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

namespace MAIN_AnimationGfxLib {

    // Version Information
    constexpr const char* getVersion() { return "0.1.0"; }
    constexpr const char* getBuildDate() { return __DATE__; }
    constexpr const char* getBuildTime() { return __TIME__; }
    
    // Animation Control Variables
    extern volatile bool animationStopRequested;
    extern volatile uint8_t targetScreenID;
    
    // Animation Configuration
    constexpr uint16_t ANIMATION_MIN_DURATION_MS = 3000;  // Minimum 3 seconds
    constexpr uint16_t DISPLAY_WIDTH = 480;
    constexpr uint16_t DISPLAY_HEIGHT = 320;
    
    // Animation States
    enum class AnimationState : uint8_t {
        IDLE = 0,
        INITIALISING,
        RUNNING,
        STOPPING,
        COMPLETED
    };
    
    // Function Declarations
    
    /**
     * @brief Initialise the animation library
     * @param gfx Pointer to Arduino_GFX display object
     * @return true if initialisation successful, false otherwise
     */
    bool initialise(Arduino_GFX* gfx);
    
    /**
     * @brief Start the animation sequence
     * @return true if animation started successfully, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the animation and prepare for transition
     * @param screenID The ESF screen ID to transition to (0-255)
     */
    void stop(uint8_t screenID = 0);
    
    /**
     * @brief Update animation (call from loop)
     * @return Current animation state
     */
    AnimationState update();
    
    /**
     * @brief Get current animation state
     * @return Current state
     */
    AnimationState getState();
    
    /**
     * @brief Check if animation is complete
     * @return true if animation has finished, false otherwise
     */
    bool isComplete();
    
    /**
     * @brief Get the target screen ID for ESF transition
     * @return Screen ID (0-255)
     */
    uint8_t getTargetScreenID();
    
    /**
     * @brief Get frame count
     * @return Number of animation frames available
     */
    uint8_t getFrameCount();
    
    /**
     * @brief Get current frame number
     * @return Current frame being displayed (0-based)
     */
    uint8_t getCurrentFrame();
    
    /**
     * @brief Get animation progress percentage
     * @return Progress 0-100%
     */
    uint8_t getProgress();

} // namespace MAIN_AnimationGfxLib

#endif // MAIN_ANIMATIONGFXLIB_H
