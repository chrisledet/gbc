#include "ppu.h"
#include "cpu.h"
#include "common.h"
#include "interrupt.h"
#include "bus.h"
#include "gui.h"
#include "lcd.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OAM_SIZE 0xA0
#define TICKS_PER_LINE 456
#define LINES_PER_FRAME 154

typedef struct {
	u8 dma_delay;
	u16 oam_src;
	u8 oam_pos;

	u32 current_frame;
	u32 ticks;
	u32 *vbuffer;

} ppu_context;

static ppu_context ctx;

// frame state
static u32 target_frame_time = 1000 / 60;
static long prev_frame_time = 0;
static long start_timer = 0;
static long frame_count = 0;

void ppu_dma_start(u8 addr) {
	// given addr is expected to be two highest bits for address
	ctx.oam_pos = 0;
	ctx.oam_src = addr * 0x100;
	ctx.dma_delay = 2;
}

bool ppu_dma_is_transferring() {
	return ctx.oam_pos < OAM_SIZE;
}

u32 ppu_get_current_frame() {
	return ctx.current_frame;
}

u8 ppu_inc_ly() {
	u8 ly = bus_read(ADDR_LY) + 1;
	bus_io_write(ADDR_LY, ly);

	u8 lyc = bus_read(ADDR_LYC);
	if (ly == lyc) {
		// set STAT.2
		u8 stat = bus_read(ADDR_STAT) | (1 << 2);
		bus_io_write(ADDR_STAT, stat);

		if (stat & 0x40) // STAT.6
			cpu_request_interrupt(INTERRUPT_LCD_STAT);
	} else {
		// unset STAT.2
		u8 stat = bus_read(ADDR_STAT) & ~(1 << 2);
		bus_io_write(ADDR_STAT, stat);
	}

	return ly;
}

void ppu_oam_tick() {
	if (ctx.oam_src) {
		// wait until delay
		if (ctx.dma_delay) {
			ctx.dma_delay--;
			return;
		}

		// copy tile data into OAM space
		u8 t = bus_read(ctx.oam_src + ctx.oam_pos);
		bus_io_write(ADDR_OAM + ctx.oam_pos, t);
		if (++ctx.oam_pos >= OAM_SIZE) {
			ctx.oam_src = 0;
			ctx.oam_pos = 0;
		}
	}
}

void ppu_oam_scan_tick() {
	if (ctx.ticks >= 80) {
		lcd_set_mode(MODE_OAM_XFER);
	}
}

void ppu_oam_xfer_tick() {
	if (ctx.ticks >= (80 + 172)) {
		lcd_set_mode(MODE_HBLANK);
	}
}

void ppu_vblank_tick() {
	if (ctx.ticks >= TICKS_PER_LINE) {
		u8 ly = ppu_inc_ly();

		if (ly >= LINES_PER_FRAME) {
			lcd_set_mode(MODE_OAM_SCAN);
			bus_io_write(ADDR_LY, 0);
		}
		ctx.ticks = 0;
	}
}

void ppu_hblank_tick() {
	if (ctx.ticks >= TICKS_PER_LINE) {
		u8 ly = ppu_inc_ly();

		if (ly >= SCREEN_HEIGHT) {
			lcd_set_mode(MODE_VBLANK);
			cpu_request_interrupt(INTERRUPT_VBLANK);

			u8 stat = bus_read(ADDR_STAT);
			if (BIT(stat, 5))
				cpu_request_interrupt(INTERRUPT_LCD_STAT);

			ctx.current_frame++;

			u32 t = gbc_get_ticks();
			u32 frame_time = t - prev_frame_time;
			if (frame_time < target_frame_time) {
				fprintf(stderr, "delaying....\n");
				gbc_delay(target_frame_time - frame_time);
			}

			if ((t - start_timer) >= 1000) {
				fprintf(stderr, "FPS=%lu\n", frame_count);
				start_timer = t;
				frame_count = 0;
			}

			frame_count++;
			prev_frame_time = gbc_get_ticks();
		} else {
			lcd_set_mode(MODE_OAM_SCAN);
		}

		ctx.ticks = 0;
	}
}


void ppu_tick() {
	ctx.ticks++;
	switch (lcd_get_mode()) {
		case MODE_OAM_SCAN:
			ppu_oam_scan_tick();
		break;
		case MODE_OAM_XFER:
			ppu_oam_xfer_tick();
		break;
		case MODE_VBLANK:
			ppu_vblank_tick();
		break;
		case MODE_HBLANK:
			ppu_hblank_tick();
		break;
	}
	ppu_oam_tick();
}

void ppu_init() {
	if (ctx.vbuffer != NULL)
		free(ctx.vbuffer);
	memset(&ctx, 0, sizeof(ctx));

	lcd_init();
	lcd_set_mode(MODE_OAM_SCAN);

	ctx.vbuffer = calloc(1, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(u32));
}
