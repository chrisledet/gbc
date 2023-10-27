#include <common.h>

typedef struct {
	u16 div;
	u8 tima;
	u8 tma;
	u8 tac;
	u16 c_div;
	u8 c_tima;
	u32 tima_frequency;
} timer_context;

typedef enum {
	INTERRUPT_NONE = 0x0,
	INTERRUPT_VBLANK = 0x1,
	INTERRUPT_LCD_STAT = 0x2,
	INTERRUPT_TIMER = 0x4,
	INTERRUPT_SERIAL = 0x8,
	INTERRUPT_JOYPAD = 0x10,
} interrupt;

void timer_init();
u8 timer_read(u16 addr);
void timer_write(u16 addr, u8 val);
bool timer_tick();
