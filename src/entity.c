#include "entity.h"

#include "color.h"
#include "container/array.h"
#include "gameloop.h"
#include "aabb.h"
#include "log/log.h"
#include "rail.h"
#include "renderer.h"
#include <string.h>

static uarray entities;
static uarray debug_entities;

typedef struct update_data {
    float dt;
    bool debug;
} update_data;

iter_result delete_entity(void* e, void* user) {
    entity* en = (entity*)e;
    bool debug = *(bool*)user;
    call_event(en->delete, en, debug);
    if(en->update)
        sfree(en->update);
    if(en->delete)
        sfree(en->delete);

    transform_free(en->trans);
#ifdef BUILD_EDITOR
    if(debug)
        sprite_free(en->spr, false);
#elif BUILD_GAME
    sprite_free(en->spr, false);
#endif

    sfree(en);

    return iter_delete;
}

iter_result update_entity_single(void* e, void* user) {
    entity* en = (entity*)e;
    aabb_2d hitbox = {
        .position = vec2_sub(transform_get_position(en->trans).xy, en->dims),
        .dimensions = vec_mul(en->dims, 2)
    };
    update_data* data = (update_data*)user;

    if(!is_game_paused()) {
        update_rail(en, data->debug ? 0 : data->dt);
        call_event(en->update, data->dt, data->debug, en, hitbox);
        sprite_update(en->spr, data->dt);
    }

    draw_sprite(transform_get_matrix(en->trans), en->spr, color_white);
    draw_debug_rect(hitbox, color_white);

    if(en->to_delete)
        return delete_entity(en, &data->debug);

    return iter_continue;
}

iter_result write_entity(void* e, void* user) {
    entity* en = (entity*)e;
    FILE* f = (FILE*)user;
    vec2 v = transform_get_position(en->trans).xy;

    fprintf(f, "%c "vec2_printstr"\n", en->character, vec2_decomp(v));
    if(en->rail_id != -1)
        fprintf(f, "| %d %f\n", en->rail_id, en->rail_position);

    return iter_continue;
}

iter_result create_from_debug(void* e, __attribute__((__unused__)) void* user) {
    entity* en = (entity*)e;

    entity* en2 = create_entity(transform_get_position(en->trans).xy, en->dims, sprite_new(sprite_get_data(en->spr)), en->update, en->delete, en->character, false);
    sprite_set_animation(en2->spr, sprite_get_anim_id(en->spr), true);
    en2->rail_position = en->rail_position;
    en2->rail_id = en->rail_id;
    sprite_set_orientation(en2->spr, sprite_get_orientation(en->spr));

    return iter_continue;
}

iter_result detach_entity(void* e, void* user) {
    entity* en = (entity*)e;

    int id = *(int*)user;
    if(en->rail_id == id) {
        en->rail_position = 0;
        en->rail_id = -1;
    } else if(en->rail_id > id) {
        en->rail_id--;
    }

    return iter_continue;
}

bool match_by_char(void* o1, void* o2, __attribute__((__unused__)) void* user) {
    entity* e = o1;
    char* c = o2;

    return e->character == *c;
}

bool match_by_pick(void* o1, void* o2, __attribute__((__unused__)) void* user) {
    entity* e = o1;
    vec2 v = *(vec2*)o2;

    aabb_2d rc = {
        .position = vec2_sub(transform_get_position(e->trans).xy, e->dims),
        .dimensions = vec_mul(e->dims, 2)
    };

    return aabb_intersects_point(rc, v);
}

void init_entities() {
    entities = uarray_new(10);
    debug_entities = uarray_new(10);
}

entity* create_entity(vec2 pos, vec2 dims, sprite s, entity_update_event* ev, entity_delete_event* del, char character, bool debug) {
    entity e = {
        .trans = transform_new(),
        .dims = dims,
        .character = character,
        .spr = s,
        .rail_id = -1,
        .rail_direction = 1
    };

    bind_event(e.update, ev);
    bind_event(e.delete, del);

    transform_translate(e.trans, pos, false);

    uarray a = debug ? debug_entities : entities;

    array_copyadd_simple(a, e);

    return array_get(a, array_size(a) - 1);
}

void attach_entity(entity* e, int rail, float pos) {
    e->rail_id = rail;
    e->rail_position = pos;
    update_rail(e, 0);
}

void detach_entities_for_delete(int rail) {
    array_foreach(debug_entities, detach_entity, &rail);
}

void update_entities(float dt, bool debug) {
    update_data data = { .dt = dt, .debug = debug };
    array_foreach(debug ? debug_entities : entities, update_entity_single, &data);
}

entity* get_entity(char c, bool debug) {
    int index = array_findp(debug ? debug_entities : entities, &c, match_by_char, NULL);
    if(index != INVALID_INDEX)
        return array_get(debug ? debug_entities : entities, index);
    return NULL;
}

entity* pick_entity(vec2 pos, bool debug) {
    int index = array_findp(debug ? debug_entities : entities, &pos, match_by_pick, NULL);
    if(index != INVALID_INDEX)
        return array_get(debug ? debug_entities : entities, index);
    return NULL;
}

void instantiate_debug_entities() {
    array_foreach(debug_entities, create_from_debug, NULL);
}

void cleanup_entities(bool debug) {
    array_foreach(debug ? debug_entities : entities, delete_entity, &debug);
}

void save_entities(FILE* infile) {
    array_foreach(debug_entities, write_entity, infile);
}
