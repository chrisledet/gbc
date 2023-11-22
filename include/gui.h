#include "common.h"
#include <SDL.h>

typedef enum {
	GUI_NONE,
	GUI_QUIT,
} gui_event;

void gui_init();
void gui_tick();
void gui_shutdown();
void gui_display_clear();
void gui_render_color(u8 color_id, int x, int y);
u64 gui_get_ticks();
gui_event gui_handle_input();
