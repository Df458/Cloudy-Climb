#include "paths.h"
#include "sprite_loader.h"
#include "spriteset.h"

#include "spike.h"
#include "player.h"

spriteset s_spike;

void init_spikes() {
    char* path = assets_path("SpikeBall.xml", NULL);
    s_spike = load_spriteset(path);
    sfree(path);
}
void cleanup_spikes() {
    spriteset_free(s_spike);
}

void update_spike(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, void* user) {
    stage_color c = get_color();
    stage_color c2 = (stage_color)user;

    bool is_active = c2 == 2 || c2 == c;
    if(c2 != COLOR_NONE)
        sprite_set_orientation(e->spr, is_active ? 0 : 1);

    if(!debug && is_active && player_check_collision(hitbox))
        kill_player();
}
entity* create_spike(vec2 pos, stage_color color, bool debug) {
    sprite spr = sprite_new(s_spike);
    sprite_set_playing(spr, true);
    char ch = 's';
    switch(color) {
        case COLOR_NONE:
            ch = 's';
            break;
        case COLOR_RED:
            sprite_set_animation(spr, "Red", true);
            ch = 'R';
            break;
        case COLOR_BLUE:
            sprite_set_animation(spr, "Blue", true);
            ch = 'U';
            break;
    }
    return create_entity(pos, (vec2){.x=32, .y=32}, spr, as_event(entity_update_event, update_spike, (void*)color), NULL, ch, debug);
}
