#ifndef GAME_LOOP_H
#define GAME_LOOP_H
#include "control.h"
#include "types.h"

#define LEVEL_COUNT 10

typedef enum game_state {
    LOGO,
    MENU,
    GAME,
    WIN,
    EDITOR
} game_state;
typedef enum stage_color {
    COLOR_RED = 0,
    COLOR_BLUE = 1,
    COLOR_NONE = 2
} stage_color;
typedef enum unlock_status {
    LOCKED = 0,
    UNLOCKED = 1,
    STARRED = 2
} unlock_status;

typedef void (*transition_target)();

void init_game(void* win);
void cleanup_game();
bool loop(float dt);
bool is_game_paused();
void on_esc(action_id action, void* user);
void move_cursor(int64 diff);
void resize_camera(int w, int h);
game_state get_state();
void set_state(game_state state);
void load_level(const char* path, bool debug);
void reset_level();
void load_level_number(uint8 level);
void save_level(const char* path);
void game_advance_level();
bool transition_to(transition_target target);
bool transition_to_state(game_state s);
void flip_color();
unlock_status get_unlocked(uint8 level);
void set_unlocked(uint8 level, unlock_status status);
stage_color get_color();

#endif
