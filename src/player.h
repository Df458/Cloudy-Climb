#ifndef PLAYER_H
#define PLAYER_H
#include "aabb.h"

typedef enum player_state {
    PLAYER_GROUND,
    PLAYER_AIR,
    PLAYER_WIN,
    PLAYER_DEAD
} player_state;

void init_player();
void cleanup_player();
void reset_player();
void update_player(float dt);
bool player_check_collision(aabb_2d hitbox);
bool try_bounce(float y);
player_state player_get_state();
void kill_player();
void player_clear_level(vec3 pos);

#endif
