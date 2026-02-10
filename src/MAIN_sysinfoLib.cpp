/**
 * @file MAIN_sysinfoLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief ESP32-S3 system information library implementation
 * @details Provides functions to query chip info, memory, flash, and runtime stats
 * @version 1.0.0
 * @date 20260210
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_sysinfoLib.h"
#include "EARS_systemDef.h"
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_system.h>
#include <esp_mac.h>

/******************************************************************************
 * Core Identification Functions
 *****************************************************************************/

/**
 * @brief Get ESP32 chip model name
 * @return String Chip model
 */
String MAIN_sysinfo_get_chip_model(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    String model = "ESP32";

    switch (chip_info.model)
    {
    case CHIP_ESP32:
        model = "ESP32";
        break;
    case CHIP_ESP32S2:
        model = "ESP32-S2";
        break;
    case CHIP_ESP32S3:
        model = "ESP32-S3";
        break;
    case CHIP_ESP32C3:
        model = "ESP32-C3";
        break;
#ifdef CHIP_ESP32C2
    case CHIP_ESP32C2:
        model = "ESP32-C2";
        break;
#endif
#ifdef CHIP_ESP32C6
    case CHIP_ESP32C6:
        model = "ESP32-C6";
        break;
#endif
#ifdef CHIP_ESP32H2
    case CHIP_ESP32H2:
        model = "ESP32-H2";
        break;
#endif
    default:
        model = "Unknown ESP32";
        break;
    }

    return model;
}

/**
 * @brief Get chip revision number
 * @return String Chip revision
 */
String MAIN_sysinfo_get_chip_revision(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    char revision[8];
    snprintf(revision, sizeof(revision), "v%d.%d",
             chip_info.revision / 100,
             chip_info.revision % 100);

    return String(revision);
}

/**
 * @brief Get CPU frequency in MHz
 * @return uint32_t CPU frequency
 */
uint32_t MAIN_sysinfo_get_cpu_freq_mhz(void)
{
    return getCpuFrequencyMhz();
}

/**
 * @brief Get number of CPU cores
 * @return uint8_t Number of cores
 */
uint8_t MAIN_sysinfo_get_cpu_cores(void)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.cores;
}

/**
 * @brief Get base MAC address as string
 * @return String MAC address
 */
String MAIN_sysinfo_get_mac_address(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return String(macStr);
}

/**
 * @brief Get chip ID (derived from MAC address)
 * @return uint64_t Unique chip ID
 */
uint64_t MAIN_sysinfo_get_chip_id(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    uint64_t chipId = 0;
    for (int i = 0; i < 6; i++)
    {
        chipId |= ((uint64_t)mac[i] << (8 * i));
    }

    return chipId;
}

/******************************************************************************
 * Memory Information Functions
 *****************************************************************************/

/**
 * @brief Get total heap size in bytes
 * @return uint32_t Total heap size
 */
uint32_t MAIN_sysinfo_get_heap_size(void)
{
    return ESP.getHeapSize();
}

/**
 * @brief Get free heap in bytes
 * @return uint32_t Free heap available
 */
uint32_t MAIN_sysinfo_get_free_heap(void)
{
    return ESP.getFreeHeap();
}

/**
 * @brief Get minimum free heap ever reached
 * @return uint32_t Minimum free heap in bytes
 */
uint32_t MAIN_sysinfo_get_min_free_heap(void)
{
    return ESP.getMinFreeHeap();
}

/**
 * @brief Get heap usage as percentage
 * @return float Heap usage (0.0 - 100.0)
 */
float MAIN_sysinfo_get_heap_usage_percent(void)
{
    uint32_t total = MAIN_sysinfo_get_heap_size();
    uint32_t free = MAIN_sysinfo_get_free_heap();

    if (total == 0)
        return 0.0f;

    return ((float)(total - free) / (float)total) * 100.0f;
}

/**
 * @brief Get total PSRAM size in bytes
 * @return uint32_t Total PSRAM size
 */
uint32_t MAIN_sysinfo_get_psram_size(void)
{
    return ESP.getPsramSize();
}

/**
 * @brief Get free PSRAM in bytes
 * @return uint32_t Free PSRAM available
 */
uint32_t MAIN_sysinfo_get_free_psram(void)
{
    return ESP.getFreePsram();
}

/**
 * @brief Get PSRAM usage as percentage
 * @return float PSRAM usage (0.0 - 100.0)
 */
float MAIN_sysinfo_get_psram_usage_percent(void)
{
    uint32_t total = MAIN_sysinfo_get_psram_size();
    uint32_t free = MAIN_sysinfo_get_free_psram();

    if (total == 0)
        return 0.0f;

    return ((float)(total - free) / (float)total) * 100.0f;
}

/**
 * @brief Check if PSRAM is available
 * @return bool True if PSRAM detected
 */
bool MAIN_sysinfo_has_psram(void)
{
    return (MAIN_sysinfo_get_psram_size() > 0);
}

/******************************************************************************
 * Flash Information Functions
 *****************************************************************************/

/**
 * @brief Get flash chip size in bytes
 * @return uint32_t Flash size
 */
uint32_t MAIN_sysinfo_get_flash_size(void)
{
    return ESP.getFlashChipSize();
}

/**
 * @brief Get flash chip size in MB
 * @return uint32_t Flash size in megabytes
 */
uint32_t MAIN_sysinfo_get_flash_size_mb(void)
{
    return MAIN_sysinfo_get_flash_size() / (1024 * 1024);
}

/**
 * @brief Get flash chip speed in MHz
 * @return uint32_t Flash speed
 */
uint32_t MAIN_sysinfo_get_flash_speed_mhz(void)
{
    return ESP.getFlashChipSpeed() / 1000000;
}

/**
 * @brief Get flash chip mode as string
 * @return String Flash mode
 */
String MAIN_sysinfo_get_flash_mode(void)
{
    FlashMode_t mode = ESP.getFlashChipMode();

    switch (mode)
    {
    case FM_QIO:
        return "QIO";
    case FM_QOUT:
        return "QOUT";
    case FM_DIO:
        return "DIO";
    case FM_DOUT:
        return "DOUT";
    case FM_FAST_READ:
        return "FAST_READ";
    case FM_SLOW_READ:
        return "SLOW_READ";
    default:
        return "UNKNOWN";
    }
}

/******************************************************************************
 * Runtime Information Functions
 *****************************************************************************/

/**
 * @brief Get system uptime in seconds
 * @return uint32_t Seconds since boot
 */
uint32_t MAIN_sysinfo_get_uptime_seconds(void)
{
    return millis() / 1000;
}

/**
 * @brief Get system uptime in milliseconds
 * @return uint32_t Milliseconds since boot
 */
uint32_t MAIN_sysinfo_get_uptime_ms(void)
{
    return millis();
}

/**
 * @brief Get uptime as formatted string
 * @return String Uptime in "HH:MM:SS" format
 */
String MAIN_sysinfo_get_uptime_string(void)
{
    uint32_t uptime_sec = MAIN_sysinfo_get_uptime_seconds();

    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;

    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);

    return String(buffer);
}

/**
 * @brief Get SDK version string
 * @return String ESP-IDF SDK version
 */
String MAIN_sysinfo_get_sdk_version(void)
{
    return String(ESP.getSdkVersion());
}

/******************************************************************************
 * Formatted Output Functions
 *****************************************************************************/

/**
 * @brief Print chip information to Serial
 * @return void
 */
void MAIN_sysinfo_print_chip(void)
{
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("CHIP INFORMATION:");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTF("Model:         %s\n", MAIN_sysinfo_get_chip_model().c_str());
    DEBUG_PRINTF("Revision:      %s\n", MAIN_sysinfo_get_chip_revision().c_str());
    DEBUG_PRINTF("Cores:         %d\n", MAIN_sysinfo_get_cpu_cores());
    DEBUG_PRINTF("CPU Freq:      %d MHz\n", MAIN_sysinfo_get_cpu_freq_mhz());
    DEBUG_PRINTF("MAC Address:   %s\n", MAIN_sysinfo_get_mac_address().c_str());
    DEBUG_PRINTF("Chip ID:       %llu\n", MAIN_sysinfo_get_chip_id());
    DEBUG_PRINTF("SDK Version:   %s\n", MAIN_sysinfo_get_sdk_version().c_str());
    DEBUG_PRINTLN();
}

/**
 * @brief Print memory information to Serial
 * @return void
 */
void MAIN_sysinfo_print_memory(void)
{
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("MEMORY INFORMATION:");
    DEBUG_PRINTLN("========================================");

    // Heap
    uint32_t heap_total = MAIN_sysinfo_get_heap_size();
    uint32_t heap_free = MAIN_sysinfo_get_free_heap();
    uint32_t heap_min = MAIN_sysinfo_get_min_free_heap();
    float heap_usage = MAIN_sysinfo_get_heap_usage_percent();

    DEBUG_PRINTF("Heap Total:    %s\n", MAIN_sysinfo_format_bytes(heap_total).c_str());
    DEBUG_PRINTF("Heap Free:     %s\n", MAIN_sysinfo_format_bytes(heap_free).c_str());
    DEBUG_PRINTF("Heap Min Free: %s\n", MAIN_sysinfo_format_bytes(heap_min).c_str());
    DEBUG_PRINTF("Heap Usage:    %s\n", MAIN_sysinfo_format_percent(heap_usage).c_str());

    // PSRAM
    if (MAIN_sysinfo_has_psram())
    {
        uint32_t psram_total = MAIN_sysinfo_get_psram_size();
        uint32_t psram_free = MAIN_sysinfo_get_free_psram();
        float psram_usage = MAIN_sysinfo_get_psram_usage_percent();

        DEBUG_PRINTF("PSRAM Total:   %s\n", MAIN_sysinfo_format_bytes(psram_total).c_str());
        DEBUG_PRINTF("PSRAM Free:    %s\n", MAIN_sysinfo_format_bytes(psram_free).c_str());
        DEBUG_PRINTF("PSRAM Usage:   %s\n", MAIN_sysinfo_format_percent(psram_usage).c_str());
    }
    else
    {
        DEBUG_PRINTLN("PSRAM:         Not available");
    }

    DEBUG_PRINTLN();
}

/**
 * @brief Print flash information to Serial
 * @return void
 */
void MAIN_sysinfo_print_flash(void)
{
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("FLASH INFORMATION:");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTF("Flash Size:    %d MB\n", MAIN_sysinfo_get_flash_size_mb());
    DEBUG_PRINTF("Flash Speed:   %d MHz\n", MAIN_sysinfo_get_flash_speed_mhz());
    DEBUG_PRINTF("Flash Mode:    %s\n", MAIN_sysinfo_get_flash_mode().c_str());
    DEBUG_PRINTLN();
}

/**
 * @brief Print all system information to Serial
 * @return void
 */
void MAIN_sysinfo_print_all(void)
{
    DEBUG_PRINTLN("\n");
    DEBUG_PRINTLN("â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—");
    DEBUG_PRINTLN("â•‘      SYSTEM INFORMATION REPORT         â•‘");
    DEBUG_PRINTLN("â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•");
    DEBUG_PRINTLN();

    MAIN_sysinfo_print_chip();
    MAIN_sysinfo_print_memory();
    MAIN_sysinfo_print_flash();

    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("RUNTIME INFORMATION:");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTF("Uptime:        %s\n", MAIN_sysinfo_get_uptime_string().c_str());
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN();
}

/******************************************************************************
 * Helper Functions
 *****************************************************************************/

/**
 * @brief Format bytes as human-readable string
 * @param bytes Number of bytes
 * @return String Formatted string
 */
String MAIN_sysinfo_format_bytes(uint32_t bytes)
{
    char buffer[32];

    if (bytes < 1024)
    {
        snprintf(buffer, sizeof(buffer), "%lu B", bytes);
    }
    else if (bytes < 1024 * 1024)
    {
        float kb = bytes / 1024.0f;
        snprintf(buffer, sizeof(buffer), "%.2f KB", kb);
    }
    else
    {
        float mb = bytes / (1024.0f * 1024.0f);
        snprintf(buffer, sizeof(buffer), "%.2f MB", mb);
    }

    return String(buffer);
}

/**
 * @brief Format percentage with 1 decimal place
 * @param value Percentage value
 * @return String Formatted string
 */
String MAIN_sysinfo_format_percent(float value)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f%%", value);
    return String(buffer);
}


/******************************************************************************
 * Library Version Information Getters
 *****************************************************************************/

// Get library name
const char* MAIN_SysInfo_getLibraryName() {
    return MAIN_SysInfo::LIB_NAME;
}

// Get encoded version as integer
uint32_t MAIN_SysInfo_getVersionEncoded() {
    return VERS_ENCODE(MAIN_SysInfo::VERSION_MAJOR, 
                       MAIN_SysInfo::VERSION_MINOR, 
                       MAIN_SysInfo::VERSION_PATCH);
}

// Get version date
const char* MAIN_SysInfo_getVersionDate() {
    return MAIN_SysInfo::VERSION_DATE;
}

// Format version as string
void MAIN_SysInfo_getVersionString(char* buffer) {
    uint32_t encoded = MAIN_SysInfo_getVersionEncoded();
    VERS_FORMAT(encoded, buffer);
}


/******************************************************************************
 * End of MAIN_sysinfoLib.cpp
 ******************************************************************************/