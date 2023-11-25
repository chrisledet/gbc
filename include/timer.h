#include "common.h"

void timer_init();
// tick timer and return whether to request interrupt
bool timer_tick();
u8 timer_read(u16 addr);
void timer_write(u16 addr, u8 val);
