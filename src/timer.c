#include <timer.h>
#include <bus.h>
#include <cpu.h>

static timer_context ctx = {0};

timer_context *get_timer_context() {
	return &ctx;
}

void timer_init() {
	// TODO set div's initial value
	// timer_write(ADDR_DIV, 0x0);
}

u16 timer_get_tima_freq() {
	u8 tac = bus_read(ADDR_TAC);
	switch (tac) {
		case 0x0: return 1024;
		case 0x1: return 16;
		case 0x2: return 64;
		case 0x3: return 256;
		default: return 1024;
	}
}

void timer_inc_div() {
	u8 n = bus_read(ADDR_DIV);
	bus_write(ADDR_DIV, n+1);
}

void timer_inc_tima() {
	u8 n = bus_read(ADDR_TIMA);
	bus_write(ADDR_TIMA, n+1);
}

u8 timer_read(u16 addr) {
	switch (addr) {
		case ADDR_DIV:
			return bus_read(ADDR_DIV);
		case ADDR_TIMA:
			return bus_read(ADDR_TIMA);
		case ADDR_TMA:
			return bus_read(ADDR_TMA);
		case ADDR_TAC:
			return bus_read(ADDR_TAC);
	}
	return 0;
}

void timer_write(u16 addr, u8 val) {
	switch (addr) {
		case ADDR_DIV:
			bus_write(ADDR_DIV, 0);
			break;
		case ADDR_TIMA:
			bus_write(ADDR_TIMA, val);
			break;
		case ADDR_TMA:
			bus_write(ADDR_TMA, val);
			break;
		case ADDR_TAC:
			bus_write(ADDR_TAC, val);
			break;
	}
}

void timer_tick(u8 cycles) {
	// u8 p_div = ctx.c_div;
	ctx.c_div += cycles;
	if (ctx.c_div >= 256) {
		timer_inc_div();
		ctx.c_div -= 256;
	}

	u8 tac = bus_read(ADDR_TAC);
	if (tac & 0x4) { // 2nd bit for control flag
		u8 tima = bus_read(ADDR_TIMA);
		ctx.c_tima += cycles;
		while (ctx.c_tima >= timer_get_tima_freq()) {
			if (tima == 0xFF) {
				u8 tma = bus_read(ADDR_TMA);
				bus_write(ADDR_TIMA, tma);
				cpu_request_interrupt(INTERRUPT_TIMER);
			} else {
				timer_inc_tima();
			}
			ctx.c_tima -= timer_get_tima_freq();
		}
	}
}
