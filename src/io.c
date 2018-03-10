#include "check.h"
#include "color.h"
#include "paths.h"

#include <stdio.h>

#include "audiomanager.h"
#include "bonus.h"
#include "cloud.h"
#include "goal.h"
#include "spike.h"
#include "entity.h"
#include "gameloop.h"
#include "io.h"
#include "player.h"
#include "rail.h"

extern audio_source a_windbgm;

void init_io() {
    init_base_resource_path(NULL);
}
void cleanup_io() {
    resource_path_free();
}

void save_game() {
    char* path = get_resource_path("", "unlocks.txt", NULL);
    FILE* unlocks_file = fopen(path, "w");
    if(unlocks_file) {
        for(uint8 i = 0; i < LEVEL_COUNT; ++i)
            fprintf(unlocks_file, "%d\n", get_unlocked(i));
        fclose(unlocks_file);
    }

    sfree(path);
}
void load_game() {
    char* path = get_resource_path("", "unlocks.txt", NULL);
    FILE* unlocks_file = fopen(path, "r");
    if(unlocks_file) {
        unlock_status status = LOCKED;
        for(uint8 i = 0; i < LEVEL_COUNT && !feof(unlocks_file); ++i) {
            if(fscanf(unlocks_file, "%d\n", (int*)&status) == 1)
                set_unlocked(i, status);
        }
        fclose(unlocks_file);
    }

    for(uint8 i = 0; i < LEVEL_COUNT; ++i) {
        info("%d", get_unlocked(i));
    }

    sfree(path);
}

void save_options(__attribute__((__unused__)) float value, __attribute__((__unused__)) void* user) {
    char* path = get_resource_path("", "options.txt", NULL);
    FILE* options_file = fopen(path, "w");
    if(options_file) {
        fprintf(options_file, "%f\n%f\n", get_volume(true), get_volume(false));
        fclose(options_file);
    }

    sfree(path);
}
void load_options() {
    char* path = get_resource_path("", "options.txt", NULL);
    FILE* options_file = fopen(path, "r");
    if(options_file) {
        float v1, v2;
        if(fscanf(options_file, "%f\n%f", &v1, &v2) == 2) {
            set_volume(v1, (void*)true);
            set_volume(v2, (void*)false);
        }
        fclose(options_file);
    }

    sfree(path);
}

void save_level(const char* path) {
    FILE* infile = fopen(path, "w+");
    check_return(infile, "Failed to save file at path: %s", , path);

    save_rails(infile);
    save_entities(infile);

    fclose(infile);
}
void load_level(const char* path, bool debug) {
    cleanup_entities(debug);
    cleanup_rails();
    reset_player();

    FILE* infile = fopen(path, "r");
    check_return(infile, "Failed to load file at path: %s", , path);

    if(get_color() != COLOR_BLUE)
        flip_color();

    entity* e = NULL;
    while(!feof(infile)) {
        vec2 v;
        vec2 v2;
        char c = fgetc(infile);
        fgetc(infile);
        switch(c) {
            case 'c':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_cloud(v, CLOUD_NORMAL, debug);
            break;
            case 'C':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_cloud(v, CLOUD_STRONG, debug);
            break;
            case 'r':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_cloud(v, CLOUD_RED, debug);
            break;
            case 'u':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_cloud(v, CLOUD_BLUE, debug);
            break;
            case 's':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_spike(v, COLOR_NONE, debug);
            break;
            case 'R':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_spike(v, COLOR_RED, debug);
            break;
            case 'U':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_spike(v, COLOR_BLUE, debug);
            break;
            case 'g':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_goal(v, debug);
            break;
            case 'b':
                if(fscanf(infile, vec2_printstr, &v.x, &v.y) == 2)
                    e = create_bonus(v, debug);
                break;
            case 'O':
                if(fscanf(infile, vec2_printstr" "vec2_printstr, &v.x, &v.y, &v2.x, &v2.y) == 4)
                    create_circle_rail(v, vec2_len(vec2_sub(v, v2)));

                e = NULL;
                break;
            case '-':
                if(fscanf(infile, vec2_printstr" "vec2_printstr, &v.x, &v.y, &v2.x, &v2.y) == 4)
                    create_straight_rail(v, v2);

                e = NULL;
                break;
            case '|':
                if(e) {
                    int id;
                    float pos;
                    if(fscanf(infile, "%d %f", &id, &pos) == 2)
                        attach_entity(e, id, pos);
                }
                break;
        }

        while(!feof(infile) && fgetc(infile) != '\n');
    }

    if(!debug)
        play_audio(a_windbgm, true, true);

    fclose(infile);
}
