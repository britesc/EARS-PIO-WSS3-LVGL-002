#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *screen_start;
    lv_obj_t *screen_config;
    lv_obj_t *screen_main;
    lv_obj_t *label_trash_0;
    lv_obj_t *label_trash_1;
    lv_obj_t *label_trash_2;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_SCREEN_START = 1,
    SCREEN_ID_SCREEN_CONFIG = 2,
    SCREEN_ID_SCREEN_MAIN = 3,
};

void create_screen_screen_start();
void tick_screen_screen_start();

void create_screen_screen_config();
void tick_screen_screen_config();

void create_screen_screen_main();
void tick_screen_screen_main();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/