#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: Light_Default_Screen
lv_style_t *get_style_light_default_screen_MAIN_DEFAULT();
void add_style_light_default_screen(lv_obj_t *obj);
void remove_style_light_default_screen(lv_obj_t *obj);

// Style: Dark_Default_Screen
lv_style_t *get_style_dark_default_screen_MAIN_DEFAULT();
void add_style_dark_default_screen(lv_obj_t *obj);
void remove_style_dark_default_screen(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/