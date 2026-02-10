/**
 * @file EARS_screenSaverLib.h
 * @author JTB & Claude Sonnet 4.2
 * @brief Screensaver library implementation header file
 * @version 2.0.0
 * @date 20260210
 * 
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

 #pragma once
#ifndef __EARS_SCREENSAVER_LIB_H__
#define __EARS_SCREENSAVER_LIB_H__

#include <Arduino.h>
#include "EARS_versionDef.h"
#include <lvgl.h>


/******************************************************************************
 * Library Version Information
 *****************************************************************************/
namespace EARS_ScreenSaver
{
    constexpr const char* LIB_NAME = "EARS_screenSaver";
    constexpr const char* VERSION_MAJOR = "2";
    constexpr const char* VERSION_MINOR = "0";
    constexpr const char* VERSION_PATCH = "0";
    constexpr const char* VERSION_DATE = "2026-02-10";
}



/**
 * @brief Screensaver modes
 * @enum ScreensaverMode
 * @description Different modes for the screensaver
 * Modes:
 * - SS_MODE_BLACK: Blank screen
 * - SS_MODE_EARS_TEXT: "EARS" text animation
 * - SS_MODE_BUILTIN_IMAGE: Built-in image display
 * - SS_MODE_USER_IMAGE: User-defined image display
 */
enum ScreensaverMode {
    SS_MODE_BLACK = 0,
    SS_MODE_EARS_TEXT = 1,
    SS_MODE_BUILTIN_IMAGE = 2,
    SS_MODE_USER_IMAGE = 3
};

/**
 * @brief Screensaver settings structure
 * @struct ScreensaverSettings
 * @description Holds configuration for the screensaver
 * Members:
 * - enabled: Whether the screensaver is enabled
 * - timeout_seconds: Inactivity timeout in seconds (0-120)
 * - mode: Screensaver mode (ScreensaverMode)
 * - animation_speed: Animation speed (1-10 scale)
 * - bounce_mode: Bounce mode (true=bounce, false=wrap)
 * - backlight_restore: Backlight level to restore to
 */
struct ScreensaverSettings {
    bool enabled;
    uint8_t timeout_seconds;        // 0-120
    ScreensaverMode mode;
    uint8_t animation_speed;        // 1-10 scale
    bool bounce_mode;               // true=bounce, false=wrap
    uint8_t backlight_restore;      // Value to restore backlight to
};

/**
 * @brief Screensaver Library Class
 * 
 */
class EARS_screenSaver {
public:
    EARS_screenSaver();
    

    // Version information getters
    static const char* getLibraryName();
    static uint32_t getVersionEncoded();
    static const char* getVersionDate();
    static void getVersionString(char* buffer);

    /**
     * @brief Initialization
     * @param display
     * @return void 
     */
    void begin(lv_display_t* display);
    
    // Settings management
    void setEnabled(bool enabled);
    void toggleEnabled();
    void setTimeout(uint8_t seconds);
    void setMode(ScreensaverMode mode);
    void setAnimationSpeed(uint8_t speed);
    void setBounceMode(bool bounce);
    
    // Control functions
    void reset();                   // Reset inactivity timer
    void activate();                // Manually activate screensaver
    void deactivate();              // Wake from screensaver
    void update();                  // Call regularly in loop
    
    // State queries
    bool isActive();
    ScreensaverSettings getSettings();
    
private:
    lv_display_t* _display;
    ScreensaverSettings _settings;
    uint32_t _last_activity_ms;
    bool _is_active;
    lv_obj_t* _screensaver_screen;
    
    // Internal functions
    void createScreensaverScreen();
    void destroyScreensaverScreen();
    void saveBacklight();
    void restoreBacklight();
    void updateAnimation();
};

/**
 * @brief Get reference to global screensaver instance
 * 
 * This function provides access to the singleton screensaver instance.
 * The instance is created on first call and persists for the lifetime
 * of the application.
 * 
 * @return EARS_screenSaver& Reference to the global screensaver instance
 * 
 * @example
 * // Initialize screensaver
 * using_screensaver().begin(display);
 * 
 * // Set timeout
 * using_screensaver().setTimeout(30);
 * 
 * // Check in loop
 * using_screensaver().update();
 */
EARS_screenSaver& using_screensaver();

#endif // __EARS_SCREENSAVER_LIB_H__

/************************************************************************
 * End of EARS_screenSaverLib.h
 ***********************************************************************/
