#include <common.h>



void timer_init();
bool timer_tick(u8 cycles); // to be called by cpu
u8 timer_read(u16 addr);
void timer_write(u16 addr, u8 val);
