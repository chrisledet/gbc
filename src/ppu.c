#include "ppu.h"
#include "common.h"
#include "bus.h"
#include "gui.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define OAM_SIZE 0xA0
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct {
	u16 oam_src;
	u8 oam_pos;
    u32 leftover;
    u32 lcd_line;
    u32 cycles;
} ppu_context;

static u32 color_palette[4] = { 0xF4FFF4, 0xC0D0C0, 0x80A080, 0x001000 };

static ppu_context ctx = {0};

void ppu_init() {
	// noop
}

void ppu_dma_oam_transfer(u8 addr) {
	ctx.oam_pos = 0;
	ctx.oam_src = addr * 0x100;
}

void ppu_tick() {
    if (ctx.oam_src) {
        bus_write(ADDR_OAM + ctx.oam_pos, bus_read(ctx.oam_src + ctx.oam_pos));
        if (++ctx.oam_pos >= OAM_SIZE) {
            ctx.oam_src = 0;
        }
    }
}
