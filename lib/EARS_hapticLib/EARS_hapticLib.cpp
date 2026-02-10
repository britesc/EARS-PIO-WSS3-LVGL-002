/**
 * @file EARS_hapticLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Manages haptic feedback motor with PWM intensity and duration control
 * @version 2.0.0
 * @date 20260210
 * 
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */
#include "EARS_hapticLib.h"

// Constructor
EARS_haptic::EARS_haptic()
    : _pin(0),
      _pwmChannel(0),
      _pwmResolution(8),
      _maxDutyCycle(255),
      _currentIntensity(0),
      _currentDuration(0),
      _initialized(false) {
}

// Initialise the haptic manager
bool EARS_haptic::begin(uint8_t pin, uint8_t pwmChannel, 
                        uint32_t pwmFrequency, uint8_t pwmResolution) {
    _pin = pin;
    _pwmChannel = pwmChannel;
    _pwmResolution = pwmResolution;
    _maxDutyCycle = (1 << pwmResolution) - 1;  // e.g., 255 for 8-bit

    // Configure PWM
    ledcSetup(_pwmChannel, pwmFrequency, _pwmResolution);
    ledcAttachPin(_pin, _pwmChannel);

    // Open NVS preferences
    if (!_preferences.begin(NVS_NAMESPACE, false)) {
        Serial.println("[HapticManager] ERROR: Failed to open NVS");
        return false;
    }

    // Determine initial settings
    if (isInitialConfig()) {
        // First time setup - use defaults
        _currentIntensity = INITIAL_CONFIG_INTENSITY;
        _currentDuration = INITIAL_CONFIG_DURATION;
        Serial.println("[HapticManager] Initial config detected");
        Serial.printf("[HapticManager] Using defaults: %d%% intensity, %dms duration\n", 
                     _currentIntensity, _currentDuration);
    } else {
        // Load saved settings
        _currentIntensity = _preferences.getUChar(NVS_INTENSITY_KEY, DEFAULT_INTENSITY);
        _currentDuration = _preferences.getUShort(NVS_DURATION_KEY, DEFAULT_DURATION);
        Serial.printf("[HapticManager] Loaded: %d%% intensity, %dms duration\n", 
                     _currentIntensity, _currentDuration);
    }

    // Turn motor off initially
    ledcWrite(_pwmChannel, 0);
    
    _initialized = true;
    Serial.printf("[HapticManager] Initialised on pin %d, PWM channel %d, freq %d Hz\n", 
                  _pin, _pwmChannel, pwmFrequency);
    
    return true;
}

// Set intensity
void EARS_haptic::setIntensity(uint8_t level) {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return;
    }

    // Constrain to valid range
    level = constrain(level, 0, 100);
    
    _currentIntensity = level;
    
    Serial.printf("[HapticManager] Intensity set to %d%%\n", level);
}

// Get current intensity
uint8_t EARS_haptic::getIntensity() const {
    return _currentIntensity;
}

// Set duration
void EARS_haptic::setDuration(uint16_t durationMs) {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return;
    }

    // Constrain to valid range
    durationMs = constrain(durationMs, MIN_DURATION, MAX_DURATION);
    
    _currentDuration = durationMs;
    
    Serial.printf("[HapticManager] Duration set to %dms\n", durationMs);
}

// Get current duration
uint16_t EARS_haptic::getDuration() const {
    return _currentDuration;
}

// Save intensity to NVS
bool EARS_haptic::saveIntensity() {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return false;
    }

    size_t written = _preferences.putUChar(NVS_INTENSITY_KEY, _currentIntensity);
    
    if (written) {
        Serial.printf("[HapticManager] Saved intensity: %d%%\n", _currentIntensity);
        return true;
    } else {
        Serial.println("[HapticManager] ERROR: Failed to save intensity");
        return false;
    }
}

// Save duration to NVS
bool EARS_haptic::saveDuration() {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return false;
    }

    size_t written = _preferences.putUShort(NVS_DURATION_KEY, _currentDuration);
    
    if (written) {
        Serial.printf("[HapticManager] Saved duration: %dms\n", _currentDuration);
        return true;
    } else {
        Serial.println("[HapticManager] ERROR: Failed to save duration");
        return false;
    }
}

// Save both settings to NVS
bool EARS_haptic::saveSettings() {
    bool intensitySaved = saveIntensity();
    bool durationSaved = saveDuration();
    
    if (intensitySaved && durationSaved) {
        Serial.println("[HapticManager] All settings saved successfully");
        return true;
    } else {
        Serial.println("[HapticManager] WARNING: Not all settings saved");
        return false;
    }
}

// Load intensity from NVS
bool EARS_haptic::loadIntensity() {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return false;
    }

    if (_preferences.isKey(NVS_INTENSITY_KEY)) {
        uint8_t savedLevel = _preferences.getUChar(NVS_INTENSITY_KEY, DEFAULT_INTENSITY);
        setIntensity(savedLevel);
        Serial.printf("[HapticManager] Loaded intensity: %d%%\n", savedLevel);
        return true;
    } else {
        Serial.println("[HapticManager] No saved intensity found");
        return false;
    }
}

// Load duration from NVS
bool EARS_haptic::loadDuration() {
    if (!_initialized) {
        Serial.println("[HapticManager] ERROR: Not initialised");
        return false;
    }

    if (_preferences.isKey(NVS_DURATION_KEY)) {
        uint16_t savedDuration = _preferences.getUShort(NVS_DURATION_KEY, DEFAULT_DURATION);
        setDuration(savedDuration);
        Serial.printf("[HapticManager] Loaded duration: %dms\n", savedDuration);
        return true;
    } else {
        Serial.println("[HapticManager] No saved duration found");
        return false;
    }
}

// Load both settings from NVS
bool EARS_haptic::loadSettings() {
    bool intensityLoaded = loadIntensity();
    bool durationLoaded = loadDuration();
    
    if (intensityLoaded && durationLoaded) {
        Serial.println("[HapticManager] All settings loaded successfully");
        return true;
    } else {
        Serial.println("[HapticManager] WARNING: Not all settings loaded");
        return false;
    }
}

// Turn haptic motor off
void EARS_haptic::off() {
    if (!_initialized) {
        return;
    }
    
    ledcWrite(_pwmChannel, 0);
}

// Vibrate using stored settings
void EARS_haptic::vibrate() {
    vibrateInternal(_currentIntensity, _currentDuration);
}

// Vibrate with custom intensity and duration
void EARS_haptic::vibrate(uint8_t intensity, uint16_t durationMs) {
    vibrateInternal(intensity, durationMs);
}

// Pulse using stored settings
void EARS_haptic::pulse() {
    vibrateInternal(_currentIntensity, _currentDuration);
}

// Pulse with custom duration (stored intensity)
void EARS_haptic::pulse(uint16_t durationMs) {
    vibrateInternal(_currentIntensity, durationMs);
}

// Pulse with custom intensity and duration
void EARS_haptic::pulse(uint8_t intensity, uint16_t durationMs) {
    vibrateInternal(intensity, durationMs);
}

// Double pulse using stored settings
void EARS_haptic::doublePulse(uint16_t gapMs) {
    if (!_initialized) {
        return;
    }
    
    vibrateInternal(_currentIntensity, _currentDuration);
    delay(gapMs);
    vibrateInternal(_currentIntensity, _currentDuration);
}

// Triple pulse using stored settings
void EARS_haptic::triplePulse(uint16_t gapMs) {
    if (!_initialized) {
        return;
    }
    
    vibrateInternal(_currentIntensity, _currentDuration);
    delay(gapMs);
    vibrateInternal(_currentIntensity, _currentDuration);
    delay(gapMs);
    vibrateInternal(_currentIntensity, _currentDuration);
}

// Button press feedback (20ms at stored intensity)
void EARS_haptic::buttonPress() {
    vibrateInternal(_currentIntensity, 20);
}

// Error feedback pattern (100ms at stored intensity)
void EARS_haptic::errorPattern() {
    vibrateInternal(_currentIntensity, 100);
}

// Success feedback pattern (double pulse at stored settings)
void EARS_haptic::successPattern() {
    doublePulse(30);
}

// Check if this is initial device configuration
bool EARS_haptic::isInitialConfig() {
    // Check if the init flag exists in NVS
    return !_preferences.isKey(NVS_INIT_FLAG_KEY);
}

// Mark initial configuration as complete
void EARS_haptic::completeInitialConfig() {
    _preferences.putBool(NVS_INIT_FLAG_KEY, true);
    
    // Save current settings
    saveSettings();
    
    Serial.println("[HapticManager] Initial config marked complete");
}

// Convert percentage to PWM duty cycle
uint32_t EARS_haptic::percentageToDutyCycle(uint8_t percentage) const {
    // Simple linear mapping: 0% = 0, 100% = maxDutyCycle
    return (_maxDutyCycle * percentage) / 100;
}

// Internal vibration function
void EARS_haptic::vibrateInternal(uint8_t intensity, uint16_t durationMs) {
    if (!_initialized) {
        return;
    }
    
    // Constrain parameters
    intensity = constrain(intensity, 0, 100);
    durationMs = constrain(durationMs, MIN_DURATION, MAX_DURATION);
    
    // Turn on motor at specified intensity
    uint32_t dutyCycle = percentageToDutyCycle(intensity);
    ledcWrite(_pwmChannel, dutyCycle);
    
    // Wait for duration
    delay(durationMs);
    
    // Turn off motor
    ledcWrite(_pwmChannel, 0);
}

/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* EARS_haptic::getLibraryName() {
    return EARS_Haptic::LIB_NAME;
}

// Get encoded version as integer
uint32_t EARS_haptic::getVersionEncoded() {
    return VERS_ENCODE(EARS_Haptic::VERSION_MAJOR, 
                       EARS_Haptic::VERSION_MINOR, 
                       EARS_Haptic::VERSION_PATCH);
}

// Get version date
const char* EARS_haptic::getVersionDate() {
    return EARS_Haptic::VERSION_DATE;
}

// Format version as string
void EARS_haptic::getVersionString(char* buffer) {
    uint32_t encoded = getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}


EARS_haptic& using_haptic() {
    static EARS_haptic instance;
    return instance;
}

/******************************************************************************
 * End of EARS_hapticLib.cpp
 ****************************************************************************/
