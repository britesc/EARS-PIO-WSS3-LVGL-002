/**
 * @file EARS_touchLib.h
 * @author JTB & Claude Sonnet 4.5
 * @brief Touch controller library for FT6236U/FT3267 chip
 * @version 1.0.0
 * @date 20260207
 *
 * @details
 * Touch controller library for Waveshare ESP32-S3 Touch LCD 3.5"
 * Based on Waveshare TouchDrvFT6X36 driver
 * Compatible with LVGL 9.3.0
 *
 * HARDWARE CONFIGURATION (verified via I2C scanner):
 * - Chip: FT6236U or FT3267 (Chip ID: 0x64)
 * - I2C Address: 0x38
 * - I2C Pins: SDA=8, SCL=7
 * - Vendor: FocalTech (ID: 0x11)
 * - Maximum Touch Points: 2
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __EARS_TOUCH_LIB_H__
#define __EARS_TOUCH_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

/******************************************************************************
 * FT6X36 Register Addresses
 *****************************************************************************/
#define FT6X36_REG_MODE 0x00
#define FT6X36_REG_GEST 0x01
#define FT6X36_REG_STATUS 0x02
#define FT6X36_REG_TOUCH1_XH 0x03
#define FT6X36_REG_TOUCH1_XL 0x04
#define FT6X36_REG_TOUCH1_YH 0x05
#define FT6X36_REG_TOUCH1_YL 0x06
#define FT6X36_REG_TOUCH2_XH 0x09
#define FT6X36_REG_TOUCH2_XL 0x0A
#define FT6X36_REG_TOUCH2_YH 0x0B
#define FT6X36_REG_TOUCH2_YL 0x0C
#define FT6X36_REG_THRESHOLD 0x80
#define FT6X36_REG_PERIOD_ACTIVE 0x88
#define FT6X36_REG_PERIOD_MONITOR 0x89
#define FT6X36_REG_POWER_MODE 0xA5
#define FT6X36_REG_FIRM_VERS 0xA6
#define FT6X36_REG_CHIP_ID 0xA3
#define FT6X36_REG_VENDOR1_ID 0xA8
#define FT6X36_REG_LIB_VERSION_H 0xA1
#define FT6X36_REG_LIB_VERSION_L 0xA2

/******************************************************************************
 * FT6X36 Constants
 *****************************************************************************/
#define FT6X36_SLAVE_ADDRESS 0x38
#define FT6X36_VENDOR_ID 0x11 // FocalTech

// Chip IDs
#define FT6206_CHIP_ID 0x06
#define FT6236_CHIP_ID 0x36
#define FT6236U_CHIP_ID 0x64 // Also FT3267
#define FT3267_CHIP_ID 0x64

/******************************************************************************
 * Touch State Enum
 *****************************************************************************/
enum TouchState
{
    TOUCH_NOT_INITIALIZED = 0,
    TOUCH_INIT_FAILED,
    TOUCH_READY
};

/******************************************************************************
 * Touch Gesture Enum
 *****************************************************************************/
/**
 * @enum TouchGesture
 * @brief Enumeration for touch gesture types
 */
enum TouchGesture
{
    GESTURE_NONE = 0,
    GESTURE_MOVE_UP,
    GESTURE_MOVE_LEFT,
    GESTURE_MOVE_DOWN,
    GESTURE_MOVE_RIGHT,
    GESTURE_ZOOM_IN,
    GESTURE_ZOOM_OUT
};

/******************************************************************************
 * Touch Power Mode Enum
 *****************************************************************************/
/**
 * @enum TouchPowerMode
 * @brief Enumeration for touch controller power modes
 */
enum TouchPowerMode
{
    POWER_ACTIVE = 0,    // ~4mA
    POWER_MONITOR = 1,   // ~3mA
    POWER_DEEP_SLEEP = 3 // ~100uA (reset pin must be pulled down to wake)
};

/******************************************************************************
 * Touch Initialization Result Structure
 *****************************************************************************/
/**
 * @struct TouchInitResult
 * @brief Structure to hold touch initialization results
 *
 * @details
 * Used by performFullInitialization() to return detailed initialization
 * status to the caller. Contains touch state, chip information, and
 * I2C configuration.
 */
struct TouchInitResult
{
    TouchState state;        // Current touch state
    uint8_t chipID;          // Chip ID (0x64 for FT6236U/FT3267)
    uint8_t vendorID;        // Vendor ID (0x11 for FocalTech)
    uint8_t firmwareVersion; // Firmware version
    String modelName;        // Chip model name
    uint8_t i2cAddress;      // I2C address used
    uint8_t sdaPin;          // SDA pin number
    uint8_t sclPin;          // SCL pin number
    uint8_t maxTouchPoints;  // Maximum simultaneous touch points
    bool lvglRegistered;     // LVGL input device registered

    TouchInitResult() : state(TOUCH_NOT_INITIALIZED),
                        chipID(0),
                        vendorID(0),
                        firmwareVersion(0),
                        modelName(""),
                        i2cAddress(0),
                        sdaPin(0),
                        sclPin(0),
                        maxTouchPoints(0),
                        lvglRegistered(false) {}
};

/******************************************************************************
 * EARS_touch Class
 *****************************************************************************/
class EARS_touch
{
public:
    EARS_touch();
    ~EARS_touch();

    /**
     * @brief Initialize touch controller
     * @param wire I2C bus instance
     * @param sda I2C SDA pin number
     * @param scl I2C SCL pin number
     * @param address I2C device address
     * @return true if initialization successful
     * @return false if initialization failed
     */
    bool begin(TwoWire &wire = Wire, uint8_t sda = 8, uint8_t scl = 7, uint8_t address = FT6X36_SLAVE_ADDRESS);

    bool isAvailable() const;
    TouchState getState() const;
    uint8_t getChipID() const;
    uint8_t getVendorID() const;
    uint8_t getFirmwareVersion() const;
    String getModelName() const;

    /**
     * @brief Get number of touch points detected
     * @param x Array to store X coordinates (size >= 2)
     * @param y Array to store Y coordinates (size >= 2)
     * @return Number of touch points (0, 1, or 2)
     */
    uint8_t getPoint(int16_t *x, int16_t *y);

    /**
     * @brief Check if screen is currently being touched
     * @return true if touch detected
     */
    bool isPressed() const;

    /**
     * @brief Get detected gesture
     * @return Gesture type
     */
    TouchGesture getGesture() const;

    void setThreshold(uint8_t threshold);
    uint8_t getThreshold() const;
    void setPowerMode(TouchPowerMode mode);
    void sleep();
    void wakeup();

    /**
     * @brief Perform complete touch controller initialization (matches EARS pattern)
     * @return TouchInitResult Detailed result of initialization
     *
     * @details
     * This function orchestrates the complete touch controller initialization:
     * 1. Initialize I2C interface via begin()
     * 2. Verify chip ID and vendor ID
     * 3. Configure touch sensitivity threshold
     * 4. Register LVGL input device
     * 5. Return detailed status for caller interpretation
     *
     * This follows the same pattern as performFullInitialization() in other
     * EARS libraries (sdCard, nvsEeprom, etc.)
     *
     * @note The caller should interpret the result to set LED patterns
     *       and update application state accordingly.
     */
    TouchInitResult performFullInitialization(uint8_t sda = 8, uint8_t scl = 7);

    /**
     * @brief LVGL touch read callback (LVGL 9.3 compatible)
     * @param indev LVGL input device instance
     * @param data LVGL input data structure
     */
    static void lvgl_touch_read(lv_indev_t *indev, lv_indev_data_t *data);

    /**
     * @brief Get singleton instance for LVGL callback
     * @return Pointer to EARS_touch instance
     */
    static EARS_touch *getInstance() { return _instance; }

private:
    TwoWire *_wire;    // I2C bus pointer
    TouchState _state; // Current touch state
    uint8_t _address;  // I2C device address
    uint8_t _chipID;   // Detected chip ID
    uint8_t _sda;      // SDA pin number
    uint8_t _scl;      // SCL pin number

    static EARS_touch *_instance; // Singleton instance for LVGL callback

    uint8_t readRegister(uint8_t reg) const;
    uint8_t readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) const;
    void writeRegister(uint8_t reg, uint8_t value);
};

/**
 * @brief Get reference to global Touch instance (Singleton pattern)
 */
EARS_touch &using_touch();

#endif // __EARS_TOUCH_LIB_H__