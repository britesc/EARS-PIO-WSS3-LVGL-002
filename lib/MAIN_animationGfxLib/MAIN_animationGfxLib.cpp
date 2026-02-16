/*
 * MAIN_animationGfxLib.cpp
 *
 * Animation Library Implementation for EARS
 * Handles startup animation display using Arduino GFX Library
 *
 * Author: EARS Development Team
 * Language: British English
 * Platform: ESP32-S3 (Waveshare 3.5" Touch LCD)
 *
 * Copyright (c) 2025 EARS Project
 */

#include "MAIN_animationGfxLib.h"

// Frame data declarations (external linkage to .c files in src/anim/)
// Frame files expected at: src/anim/MAIN_animFrame1.c, etc.
extern "C"
{
    extern const unsigned char MAIN_animFrame1_map[];
    extern const unsigned char MAIN_animFrame2_map[];
    extern const unsigned char MAIN_animFrame3_map[];
}

namespace MAIN_AnimationGfxLib
{

    // Private namespace variables
    namespace
    {
        Arduino_GFX *gfxDisplay = nullptr;
        AnimationState currentState = AnimationState::IDLE;

        // Frame configuration
        constexpr uint8_t FRAME_COUNT = 3;
        constexpr uint16_t FRAME_WIDTH = 104;
        constexpr uint16_t FRAME_HEIGHT = 104;
        constexpr uint16_t FRAME_DURATION_MS = 200; // 200ms per frame

        // Timing variables
        uint32_t animationStartTime = 0;
        uint32_t lastFrameTime = 0;
        uint8_t currentFrameIndex = 0;

        // Calculated positions for centring frames
        int16_t frameX = 0;
        int16_t frameY = 0;

        // Frame pointers array
        const unsigned char *frames[FRAME_COUNT] = {
            MAIN_animFrame1_map,
            MAIN_animFrame2_map,
            MAIN_animFrame3_map};
    }

    // Public control variables
    volatile bool animationStopRequested = false;
    volatile uint8_t targetScreenID = 0;

    /**
     * @brief Display the current frame
     */
    void displayCurrentFrame()
    {
        if (gfxDisplay == nullptr || currentFrameIndex >= FRAME_COUNT)
        {
            return;
        }

        // Draw the current frame centred on screen
        // Arduino GFX uses draw16bitRGBBitmap for RGB565 data
        gfxDisplay->draw16bitRGBBitmap(
            frameX,
            frameY,
            (uint16_t *)frames[currentFrameIndex],
            FRAME_WIDTH,
            FRAME_HEIGHT);
    }

    /**
     * @brief Initialise the animation library
     */
    bool initialise(Arduino_GFX *gfx)
    {
        if (gfx == nullptr)
        {
            return false;
        }

        gfxDisplay = gfx;

        // Calculate frame positioning (centre on screen)
        frameX = (DISPLAY_WIDTH - FRAME_WIDTH) / 2;
        frameY = (DISPLAY_HEIGHT - FRAME_HEIGHT) / 2;

        // Reset state
        currentState = AnimationState::IDLE;
        currentFrameIndex = 0;
        animationStopRequested = false;
        targetScreenID = 0;

        return true;
    }

    /**
     * @brief Start the animation sequence
     */
    bool start()
    {
        if (gfxDisplay == nullptr)
        {
            return false;
        }

        if (currentState != AnimationState::IDLE)
        {
            return false; // Already running or in wrong state
        }

        currentState = AnimationState::INITIALISING;

        // Clear screen to black
        gfxDisplay->fillScreen(0x0000); // Black in RGB565

        // Record start time
        animationStartTime = millis();
        lastFrameTime = animationStartTime;
        currentFrameIndex = 0;

        // Display first frame immediately
        displayCurrentFrame();

        currentState = AnimationState::RUNNING;

        return true;
    }

    /**
     * @brief Stop the animation and prepare for transition
     */
    void stop(uint8_t screenID)
    {
        targetScreenID = screenID;
        animationStopRequested = true;
        currentState = AnimationState::STOPPING;
    }

    /**
     * @brief Update animation (call from loop)
     */
    AnimationState update()
    {
        if (currentState != AnimationState::RUNNING)
        {
            return currentState;
        }

        uint32_t currentTime = millis();

        // Check if stop was requested
        if (animationStopRequested)
        {
            currentState = AnimationState::COMPLETED;
            return currentState;
        }

        // Check if it's time to advance to next frame
        if (currentTime - lastFrameTime >= FRAME_DURATION_MS)
        {
            lastFrameTime = currentTime;
            currentFrameIndex++;

            // Check if animation is complete
            if (currentFrameIndex >= FRAME_COUNT)
            {
                // Loop back to start or complete based on total time
                uint32_t totalElapsed = currentTime - animationStartTime;

                if (totalElapsed >= ANIMATION_MIN_DURATION_MS)
                {
                    // Minimum duration reached, complete animation
                    currentState = AnimationState::COMPLETED;
                }
                else
                {
                    // Loop frames until minimum duration reached
                    currentFrameIndex = 0;
                }
            }

            // Display the new frame if still running
            if (currentState == AnimationState::RUNNING)
            {
                displayCurrentFrame();
            }
        }

        return currentState;
    }

    /**
     * @brief Get current animation state
     */
    AnimationState getState()
    {
        return currentState;
    }

    /**
     * @brief Check if animation is complete
     */
    bool isComplete()
    {
        return (currentState == AnimationState::COMPLETED);
    }

    /**
     * @brief Get the target screen ID for ESF transition
     */
    uint8_t getTargetScreenID()
    {
        return targetScreenID;
    }

    /**
     * @brief Get frame count
     */
    uint8_t getFrameCount()
    {
        return FRAME_COUNT;
    }

    /**
     * @brief Get current frame number
     */
    uint8_t getCurrentFrame()
    {
        return currentFrameIndex;
    }

    /**
     * @brief Get animation progress percentage
     */
    uint8_t getProgress()
    {
        if (currentState == AnimationState::IDLE)
        {
            return 0;
        }

        if (currentState == AnimationState::COMPLETED)
        {
            return 100;
        }

        uint32_t currentTime = millis();
        uint32_t elapsed = currentTime - animationStartTime;

        if (elapsed >= ANIMATION_MIN_DURATION_MS)
        {
            return 100;
        }

        return (elapsed * 100) / ANIMATION_MIN_DURATION_MS;
    }

} // namespace MAIN_AnimationGfxLib