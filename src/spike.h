#ifndef SPIKE_H
#define SPIKE_H
#include "entity.h"
#include "gameloop.h"

void init_spikes();
void cleanup_spikes();

entity* create_spike(vec2 pos, stage_color color, bool debug);

#endif
