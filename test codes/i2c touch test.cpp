/**
 * @file i2c_pin_scanner.cpp
 * @brief Scans multiple I2C pin combinations to find touch controller
 * @version 1.0.0
 * @date 2026-02-07
 *
 * Purpose: Determine which I2C pins connect to the FT6X36 touch controller
 * Tests both pin combinations documented in different sources
 */

#include <Arduino.h>
#include <Wire.h>

// Pin combinations to test
struct I2CPins
{
    uint8_t sda;
    uint8_t scl;
    const char *source;
};

I2CPins pinCombinations[] = {
    {38, 39, "EARS_ws35tlcdPins.h"},
    {8, 7, "Waveshare Arduino Example"},
    {4, 5, "Common ESP32 Default"},
    {21, 22, "ESP32 Standard I2C"}};

// Common I2C addresses for touch controllers
uint8_t touchAddresses[] = {
    0x38, // FT6X36, FT5x06
    0x5D, // GT911 (primary)
    0x14  // GT911 (secondary)
};

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n╔════════════════════════════════════════════════════╗");
    Serial.println("║   EARS I2C Touch Controller Pin Scanner v1.0.0   ║");
    Serial.println("╚════════════════════════════════════════════════════╝\n");

    Serial.println("Testing I2C pin combinations...\n");

    bool foundDevice = false;

    for (uint8_t p = 0; p < sizeof(pinCombinations) / sizeof(I2CPins); p++)
    {
        I2CPins pins = pinCombinations[p];

        Serial.printf("─── Testing: SDA=%d, SCL=%d (%s) ───\n",
                      pins.sda, pins.scl, pins.source);

        // Initialize I2C with these pins
        Wire.begin(pins.sda, pins.scl);
        delay(100);

        // Scan for devices
        for (uint8_t a = 0; a < sizeof(touchAddresses); a++)
        {
            uint8_t addr = touchAddresses[a];

            Wire.beginTransmission(addr);
            uint8_t error = Wire.endTransmission();

            if (error == 0)
            {
                Serial.printf("  ✓ FOUND DEVICE at address 0x%02X!\n", addr);

                // Try to read chip ID at register 0xA3 (FT6X36 chip ID register)
                Wire.beginTransmission(addr);
                Wire.write(0xA3);
                Wire.endTransmission(false);

                if (Wire.requestFrom(addr, (uint8_t)1) == 1)
                {
                    uint8_t chipId = Wire.read();
                    Serial.printf("    Chip ID at reg 0xA3: 0x%02X", chipId);

                    // Identify chip
                    switch (chipId)
                    {
                    case 0x06:
                        Serial.println(" (FT6206)");
                        break;
                    case 0x36:
                        Serial.println(" (FT6236)");
                        break;
                    case 0x64:
                        Serial.println(" (FT6236U or FT3267)");
                        break;
                    case 0x55:
                        Serial.println(" (FT5206)");
                        break;
                    case 0x0A:
                        Serial.println(" (FT5316)");
                        break;
                    default:
                        Serial.printf(" (Unknown FT chip)\n");
                        break;
                    }

                    // Try to read vendor ID at register 0xA8
                    Wire.beginTransmission(addr);
                    Wire.write(0xA8);
                    Wire.endTransmission(false);

                    if (Wire.requestFrom(addr, (uint8_t)1) == 1)
                    {
                        uint8_t vendorId = Wire.read();
                        Serial.printf("    Vendor ID at reg 0xA8: 0x%02X", vendorId);
                        if (vendorId == 0x11)
                        {
                            Serial.println(" (FocalTech - CORRECT!)");
                        }
                        else
                        {
                            Serial.println(" (Unexpected)");
                        }
                    }

                    // Try to read firmware version at register 0xA6
                    Wire.beginTransmission(addr);
                    Wire.write(0xA6);
                    Wire.endTransmission(false);

                    if (Wire.requestFrom(addr, (uint8_t)1) == 1)
                    {
                        uint8_t fwVer = Wire.read();
                        Serial.printf("    Firmware Version: 0x%02X\n", fwVer);
                    }
                }

                foundDevice = true;
            }
            else if (error == 4)
            {
                // Other error - might be wrong address
            }
        }

        Wire.end();
        delay(100);
        Serial.println();
    }

    Serial.println("\n╔════════════════════════════════════════════════════╗");
    if (foundDevice)
    {
        Serial.println("║  SCAN COMPLETE - Touch controller detected!      ║");
    }
    else
    {
        Serial.println("║  SCAN COMPLETE - No touch controller found        ║");
    }
    Serial.println("╚════════════════════════════════════════════════════╝\n");

    if (!foundDevice)
    {
        Serial.println("Recommendations:");
        Serial.println("1. Check touch controller power supply");
        Serial.println("2. Verify I2C pull-up resistors are present");
        Serial.println("3. Check for hardware issues or incorrect wiring");
        Serial.println("4. Try other pin combinations if available\n");
    }
}

void loop()
{
    // Re-scan every 10 seconds
    delay(10000);

    Serial.println("\n[Re-scanning in 10 seconds...]");
}