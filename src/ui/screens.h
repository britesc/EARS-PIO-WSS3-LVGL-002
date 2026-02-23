#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_SCREEN_START = 1,
    SCREEN_ID_SCREEN_CONFIG = 2,
    SCREEN_ID_SCREEN_MAIN = 3,
    _SCREEN_ID_LAST = 3
};

typedef struct _objects_t {
    lv_obj_t *screen_start;
    lv_obj_t *screen_config;
    lv_obj_t *screen_main;
    lv_obj_t *obj0;
    lv_obj_t *obj0__b5_info_bar_container;
    lv_obj_t *obj0__b5_info_bar_container_label_scroll;
    lv_obj_t *obj1;
    lv_obj_t *obj1__b2_header_bar_container;
    lv_obj_t *obj1__b2_header_bar_container_label;
    lv_obj_t *obj1__b2_header_bar_image_left;
    lv_obj_t *obj1__b2_header_bar_image_right;
    lv_obj_t *obj2;
    lv_obj_t *obj2__obj0;
    lv_obj_t *obj2__t1_label_test;
    lv_obj_t *obj3;
    lv_obj_t *obj3__w1_icon_bar_container;
    lv_obj_t *obj3__w1_icon_bar_container_label_time;
    lv_obj_t *obj3__w1_icon_bar_container_label_battery;
    lv_obj_t *obj4;
    lv_obj_t *obj4__b5_info_bar_container;
    lv_obj_t *obj4__b5_info_bar_container_label_scroll;
    lv_obj_t *w2_header_bar;
    lv_obj_t *w2_header_bar__b2_header_bar_container;
    lv_obj_t *w2_header_bar__b2_header_bar_container_label;
    lv_obj_t *w2_header_bar__b2_header_bar_image_left;
    lv_obj_t *w2_header_bar__b2_header_bar_image_right;
    lv_obj_t *obj5;
    lv_obj_t *obj5__obj0;
    lv_obj_t *obj5__t1_label_test;
    lv_obj_t *obj6;
    lv_obj_t *obj6__w1_icon_bar_container;
    lv_obj_t *obj6__w1_icon_bar_container_label_time;
    lv_obj_t *obj6__w1_icon_bar_container_label_battery;
    lv_obj_t *obj7;
    lv_obj_t *obj7__b5_info_bar_container;
    lv_obj_t *obj7__b5_info_bar_container_label_scroll;
    lv_obj_t *label_trash_0;
    lv_obj_t *label_trash_1;
    lv_obj_t *label_trash_2;
} objects_t;

extern objects_t objects;

void create_screen_screen_start();
void tick_screen_screen_start();

void create_screen_screen_config();
void tick_screen_screen_config();

void create_screen_screen_main();
void tick_screen_screen_main();

void create_user_widget_b5_info_bar(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_b5_info_bar(void *flowState, int startWidgetIndex);

void create_user_widget_b2_header_bar(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_b2_header_bar(void *flowState, int startWidgetIndex);

void create_user_widget_t1_test_widget(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_t1_test_widget(void *flowState, int startWidgetIndex);

void create_user_widget_w1_icon_bar(lv_obj_t *parent_obj, void *flowState, int startWidgetIndex);
void tick_user_widget_w1_icon_bar(void *flowState, int startWidgetIndex);

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/