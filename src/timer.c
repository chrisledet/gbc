#include <timer.h>
#include <cpu.h>

static timer_context ctx = {0};

timer_context *get_timer_context() {
	return &ctx;
}

void timer_init() {
	ctx.div = 0xAC00;
}

u8 timer_read(u16 addr) {
	switch (addr) {
		case 0xFF04:
			return ctx.div >> 8;
		case 0xFF05:
			return ctx.tima;
		case 0xFF06:
			return ctx.tma;
		case 0xFF07:
			return ctx.tac;
	}
	return 0;
}

void timer_write(u16 addr, u8 val) {
	switch (addr) {
		case 0xFF04:
			ctx.div = 0; // always 0 when written
		break;

		case 0xFF05:
			ctx.tima = val;
		break;

		case 0xFF06:
			ctx.tma = val;
		break;

		case 0xFF07:
			ctx.tac = val;
		break;
	}
}

void timer_tick() {

}
