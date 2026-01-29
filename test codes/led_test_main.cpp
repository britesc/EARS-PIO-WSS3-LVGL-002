/**
 * @file led_test_main.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Simple LED hardware test - verifies breadboard wiring
 * @version 0.1.0
 * @date 20260128
 *
 * PURPOSE: Basic test to confirm LED hardware is wired correctly
 *
 * WIRING CHECK:
 * - GPIO40 (Pin 11) → 220Ω → Red LED (+) → (-) GND
 * - GPIO41 (Pin 13) → 220Ω → Yellow LED (+) → (-) GND
 * - GPIO42 (Pin 15) → 220Ω → Green LED (+) → (-) GND
 * - Pin 29 GND → Breadboard GND rail
 *
 * EXPECTED BEHAVIOR:
 * 1. All LEDs turn ON for 2 seconds
 * 2. All LEDs turn OFF for 1 second
 * 3. Red blinks 3 times
 * 4. Yellow blinks 3 times
 * 5. Green blinks 3 times
 * 6. Repeat cycle forever
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#include <Arduino.h>

// LED GPIO Definitions
#define LED_RED 40    // Pin 11 (left side)
#define LED_YELLOW 41 // Pin 13 (left side)
#define LED_GREEN 42  // Pin 15 (left side)

void setup()
{
    // Initialize Serial for debugging
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n");
    Serial.println("========================================");
    Serial.println("  EARS LED Hardware Test");
    Serial.println("========================================");
    Serial.println("Testing GPIO40 (Red), GPIO41 (Yellow), GPIO42 (Green)");
    Serial.println();

    // Configure GPIO pins as outputs
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_YELLOW, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);

    // Start with all LEDs off
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);

    Serial.println("[INIT] LED GPIOs configured as outputs");
    Serial.println("[INIT] Starting test sequence...");
    Serial.println();
}

void loop()
{
    // ========================================================================
    // TEST 1: All LEDs ON
    // ========================================================================
    Serial.println("[TEST] All LEDs ON for 2 seconds");
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(2000);

    // ========================================================================
    // TEST 2: All LEDs OFF
    // ========================================================================
    Serial.println("[TEST] All LEDs OFF for 1 second");
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(1000);

    // ========================================================================
    // TEST 3: Red LED Blink
    // ========================================================================
    Serial.println("[TEST] Red LED blinking 3 times");
    for (int i = 0; i < 3; i++)
    {
        Serial.printf("  Red blink %d\n", i + 1);
        digitalWrite(LED_RED, HIGH);
        delay(300);
        digitalWrite(LED_RED, LOW);
        delay(300);
    }
    delay(500);

    // ========================================================================
    // TEST 4: Yellow LED Blink
    // ========================================================================
    Serial.println("[TEST] Yellow LED blinking 3 times");
    for (int i = 0; i < 3; i++)
    {
        Serial.printf("  Yellow blink %d\n", i + 1);
        digitalWrite(LED_YELLOW, HIGH);
        delay(300);
        digitalWrite(LED_YELLOW, LOW);
        delay(300);
    }
    delay(500);

    // ========================================================================
    // TEST 5: Green LED Blink
    // ========================================================================
    Serial.println("[TEST] Green LED blinking 3 times");
    for (int i = 0; i < 3; i++)
    {
        Serial.printf("  Green blink %d\n", i + 1);
        digitalWrite(LED_GREEN, HIGH);
        delay(300);
        digitalWrite(LED_GREEN, LOW);
        delay(300);
    }
    delay(500);

    // ========================================================================
    // TEST 6: Rapid Sequence
    // ========================================================================
    Serial.println("[TEST] Rapid sequence (Red->Yellow->Green)");
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(LED_RED, HIGH);
        delay(100);
        digitalWrite(LED_RED, LOW);

        digitalWrite(LED_YELLOW, HIGH);
        delay(100);
        digitalWrite(LED_YELLOW, LOW);

        digitalWrite(LED_GREEN, HIGH);
        delay(100);
        digitalWrite(LED_GREEN, LOW);
    }

    Serial.println();
    Serial.println("[CYCLE] Test cycle complete - repeating in 2 seconds...");
    Serial.println("========================================");
    Serial.println();
    delay(2000);
}

// ============================================================================
// END OF LED HARDWARE TEST
// ============================================================================