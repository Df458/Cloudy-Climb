#include "audio_loader.h"
#include "audio_player.h"
#include "audio_source.h"
#include "audio_system.h"
#include "camera.h"
#include "color.h"
#include "container/array.h"
#include "control.h"
#include "font_loader.h"
#include "font.h"
#include "interpolate.h"
#include "log/log.h"
#include "paths.h"
#include "shader.h"
#include "texture_loader.h"
#include "vector.h"
#include "audiomanager.h"
#include "window.h"

#include "menu.h"
#include "renderer.h"

#include <math.h>

font f_menu;
sarray menu_stack;
gltex t_bonus_icon;

static gltex t_button_l;
static gltex t_button_r;
static gltex t_bar_back;
static gltex t_bar_front;

audio_source a_activate;
static audio_source a_inactive;
static audio_source a_scroll;

menu* closing_menu = NULL;
slider* active_slider = NULL;
slider* closing_slider = NULL;

camera c_ui;
extern shader s_default;

extern action_id a_primary;

void init_menus(void* win) {
    char* path = assets_path("CaveatBrush-Regular.ttf", NULL);
    f_menu = load_font(path, 32);
    sfree(path);
    path = assets_path("Bonus_Icon.png", NULL);
    t_bonus_icon = load_texture_gl(path);
    sfree(path);
    path = assets_path("Button_L.png", NULL);
    t_button_l = load_texture_gl(path);
    sfree(path);
    path = assets_path("Button_R.png", NULL);
    t_button_r = load_texture_gl(path);
    sfree(path);
    path = assets_path("Bar_Empty.png", NULL);
    t_bar_back = load_texture_gl(path);
    sfree(path);
    path = assets_path("Bar_Filled.png", NULL);
    t_bar_front = load_texture_gl(path);
    sfree(path);
    path = assets_path("SelectChirp02.wav", NULL);
    a_activate = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Inactive.wav", NULL);
    a_inactive = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Scroll.wav", NULL);
    a_scroll = load_audio_source(path, true);
    sfree(path);

    c_ui = window_create_2d_camera(win);

    menu_stack = sarray_new(4);
}
void cleanup_menus() {
    /* font_free(f_menu); */ // FIXME: Crash in FT_Done_Face?
    array_free(menu_stack);
    glDeleteTextures(1, &t_bonus_icon.handle);
    glDeleteTextures(1, &t_button_l.handle);
    glDeleteTextures(1, &t_button_r.handle);
    glDeleteTextures(1, &t_bar_back.handle);
    glDeleteTextures(1, &t_bar_front.handle);

    audio_source_free(a_activate);
    audio_source_free(a_inactive);
    audio_source_free(a_scroll);

    closing_menu = NULL;
    active_slider = NULL;
    closing_slider = NULL;

    camera_free(c_ui);
}

menu* create_menu(menu_entry entries[], int entry_count, menu_select_event* ev) {
    menu* m = msalloc(menu);
    *m = (menu) {
        .selected = 0,
        .margin = 0,
        .entry_count = entry_count,
        .entries = entries
    };

    m->position.y = -1;

    for(int i = 0; i < entry_count; ++i) {
        entries[i].display_text = text_new(f_menu, entries[i].name);
        text_set_align(entries[i].display_text, TEXT_ALIGN_CENTER);
    }

    bind_event(m->activate, ev);

    return m;
}
void draw_slider(slider* s, bool in) {
    vec2 pos = { .x = -368, .y = 50 };

    draw_texture(mat4_translate(mat4_scale(mat4_ident, 1), pos), t_button_l, color_white);
    pos.x += 48;
    float v = -0.01;
    for(int i = 0; i < 20; ++i) {
        v += 0.05;
        draw_texture(mat4_translate(mat4_scale(mat4_ident, 1), pos), v > s->value ? t_bar_back : t_bar_front, color_white);
        pos.x += 32;
    }
    pos.x += 16;
    draw_texture(mat4_translate(mat4_scale(mat4_ident, 1), pos), t_button_r, color_white);
}
void draw_menu(menu* m) {
    vec3 offset = (vec3){ .x = m->position.x, .y = 100 - (m->position.y * 260), .z = -1 };
    for(int i = 0; i < m->entry_count; ++i) {
        vec4 color = i == m->selected ? color_yellow : color_white;
        if(!m->entries[i].active)
            color = vec_mul(color, 0.5f);

        color.a = 1 - fabs(m->position.y);

        if(m->entries[i].special) {
            vec3 pos = (vec3) {
                .x = offset.x - m->margin * 0.25f - 12,
                    .y = offset.y + 4
            };
            draw_texture(mat4_translate(mat4_scale(mat4_ident, i == m->selected ? 1.0f : 0.75f), pos), t_bonus_icon, (vec4){ .r=1,.g=1,.b=1,.a=color.a });
        }
        shader_bind_uniform_name(s_default, "u_color", color);
        text_draw(m->entries[i].display_text, s_default, mat4_mul(camera_get_vp(c_ui), mat4_translate(mat4_scale(mat4_ident, i == m->selected ? 1.0f : 0.75f), offset)));

        offset.x += m->margin;
    }
}
void update_menus(float dt) {
    if(active_slider) {
        draw_slider(active_slider, true);
    }

    if(closing_menu) {
        closing_menu->position.x = lerp(closing_menu->position.x, closing_menu->selected * -closing_menu->margin, 0.15);
        closing_menu->position.y = lerp(closing_menu->position.y, -1, 0.15);
        draw_menu(closing_menu);
    }
    if(array_size(menu_stack) > 0) {
        menu* m = array_get(menu_stack, array_size(menu_stack) - 1);
        m->position.x = lerp(m->position.x, m->selected * -m->margin, 0.15);
        m->position.y = lerp(m->position.y, 0, 0.15);
        draw_menu(m);

        if(action_is_triggered(a_primary)) {
            if(!active_slider) {
                if(m->entries[m->selected].active) {
                    play_audio(a_activate, false, false);
                    call_event(m->activate, m->selected);
                    if(m->entries[m->selected].submenu)
                        set_menu(m->entries[m->selected].submenu, false);
                    else if(m->entries[m->selected].attached_slider)
                        active_slider = m->entries[m->selected].attached_slider;
                } else {
                    play_audio(a_inactive, false, false);
                }
            } else {
                play_audio(a_activate, false, false);
                call_event(active_slider->value_set, active_slider->value);
                closing_slider = active_slider;
                active_slider = NULL;
            }
        }
    }
    if(array_size(menu_stack) > 1) {
        menu* m = array_get(menu_stack, array_size(menu_stack) - 2);
        m->position.x = lerp(m->position.x, m->selected * -m->margin, 0.15);
        m->position.y = lerp(m->position.y, 1, 0.15);
        draw_menu(m);
    }
}

void scroll_menu(int64 diff) {
    if(array_size(menu_stack) > 0) {
        if(!active_slider) {
            menu* m = array_get(menu_stack, array_size(menu_stack) - 1);

            m->selected += diff;

            if(m->selected >= m->entry_count)
                m->selected -= m->entry_count;
            else if(m->selected < 0)
                m->selected += m->entry_count;
        } else {
            active_slider->value += diff * 0.05f;
            active_slider->value = clamp(active_slider->value, active_slider->min, active_slider->max);
            call_event(active_slider->value_changed, active_slider->value);
        }

        play_audio(a_scroll, false, false);
    }
}

bool in_menu(menu* m) {
    return array_contains(menu_stack, m);
}

void set_menu(menu* m, bool replace) {
    closing_menu = NULL;

    if(replace)
        close_menu(true);

    if(!array_contains(menu_stack, m)) {
        array_add(menu_stack, m);
        m->selected = 0;
        m->position.x = 0;
    }
}

void close_menu(bool all) {
    if(array_size(menu_stack) > 0) {
        if(!all)
            closing_menu = array_get(menu_stack, array_size(menu_stack) - 1);
        do {
            array_pop(menu_stack);
        } while(all && array_size(menu_stack) > 0);
    }
}
