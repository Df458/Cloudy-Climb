#ifndef CLOUDY_EDITOR_H
#define CLOUDY_EDITOR_H
#include "gameloop.h"

typedef enum tool {
    TOOL_MOVE,
    TOOL_GOAL,
    TOOL_BONUS,
    TOOL_CLOUD,
    TOOL_STRONG_CLOUD,
    TOOL_SPIKE,
    TOOL_RAIL,
    TOOL_STRAIGHT_RAIL,
    TOOL_CIRCULAR_RAIL
} tool;
typedef enum cursor_state {
    CURSOR_DEFAULT,
    CURSOR_CAN_DRAG,
    CURSOR_DRAG,
} cursor_state;

void init_editor();

void update_editor(float dt);

void set_tool(tool t);
void set_tool_color(stage_color color);

void set_tool_enabled(bool enabled);
void set_snap_to_rail(bool snap);

void update_tool(float dt);

cursor_state get_cursor_state();

#endif
