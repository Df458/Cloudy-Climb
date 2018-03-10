#include "aabb.h"
#include "audio_loader.h"
#include "audio_source.h"
#include "paths.h"
#include "sprite_loader.h"
#include "spriteset.h"

#include "bonus.h"
#include "player.h"
#include "audiomanager.h"

spriteset s_bonus;
static bool bonus_collected = false;
static audio_source a_bonus;

void update_bonus(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    if(!debug && !bonus_collected && player_check_collision(hitbox)) {
        sprite_set_animation(e->spr, "Collected", false);
        bonus_collected = true;
        play_audio(a_bonus, false, false);
    }
}

void init_bonus() {
    char* path = assets_path("Bonus.xml", NULL);
    s_bonus = load_spriteset(path);
    sfree(path);
    path = assets_path("Bonus.wav", NULL);
    a_bonus = load_audio_source(path, true);
    sfree(path);
}
void cleanup_bonus() {
    spriteset_free(s_bonus);
    audio_source_free(a_bonus);
}
bool is_bonus_collected() {
    return bonus_collected;
}
entity* create_bonus(vec2 pos, bool debug) {
    bonus_collected = false;
    entity* e = get_entity('b', debug);
    if(!e) {
        sprite spr = sprite_new(s_bonus);
        sprite_set_playing(spr, true);
        return create_entity(pos, (vec2){.x=32, .y=32}, spr, as_event(entity_update_event, update_bonus, spr), NULL, 'b', debug);
    } else {
        transform_translate(e->trans, pos, false);
        return e;
    }
}
