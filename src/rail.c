#include "rail.h"

#include "camera.h"
#include "color.h"
#include "container/array.h"
#include "debug_draw.h"
#include "interpolate.h"
#include "log/log.h"
#include "mesh.h"
#include "paths.h"
#include "renderer.h"
#include "shader_init.h"
#include "texture_loader.h"

#include <math.h>

gltex t_rail;
gltex t_stopper;

static sarray rails;

extern camera c_main;
extern shader s_default;

void init_rails() {
    char* path = assets_path("Line.png", NULL);
    t_rail = load_texture_gl(path);
    sfree(path);
    path = assets_path("Stopper.png", NULL);
    t_stopper = load_texture_gl(path);
    sfree(path);

    rails = sarray_new(4);
}

iter_result cleanup_rail(void* r, void* user) {
    mesh_free(((rail*)r)->m);

    return iter_delete;
}

void cleanup_rails() {
    array_foreach(rails, cleanup_rail, NULL);
}
void cleanup_rails_final() {
    cleanup_rails();
    glDeleteTextures(1, &t_rail.handle);
    glDeleteTextures(1, &t_stopper.handle);
}

void generate_rail_mesh(rail* r) {
    vt_pt* data = mscalloc(r->is_circle ? 82 : 4, vt_pt);
    if(r->is_circle) {
        float len = fabs(r->b.y - r->a.y);
        for(int i = 0; i < 41; ++i) {
            vec2 v = {
                .x = cos(i * 0.05 * PI),
                .y = sin(i * 0.05 * PI),
            };
            data[i * 2 + 0] = (vt_pt){
                .position = {
                    .x = v.x * (len + 8) + r->a.x,
                    .y = v.y * (len + 8) + r->a.y,
                },
                .uv = {
                    .x = 0,
                    .y = 0,
                }
            };
            data[i * 2 + 1] = (vt_pt){
                .position = {
                    .x = v.x * (len - 8) + r->a.x,
                    .y = v.y * (len - 8) + r->a.y,
                },
                .uv = {
                    .x = 0,
                    .y = 1,
                }
            };
        }
    } else {
        vec3 diff = (vec3){ .xy = vec2_sub(r->b, r->a) };
        if(vec_len_squared(diff) > 0) {
            vec2 offset = vec_mul(vec_normalize(vec3_cross(diff, vec3_forward).xy), 8);

            data[0] = (vt_pt){
                .position = (vec3){ .xy = vec_add(r->a, offset) },
                    .uv = {
                        .x = 0,
                        .y = 0,
                    }
            };
            data[1] = (vt_pt){
                .position = (vec3){ .xy = vec_sub(r->a, offset) },
                    .uv = {
                        .x = 0,
                        .y = 1,
                    }
            };
            data[2] = (vt_pt){
                .position = (vec3){ .xy = vec_add(r->b, offset) },
                    .uv = {
                        .x = 1,
                        .y = 0,
                    }
            };
            data[3] = (vt_pt){
                .position = (vec3){ .xy = vec_sub(r->b, offset) },
                    .uv = {
                        .x = 1,
                        .y = 1,
                    }
            };
        }
    }

    mesh_set_data(r->m, r->is_circle ? 82 : 4, data);
    sfree(data);
}

iter_result draw_rail(void* e, void* user) {
    rail* r = (rail*)e;

    if(!r->is_circle) {
        draw_texture(mat4_translate(mat4_ident, r->a), t_stopper, color_white);
        draw_texture(mat4_translate(mat4_ident, r->b), t_stopper, color_white);
    }

    glUseProgram(s_default.id);
    shader_bind_uniform_name(s_default, "u_color", color_white);
    shader_bind_uniform_name(s_default, "u_transform", get_camera_matrix());
    shader_bind_uniform_texture_name(s_default, "u_texture", t_rail, GL_TEXTURE0);

    mesh_render(s_default, r->m, GL_TRIANGLE_STRIP, "i_pos", VT_POSITION, "i_uv", VT_TEXTURE);

    return iter_continue;
}
iter_result match_rail_by_closest(void* r, void* m) {
    struct {
        vec2 pos;
        float distance;
        rail* r;
    } *match = m;
    rail* ra = r;

    int id = rail_to_id(ra);
    float d = vec_len_squared(vec_sub(match->pos, rail_position(id, snap_to_rail(id, match->pos))));

    if(d < match->distance || !match->r) {
        match->r = ra;
        match->distance = d;
    }

    return iter_continue;
}
iter_result write_rail(void* r, void* f) {
    rail* ra = r;

    fprintf(f, "%c "vec2_printstr" "vec2_printstr"\n", ra->is_circle ? 'O' : '-', vec2_decomp(ra->a), vec2_decomp(ra->b));

    return iter_continue;
}
bool match_rail_by_pick(void* o1, void* o2, void* user) {
    rail* r = o1;
    vec2 v = *(vec2*)o2;

    if(r->is_circle) {
        float dist = vec2_len(vec2_sub(r->a, v));
        float radius = fabs(r->b.y - r->a.y);
        return fabs(dist - radius) < 8;
    } else {
        return vec2_len(vec2_sub(r->a, v)) <= 8 || vec2_len(vec2_sub(r->b, v)) <= 8;
    }
}
void draw_rails() {
    array_foreach(rails, draw_rail, NULL);
}

int create_straight_rail(vec2 start, vec2 end) {
    rail r = (rail) {
        .a = start,
        .b = end,
        .is_circle = false,
        .m = mesh_new_type(1, VT_POSITION/* | VT_TEXTURE*/, NULL)
    };

    generate_rail_mesh(&r);

    array_copyadd_simple(rails, r);
    return array_size(rails) - 1;
}
int create_circle_rail(vec2 center, float radius) {
    rail r = (rail) {
        .a = center,
        .b = (vec2){ .x = center.x, .y = center.y - radius },
        .is_circle = true,
        .m = mesh_new_type(1, VT_POSITION | VT_TEXTURE, NULL)
    };

    generate_rail_mesh(&r);

    array_copyadd_simple(rails, r);
    return array_size(rails) - 1;
}
void update_rail(entity* e, float dt) {
    if(e->rail_id >= 0) {
        float len = rail_length(e->rail_id);
        if(!eq0(len)) {
            e->rail_position += (100.0f / len) * dt * e->rail_direction;
            if(e->rail_position > 1) {
                if(!((rail*)array_get(rails, e->rail_id))->is_circle) {
                    e->rail_position = 1 - (e->rail_position - 1);
                    e->rail_direction *= -1;
                } else {
                    e->rail_position -= 1;
                }
            } else if(e->rail_position < 0) {
                e->rail_position *= -1;
                e->rail_direction *= -1;
            }
            transform_translate(e->trans, rail_position(e->rail_id, e->rail_position), false);
        }
    }
}
float snap_to_rail(int rail_id, vec2 v) {
    rail* r = array_get(rails, rail_id);

    vec2 pos = vec2_sub(v, r->a);

    if(r->is_circle) {
        float rad = atan2(-pos.y, -pos.x);
        return (rad / (PI * 2) + 1);
    } else {
        vec2 diff = vec2_sub(r->b, r->a);
        float projected = vec2_dot(pos, diff) * (1 / vec2_len_squared(diff));
        return clamp(projected, 0, 1);
    }
}
vec2 rail_position(int rail_id, float position) {
    rail* r = array_get(rails, rail_id);

    if(r->is_circle)
        return (vec2) {
            .x = cos(PI * position * 2) * (r->b.y - r->a.y) + r->a.x,
            .y = sin(PI * position * 2) * (r->b.y - r->a.y) + r->a.y,
        };
    else
        return (vec2) {
            .x = lerp(r->a.x, r->b.x, position),
            .y = lerp(r->a.y, r->b.y, position),
        };
}
float rail_length(int rail_id) {
    rail* r = array_get(rails, rail_id);

    if(r->is_circle) {
        return fabs(r->b.y - r->a.y) * PI * 2;
    } else {
        vec2 diff = vec2_sub(r->b, r->a);
        return vec2_len(diff);
    }
}
rail* pick_rail(vec2 pos) {
    int index = array_findp(rails, &pos, match_rail_by_pick, NULL);
    if(index != INVALID_INDEX)
        return array_get(rails, index);
    return NULL;
}
rail* closest_rail(vec2 pos) {
    struct {
        vec2 pos;
        float distance;
        rail* r;
    } match;
    match.pos = pos;
    match.r = NULL;

    array_foreach(rails, match_rail_by_closest, &match);

    return match.r;
}
int rail_to_id(rail* r) {
    return array_find(rails, r);
}
void remove_rail(rail* r) {
    array_remove(rails, r);
    cleanup_rail(r, NULL);
    sfree(r);
}

void save_rails(FILE* infile) {
    array_foreach(rails, write_rail, infile);
}
