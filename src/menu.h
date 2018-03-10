#include "types.h"
#include "text.h"
#include "vector.h"

declare(struct, menu)
event(menu_select_event, int selected)
event(slider_event, float value)

typedef struct slider {
    float min;
    float max;
    float value;

    float screen_position;

    slider_event* value_changed;
    slider_event* value_set;
} slider;
typedef struct menu_entry {
    char* name;
    bool active;
    bool special;
    menu* submenu;
    text display_text;

    slider* attached_slider;
} menu_entry;
typedef struct menu {
    int selected;
    vec2 position;
    float margin;
    menu_select_event* activate;

    menu_entry* entries;
    int entry_count;
} menu;

void init_menus();
void cleanup_menus();

menu* create_menu(menu_entry entries[], int entry_count, menu_select_event* ev);
void free_menu(menu* m);
void update_menus(float dt);
void scroll_menu(int64 diff);

bool in_menu(menu* m);

void set_menu(menu* m, bool replace);
void close_menu(bool all);
