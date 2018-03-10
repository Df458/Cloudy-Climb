#ifndef IO_H
#define IO_H
#include "types.h"

void init_io();
void cleanup_io();

void save_game();
void load_game();

void save_options(float value, void* user);
void load_options();

void save_level(const char* path);
void load_level(const char* path, bool debug);

#endif
