/**
 * @file main.cpp - STEP 4 DEVELOPMENT SCREEN FIXED
 * @brief EARS with live development screen - DIRECT BACKLIGHT CONTROL
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "EARS_versionDef.h"
#include "EARS_systemDef.h"
#include "EARS_toolsVersionDef.h"
#include "EARS_ws35tlcdPins.h"
#include "EARS_rgb565ColoursDef.h"
#include "MAIN_drawingLib.h"

#include <Arduino_GFX_Library.h>

// Display object
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, SPI_SCLK, SPI_MOSI, SPI_MISO);
Arduino_GFX *gfx = new Arduino_ST7796(bus, LCD_RST, 1, true, TFT_HEIGHT, TFT_WIDTH);

// FreeRTOS
TaskHandle_t Core0_Task_Handle = NULL;
TaskHandle_t Core1_Task_Handle = NULL;
SemaphoreHandle_t xDisplayMutex = NULL;

#define CORE0_STACK_SIZE 8192
#define CORE1_STACK_SIZE 4096
#define CORE0_PRIORITY 2
#define CORE1_PRIORITY 1

// Development Mode Variables
volatile uint32_t core0_heartbeat = 0;
volatile uint32_t core1_heartbeat = 0;
volatile uint32_t display_updates = 0;

// Function Prototypes
void Core0_UI_Task(void *parameter);
void Core1_Background_Task(void *parameter);
void initialise_serial();
void initialise_display();
void draw_development_screen();
void update_development_screen();

void setup()
{
    initialise_serial();

    DEBUG_PRINTLN("\n\n");
    DEBUG_PRINTLN("════════════════════════════════════════════════════════════");
    DEBUG_PRINTLN("  EARS - Equipment & Ammunition Reporting System");
    DEBUG_PRINTLN("════════════════════════════════════════════════════════════");
    DEBUG_PRINTF("  Version:    %s.%s.%s %s\n",
                 EARS_APP_VERSION_MAJOR, EARS_APP_VERSION_MINOR,
                 EARS_APP_VERSION_PATCH, EARS_STATUS);
    DEBUG_PRINTF("  Compiler:   %s\n", EARS_XTENSA_COMPILER_VERSION);
    DEBUG_PRINTF("  Platform:   %s\n", EARS_ESPRESSIF_PLATFORM_VERSION);
    DEBUG_PRINTLN("════════════════════════════════════════════════════════════");
    DEBUG_PRINTLN();

    DEBUG_PRINTLN("[INIT] Creating synchronisation primitives...");
    xDisplayMutex = xSemaphoreCreateMutex();
    if (xDisplayMutex == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create display mutex!");
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Synchronisation primitives created");

    initialise_display();

    DEBUG_PRINTLN("[INIT] Creating FreeRTOS tasks...");
    xTaskCreatePinnedToCore(Core0_UI_Task, "Core0_UI", CORE0_STACK_SIZE,
                            NULL, CORE0_PRIORITY, &Core0_Task_Handle, 0);
    if (Core0_Task_Handle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create Core 0 task!");
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Core 0 UI task created");

    xTaskCreatePinnedToCore(Core1_Background_Task, "Core1_Background",
                            CORE1_STACK_SIZE, NULL, CORE1_PRIORITY,
                            &Core1_Task_Handle, 1);
    if (Core1_Task_Handle == NULL)
    {
        DEBUG_PRINTLN("[ERROR] Failed to create Core 1 task!");
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Core 1 background task created");

    DEBUG_PRINTLN("[INIT] System initialisation complete\n");
}

void loop()
{
    delay(1000);
}

void Core0_UI_Task(void *parameter)
{
    DEBUG_PRINTLN("[CORE0] UI Task started");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);

    while (1)
    {
        core0_heartbeat++;

        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE)
        {
            update_development_screen();
            xSemaphoreGive(xDisplayMutex);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Core1_Background_Task(void *parameter)
{
    DEBUG_PRINTLN("[CORE1] Background Task started");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);

    while (1)
    {
        core1_heartbeat++;
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void initialise_serial()
{
#if EARS_DEBUG == 1
    Serial.begin(EARS_DEBUG_BAUD_RATE);
    delay(500);
    uint32_t timeout = millis();
    while (!Serial && (millis() - timeout < 2000))
        delay(10);
#endif
}

void initialise_display()
{
    DEBUG_PRINTLN("[INIT] Initialising display...");

    // DIRECT backlight control (like Steps 1 & 2 that worked!)
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
    DEBUG_PRINTLN("[OK] Backlight ON");

    if (!gfx->begin())
    {
        DEBUG_PRINTLN("[ERROR] Display init failed!");
        while (1)
            delay(1000);
    }
    DEBUG_PRINTLN("[OK] Display initialized");

    // Colour test bars
    DEBUG_PRINTLN("[TEST] Drawing colour bars...");
    MAIN_clear_screen(gfx, EARS_RGB565_BLACK);

    uint16_t barWidth = TFT_WIDTH / 8;
    MAIN_draw_filled_rect(gfx, 0 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_RED);
    MAIN_draw_filled_rect(gfx, 1 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GREEN);
    MAIN_draw_filled_rect(gfx, 2 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_BLUE);
    MAIN_draw_filled_rect(gfx, 3 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_YELLOW);
    MAIN_draw_filled_rect(gfx, 4 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_CYAN);
    MAIN_draw_filled_rect(gfx, 5 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_MAGENTA);
    MAIN_draw_filled_rect(gfx, 6 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_WHITE);
    MAIN_draw_filled_rect(gfx, 7 * barWidth, 0, barWidth, TFT_HEIGHT, EARS_RGB565_GRAY);

    DEBUG_PRINTLN("[OK] Colour bars drawn");
    delay(2000);

    draw_development_screen();
    DEBUG_PRINTLN("[OK] Development screen displayed");
}

void draw_development_screen()
{
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
    gfx->print("ESP32-S3 @ 240MHz");
    gfx->setCursor(250, 90);
    gfx->printf("Compiler: %s", EARS_XTENSA_COMPILER_VERSION);
    gfx->setCursor(250, 105);
    gfx->printf("Platform: %s", EARS_ESPRESSIF_PLATFORM_VERSION);

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
    gfx->print("Waiting for LVGL integration...");
}

void update_development_screen()
{
    display_updates++;
    uint32_t uptime_sec = millis() / 1000;

    // Clear update area
    MAIN_draw_filled_rect(gfx, 120, 145, 350, 100, EARS_RGB565_BLACK);

    gfx->setTextColor(EARS_RGB565_WHITE);
    gfx->setTextSize(1);

    gfx->setCursor(120, 150);
    gfx->printf("Running (%lu beats)", core0_heartbeat);

    gfx->setCursor(120, 180);
    gfx->printf("Running (%lu beats)", core1_heartbeat);

    gfx->setCursor(120, 210);
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    gfx->printf("%02lu:%02lu:%02lu", hours, minutes, seconds);

    gfx->setCursor(120, 240);
    gfx->printf("%lu updates", display_updates);

    // Heartbeat indicator
    uint16_t color = (core0_heartbeat % 2) ? EARS_RGB565_GREEN : EARS_RGB565_DARKGRAY;
    gfx->fillCircle(450, 155, 8, color);
}