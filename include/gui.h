#include <common.h>

typedef enum {
	GUI_NONE,
	GUI_QUIT,
} gui_event;

void gui_init();
void gui_tick();
u64 gui_get_ticks();
gui_event gui_handle_input();
