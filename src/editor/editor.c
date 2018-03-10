#include "camera.h"
#include "color.h"
#include "debug_draw.h"
#include "input.h"
#include "interpolate.h"
#include "log/log.h"
#include "matrix.h"
#include "mesh.h"
#include "shader.h"
#include "spriteset.h"
#include "sprite.h"
#include "texture.h"
#include "transform.h"
#include "window.h"

#include <math.h>

#include "editor/editor.h"
#include "bonus.h"
#include "cloud.h"
#include "entity.h"
#include "rail.h"
#include "renderer.h"
#include "spike.h"
#include "gameloop.h"
#include "goal.h"

static tool current_tool = TOOL_GOAL;
static stage_color current_color = COLOR_NONE;
static bool is_tool_enabled = false;
static cursor_state current_state = CURSOR_DEFAULT;

extern vec4 mouse_position;
static vec4 mouse_position_unlocked;
extern void* window;
extern camera c_main;

extern gltex  t_back;
extern gltex  t_front;
extern spriteset s_spike;
extern spriteset s_cloud;
extern spriteset s_strong_cloud;
extern spriteset s_bonus;
extern spriteset s_goal;
extern shader s_default;

static sprite spike_sprite;
static sprite cloud_sprite;
static sprite strong_cloud_sprite;
static sprite bonus_sprite;
static sprite goal_sprite;
extern gltex t_stopper;

static entity* selected_entity = NULL;
static rail* selected_rail = NULL;
static bool selected_rail_b_end = false;

bool rail_placed = false;
static rail edit_rail;
static bool do_snap_to_rail = false;

action_id a_place;
action_id a_precise;
action_id a_delete;

void place_request(action_id id, void* user) {
    if(get_state() == EDITOR && mouse_position.y < 112 && is_tool_enabled) {
        entity* e = NULL;

        switch(current_tool) {
            case TOOL_MOVE:
                selected_entity = pick_entity(mouse_position_unlocked.xy, true);
                selected_entity->rail_id = -1;
                current_state = CURSOR_DRAG;
                break;
            case TOOL_RAIL:
                selected_rail = pick_rail(mouse_position_unlocked.xy);
                if(selected_rail && !selected_rail->is_circle)
                    selected_rail_b_end = vec2_len_squared(vec2_sub(mouse_position_unlocked.xy, selected_rail->a)) > vec2_len_squared(vec2_sub(mouse_position_unlocked.xy, selected_rail->b));
                current_state = CURSOR_DRAG;
                break;
            case TOOL_BONUS:
                e = create_bonus(mouse_position.xy, true);
                break;
            case TOOL_GOAL:
                e = create_goal(mouse_position.xy, true);
                break;
            case TOOL_CLOUD:
                e = create_cloud(mouse_position.xy, current_color == COLOR_NONE ? CLOUD_NORMAL : CLOUD_RED + current_color, true);
                break;
            case TOOL_STRONG_CLOUD:
                e = create_cloud(mouse_position.xy, CLOUD_STRONG, true);
                break;
            case TOOL_SPIKE:
                e = create_spike(mouse_position.xy, current_color, true);
                break;
            case TOOL_CIRCULAR_RAIL:
            case TOOL_STRAIGHT_RAIL:
                rail_placed = true;
                edit_rail.is_circle = current_tool == TOOL_CIRCULAR_RAIL;
                edit_rail.a = mouse_position.xy;
                edit_rail.b = mouse_position.xy;
                break;
            default:
                break;
        }

        if(selected_rail && e)
            attach_entity(e, rail_to_id(selected_rail), snap_to_rail(rail_to_id(selected_rail), mouse_position_unlocked.xy));
    }
}

void delete_request(action_id id, void* user) {
    if(get_state() == EDITOR && is_tool_enabled && selected_entity == NULL && selected_rail == NULL) {
        if(current_tool != TOOL_RAIL && current_tool != TOOL_STRAIGHT_RAIL && current_tool != TOOL_CIRCULAR_RAIL) {
            entity* en = pick_entity(mouse_position_unlocked.xy, true);

            if(en)
                en->to_delete = true;
        } else {
            rail* r = NULL;

            if((r = pick_rail(mouse_position_unlocked.xy))) {
                detach_entities_for_delete(rail_to_id(r));
                remove_rail(r);
            }
        }
    }
}

void update_editor(float dt) {
    draw_texture(mat4_translate(mat4_ident, (vec3){.z=1}), t_back, color_white);
    draw_texture(mat4_translate(mat4_ident, (vec3){.z=-1}), t_front, color_white);

    draw_rails();
    update_entities(dt, true);

    if(is_tool_enabled) {
        update_tool(dt);
        if(selected_entity == NULL) {
            if(current_tool == TOOL_MOVE && pick_entity(mouse_position_unlocked.xy, true))
                current_state = CURSOR_CAN_DRAG;
            else
                current_state = CURSOR_DEFAULT;
        }
        if(selected_rail == NULL) {
            if(current_tool == TOOL_RAIL && pick_rail(mouse_position_unlocked.xy))
                current_state = CURSOR_CAN_DRAG;
            else
                current_state = CURSOR_DEFAULT;
        }

        if(rail_placed) {
            generate_rail_mesh(&edit_rail);
            draw_rail(&edit_rail, NULL);
        }
    }

    vt_pc start = (vt_pc){ .position = { .x = -600, .y = -1000 }, .color = color_red };
    vt_pc end = (vt_pc){ .position = { .x = 600, .y = -1000 }, .color = color_red };
    debug_draw_line(get_camera_matrix(), start, end, 3);

    vec2 dims = get_window_dims(window);
    glViewport(0, 0, vec2_decomp(dims));
}

void set_tool(tool t) {
    current_tool = t;

    selected_rail = NULL;
}

void set_tool_color(stage_color color) {
    current_color = color;
    if(color != COLOR_NONE && color != get_color())
        flip_color();

    switch(color) {
        case COLOR_NONE:
            sprite_set_animation(cloud_sprite, NULL, true);
            sprite_set_animation(spike_sprite, NULL, true);
            break;
        case COLOR_RED:
            sprite_set_animation(cloud_sprite, "Red", true);
            sprite_set_animation(spike_sprite, "Red", true);
            break;
        case COLOR_BLUE:
            sprite_set_animation(cloud_sprite, "Blue", true);
            sprite_set_animation(spike_sprite, "Blue", true);
            break;
    }
}

void update_tool(float dt) {
    vec2 screen_mouse_position = get_mouse_position(window);
    mouse_position.xy = screen_mouse_position;
    mouse_position.z = -0.1;
    mouse_position.w = 1;

    float mouse_dist = fabs(screen_mouse_position.y);
    if(mouse_dist > 0.8) {
        transform_translate(camera_get_transform(c_main), (vec2){ .y = (screen_mouse_position.y - (screen_mouse_position.y / mouse_dist * 0.8)) * -3000 * dt }, true);
        if(camera_get_position(c_main).y > 0)
            transform_translate(camera_get_transform(c_main), (vec2){ 0 }, false);
    }

    mouse_position = mat4_mul(mat4_invert(camera_get_vp(c_main)), mouse_position);
    float grid_size = (action_is_active(a_precise) ? 12 : 48);
    mouse_position_unlocked = mouse_position;
    mouse_position.x = floor(mouse_position.x / grid_size + 0.5f) * grid_size;
    mouse_position.y = floor(mouse_position.y / grid_size + 0.5f) * grid_size;

    if(do_snap_to_rail && current_tool != TOOL_RAIL && current_tool != TOOL_CIRCULAR_RAIL && current_tool != TOOL_STRAIGHT_RAIL) {
        selected_rail = closest_rail(mouse_position_unlocked.xy);
        if(selected_rail)
            mouse_position.xy = rail_position(rail_to_id(selected_rail), snap_to_rail(rail_to_id(selected_rail), mouse_position_unlocked.xy));
    }

    vec4 color = color_white;
    color.a = 0.7f;

    if(mouse_position.y < 112) {
        switch(current_tool) {
            case TOOL_MOVE:
                if(selected_entity) {
                    transform_translate(selected_entity->trans, mouse_position.xy, false);

                    if(!action_is_active(a_place)) {
                        if(selected_rail)
                            attach_entity(selected_entity, rail_to_id(selected_rail), snap_to_rail(rail_to_id(selected_rail), mouse_position_unlocked.xy));
                        selected_entity = NULL;
                    }
                }
            break;
            case TOOL_BONUS:
                draw_sprite(mat4_translate(mat4_ident, mouse_position), bonus_sprite, color);
            break;
            case TOOL_GOAL:
                sprite_update(goal_sprite, dt);
                draw_sprite(mat4_translate(mat4_ident, mouse_position), goal_sprite, color);
            break;
            case TOOL_CLOUD:
                sprite_update(cloud_sprite, dt);
                draw_sprite(mat4_translate(mat4_ident, mouse_position), cloud_sprite, color);
            break;
            case TOOL_STRONG_CLOUD:
                sprite_update(strong_cloud_sprite, dt);
                draw_sprite(mat4_translate(mat4_ident, mouse_position), strong_cloud_sprite, color);
            break;
            case TOOL_SPIKE:
                sprite_update(spike_sprite, dt);
                draw_sprite(mat4_translate(mat4_ident, mouse_position), spike_sprite, color);
            break;
            case TOOL_STRAIGHT_RAIL:
            case TOOL_CIRCULAR_RAIL:
                draw_texture(mat4_translate(mat4_ident, mouse_position), t_stopper, color);

                if(edit_rail.is_circle)
                    edit_rail.b.y = edit_rail.a.y + vec2_len(vec2_sub(mouse_position.xy, edit_rail.a));
                else
                    edit_rail.b = mouse_position.xy;

                if(!action_is_active(a_place) && rail_placed) {
                    rail_placed = false;
                    if(edit_rail.is_circle)
                        create_circle_rail(edit_rail.a, fabs(edit_rail.b.y - edit_rail.a.y));
                    else
                        create_straight_rail(edit_rail.a, edit_rail.b);
                }
            break;
            case TOOL_RAIL:
                if(selected_rail) {
                    if(selected_rail->is_circle)
                        selected_rail->b.y = selected_rail->a.y + vec2_len(vec2_sub(mouse_position.xy, selected_rail->a));
                    else if(selected_rail_b_end)
                        selected_rail->b = mouse_position.xy;
                    else
                        selected_rail->a = mouse_position.xy;

                    generate_rail_mesh(selected_rail);

                    if(!action_is_active(a_place))
                        selected_rail = NULL;
                }
            break;
            default:
            break;
        }
    }

}

void set_tool_enabled(bool enabled) {
    is_tool_enabled = enabled;
}
void set_snap_to_rail(bool snap) {
    do_snap_to_rail = snap;

    if(!do_snap_to_rail && current_tool != TOOL_RAIL && current_tool != TOOL_CIRCULAR_RAIL && current_tool != TOOL_STRAIGHT_RAIL)
        selected_rail = NULL;
}

void init_editor() {
    a_place = input_add_mouse_button_action(MB_LEFT, as_event(action_event, place_request, (void*)0));
    a_delete = input_add_mouse_button_action(MB_RIGHT, as_event(action_event, delete_request, (void*)0));
    a_precise = input_add_key_action(K_LEFT_SHIFT, NULL);

    spike_sprite = sprite_new(s_spike);
    sprite_set_playing(spike_sprite, true);

    cloud_sprite = sprite_new(s_cloud);
    sprite_set_playing(cloud_sprite, true);

    strong_cloud_sprite = sprite_new(s_strong_cloud);
    sprite_set_playing(strong_cloud_sprite, true);

    bonus_sprite = sprite_new(s_bonus);
    sprite_set_playing(bonus_sprite, true);

    goal_sprite = sprite_new(s_goal);
    sprite_set_playing(goal_sprite, true);

    edit_rail.m = mesh_new_type(1, VT_POSITION/* | VT_TEXTURE*/, NULL);
}

cursor_state get_cursor_state() {
    return current_state;
}
