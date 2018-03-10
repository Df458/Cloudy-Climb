#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H
#include "audio_source.h"

void init_audiomanager();
void cleanup_audiomanager();

float get_volume(bool bgm);
void set_volume(float value, void* user);
void update_audio(float dt);
void play_audio(audio_source src, bool is_bgm, bool loop);

#endif
