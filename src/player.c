#include "audio_loader.h"
#include "audio_source.h"
#include "camera.h"
#include "color.h"
#include "control.h"
#include "interpolate.h"
#include "paths.h"
#include "sprite_loader.h"
#include "transform.h"
#include "vector.h"
#include "audiomanager.h"

#include "player.h"
#include "gameloop.h"
#include "renderer.h"

#define GRAVITY 1500
#define HSPEED 500
#define SUPER_TIMER 0.25

sprite player_sprite = NULL;

static audio_source a_hit = NULL;
static audio_source a_dead = NULL;
static audio_source a_bounce = NULL;
static audio_source a_win = NULL;
static audio_source a_super = NULL;
static audio_source a_ingame = NULL;

static transform player_transform = NULL;
static const vec2 player_dims = { .x = 32, .y = 32 };
static vec2 player_velocity = { 0 };

static float jump_cooldown = 0;
static float cloud_cooldown = 0;

static vec3 goal_location = { 0 };
static float goal_timer = 0;

static player_state state = PLAYER_GROUND;

extern camera c_main;
extern axis_id   a_player_motion;
extern action_id a_primary;

void init_player() {
    char* path = assets_path("Player.xml", NULL);
    spriteset set = load_spriteset(path);
    player_sprite = sprite_new(set);
    sfree(path);
    path = assets_path("Hit.wav", NULL);
    a_hit = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Dead.wav", NULL);
    a_dead = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Bounce.wav", NULL);
    a_bounce = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Win.wav", NULL);
    a_win = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Super.wav", NULL);
    a_super = load_audio_source(path, true);
    sfree(path);
    path = assets_path("Ingame.wav", NULL);
    a_ingame = load_audio_source(path, true);
    sfree(path);

    player_transform = transform_new();
    vec3 vec = (vec3){ .y = 112, .z = 0 };
    transform_translate(player_transform, vec, false);
}

void cleanup_player() {
    sprite_free(player_sprite, true);

    audio_source_free(a_hit);
    audio_source_free(a_dead);
    audio_source_free(a_bounce);
    audio_source_free(a_win);
    audio_source_free(a_super);
    audio_source_free(a_ingame);

    transform_free(player_transform);
}

void reset_player() {
    player_velocity = (vec2){ 0 };
    transform_translate(player_transform, (vec2){ .y = 112 }, false);
    sprite_set_animation(player_sprite, NULL, false);

    goal_timer = 0;

    transform_translate(camera_get_transform(c_main), (vec2){.y=0}, false);

    state = PLAYER_GROUND;
}

void update_player(float dt) {
    vec2 position = transform_get_position(player_transform).xy;
    vec2 cam_position = camera_get_position(c_main).xy;

    if(!is_game_paused()) {
        if(jump_cooldown > 0)
            jump_cooldown -= dt;
        if(cloud_cooldown > 0)
            cloud_cooldown -= dt;

        switch(state) {
            case PLAYER_GROUND:
                player_velocity = (vec2){ 0 };

                if(action_is_triggered(a_primary)) {
                    sprite_set_animation(player_sprite, "Super Bounce", true);
                    play_audio(a_super, false, false);
                    play_audio(a_ingame, true, true);
                    player_velocity.y = -2000;

                    state = PLAYER_AIR;
                }
            break;

            case PLAYER_AIR:
                player_velocity.x = HSPEED * get_axis_value(a_player_motion);
                player_velocity.y += GRAVITY * dt;
                if(action_is_triggered(a_primary)) {
                    if(cloud_cooldown > 0) {
                        sprite_set_animation(player_sprite, "Super Bounce", true);
                        play_audio(a_super, false, false);
                        player_velocity.y -= 400;
                    } else {
                        jump_cooldown = SUPER_TIMER;
                    }
                }

                position = vec_add(position, vec_mul(player_velocity, dt));
            break;

            case PLAYER_DEAD:
                player_velocity.y += GRAVITY * dt;

                if(position.y > cam_position.y + 640 && transition_to(reset_level))
                    play_audio(a_dead, true, false);

                position.y += player_velocity.y * dt;
            break;

            case PLAYER_WIN:
                if(position.y >= goal_location.y && player_velocity.y >= 0) {
                    player_velocity.y = 0;
                    sprite_set_animation(player_sprite, "Win", false);

                    if(eq0(goal_timer))
                        play_audio(a_win, true, false);

                    if((goal_timer += dt) > 2.1)
                        transition_to(game_advance_level);
                } else {
                    player_velocity.y += GRAVITY * dt;
                }

                position = vec_add(position, vec_mul(player_velocity, dt));
            break;
        }
        sprite_update(player_sprite, dt);
    }

    if(state != PLAYER_DEAD) {
        if(position.y < -1000 || player_velocity.y < 0) {
            cam_position.y = lerp(cam_position.y, position.y, 0.5);
            if(cam_position.y > 0)
                cam_position.y = 0;
        }
        if(player_velocity.y > 0) {
            sprite_set_animation(player_sprite, "Falling", false);
            if(position.y > -700)
                kill_player();
        }
    }

    transform_translate(player_transform, position, false);
    transform_translate(camera_get_transform(c_main), cam_position, false);

    aabb_2d rect = (aabb_2d){
        .position = vec2_sub(transform_get_position(player_transform).xy, player_dims),
        .dimensions = vec2_mul(player_dims, 2)
    };
    draw_debug_rect(rect, color_white);
    draw_sprite(transform_get_matrix(player_transform), player_sprite, color_white);
}

bool player_check_collision(aabb_2d hitbox) {
    aabb_2d rect = (aabb_2d){
        .position = vec2_sub(transform_get_position(player_transform).xy, player_dims),
        .dimensions = vec2_mul(player_dims, 2)
    };

    return state == PLAYER_AIR && aabb_intersects(hitbox, rect);
}

bool try_bounce(float y) {
    float player_y = transform_get_position(player_transform).y - (player_dims.y * 2);
    if(player_y < y && player_velocity.y >= 0 && state == PLAYER_AIR) {
        sprite_set_animation(player_sprite, jump_cooldown > 0 ? "Super Bounce" : "Bounce", true);

        play_audio(a_bounce, false, false);

        player_velocity.y = jump_cooldown > 0 ? -1200 : -800;
        if(jump_cooldown <= 0)
            cloud_cooldown = .1;
        else
            play_audio(a_super, false, false);
        return true;
    }
    return false;
}

void kill_player() {
    if(state == PLAYER_AIR) {
        sprite_set_animation(player_sprite, "Dead", true);
        player_velocity.y = -600;
        play_audio(a_hit, false, false);
        play_audio(NULL, true, false);

        state = PLAYER_DEAD;
    }
}

player_state player_get_state() {
    return state;
}

void player_clear_level(vec3 pos) {
    if(state == PLAYER_AIR) {
        goal_location = vec3_add(pos, (vec3){ .y = 50 });
        player_velocity.x = 0;

        state = PLAYER_WIN;
    }
}
