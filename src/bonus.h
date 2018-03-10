#ifndef CLOUDY_BONUS
#define CLOUDY_BONUS
#include "entity.h"

void init_bonus();
void cleanup_bonus();
bool is_bonus_collected();
entity* create_bonus(vec2 pos, bool debug);

#endif
