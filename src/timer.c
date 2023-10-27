#include <timer.h>
#include <bus.h>

static timer_context ctx = {0};

timer_context *get_timer_context() {
	return &ctx;
}

void timer_init() {
	// TODO set div's initial value
	// timer_write(ADDR_DIV, 0x0);
}

u16 timer_get_tima_freq() {
	switch (ctx.tac) {
		case 0x0: return 1024;
		case 0x1: return 16;
		case 0x2: return 64;
		case 0x3: return 256;
		default: return 1024;
	}
}

u8 timer_read(u16 addr) {
	switch (addr) {
		case ADDR_DIV:
			return ctx.div >> 8;
		case ADDR_TIMA:
			return ctx.tima;
		case ADDR_TMA:
			return ctx.tma;
		case ADDR_TAC:
			return ctx.tac;
	}
	return 0;
}

void timer_write(u16 addr, u8 val) {
	switch (addr) {
		case ADDR_DIV:
			ctx.div = 0;
			break;
		case ADDR_TIMA:
			ctx.tima = val;
			break;
		case ADDR_TMA:
			ctx.tma = val;
			break;
		case ADDR_TAC:
			ctx.tac = val;
			break;
	}
}

bool timer_tick(u8 cycles) {
	ctx.c_div += cycles;
	if (ctx.c_div >= 0x100) {
		ctx.div++;
		ctx.c_div -= 0x100;
	}

	if (ctx.tac & 0x4) { // 2nd bit for control flag
		ctx.c_tima += cycles;
		while (ctx.c_tima >= timer_get_tima_freq()) {
			if (ctx.tima == 0xFF) {
				ctx.tima = ctx.tma;
				return true;
			} else {
				ctx.tima++;
			}
			ctx.c_tima -= timer_get_tima_freq();
		}
	}

	return false;
}
