#include "timer.h"
#include <stdio.h>

typedef struct {
	u16 div;
	u8 tima;
	u8 tma;
	u8 tac;
} timer_context;

static timer_context ctx = {0};

u8 timer_read(u16 addr) {
	switch (addr) {
		case ADDR_DIV:
			return (ctx.div & 0xFF) >> 8;
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

bool timer_tick() {
	u16 p_div = ctx.div++;
	bool timer_update = false;
	switch (ctx.tac & 0x3) {
		case 0x0:
			timer_update = (p_div & (1 << 9)) && !(ctx.div & (1 << 9));
		break;
		case 0x1:
			timer_update = (p_div & (1 << 3)) && !(ctx.div & (1 << 3));
		break;
		case 0x2:
			timer_update = (p_div & (1 << 5)) && !(ctx.div & (1 << 5));
		break;
		case 0x3:
			timer_update = (p_div & (1 << 7)) && !(ctx.div & (1 << 7));
		break;
	}

	 // bit 2 for tima enable flag
	if ((ctx.tac & 0x4) && timer_update) {
		ctx.tima++;
		if (ctx.tima == 0xFF) {
			ctx.tima = ctx.tma;
			return true;
		}
	}

	return false;
}

void timer_init() {
	ctx.div = 0xAC00;
}
