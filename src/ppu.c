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

static pixel_fifo_context fifo_ctx;
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
	return !!ctx.oam_src;
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

void ppu_fifo_pixel_push(u32 color) {
	fifo_entry *e = calloc(1, sizeof(fifo_entry));
	e->next = NULL;
	e->color = color;

	if (!fifo_ctx.pixel_fifo.head) {
		fifo_ctx.pixel_fifo.head = e;
		fifo_ctx.pixel_fifo.tail = e;
	} else {
		fifo_ctx.pixel_fifo.tail->next = e;
		fifo_ctx.pixel_fifo.tail = e;
	}

	fifo_ctx.pixel_fifo.size++;
}

u32 ppu_fifo_pixel_pop() {
	if (fifo_ctx.pixel_fifo.size <= 0) {
		return 0;
	}

	fifo_entry *e = fifo_ctx.pixel_fifo.head;
	fifo_ctx.pixel_fifo.head = e->next;
	fifo_ctx.pixel_fifo.size--;
	u32 c = e->color;
	free(e);
	return c;
}

void ppu_fifo_reset() {
	while (fifo_ctx.pixel_fifo.size)
		ppu_fifo_pixel_pop();
	
	fifo_ctx.pixel_fifo.head = 0;
}

bool ppu_fifo_add() {
	if (fifo_ctx.pixel_fifo.size > 8)
		return false;

	u8 sx = bus_read(ADDR_SCX);
	int x = fifo_ctx.fetch_x - (8 - (sx % 8));

	for (int i = 0; i < 8; ++i) {
		int b = 7 - i;
		u8 hi = !!(fifo_ctx.bgw_fetch[1] & (1 << b));
		u8 lo = !!(fifo_ctx.bgw_fetch[2] & (1 << b)) << 1;
		u32 c = lcd_get_context()->bg_colors[hi | lo];

		if (x >= 0) {
			ppu_fifo_pixel_push(c);
			fifo_ctx.fifo_x++;
		}
	}
	return true;
}

void ppu_fifo_fetch() {
	switch (fifo_ctx.mode) {
		case FIFO_MODE_TILE: {
			if (lcd_is_bgw_enabled()) {
				u16 idx = ((fifo_ctx.map_x / 8) + (fifo_ctx.map_y / 8)) * 32;
				fifo_ctx.bgw_fetch[0] = bus_read(lcd_bg_map_addr() + idx);

				if (lcd_bgw_data_addr() == 0x8800) {
					fifo_ctx.bgw_fetch[0] += 128;
				}
			}

			fifo_ctx.mode = FIFO_MODE_DATA0;
			fifo_ctx.fetch_x += 8;
		}
		break;
		case FIFO_MODE_DATA0: {
			u16 idx = fifo_ctx.bgw_fetch[1] * 16 + fifo_ctx.tile_y;
			fifo_ctx.bgw_fetch[1] = bus_read(lcd_bgw_data_addr() + idx);
			fifo_ctx.mode = FIFO_MODE_DATA1;
		}
		break;
		case FIFO_MODE_DATA1: {
			u16 idx = fifo_ctx.bgw_fetch[2] * 16 + (fifo_ctx.tile_y + 1);
			fifo_ctx.bgw_fetch[2] = bus_read(lcd_bgw_data_addr() + idx);
			fifo_ctx.mode = FIFO_MODE_IDLE;
		}
		break;
		case FIFO_MODE_IDLE: {
			fifo_ctx.mode = FIFO_MODE_PUSH;
		}
		break;
		case FIFO_MODE_PUSH: {
			if (ppu_fifo_add())
				fifo_ctx.mode = FIFO_MODE_TILE;
		}
		break;
	}
}

void ppu_fifo_push() {
	if (fifo_ctx.pixel_fifo.size > 8) {
		u32 color = ppu_fifo_pixel_pop();
		u8 ly = bus_read(ADDR_LY);
		u8 sx = bus_read(ADDR_SCX);

		if (fifo_ctx.line_x >= (sx & 8)) {
			int idx = fifo_ctx.pushed_x + ly * SCREEN_WIDTH;
			ctx.vbuffer[idx] = color;
			fifo_ctx.pushed_x++;
		}

		fifo_ctx.line_x++;
	}
}

void ppu_fifo_tick() {
	u8 ly = bus_read(ADDR_LY);
	u8 sy = bus_read(ADDR_SCY);
	u8 sx = bus_read(ADDR_SCX);
	fifo_ctx.map_y = ly + sy;
	fifo_ctx.map_x = fifo_ctx.fetch_x + sx;
	fifo_ctx.tile_y = ((ly + sy) % 8) * 2;

	if (!(ctx.ticks & 1)) {
		ppu_fifo_fetch();
	}

	ppu_fifo_push();
}
 
void ppu_copy_into_oam() {
	if (ctx.oam_src) {
		// wait until delay
		if (ctx.dma_delay) {
			ctx.dma_delay--;
			return;
		}

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

		fifo_ctx.mode = FIFO_MODE_TILE;
		fifo_ctx.line_x = 0;
		fifo_ctx.pushed_x = 0;
		fifo_ctx.fetch_x = 0;
		fifo_ctx.fifo_x = 0;
	}
}

void ppu_oam_xfer_tick() {
	ppu_fifo_tick();
	if (fifo_ctx.pushed_x >= SCREEN_WIDTH) {
		ppu_fifo_reset();
		lcd_set_mode(MODE_HBLANK);

		u8 stat = bus_read(ADDR_STAT);
		if (BIT(stat, 3))
			cpu_request_interrupt(INTERRUPT_LCD_STAT);
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
				u32 d = target_frame_time - frame_time;
				gbc_delay(d);
			}

			if ((t - start_timer) >= 1000) {
				fprintf(stderr, "DEBUG: FPS=%lu\n", frame_count);
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
	ppu_copy_into_oam();
}

void ppu_init() {
	// if (ctx.vbuffer != NULL)
	// 	free(ctx.vbuffer);
	memset(&ctx, 0, sizeof(ctx));

	ctx.current_frame = 0;
	ctx.ticks = 0;
	ctx.vbuffer = calloc(1, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(int));
	fifo_ctx.line_x = 0;
	fifo_ctx.pushed_x = 0;
	fifo_ctx.fetch_x = 0;
	fifo_ctx.pixel_fifo.size = 0;
	fifo_ctx.pixel_fifo.head = NULL;
	fifo_ctx.pixel_fifo.tail = NULL;
	fifo_ctx.mode = FIFO_MODE_TILE;

	lcd_init();
	lcd_set_mode(MODE_OAM_SCAN);
}

ppu_context *ppu_get_context() {
	return &ctx;
}
