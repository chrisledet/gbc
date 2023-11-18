#include <common.h>

typedef enum {
	INTERRUPT_NONE = 0x0,
	INTERRUPT_VBLANK = 0x1,
	INTERRUPT_LCD_STAT = 0x2,
	INTERRUPT_TIMER = 0x4,
	INTERRUPT_SERIAL = 0x8,
	INTERRUPT_JOYPAD = 0x10,
} interrupt;

void timer_init();
bool timer_tick(u8 cycles); // to be called by cpu
u8 timer_read(u16 addr);
void timer_write(u16 addr, u8 val);
