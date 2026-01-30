/**
 * @file MAIN_developmentFeaturesLib.cpp
 * @author Julian (51fiftyone51fiftyone@gmail.com)
 * @brief Development features library implementation
 * @details Contains all development/debugging features that compile out in production
 * @version 0.1.0
 * @date 20260128
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MAIN_developmentFeaturesLib.h"
#include "MAIN_drawingLib.h"
#include "MAIN_sysinfoLib.h"
#include "EARS_versionDef.h"
#include "EARS_systemDef.h"
#include "EARS_toolsVersionDef.h"
#include "EARS_rgb565ColoursDef.h"
#include "EARS_ws35tlcdPins.h"

/******************************************************************************
 * Development Mode Variables
 *****************************************************************************/
volatile uint32_t dev_core0_heartbeat = 0;
volatile uint32_t dev_core1_heartbeat = 0;
volatile uint32_t dev_display_updates = 0;

/******************************************************************************
 * Boot Banner Functions
 *****************************************************************************/

/**
 * @brief Print EARS boot banner
 * @return void
 */
void DEV_print_boot_banner(void)
{
    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════════════════════════╗");
    Serial.println("║  EARS - Equipment & Ammunition Reporting System           ║");
    Serial.println("╚════════════════════════════════════════════════════════════╝");
    Serial.printf("  Version:    %s.%s.%s %s\n",
                  EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR,
                  EARS_APP_VERSION_PATCH, EARS_STATUS);
    Serial.printf("  Compiler:   %s\n", EARS_XTENSA_COMPILER_VERSION);
    Serial.printf("  Platform:   %s\n", EARS_ESPRESSIF_PLATFORM_VERSION);
    Serial.println("╚════════════════════════════════════════════════════════════╝");
    Serial.println();
}

/**
 * @brief Print complete system information report
 * @return void
 */
void DEV_print_system_info(void)
{
    MAIN_sysinfo_print_all();
}

/******************************************************************************
 * Development Screen Functions
 *****************************************************************************/

/**
 * @brief Draw the initial development screen layout
 * @param gfx Pointer to Arduino_GFX display object
 * @return void
 */
void DEV_draw_screen(Arduino_GFX *gfx)
{
    if (gfx == nullptr)
        return;

    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    // Title bar
    MAIN_draw_filled_rect(gfx, 0, 0, TFT_WIDTH, 40, EARS_RGB565_RS_PRIMARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 12);
    gfx->print("EARS - DEVELOPMENT MODE");

    // Version box
    MAIN_draw_rounded_rect(gfx, 10, 50, 220, 80, 5, EARS_RGB565_CS_PRIMARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(20, 60);
    gfx->print("Version:");
    gfx->setTextSize(2);
    gfx->setCursor(20, 75);
    gfx->printf("%s.%s.%s", EARS_APP_VERSION_MAJOR,
                EARS_APP_VERSION_MINOR, EARS_APP_VERSION_PATCH);
    gfx->setTextSize(1);
    gfx->setCursor(20, 105);
    gfx->print(EARS_STATUS);

    // System Info box
    MAIN_draw_rounded_rect(gfx, 240, 50, 230, 80, 5, EARS_RGB565_CS_SECONDARY);
    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(250, 60);
    gfx->print("Platform:");
    gfx->setCursor(250, 75);
    gfx->printf("%s @ %dMHz", MAIN_sysinfo_get_chip_model().c_str(),
                MAIN_sysinfo_get_cpu_freq_mhz());
    gfx->setCursor(250, 90);
    gfx->printf("Heap: %s free", MAIN_sysinfo_format_bytes(MAIN_sysinfo_get_free_heap()).c_str());
    gfx->setCursor(250, 105);
    gfx->printf("PSRAM: %s free", MAIN_sysinfo_format_bytes(MAIN_sysinfo_get_free_psram()).c_str());

    // Status labels
    gfx->setTextColor(EARS_RGB565_CS_TEXT);
    gfx->setTextSize(1);
    gfx->setCursor(10, 150);
    gfx->print("Core 0 (UI):");
    gfx->setCursor(10, 180);
    gfx->print("Core 1 (BG):");
    gfx->setCursor(10, 210);
    gfx->print("Uptime:");
    gfx->setCursor(10, 240);
    gfx->print("Display:");

    // Footer
    gfx->setTextColor(EARS_RGB565_GRAY);
    gfx->setCursor(10, 300);
    gfx->print("LED Heartbeat: GPIO42 (Green)");
}

/**
 * @brief Update live statistics on development screen
 * @param gfx Pointer to Arduino_GFX display object
 * @return void
 */
void DEV_update_screen(Arduino_GFX *gfx)
{
    if (gfx == nullptr)
        return;

    dev_display_updates++;
    uint32_t uptime_sec = millis() / 1000;

    // Clear update area
    MAIN_draw_filled_rect(gfx, 120, 145, 350, 100, EARS_RGB565_BLACK);

    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);

    // Core 0 heartbeat
    gfx->setCursor(120, 150);
    gfx->printf("Running (%lu beats)", dev_core0_heartbeat);

    // Core 1 heartbeat
    gfx->setCursor(120, 180);
    gfx->printf("Running (%lu beats)", dev_core1_heartbeat);

    // Uptime
    gfx->setCursor(120, 210);
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    gfx->printf("%02lu:%02lu:%02lu", hours, minutes, seconds);

    // Display updates
    gfx->setCursor(120, 240);
    gfx->printf("%lu updates", dev_display_updates);

    // Heartbeat indicator (flashing dot)
    uint16_t color = (dev_core0_heartbeat % 2) ? EARS_RGB565_GREEN : EARS_RGB565_DARKGRAY;
    gfx->fillCircle(450, 155, 8, color);
}

/******************************************************************************
 * Heartbeat Management Functions
 *****************************************************************************/

/**
 * @brief Increment Core 0 heartbeat counter
 * @return void
 */
void DEV_increment_core0_heartbeat(void)
{
    dev_core0_heartbeat++;
}

/**
 * @brief Increment Core 1 heartbeat counter
 * @return void
 */
void DEV_increment_core1_heartbeat(void)
{
    dev_core1_heartbeat++;
}

/**
 * @brief Increment display update counter
 * @return void
 */
void DEV_increment_display_updates(void)
{
    dev_display_updates++;
}

/**
 * @brief Get Core 0 heartbeat count
 * @return uint32_t Current heartbeat count
 */
uint32_t DEV_get_core0_heartbeat(void)
{
    return dev_core0_heartbeat;
}

/**
 * @brief Get Core 1 heartbeat count
 * @return uint32_t Current heartbeat count
 */
uint32_t DEV_get_core1_heartbeat(void)
{
    return dev_core1_heartbeat;
}

/**
 * @brief Get display update count
 * @return uint32_t Current update count
 */
uint32_t DEV_get_display_updates(void)
{
    return dev_display_updates;
}

/******************************************************************************
 * End of MAIN_developmentFeaturesLib.cpp
 ******************************************************************************/