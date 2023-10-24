#include <common.h>
// #include <stdlib.h>

typedef struct {
	u16 c_div;
	u8 c_tima;
	u32 tima_frequency;
} timer_context;

void timer_init();
u8 timer_read(u16 addr);
void timer_write(u16 addr, u8 val);
void timer_tick();
