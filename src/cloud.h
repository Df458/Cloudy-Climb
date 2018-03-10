#ifndef CLOUD_H
#define CLOUD_H
#include "entity.h"

typedef enum cloud_type {
    CLOUD_NORMAL = 0,
    CLOUD_STRONG = 1,
    CLOUD_RED = 2,
    CLOUD_BLUE = 3
} cloud_type;

void init_clouds();
void cleanup_clouds();

entity* create_cloud(vec2 pos, cloud_type type, bool debug);

#endif
