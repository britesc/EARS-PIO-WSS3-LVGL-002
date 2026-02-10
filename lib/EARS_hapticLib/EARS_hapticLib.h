/**
 * @file EARS_hapticLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Manages haptic feedback motor with PWM intensity and duration control
 * @version 2.0.0
 * @date 20260210
 * 
 * Features:
 * - Analogue PWM intensity control (0-100%)
 * - Duration control (milliseconds)
 * - NVS storage for both intensity and duration preferences
 * - Vibration patterns using stored settings
 * - Default 100% intensity, 50ms duration
 * - Compatible with EEZ Studio Flow dual-slider control
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __EARS_HAPTIC_LIB_H__
#define __EARS_HAPTIC_LIB_H__

/******************************************************************************
 * Includes Information
 *****************************************************************************/
#include <Arduino.h>
#include <Preferences.h>
#include "EARS_versionDef.h"

/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace EARS_Haptic
{
    constexpr const char* LIB_NAME = "EARS_haptic";
    constexpr const char* VERSION_MAJOR = "2";
    constexpr const char* VERSION_MINOR = "0";
    constexpr const char* VERSION_PATCH = "0";
    constexpr const char* VERSION_DATE = "2026-02-10";
}


class EARS_haptic {
public:
    /**
     * @brief Construct a new Haptic Manager
     */
    EARS_haptic();

    // Version information getters
    static const char* getLibraryName();
    static uint32_t getVersionEncoded();
    static const char* getVersionDate();
    static void getVersionString(char* buffer);

    /**
     * @brief Initialise the haptic manager
     * @param pin GPIO pin for haptic motor control
     * @param pwmChannel PWM channel to use (0-15)
     * @param pwmFrequency PWM frequency in Hz (default 1000Hz for motors)
     * @param pwmResolution PWM resolution in bits (default 8)
     * @return true if initialisation successful
     */
    bool begin(uint8_t pin, uint8_t pwmChannel = 1, 
               uint32_t pwmFrequency = 1000, uint8_t pwmResolution = 8);

    /**
     * @brief Set haptic intensity level
     * @param level Intensity level (0 = OFF, 100 = FULL INTENSITY)
     */
    void setIntensity(uint8_t level);

    /**
     * @brief Get current haptic intensity level
     * @return uint8_t Current intensity (0-100)
     */
    uint8_t getIntensity() const;

    /**
     * @brief Set haptic duration in milliseconds
     * @param durationMs Duration in milliseconds (1-5000ms)
     */
    void setDuration(uint16_t durationMs);

    /**
     * @brief Get current haptic duration
     * @return uint16_t Current duration in milliseconds
     */
    uint16_t getDuration() const;

    /**
     * @brief Save current intensity to NVS
     * @return true if save successful
     */
    bool saveIntensity();

    /**
     * @brief Save current duration to NVS
     * @return true if save successful
     */
    bool saveDuration();

    /**
     * @brief Save both intensity and duration to NVS
     * @return true if both saves successful
     */
    bool saveSettings();

    /**
     * @brief Load intensity from NVS
     * @return true if load successful
     */
    bool loadIntensity();

    /**
     * @brief Load duration from NVS
     * @return true if load successful
     */
    bool loadDuration();

    /**
     * @brief Load both intensity and duration from NVS
     * @return true if both loads successful
     */
    bool loadSettings();

    /**
     * @brief Turn haptic motor off
     */
    void off();

    /**
     * @brief Vibrate using stored intensity and duration
     */
    void vibrate();

    /**
     * @brief Vibrate with custom intensity and duration
     * @param intensity Intensity level (0-100)
     * @param durationMs Duration in milliseconds
     */
    void vibrate(uint8_t intensity, uint16_t durationMs);

    /**
     * @brief Single pulse using stored settings
     */
    void pulse();

    /**
     * @brief Single pulse with custom duration (uses stored intensity)
     * @param durationMs Duration of pulse in milliseconds
     */
    void pulse(uint16_t durationMs);

    /**
     * @brief Single pulse with custom intensity and duration
     * @param intensity Intensity level (0-100)
     * @param durationMs Duration in milliseconds
     */
    void pulse(uint8_t intensity, uint16_t durationMs);

    /**
     * @brief Double pulse using stored settings
     * @param gapMs Gap between pulses in milliseconds (default 50ms)
     */
    void doublePulse(uint16_t gapMs = 50);

    /**
     * @brief Triple pulse using stored settings
     * @param gapMs Gap between pulses in milliseconds (default 30ms)
     */
    void triplePulse(uint16_t gapMs = 30);

    /**
     * @brief Button press feedback (20ms at stored intensity)
     */
    void buttonPress();

    /**
     * @brief Error feedback pattern (100ms at stored intensity)
     */
    void errorPattern();

    /**
     * @brief Success feedback pattern (double pulse at stored settings)
     */
    void successPattern();

    /**
     * @brief Check if this is initial device configuration
     * @return true if NVS has never been written
     */
    bool isInitialConfig();

    /**
     * @brief Mark initial configuration as complete
     */
    void completeInitialConfig();

private:
    uint8_t _pin;
    uint8_t _pwmChannel;
    uint8_t _pwmResolution;
    uint32_t _maxDutyCycle;
    
    uint8_t _currentIntensity;
    uint16_t _currentDuration;
    bool _initialized;

    Preferences _preferences;

    // NVS keys
    static constexpr const char* NVS_NAMESPACE = "haptic";
    static constexpr const char* NVS_INTENSITY_KEY = "intensity";
    static constexpr const char* NVS_DURATION_KEY = "duration";
    static constexpr const char* NVS_INIT_FLAG_KEY = "init_done";

    // Default values
    static constexpr uint8_t DEFAULT_INTENSITY = 100;
    static constexpr uint16_t DEFAULT_DURATION = 50;
    static constexpr uint8_t INITIAL_CONFIG_INTENSITY = 100;
    static constexpr uint16_t INITIAL_CONFIG_DURATION = 50;

    // Duration limits
    static constexpr uint16_t MIN_DURATION = 1;
    static constexpr uint16_t MAX_DURATION = 5000;

    /**
     * @brief Convert percentage (0-100) to PWM duty cycle
     * @param percentage Intensity percentage
     * @return uint32_t PWM duty cycle value
     */
    uint32_t percentageToDutyCycle(uint8_t percentage) const;

    /**
     * @brief Internal vibration function
     * @param intensity Intensity (0-100)
     * @param durationMs Duration in milliseconds
     */
    void vibrateInternal(uint8_t intensity, uint16_t durationMs);
};

// Global instance access function
EARS_haptic& using_haptic();

#endif // __EARS_HAPTIC_LIB_H__

/******************************************************************************
 * End of EARS_hapticLib.h
 ******************************************************************************/
