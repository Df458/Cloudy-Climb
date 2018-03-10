#include "aabb.h"
#include "paths.h"
#include "sprite_loader.h"
#include "spriteset.h"

#include "goal.h"
#include "player.h"

spriteset s_goal;

void init_goal() {
    char* path = assets_path("Goal.xml", NULL);
    s_goal = load_spriteset(path);
    sfree(path);
}
void cleanup_goal() {
    spriteset_free(s_goal);
}

void update_goal(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    if(!debug && player_check_collision(hitbox))
            player_clear_level(transform_get_position(e->trans));
}
entity* create_goal(vec2 pos, bool debug) {
    entity* e = get_entity('g', debug);
    if(!e) {
        sprite spr = sprite_new(s_goal);
        sprite_set_playing(spr, true);
        return create_entity(pos, (vec2){.x=130, .y=100}, spr, as_event(entity_update_event, update_goal, NULL), NULL, 'g', debug);
    } else {
        transform_translate(e->trans, pos, false);
        return e;
    }
}
