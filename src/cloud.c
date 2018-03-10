#include "audio_loader.h"
#include "audio_source.h"
#include "color.h"
#include "debug_draw.h"
#include "paths.h"
#include "sprite_loader.h"
#include "spriteset.h"

#include "cloud.h"
#include "gameloop.h"
#include "player.h"
#include "renderer.h"

spriteset s_cloud;
spriteset s_strong_cloud;

void init_clouds() {
    char* path = assets_path("Cloud.xml", NULL);
    s_cloud = load_spriteset(path);
    sfree(path);
    path = assets_path("Cloud_Strong.xml", NULL);
    s_strong_cloud = load_spriteset(path);
    sfree(path);
}
void cleanup_clouds() {
    spriteset_free(s_cloud);
    spriteset_free(s_strong_cloud);
}

bool cloud_check_collision(aabb_2d hitbox) {
    return player_check_collision(hitbox) && try_bounce(hitbox.position.y);
}

void cloud_draw_extents(aabb_2d hitbox) {
    vt_pc start = (vt_pc){ .position = { .x = hitbox.position.x, .y = hitbox.position.y - 120 - 48 }, .color = color_red };
    vt_pc end = (vt_pc){ .position = { .x = hitbox.position.x + hitbox.dimensions.x, .y = hitbox.position.y - 120 - 48 }, .color = color_red };
    debug_draw_line(get_camera_matrix(), start, end, 3);

    start.position.y -= 360;
    start.color = color_green;
    end.position.y -= 360;
    end.color = color_green;
    debug_draw_line(get_camera_matrix(), start, end, 3);
}

void update_cloud(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    if(!debug) {
        if(sprite_get_anim_id(e->spr) == 0 && cloud_check_collision(hitbox))
            sprite_set_animation(e->spr, "Bounce", true);
        else if(sprite_get_anim_id(e->spr) != 0 && !sprite_get_playing(e->spr))
            e->to_delete = true;
    }

#ifdef BUILD_EDITOR
    cloud_draw_extents(hitbox);
#endif
}
void update_strong_cloud(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    if(!debug && cloud_check_collision(hitbox))
        sprite_set_animation(e->spr, "Strong Bounce", false);

#ifdef BUILD_EDITOR
    cloud_draw_extents(hitbox);
#endif
}
void update_red_cloud(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    stage_color c = get_color();
    sprite_set_orientation(e->spr, c);
    if(c == COLOR_RED && !debug && cloud_check_collision(hitbox))
        flip_color();

#ifdef BUILD_EDITOR
    cloud_draw_extents(hitbox);
#endif
}
void update_blue_cloud(__attribute__((__unused__)) float dt, bool debug, entity* e, aabb_2d hitbox, __attribute__((__unused__)) void* user) {
    stage_color c = get_color();
    sprite_set_orientation(e->spr, 1 - c);
    if(c == COLOR_BLUE && !debug && cloud_check_collision(hitbox))
        flip_color();

#ifdef BUILD_EDITOR
    cloud_draw_extents(hitbox);
#endif
}
entity* create_cloud(vec2 pos, cloud_type type, bool debug) {
    sprite spr = sprite_new(type == CLOUD_STRONG ? s_strong_cloud : s_cloud);
    sprite_set_playing(spr, true);
    char ch = 'c';
    entity_update_event* update = NULL;
    switch(type) {
        case CLOUD_NORMAL:
            ch = 'c';
            update = as_event(entity_update_event, update_cloud, NULL);
            break;
        case CLOUD_STRONG:
            ch = 'C';
            update = as_event(entity_update_event, update_strong_cloud, NULL);
            break;
        case CLOUD_RED:
            ch = 'r';
            sprite_set_animation(spr, "Red", true);
            update = as_event(entity_update_event, update_red_cloud, NULL);
            break;
        case CLOUD_BLUE:
            ch = 'u';
            sprite_set_animation(spr, "Blue", true);
            update = as_event(entity_update_event, update_blue_cloud, NULL);
            break;
    }
    return create_entity(pos, (vec2){.x=96, .y=48}, spr, update, NULL, ch, debug);
}
