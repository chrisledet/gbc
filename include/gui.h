#include <common.h>

typedef enum {
	GUI_NONE,
	GUI_QUIT,
} gui_event;

void gui_init();
void gui_tick();
void gui_shutdown();
void gui_clear();
u64 gui_get_ticks();
u32 *gui_get_frame_buffer();
bool gui_get_quit();
