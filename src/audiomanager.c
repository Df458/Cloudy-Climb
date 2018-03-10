#include "audio_player.h"
#include "audio_system.h"
#include "container/array.h"

#include "audiomanager.h"

static uarray se;
static audio_player bgm = NULL;
static float bgm_vol = 1;
static float se_vol = 1;

iter_result cleanup_audio_single(void* data, __attribute__((__unused__)) void* user) {
    audio_player_free(data, false);
    return iter_continue;
}
iter_result update_volume_single(void* audio, void* user) {
    audio_player_set_gain((audio_player)audio, *(float*)user);

    return iter_continue;
}
iter_result update_audio_single(void* audio, void* user) {
    audio_player_update((audio_player)audio, *(float*)user);

    if(!audio_player_get_playing(audio)) {
        audio_player_free(audio, false);
        return iter_delete;
    }

    return iter_continue;
}

void init_audiomanager() {
    audio_init();

    se = uarray_new(10);
}
void cleanup_audiomanager() {
    if(bgm)
        audio_player_free(bgm, false);

    array_foreach(se, cleanup_audio_single, NULL);
    array_free(se);

    audio_cleanup();
}

float get_volume(bool bgm) {
    return bgm ? bgm_vol : se_vol;
}
void set_volume(float value, void* user) {
    if(user) {
        if(bgm)
            audio_player_set_gain(bgm, value);
        bgm_vol = value;
    } else {
        array_foreach(se, update_volume_single, &value);
        se_vol = value;
    }
}
void update_audio(float dt) {
    if(bgm)
        audio_player_update(bgm, dt);

    array_foreach(se, update_audio_single, &dt);
}
void play_audio(audio_source src, bool is_bgm, bool loop) {
    audio_player player = src ? audio_player_new(src) : NULL;
    if(is_bgm) {
        if(bgm)
            audio_player_free(bgm, false);
        if(player)
            audio_player_set_gain(player, bgm_vol);
        bgm = player;
    } else if(player) {
        audio_player_set_gain(player, se_vol);
        array_add(se, player);
    }

    if(player) {
        audio_player_set_loop(player, loop);
        audio_player_set_playing(player, true);
    }
}
