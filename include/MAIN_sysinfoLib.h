/**
 * @file MAIN_sysinfoLib.h
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief ESP32-S3 system information library for EARS
 * @details Provides functions to query chip info, memory, flash, and runtime stats
 * @version 0.1.0
 * @date 20260128
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#pragma once
#ifndef __MAIN_SYSINFO_LIB_H__
#define __MAIN_SYSINFO_LIB_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Core Identification Functions
 *****************************************************************************/

/**
 * @brief Get ESP32 chip model name
 * @return String Chip model (e.g., "ESP32-S3")
 */
String MAIN_sysinfo_get_chip_model(void);

/**
 * @brief Get chip revision number
 * @return String Chip revision (e.g., "v0.1")
 */
String MAIN_sysinfo_get_chip_revision(void);

/**
 * @brief Get CPU frequency in MHz
 * @return uint32_t CPU frequency (e.g., 240)
 */
uint32_t MAIN_sysinfo_get_cpu_freq_mhz(void);

/**
 * @brief Get number of CPU cores
 * @return uint8_t Number of cores (2 for ESP32-S3)
 */
uint8_t MAIN_sysinfo_get_cpu_cores(void);

/**
 * @brief Get base MAC address as string
 * @return String MAC address in format "XX:XX:XX:XX:XX:XX"
 */
String MAIN_sysinfo_get_mac_address(void);

/**
 * @brief Get chip ID (derived from MAC address)
 * @return uint64_t Unique chip ID
 */
uint64_t MAIN_sysinfo_get_chip_id(void);

/******************************************************************************
 * Memory Information Functions
 *****************************************************************************/

/**
 * @brief Get total heap size in bytes
 * @return uint32_t Total heap size
 */
uint32_t MAIN_sysinfo_get_heap_size(void);

/**
 * @brief Get free heap in bytes
 * @return uint32_t Free heap available
 */
uint32_t MAIN_sysinfo_get_free_heap(void);

/**
 * @brief Get minimum free heap ever reached (low water mark)
 * @return uint32_t Minimum free heap in bytes
 */
uint32_t MAIN_sysinfo_get_min_free_heap(void);

/**
 * @brief Get heap usage as percentage
 * @return float Heap usage (0.0 - 100.0)
 */
float MAIN_sysinfo_get_heap_usage_percent(void);

/**
 * @brief Get total PSRAM size in bytes
 * @return uint32_t Total PSRAM size (0 if not available)
 */
uint32_t MAIN_sysinfo_get_psram_size(void);

/**
 * @brief Get free PSRAM in bytes
 * @return uint32_t Free PSRAM available
 */
uint32_t MAIN_sysinfo_get_free_psram(void);

/**
 * @brief Get PSRAM usage as percentage
 * @return float PSRAM usage (0.0 - 100.0)
 */
float MAIN_sysinfo_get_psram_usage_percent(void);

/**
 * @brief Check if PSRAM is available
 * @return bool True if PSRAM detected
 */
bool MAIN_sysinfo_has_psram(void);

/******************************************************************************
 * Flash Information Functions
 *****************************************************************************/

/**
 * @brief Get flash chip size in bytes
 * @return uint32_t Flash size
 */
uint32_t MAIN_sysinfo_get_flash_size(void);

/**
 * @brief Get flash chip size in MB
 * @return uint32_t Flash size in megabytes
 */
uint32_t MAIN_sysinfo_get_flash_size_mb(void);

/**
 * @brief Get flash chip speed in MHz
 * @return uint32_t Flash speed
 */
uint32_t MAIN_sysinfo_get_flash_speed_mhz(void);

/**
 * @brief Get flash chip mode as string
 * @return String Flash mode ("QIO", "QOUT", "DIO", "DOUT", "UNKNOWN")
 */
String MAIN_sysinfo_get_flash_mode(void);

/******************************************************************************
 * Runtime Information Functions
 *****************************************************************************/

/**
 * @brief Get system uptime in seconds
 * @return uint32_t Seconds since boot
 */
uint32_t MAIN_sysinfo_get_uptime_seconds(void);

/**
 * @brief Get system uptime in milliseconds
 * @return uint32_t Milliseconds since boot
 */
uint32_t MAIN_sysinfo_get_uptime_ms(void);

/**
 * @brief Get uptime as formatted string
 * @return String Uptime in "HH:MM:SS" format
 */
String MAIN_sysinfo_get_uptime_string(void);

/**
 * @brief Get SDK version string
 * @return String ESP-IDF SDK version
 */
String MAIN_sysinfo_get_sdk_version(void);

/******************************************************************************
 * Formatted Output Functions
 *****************************************************************************/

/**
 * @brief Print all system information to Serial
 * @return void
 */
void MAIN_sysinfo_print_all(void);

/**
 * @brief Print memory information to Serial
 * @return void
 */
void MAIN_sysinfo_print_memory(void);

/**
 * @brief Print chip information to Serial
 * @return void
 */
void MAIN_sysinfo_print_chip(void);

/**
 * @brief Print flash information to Serial
 * @return void
 */
void MAIN_sysinfo_print_flash(void);

/******************************************************************************
 * Helper Functions for Display
 *****************************************************************************/

/**
 * @brief Format bytes as human-readable string
 * @param bytes Number of bytes
 * @return String Formatted string (e.g., "123.45 KB")
 */
String MAIN_sysinfo_format_bytes(uint32_t bytes);

/**
 * @brief Format percentage with 1 decimal place
 * @param value Percentage value (0.0 - 100.0)
 * @return String Formatted string (e.g., "45.2%")
 */
String MAIN_sysinfo_format_percent(float value);

#endif // __MAIN_SYSINFO_LIB_H__

/******************************************************************************
 * End of MAIN_sysinfoLib.h
 ******************************************************************************/