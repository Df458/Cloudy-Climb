#ifndef RAIL_H
#define RAIL_H
#include "entity.h"
#include "container/delegate.h"
#include "mesh.h"
#include "vector.h"

typedef struct rail {
    vec2 a;
    vec2 b;
    bool is_circle;
    mesh m;
} rail;

void init_rails();
void cleanup_rails();
void cleanup_rails_final();
int create_straight_rail(vec2 start, vec2 end);
int create_circle_rail(vec2 center, float radius);
void generate_rail_mesh(rail* r);
iter_result draw_rail(void* e, void* user);
void draw_rails();
void update_rail(entity* e, float dt);
float snap_to_rail(int rail_id, vec2 v);
vec2 rail_position(int rail_id, float position);
float rail_length(int rail_id);
rail* pick_rail(vec2 pos);
rail* closest_rail(vec2 pos);
int rail_to_id(rail* r);
void remove_rail(rail* r);
void save_rails(FILE* infile);

#endif
