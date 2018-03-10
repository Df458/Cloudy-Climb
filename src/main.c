#include "mainloop.h"
#include "paths.h"
#include "window.h"

#include "audiomanager.h"
#include "gameloop.h"
#include "renderer.h"

static void* win;

bool loop_fn(mainloop l, float dt) {

    bool should_continue = loop(dt);
    window_redraw(win);
    return !window_should_close(win) && should_continue;
}

int main() {
    win = window_new_default(1280, 720, "Cloudy Climb");

    init_game(win);

    mainloop_create_run(loop_fn);

    cleanup_game();

    window_free_final(win);

    return 0;
}
