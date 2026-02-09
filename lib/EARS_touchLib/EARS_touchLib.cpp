/**
 * @file EARS_touchLib.cpp
 * @author JTB & Claude Sonnet 4.5
 * @brief Touch controller library implementation for FT6236U/FT3267
 * @version 1.0.0
 * @date 20260207
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#include "EARS_touchLib.h"

// Singleton instance for LVGL callback
EARS_touch *EARS_touch::_instance = nullptr;

EARS_touch::EARS_touch() : _wire(nullptr),
                           _state(TOUCH_NOT_INITIALIZED),
                           _address(FT6X36_SLAVE_ADDRESS),
                           _chipID(0),
                           _sda(0),
                           _scl(0)
{
    _instance = this;
}

EARS_touch::~EARS_touch()
{
    if (_wire)
    {
        _wire->end();
    }
    if (_instance == this)
    {
        _instance = nullptr;
    }
}

bool EARS_touch::begin(TwoWire &wire, uint8_t sda, uint8_t scl, uint8_t address)
{
    Serial.println("[TOUCH] Initializing touch controller...");

    _wire = &wire;
    _address = address;
    _sda = sda;
    _scl = scl;

    // Step 1: Initialize I2C bus
    _wire->begin(sda, scl);
    _wire->setClock(400000); // 400kHz fast mode
    Serial.printf("[TOUCH] I2C initialized (SDA=%d, SCL=%d, 400kHz)\n", sda, scl);

    delay(50); // Allow touch controller to stabilize

    // Step 2: Verify vendor ID
    uint8_t vendorID = readRegister(FT6X36_REG_VENDOR1_ID);
    if (vendorID != FT6X36_VENDOR_ID)
    {
        Serial.printf("[TOUCH] ERROR: Vendor ID mismatch! Got 0x%02X, expected 0x%02X\n",
                      vendorID, FT6X36_VENDOR_ID);
        _state = TOUCH_INIT_FAILED;
        return false;
    }
    Serial.printf("[TOUCH] Vendor ID verified: 0x%02X (FocalTech)\n", vendorID);

    // Step 3: Read and verify chip ID
    _chipID = readRegister(FT6X36_REG_CHIP_ID);
    if (_chipID != FT6206_CHIP_ID &&
        _chipID != FT6236_CHIP_ID &&
        _chipID != FT6236U_CHIP_ID)
    {
        Serial.printf("[TOUCH] ERROR: Chip ID mismatch! Got 0x%02X\n", _chipID);
        _state = TOUCH_INIT_FAILED;
        return false;
    }
    Serial.printf("[TOUCH] Chip ID verified: 0x%02X (%s)\n", _chipID, getModelName().c_str());

    // Step 4: Display firmware information
    uint8_t fwVersion = getFirmwareVersion();
    Serial.printf("[TOUCH] Firmware Version: 0x%02X\n", fwVersion);

    // Step 5: Configure touch controller
    setThreshold(60); // Default threshold (lower = more sensitive)
    Serial.printf("[TOUCH] Threshold set: %d\n", getThreshold());

    uint8_t activeRate = readRegister(FT6X36_REG_PERIOD_ACTIVE);
    uint8_t monitorRate = readRegister(FT6X36_REG_PERIOD_MONITOR);
    Serial.printf("[TOUCH] Active Rate: %d Hz\n", activeRate);
    Serial.printf("[TOUCH] Monitor Period: %d\n", monitorRate);

    _state = TOUCH_READY;
    Serial.println("[TOUCH] ✅ Touch controller ready!");

    return true;
}

TouchInitResult EARS_touch::performFullInitialization(uint8_t sda, uint8_t scl)
{
    TouchInitResult result;

    // Step 1: Initialize touch controller via begin()
    if (!begin(Wire, sda, scl, FT6X36_SLAVE_ADDRESS))
    {
        result.state = TOUCH_INIT_FAILED;
        return result;
    }

    // Step 2: Populate result structure
    result.state = _state;
    result.chipID = _chipID;
    result.vendorID = getVendorID();
    result.firmwareVersion = getFirmwareVersion();
    result.modelName = getModelName();
    result.i2cAddress = _address;
    result.sdaPin = sda;
    result.sclPin = scl;
    result.maxTouchPoints = 2; // FT6236U supports 2 touch points

    // Step 3: Register LVGL input device
    lv_indev_t *touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, EARS_touch::lvgl_touch_read);
    result.lvglRegistered = true;

    Serial.println("[TOUCH] LVGL input device registered");

    return result;
}

bool EARS_touch::isAvailable() const
{
    return (_state == TOUCH_READY);
}

TouchState EARS_touch::getState() const
{
    return _state;
}

uint8_t EARS_touch::getChipID() const
{
    return _chipID;
}

uint8_t EARS_touch::getVendorID() const
{
    return readRegister(FT6X36_REG_VENDOR1_ID);
}

uint8_t EARS_touch::getFirmwareVersion() const
{
    return readRegister(FT6X36_REG_FIRM_VERS);
}

String EARS_touch::getModelName() const
{
    switch (_chipID)
    {
    case FT6206_CHIP_ID:
        return "FT6206";
    case FT6236_CHIP_ID:
        return "FT6236";
    case FT6236U_CHIP_ID:
        return "FT6236U/FT3267";
    default:
        return "Unknown";
    }
}

uint8_t EARS_touch::getPoint(int16_t *x, int16_t *y)
{
    uint8_t buffer[16];

    if (!x || !y)
        return 0;

    if (!isAvailable())
        return 0;

    // Read touch data registers (0x00 to 0x0F)
    if (readRegisters(FT6X36_REG_MODE, buffer, 16) != 16)
        return 0;

    // REG 0x02: Number of touch points
    uint8_t numPoints = buffer[2] & 0x0F;

    // Validate number of points
    if (numPoints == 0 || numPoints == 0x0F)
        return 0;

    // REG 0x03-0x06: First touch point
    uint16_t posX = ((buffer[3] & 0x0F) << 8) | buffer[4];
    uint16_t posY = ((buffer[5] & 0x0F) << 8) | buffer[6];

    x[0] = posX;
    y[0] = posY;

    // If second touch point exists
    if (numPoints == 2)
    {
        // REG 0x09-0x0C: Second touch point
        posX = ((buffer[9] & 0x0F) << 8) | buffer[10];
        posY = ((buffer[11] & 0x0F) << 8) | buffer[12];

        x[1] = posX;
        y[1] = posY;
    }

    return numPoints;
}

bool EARS_touch::isPressed() const
{
    if (!isAvailable())
        return false;

    uint8_t status = readRegister(FT6X36_REG_STATUS);
    uint8_t numPoints = status & 0x0F;
    return (numPoints > 0 && numPoints < 0x0F);
}

TouchGesture EARS_touch::getGesture() const
{
    if (!isAvailable())
        return GESTURE_NONE;

    uint8_t gestureID = readRegister(FT6X36_REG_GEST);

    switch (gestureID)
    {
    case 0x10:
        return GESTURE_MOVE_UP;
    case 0x14:
        return GESTURE_MOVE_RIGHT;
    case 0x18:
        return GESTURE_MOVE_DOWN;
    case 0x1C:
        return GESTURE_MOVE_LEFT;
    case 0x48:
        return GESTURE_ZOOM_IN;
    case 0x49:
        return GESTURE_ZOOM_OUT;
    default:
        return GESTURE_NONE;
    }
}

void EARS_touch::setThreshold(uint8_t threshold)
{
    writeRegister(FT6X36_REG_THRESHOLD, threshold);
}

uint8_t EARS_touch::getThreshold() const
{
    return readRegister(FT6X36_REG_THRESHOLD);
}

void EARS_touch::setPowerMode(TouchPowerMode mode)
{
    writeRegister(FT6X36_REG_POWER_MODE, (uint8_t)mode);
}

void EARS_touch::sleep()
{
    setPowerMode(POWER_DEEP_SLEEP);
}

void EARS_touch::wakeup()
{
    setPowerMode(POWER_ACTIVE);
    delay(10);
}

void EARS_touch::lvgl_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    EARS_touch *touch = getInstance();

    if (!touch || !touch->isAvailable())
    {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    int16_t x[2], y[2];
    uint8_t touchCount = touch->getPoint(x, y);

    if (touchCount > 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;

        // Standard rotation 1 (90° clockwise) transformation
        // Touch panel reports in portrait: X=[0-319], Y=[0-479]
        // Display is landscape: width=480, height=320
        // Transform: new_x = old_y, new_y = 319 - old_x
        data->point.x = y[0];
        data->point.y = 319 - x[0];

#if EARS_DEBUG == 1
        Serial.printf("[TOUCH DEBUG] Touch X=%d Y=%d → Display X=%d Y=%d\n",
                      x[0], y[0], data->point.x, data->point.y);
#endif
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

uint8_t EARS_touch::readRegister(uint8_t reg) const
{
    if (!_wire)
        return 0xFF;

    _wire->beginTransmission(_address);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0)
        return 0xFF;

    if (_wire->requestFrom(_address, (uint8_t)1) != 1)
        return 0xFF;

    return _wire->read();
}

uint8_t EARS_touch::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) const
{
    if (!_wire || !buffer)
        return 0;

    _wire->beginTransmission(_address);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0)
        return 0;

    uint8_t bytesRead = _wire->requestFrom(_address, length);
    for (uint8_t i = 0; i < bytesRead; i++)
    {
        buffer[i] = _wire->read();
    }

    return bytesRead;
}

void EARS_touch::writeRegister(uint8_t reg, uint8_t value)
{
    if (!_wire)
        return;

    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write(value);
    _wire->endTransmission();
}

/**
 * @brief Get reference to global Touch instance (Singleton pattern)
 */
EARS_touch &using_touch()
{
    static EARS_touch touch;
    return touch;
}