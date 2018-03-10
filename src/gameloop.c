#include "audio_loader.h"
#include "camera.h"
#include "check.h"
#include "color.h"
#include "font_loader.h"
#include "font.h"
#include "interpolate.h"
#include "menu.h"
#include "paths.h"
#include "shader.h"
#include "stringutil.h"
#include "texture_loader.h"
#include "window.h"

#include <GL/glew.h>
#include <GL/gl.h>
#include <locale.h>
#include <stdlib.h>
#include <time.h>

#include "bonus.h"
#include "gameloop.h"
#include "cloud.h"
#include "io.h"
#include "rail.h"
#include "renderer.h"
#include "spike.h"
#include "entity.h"
#include "game_controls.h"
#include "goal.h"
#include "player.h"
#include "renderer.h"
#include "audiomanager.h"
#include "editor/editor.h"

static game_state state = LOGO;
static game_state next_state = -1;
static bool quit_requested = false;
static float timer;

static const char win_str[] ="Congratulations!\n\nAfter a long and scary journey,\nthe cloud prince has made it home safe and sound,\nand it's all thanks to your help.\n\nMaybe he'll go on another adventure someday, when he's big and strong.\nFor now, he's earned his rest and so have you!\n\n\n\nThank you for playing!";

static const vec4 transition_color = { .r = 34.0f/256.0f, .g = 32.0f/256.0f, .b = 52.0f/256.0f, .a = 1.0f};

vec4 mouse_position;

static gltex  t_logo;
extern font f_menu;
gltex  t_back;
gltex  t_front;
static gltex  t_title;
static gltex  t_win;
static gltex  t_pause;
extern gltex  t_bonus_icon;
audio_source a_windbgm;
static audio_source a_pause;
static audio_source a_title;
static audio_source a_ending;
extern audio_source a_activate;
static audio_source a_switch;

static text paused_text;
static text counter_text;
static text win_text;

static bool in_transition = false;
static bool is_paused = false;
static float transition_time = 0.0f;
static float win_timer = 0.0f;
static transition_target next_target = NULL;

void* window;

camera c_main;
shader s_default;
shader s_transition;
static unlock_status unlocked_levels[LEVEL_COUNT] = {
    UNLOCKED,
    [1 ... LEVEL_COUNT - 1] = LOCKED
};
static int current_level = 0;
static stage_color current_color = COLOR_BLUE;

extern axis_id   a_player_motion;
extern action_id a_primary;
extern action_id a_quit;

static menu_entry main_menu_entries[4] = {
    {
        .name = "Start Game",
        .active = true
    },
    {
        .name = "Level Select",
        .active = true
    },
    {
        .name = "Options",
        .active = true
    },
    {
        .name = "Quit",
        .active = true
    },
};
static menu* main_menu = NULL;
static menu_entry pause_menu_entries[4] = {
    {
        .name = "Resume",
        .active = true
    },
    {
        .name = "Options",
        .active = true
    },
    {
        .name = "Exit Level",
        .active = true
    },
    {
        .name = "Quit",
        .active = true
    },
};
static menu* pause_menu = NULL;
static menu_entry options_menu_entries[3] = {
    {
        .name = "Music Volume",
        .active = true
    },
    {
        .name = "Sound Volume",
        .active = true
    },
    {
        .name = "Back",
        .active = true
    },
};
static menu* options_menu;
static menu_entry levels_menu_entries[LEVEL_COUNT + 1];
static menu* levels_menu;

void options_menu_activate(int selected, __attribute__((__unused__)) void* user) {
    if(selected == 2)
        close_menu(false);
}

void pause_menu_activate(int selected, void* user) {
    switch(selected) {
        case 0:
            close_menu(true);
            is_paused = false;
            break;
        case 2:
            transition_to_state(MENU);
            is_paused = false;
            break;
        case 3:
            quit_requested = true;
            break;

    }
}

void levels_menu_activate(int selected, void* user) {
    if(selected == LEVEL_COUNT) {
        close_menu(false);
        return;
    }

    current_level = selected;
    transition_to_state(GAME);
}

void main_menu_activate(int selected, void* user) {
    switch(selected) {
        case 0:
            current_level = 0;
            transition_to_state(GAME);
            break;
        case 3:
            quit_requested = true;
            break;
    }
}

void update_logo(float dt) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    timer += dt;

    vec4 color = color_white;
    color.a = lerp(0.0f, 1.0f, timer) + lerp(0.0f, -1.0f, timer - 2.5);
    draw_texture(mat4_ident, t_logo, color);

    if(timer >= 4.0f)
        set_state(MENU);
}

void update_menu(float dt) {
    glUseProgram(s_default.id);

    draw_texture(mat4_translate(mat4_ident, (vec3){.z=2}), t_back, color_white);
    draw_texture(mat4_translate(mat4_ident, (vec3){.y=-140}), t_title, color_white);
    draw_texture(mat4_translate(mat4_ident, (vec3){.z=1}), t_front, color_white);

    update_menus(dt);

    shader_bind_uniform_name(s_default, "u_color", color_white);
    vec3 v = (vec3){.x=628, .y=330, .z=-1};
    int count = 0;
    for(int i = 0; i < LEVEL_COUNT; ++i)
        if(unlocked_levels[i] == STARRED)
            count++;
    text_set_str(counter_text, "%2d/%2d", count, LEVEL_COUNT);
    draw_text(v, counter_text, color_white);
    v.x -= 78;
    v.y += 6;
    draw_texture_ui(mat4_translate(mat4_ident, v), t_bonus_icon, color_white);
}

void update_game(float dt) {
    glUseProgram(s_default.id);

    draw_texture(mat4_translate(mat4_ident, (vec3){.z=1}), t_back, color_white);
    draw_texture(mat4_translate(mat4_ident, (vec3){.z=-1}), t_front, color_white);
    update_player(dt);

    update_entities(player_get_state() == PLAYER_GROUND ? 0 : dt, false);

    draw_rails();

    if(is_paused) {
        vec3 v = (vec3){.y=-140, .z=-1};
        draw_texture_ui(mat4_translate(mat4_ident, v), t_pause, color_white);
        v.y += 120;
        draw_text(v, paused_text, color_white);
        update_menus(dt);
    }
}

void update_win(float dt) {
    glUseProgram(s_default.id);

    glClearColor(vec4_decomp(transition_color));

    win_timer += dt * (action_is_active(a_primary) ? 10 : 1);

    float c_val = min(lerp(1, 0, (win_timer - 1) * 0.5), 1) * 0.4;
    float c_val_2 = min(lerp(1, 0, (win_timer - 28) * 0.5), 1);
    float t_val = lerp(390, -760, (win_timer - 3) * 0.04);

    vec4 col = color_white;
    col.rgb = vec3_mul(col.rgb, 0.6 + c_val);
    col.a = c_val_2;

    draw_texture(mat4_ident, t_win, col);

    draw_text((vec3){.x=0, .y=t_val, .z=-1}, win_text, color_white);

    if(action_is_triggered(a_quit) || win_timer > 31)
        transition_to_state(MENU);
}

void init_game(void* win) {
    init_io();
    init_renderer(win);
    init_audiomanager();

    window = win;
    srand(time(NULL));

    c_main = window_create_2d_camera(win);

    init_controls();
    init_menus(win);

    init_player();

    init_bonus();
    init_goal();
    init_spikes();

    init_clouds();
    init_rails();
    init_entities();

    setlocale(LC_NUMERIC, "C");

#ifdef BUILD_EDITOR
    init_editor();
#endif

    char* path = assets_path("logo.png", NULL);
    t_logo     = load_texture_gl(path);
    sfree(path);

    path = assets_path("GrassBack.png", NULL);
    t_back = load_texture_gl(path);
    sfree(path);
    path = assets_path("GrassFront.png", NULL);
    t_front = load_texture_gl(path);
    sfree(path);
    path = assets_path("Title.png", NULL);
    t_title = load_texture_gl(path);
    sfree(path);
    path = assets_path("Win.png", NULL);
    t_win = load_texture_gl(path);
    sfree(path);
    path = assets_path("Pause.png", NULL);
    t_pause = load_texture_gl(path);
    sfree(path);

    path = assets_path("Wind.wav", NULL);
    a_windbgm = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Pause.wav", NULL);
    a_pause = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Title.wav", NULL);
    a_title = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Ending.wav", NULL);
    a_ending = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Switch.wav", NULL);
    a_switch = load_audio_source(path, true);
    sfree(path);

    paused_text = text_new(f_menu, "Paused");
    text_set_align(paused_text, TEXT_ALIGN_CENTER);

    counter_text = text_new(f_menu, "%2d/%2d", 0, LEVEL_COUNT);
    text_set_align(counter_text, TEXT_ALIGN_RIGHT);

    win_text = text_new(f_menu, win_str);
    text_set_align(win_text, TEXT_ALIGN_TOP);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    load_options();

    options_menu = create_menu(options_menu_entries, 3, as_event(menu_select_event, options_menu_activate, NULL));
    options_menu_entries[0].attached_slider = msalloc(slider);
    options_menu_entries[0].attached_slider->min = 0;
    options_menu_entries[0].attached_slider->max = 1;
    options_menu_entries[0].attached_slider->value = get_volume(true);
    options_menu_entries[0].attached_slider->value_changed = NULL;
    options_menu_entries[0].attached_slider->value_set = NULL;
    bind_event(options_menu_entries[0].attached_slider->value_changed, as_event(slider_event, set_volume, (void*)1));
    bind_event(options_menu_entries[0].attached_slider->value_set, as_event(slider_event, save_options, NULL));
    options_menu_entries[1].attached_slider = msalloc(slider);
    options_menu_entries[1].attached_slider->min = 0;
    options_menu_entries[1].attached_slider->max = 1;
    options_menu_entries[1].attached_slider->value = get_volume(false);
    options_menu_entries[1].attached_slider->value_changed = NULL;
    options_menu_entries[1].attached_slider->value_set = NULL;
    bind_event(options_menu_entries[1].attached_slider->value_changed, as_event(slider_event, set_volume, NULL));
    bind_event(options_menu_entries[1].attached_slider->value_set, as_event(slider_event, save_options, NULL));
    options_menu->margin = 200;

    pause_menu = create_menu(pause_menu_entries, 4, as_event(menu_select_event, pause_menu_activate, NULL));
    pause_menu->margin = 200;
    pause_menu_entries[1].submenu = options_menu;

    for(int i = 0; i < LEVEL_COUNT; ++i) {
        levels_menu_entries[i].name = saprintf("Level %d", i + 1);
        levels_menu_entries[i].active = unlocked_levels[i];
    }
    levels_menu_entries[LEVEL_COUNT].name = "Back";
    levels_menu_entries[LEVEL_COUNT].active = true;
    levels_menu = create_menu(levels_menu_entries, LEVEL_COUNT + 1, as_event(menu_select_event, levels_menu_activate, NULL));
    levels_menu->margin = 200;

    main_menu = create_menu(main_menu_entries, 4, as_event(menu_select_event, main_menu_activate, NULL));
    main_menu->margin = 200;
    main_menu_entries[2].submenu = options_menu;
    main_menu_entries[1].submenu = levels_menu;

    load_game();
}

void cleanup_game() {
    cleanup_io();
    cleanup_renderer();
    cleanup_audiomanager();
    cleanup_menus();
    cleanup_controls();

    // Cleanup systems
    cleanup_entities(true);
    cleanup_entities(false);
    cleanup_rails_final();

    // Cleanup entity assets
    cleanup_player();
    cleanup_bonus();
    cleanup_clouds();
    cleanup_goal();
    cleanup_spikes();
}

bool loop(float dt) {
    if(in_transition) {
        transition_time += dt;
        if(transition_time >= 1.25f) {
            transition_time = 0.0f;
            in_transition = false;
        } else if(next_target && transition_time >= 0.5f) {
            next_target();
            next_target = NULL;
        } else if(next_state != -1 && transition_time >= 0.5f) {
            set_state(next_state);
            next_state = -1;
        }
    }
    update_audio(dt);
    begin_frame();

    switch(state) {
        case LOGO:
            update_logo(dt);
            break;
        case MENU:
            update_menu(dt);
            break;
        case GAME:
            update_game(dt);
            break;
        case EDITOR:
#ifdef BUILD_EDITOR
            update_editor(dt);
#endif
            break;
        case WIN:
            update_win(dt);
            break;
    }

    end_frame(transition_time);

    return !quit_requested;
}

bool is_game_paused() {
    return is_paused;
}

void on_esc(action_id action, void* user) {
    if(state == GAME) {
        if(is_paused) {
            is_paused = false;
            play_audio(a_activate, false, false);
            close_menu(true);
        } else {
            is_paused = true;
            play_audio(a_pause, false, false);
            set_menu(pause_menu, false);
        }
    } else if(state == MENU || state == LOGO) {
        quit_requested = true;
    }
}

game_state get_state() {
    return state;
}

void set_state(game_state s) {
    state = s;
    glClearColor(95.0 / 255.0, 205.0 / 255.0, 228.0 / 255.0, 1);

    switch(state) {
        case MENU:
            transform_translate(camera_get_transform(c_main), (vec2){.y=0}, false);
            set_menu(main_menu, false);
            play_audio(a_title, true, true);
            break;
        case GAME:
            close_menu(true);
#ifdef BUILD_EDITOR
            instantiate_debug_entities();
            current_color = COLOR_BLUE;
            transform_translate(camera_get_transform(c_main), (vec2){ .y = 0 }, false);
            reset_player();
            play_audio(a_windbgm, true, true);
#elif BUILD_GAME
            load_level_number(current_level + 1);
#endif
            break;
        case EDITOR:
            cleanup_entities(false);
            play_audio(NULL, true, false);
            break;
        case WIN:
            transform_translate(camera_get_transform(c_main), (vec2){.y=0}, false);

            play_audio(a_ending, true, false);
            break;
        default:
            break;
    }
}

void resize_camera(int w, int h) {
    projection_settings settings = (projection_settings) {
        .dims = {
            .x = w,
            .y = h,
            .z = -1,
            .w = 100
        },
        .is_ortho = true
    };
    camera_set_projection(c_main, settings);
}

void reset_level() {
#ifdef BUILD_EDITOR
    cleanup_entities(false);
    reset_player();

    instantiate_debug_entities();
    current_color = COLOR_BLUE;

    play_audio(a_windbgm, true, false);
#elif BUILD_GAME
    load_level_number(current_level + 1);
#endif
}

void load_level_number(uint8 level) {
    char buf[13] = { 0 };
    snprintf(buf, 13, "level%d.lev", level);

    char* path = assets_path(buf, NULL);
    load_level(path, false);
    sfree(path);
}

void game_advance_level() {
#ifdef BUILD_GAME
    if(unlocked_levels[current_level] != STARRED && is_bonus_collected())
        set_unlocked(current_level, STARRED);

    current_level++;

    if(unlocked_levels[current_level] == LOCKED)
        set_unlocked(current_level, UNLOCKED);

    save_game();

    if(current_level == LEVEL_COUNT)
        set_state(WIN);
    else
        load_level_number(current_level + 1);
#else
    reset_level();
#endif
}

bool transition_to(transition_target target) {
    bool prev = in_transition;
    in_transition = true;
    next_target = target;

    return !prev;
}
bool transition_to_state(game_state s) {
    bool prev = in_transition;
    in_transition = true;
    next_state = s;

    return !prev;
}

void flip_color() {
    current_color = 1 - current_color;
    play_audio(a_switch, false, false);
}
stage_color get_color() {
    return current_color;
}

unlock_status get_unlocked(uint8 level) {
    if(level < LEVEL_COUNT)
        return unlocked_levels[level];
    return LOCKED;
}
void set_unlocked(uint8 level, unlock_status status) {
    if(level < LEVEL_COUNT) {
        unlocked_levels[level] = status;

        if(levels_menu) {
            for(int i = 0; i < levels_menu->entry_count - 1; ++i) {
                levels_menu->entries[i].active = unlocked_levels[i] > LOCKED;
                levels_menu->entries[i].special = unlocked_levels[i] == STARRED;
            }
        }
    }
}
