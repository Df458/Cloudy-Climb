#include "input.h"

#include "game_controls.h"
#include "gameloop.h"
#include "menu.h"

axis_id   a_player_motion;
action_id a_primary;
action_id a_quit;

void selection_changed(__attribute__((__unused__)) action_id action, void* user) {
    scroll_menu((int64)user);
}

void init_controls() {
    input_add_key_action(K_RIGHT, as_event(action_event, selection_changed, (void*)1));
    input_add_key_action(K_LEFT, as_event(action_event, selection_changed, (void*)-1));

#ifdef BUILD_GAME
    input_add_key_action(K_ESCAPE, as_event(action_event, on_esc, NULL));
#endif

    a_primary = input_add_key_action(K_SPACE, NULL);
    input_bind_key_action(K_ENTER, a_primary);
    input_bind_key_action(K_Z, a_primary);

    a_player_motion = input_add_key_axis(K_RIGHT, 1, 1, true);
    input_bind_key_axis(K_LEFT, a_player_motion, -1);
}
void cleanup_controls() {
    clear_input_bindings();
}
