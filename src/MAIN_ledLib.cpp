/**
 * @file MAIN_ledLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief LED indicator management library implementation
 * @details Provides hardware debugging LEDs for visual system status
 * @version 0.1.0
 * @date 20260128
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_ledLib.h"
#include "EARS_systemDef.h"

/******************************************************************************
 * Initialisation Functions
 *****************************************************************************/

/**
 * @brief Initialise all LED GPIOs
 * @details Sets all LED pins to OUTPUT mode and turns them off
 * @return void
 */
void MAIN_led_init(void)
{
    // Configure GPIO pins as outputs
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    // Turn all LEDs off initially
    MAIN_led_all_off();

    DEBUG_PRINTLN("[LED] Initialisation complete");
    DEBUG_PRINTF("[LED] Red LED on GPIO%d\n", LED_RED_PIN);
    DEBUG_PRINTF("[LED] Yellow LED on GPIO%d\n", LED_YELLOW_PIN);
    DEBUG_PRINTF("[LED] Green LED on GPIO%d\n", LED_GREEN_PIN);
}

/******************************************************************************
 * Red LED Functions (Error Indication)
 *****************************************************************************/

/**
 * @brief Turn red LED on
 * @return void
 */
void MAIN_led_red_on(void)
{
    digitalWrite(LED_RED_PIN, LED_ON);
    DEBUG_PRINTLN("[LED] Red ON");
}

/**
 * @brief Turn red LED off
 * @return void
 */
void MAIN_led_red_off(void)
{
    digitalWrite(LED_RED_PIN, LED_OFF);
    DEBUG_PRINTLN("[LED] Red OFF");
}

/**
 * @brief Toggle red LED state
 * @return void
 */
void MAIN_led_red_toggle(void)
{
    digitalWrite(LED_RED_PIN, !digitalRead(LED_RED_PIN));
}

/**
 * @brief Set red LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_red_set(uint8_t state)
{
    digitalWrite(LED_RED_PIN, state);
}

/******************************************************************************
 * Yellow LED Functions (Warning Indication)
 *****************************************************************************/

/**
 * @brief Turn yellow LED on
 * @return void
 */
void MAIN_led_yellow_on(void)
{
    digitalWrite(LED_YELLOW_PIN, LED_ON);
    DEBUG_PRINTLN("[LED] Yellow ON");
}

/**
 * @brief Turn yellow LED off
 * @return void
 */
void MAIN_led_yellow_off(void)
{
    digitalWrite(LED_YELLOW_PIN, LED_OFF);
    DEBUG_PRINTLN("[LED] Yellow OFF");
}

/**
 * @brief Toggle yellow LED state
 * @return void
 */
void MAIN_led_yellow_toggle(void)
{
    digitalWrite(LED_YELLOW_PIN, !digitalRead(LED_YELLOW_PIN));
}

/**
 * @brief Set yellow LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_yellow_set(uint8_t state)
{
    digitalWrite(LED_YELLOW_PIN, state);
}

/******************************************************************************
 * Green LED Functions (Heartbeat/Status Indication)
 *****************************************************************************/

/**
 * @brief Turn green LED on
 * @return void
 */
void MAIN_led_green_on(void)
{
    digitalWrite(LED_GREEN_PIN, LED_ON);
    DEBUG_PRINTLN("[LED] Green ON");
}

/**
 * @brief Turn green LED off
 * @return void
 */
void MAIN_led_green_off(void)
{
    digitalWrite(LED_GREEN_PIN, LED_OFF);
    DEBUG_PRINTLN("[LED] Green OFF");
}

/**
 * @brief Toggle green LED state
 * @return void
 */
void MAIN_led_green_toggle(void)
{
    digitalWrite(LED_GREEN_PIN, !digitalRead(LED_GREEN_PIN));
}

/**
 * @brief Set green LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_green_set(uint8_t state)
{
    digitalWrite(LED_GREEN_PIN, state);
}

/******************************************************************************
 * Multi-LED Control Functions
 *****************************************************************************/

/**
 * @brief Turn all LEDs on
 * @return void
 */
void MAIN_led_all_on(void)
{
    digitalWrite(LED_RED_PIN, LED_ON);
    digitalWrite(LED_YELLOW_PIN, LED_ON);
    digitalWrite(LED_GREEN_PIN, LED_ON);
    DEBUG_PRINTLN("[LED] All LEDs ON");
}

/**
 * @brief Turn all LEDs off
 * @return void
 */
void MAIN_led_all_off(void)
{
    digitalWrite(LED_RED_PIN, LED_OFF);
    digitalWrite(LED_YELLOW_PIN, LED_OFF);
    digitalWrite(LED_GREEN_PIN, LED_OFF);
    DEBUG_PRINTLN("[LED] All LEDs OFF");
}

/******************************************************************************
 * LED Pattern Functions
 *****************************************************************************/

/**
 * @brief Flash all LEDs in sequence
 * @param delay_ms Delay between each LED in milliseconds
 * @return void
 */
void MAIN_led_test_sequence(uint16_t delay_ms)
{
    DEBUG_PRINTLN("[LED] Running test sequence");

    // Turn all off first
    MAIN_led_all_off();
    delay(delay_ms);

    // Red
    MAIN_led_red_on();
    delay(delay_ms);
    MAIN_led_red_off();
    delay(delay_ms);

    // Yellow
    MAIN_led_yellow_on();
    delay(delay_ms);
    MAIN_led_yellow_off();
    delay(delay_ms);

    // Green
    MAIN_led_green_on();
    delay(delay_ms);
    MAIN_led_green_off();
    delay(delay_ms);

    // All together
    MAIN_led_all_on();
    delay(delay_ms);
    MAIN_led_all_off();

    DEBUG_PRINTLN("[LED] Test sequence complete");
}

/**
 * @brief Display error pattern on red LED
 * @details Fast blinking pattern to indicate critical error
 * @param count Number of blink cycles
 * @return void
 */
void MAIN_led_error_pattern(uint8_t count)
{
    DEBUG_PRINTF("[LED] Error pattern (%d cycles)\n", count);

    for (uint8_t i = 0; i < count; i++)
    {
        MAIN_led_red_on();
        delay(100); // Fast blink
        MAIN_led_red_off();
        delay(100);
    }
}

/**
 * @brief Display warning pattern on yellow LED
 * @details Slow blinking pattern to indicate warning
 * @param count Number of blink cycles
 * @return void
 */
void MAIN_led_warning_pattern(uint8_t count)
{
    DEBUG_PRINTF("[LED] Warning pattern (%d cycles)\n", count);

    for (uint8_t i = 0; i < count; i++)
    {
        MAIN_led_yellow_on();
        delay(500); // Slow blink
        MAIN_led_yellow_off();
        delay(500);
    }
}

/**
 * @brief Display success pattern on green LED
 * @details Quick double-blink to indicate successful operation
 * @return void
 */
void MAIN_led_success_pattern(void)
{
    DEBUG_PRINTLN("[LED] Success pattern");

    // Double blink
    MAIN_led_green_on();
    delay(100);
    MAIN_led_green_off();
    delay(100);
    MAIN_led_green_on();
    delay(100);
    MAIN_led_green_off();
}

/******************************************************************************
 * End of MAIN_ledLib.cpp
 ******************************************************************************/