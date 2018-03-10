#ifndef ENTITY_H
#define ENTITY_H
#include "sprite.h"
#include "transform.h"
#include "vector.h"
#include <stdio.h>

declare(struct, entity)

event(entity_update_event, float dt, bool debug, entity* e, aabb_2d hitbox)
event(entity_delete_event, entity* e, bool debug)

typedef struct entity {
    transform trans;
    vec2 dims;
    entity_update_event* update;
    entity_delete_event* delete;
    char character;
    bool to_delete;
    sprite spr;

    int rail_id;
    float rail_position;
    float rail_direction;
} entity;

void init_entities();
entity* create_entity(vec2 pos, vec2 dims, sprite s, entity_update_event* ev, entity_delete_event* del, char character, bool debug);
void update_entities(float dt, bool debug);
void attach_entity(entity* e, int rail, float pos);
void detach_entities_for_delete(int rail);

entity* get_entity(char c, bool debug);
entity* pick_entity(vec2 pos, bool debug);

void instantiate_debug_entities();

void cleanup_entities(bool debug);

void save_entities(FILE* infile);

#endif
