/**
 * @file EARS_touchDemoUI.cpp
 * @author JTB
 * @brief Touch demonstration UI implementation
 * @version 1.0.0
 * @date 20260207
 *
 * @copyright Copyright (c) 2026 JTB. All rights reserved.
 */

#include "EARS_touchDemoUI.h"
#include "EARS_rgb565ColoursDef.h"

// Global widget pointers for event handlers
static lv_obj_t *switch_label = NULL;
static lv_obj_t *slider_label = NULL;

/**
 * @brief Event handler for switch
 */
static void switch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (switch_label != NULL)
        {
            if (lv_obj_has_state(sw, LV_STATE_CHECKED))
            {
                lv_label_set_text(switch_label, "Switch: ON");
                lv_obj_set_style_text_color(switch_label, lv_color_hex(EARS_RGB565_GREEN), 0);
            }
            else
            {
                lv_label_set_text(switch_label, "Switch: OFF");
                lv_obj_set_style_text_color(switch_label, lv_color_hex(EARS_RGB565_RED), 0);
            }
        }
    }
}

/**
 * @brief Event handler for slider
 */
static void slider_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        if (slider_label != NULL)
        {
            int32_t value = lv_slider_get_value(slider);
            lv_label_set_text_fmt(slider_label, "Slider: %d%%", (int)value);
        }
    }
}

/**
 * @brief Create touch demonstration UI
 */
void create_touch_demo_ui()
{
    // Get active screen
    lv_obj_t *screen = lv_screen_active();

    // Set background colour
    lv_obj_set_style_bg_color(screen, lv_color_hex(EARS_RGB565_BLACK), 0);

    // Create title label
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "EARS v0.11.0\nTouch Demo");
    lv_obj_set_style_text_color(title, lv_color_hex(EARS_RGB565_WHITE), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Create chip info label
    lv_obj_t *chip_info = lv_label_create(screen);
    lv_label_set_text(chip_info, "FT6236U/FT3267\nI2C: 0x38 @ SDA=8, SCL=7");
    lv_obj_set_style_text_color(chip_info, lv_color_hex(EARS_RGB565_CYAN), 0);
    lv_obj_set_style_text_align(chip_info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(chip_info, LV_ALIGN_TOP_MID, 0, 80);

    // Create container for widgets
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, 400, 180);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x1A1A1A), 0);
    lv_obj_set_style_border_color(container, lv_color_hex(EARS_RGB565_CYAN), 0);
    lv_obj_set_style_border_width(container, 2, 0);
    lv_obj_set_style_radius(container, 10, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 10);

    // CRITICAL: Disable scrolling so touch events reach the widgets
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    // Create switch label
    switch_label = lv_label_create(container);
    lv_label_set_text(switch_label, "Switch: OFF");
    lv_obj_set_style_text_color(switch_label, lv_color_hex(EARS_RGB565_RED), 0);
    lv_obj_align(switch_label, LV_ALIGN_TOP_LEFT, 20, 20);

    // Create switch
    lv_obj_t *sw = lv_switch_create(container);
    lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -20, 15);
    lv_obj_set_style_bg_color(sw, lv_color_hex(EARS_RGB565_DARKGRAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw, lv_color_hex(EARS_RGB565_GREEN), LV_PART_INDICATOR);
    lv_obj_add_event_cb(sw, switch_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Create slider label
    slider_label = lv_label_create(container);
    lv_label_set_text(slider_label, "Slider: 50%");
    lv_obj_set_style_text_color(slider_label, lv_color_hex(EARS_RGB565_WHITE), 0);
    lv_obj_align(slider_label, LV_ALIGN_TOP_LEFT, 20, 80);

    // Create slider
    lv_obj_t *slider = lv_slider_create(container);
    lv_obj_set_size(slider, 350, 20);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 20, 110);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(EARS_RGB565_DARKGRAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(EARS_RGB565_CYAN), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(EARS_RGB565_WHITE), LV_PART_KNOB);
    lv_obj_add_event_cb(slider, slider_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    // Create instructions label
    lv_obj_t *instructions = lv_label_create(screen);
    lv_label_set_text(instructions, "Tap switch to toggle • Drag slider to adjust");
    lv_obj_set_style_text_color(instructions, lv_color_hex(EARS_RGB565_YELLOW), 0);
    lv_obj_set_style_text_align(instructions, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(instructions, LV_ALIGN_BOTTOM_MID, 0, -20);
}