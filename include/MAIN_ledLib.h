/**
 * @file MAIN_ledLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief LED indicator management library for EARS development breadboard
 * @details Provides hardware debugging LEDs for visual system status
 * @version 0.1.0
 * @date 20260128
 *
 * Hardware Configuration:
 * - Red LED (GPIO40):    Critical errors
 * - Yellow LED (GPIO41): Warnings
 * - Green LED (GPIO42):  Heartbeat/Status OK
 * - Common GND via Pin 29
 *
 * Each LED uses a 220Ω current-limiting resistor
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_LED_LIB_H__
#define __MAIN_LED_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * GPIO Pin Definitions
 *****************************************************************************/
#define LED_RED_PIN 40    // Pin 11 (left side) - Critical Errors
#define LED_YELLOW_PIN 41 // Pin 13 (left side) - Warnings
#define LED_GREEN_PIN 42  // Pin 15 (left side) - Heartbeat/Status

/******************************************************************************
 * LED State Definitions
 *****************************************************************************/
#define LED_ON HIGH
#define LED_OFF LOW

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

/**
 * @brief Initialise all LED GPIOs
 * @details Sets all LED pins to OUTPUT mode and turns them off
 * @return void
 */
void MAIN_led_init(void);

/**
 * @brief Turn red LED on
 * @details Indicates critical error condition
 * @return void
 */
void MAIN_led_red_on(void);

/**
 * @brief Turn red LED off
 * @return void
 */
void MAIN_led_red_off(void);

/**
 * @brief Toggle red LED state
 * @return void
 */
void MAIN_led_red_toggle(void);

/**
 * @brief Turn yellow LED on
 * @details Indicates warning condition
 * @return void
 */
void MAIN_led_yellow_on(void);

/**
 * @brief Turn yellow LED off
 * @return void
 */
void MAIN_led_yellow_off(void);

/**
 * @brief Toggle yellow LED state
 * @return void
 */
void MAIN_led_yellow_toggle(void);

/**
 * @brief Turn green LED on
 * @details Indicates system healthy/operational
 * @return void
 */
void MAIN_led_green_on(void);

/**
 * @brief Turn green LED off
 * @return void
 */
void MAIN_led_green_off(void);

/**
 * @brief Toggle green LED state
 * @details Useful for heartbeat indication
 * @return void
 */
void MAIN_led_green_toggle(void);

/**
 * @brief Turn all LEDs on
 * @details Useful for power-on self-test
 * @return void
 */
void MAIN_led_all_on(void);

/**
 * @brief Turn all LEDs off
 * @return void
 */
void MAIN_led_all_off(void);

/**
 * @brief Flash all LEDs in sequence
 * @param delay_ms Delay between each LED in milliseconds
 * @return void
 */
void MAIN_led_test_sequence(uint16_t delay_ms);

/**
 * @brief Display error pattern on red LED
 * @details Fast blinking pattern to indicate critical error
 * @param count Number of blink cycles
 * @return void
 */
void MAIN_led_error_pattern(uint8_t count);

/**
 * @brief Display warning pattern on yellow LED
 * @details Slow blinking pattern to indicate warning
 * @param count Number of blink cycles
 * @return void
 */
void MAIN_led_warning_pattern(uint8_t count);

/**
 * @brief Display success pattern on green LED
 * @details Quick double-blink to indicate successful operation
 * @return void
 */
void MAIN_led_success_pattern(void);

/**
 * @brief Set red LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_red_set(uint8_t state);

/**
 * @brief Set yellow LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_yellow_set(uint8_t state);

/**
 * @brief Set green LED state directly
 * @param state HIGH (on) or LOW (off)
 * @return void
 */
void MAIN_led_green_set(uint8_t state);

#endif // __MAIN_LED_LIB_H__

/******************************************************************************
 * End of MAIN_ledLib.h
 ******************************************************************************/