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

// Style: Dark_IconBar_Background 
lv_style_t *get_style_dark_icon_bar_background__MAIN_DEFAULT();
void add_style_dark_icon_bar_background_(lv_obj_t *obj);
void remove_style_dark_icon_bar_background_(lv_obj_t *obj);

// Style: Dark_IconBar_Text
lv_style_t *get_style_dark_icon_bar_text_MAIN_DEFAULT();
void add_style_dark_icon_bar_text(lv_obj_t *obj);
void remove_style_dark_icon_bar_text(lv_obj_t *obj);

// Style: Dark_InfoBar_ScrollText
lv_style_t *get_style_dark_info_bar_scroll_text_MAIN_DEFAULT();
void add_style_dark_info_bar_scroll_text(lv_obj_t *obj);
void remove_style_dark_info_bar_scroll_text(lv_obj_t *obj);

// Style: Dark_HeaderBar_Title
lv_style_t *get_style_dark_header_bar_title_MAIN_DEFAULT();
void add_style_dark_header_bar_title(lv_obj_t *obj);
void remove_style_dark_header_bar_title(lv_obj_t *obj);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/