#include "ppu.h"
#include "common.h"
#include "bus.h"
#include "gui.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OAM_SIZE 0xA0

typedef struct {
    u8 dma_delay;
	u16 oam_src;
	u8 oam_pos;
} ppu_context;

static ppu_context ctx;

void ppu_init() {
    memset(&ctx, 0, sizeof(ctx));
}

void ppu_dma_start(u8 addr) {
    // given addr is expected to be two highest bits for address
	ctx.oam_pos = 0;
	ctx.oam_src = addr * 0x100;
    ctx.dma_delay = 2;
}

bool ppu_dma_is_transferring() {
    return ctx.oam_pos < OAM_SIZE;
}

void ppu_tick() {
    if (ctx.oam_src) {
        // wait until delay
        if (ctx.dma_delay) {
            ctx.dma_delay--;
            return;
        }

        // copy tile data into OAM space
        u8 t = bus_read(ctx.oam_src + ctx.oam_pos);
        bus_write(ADDR_OAM + ctx.oam_pos, t);
        if (++ctx.oam_pos >= OAM_SIZE) {
            ctx.oam_src = 0;
            ctx.oam_pos = 0;
        }
    }
}
