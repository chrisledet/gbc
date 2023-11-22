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

void ppu_display_bg_data() {
    // width & height of the tile map in tiles
    static const u16 TILE_MAP_HEIGHT = 32, TILE_MAP_WIDTH = 32;
    // size of a single tile in bytes
    static const u16 TILE_SIZE = 16;

    u8 lcdc = bus_read(ADDR_LCDC);
    // starting address of the tileset data in VRAM
    u16 TILE_DATA_ADDR = lcdc & 0x10 ? 0x8000 : 0x8800;
    // starting address of the background tile map in VRAM
    u16 TILE_MAP_ADDR = lcdc & 0x8 ? 0x9C00 : 0x9800;

    // tile memory is 16 x 16 tile maps
    // tile maps are indexes to the tile data in vram
    for (u8 map_y = 0; map_y < TILE_MAP_HEIGHT; ++map_y) {
        for (u8 map_x = 0; map_x < TILE_MAP_WIDTH; ++map_x) {
            // Get tile index from the tile map
            u8 tile_idx = bus_read(TILE_MAP_ADDR + map_y * TILE_SIZE + map_x);

            // Get tile data from vram
            u8* tile_data = calloc(1, 64 * sizeof(u8));
            for (u8 y = 0; y < 16; y += 2) {
                u16 addr = TILE_DATA_ADDR + (tile_idx * TILE_SIZE);
                u8 left = bus_read(addr + y);
                u8 right = bus_read(addr + y + 1);

                for (u8 x = 0; x < 8; x++) {
                    u8 c_left = (left & (0x80 >> x) >> x);
                    u8 c_right = (right & (0x80 >> x) >> x);

                    if (tile_data != NULL) {
                        u16 hhhh = (y/2) * 8 + x;
                        tile_data[hhhh] = (c_right ? 2 : 0) + (c_left ? 1 : 0);
                    }
                }
            }

            /// Convert tile data to pixels and blit to the surface
            for (u8 y = 0; y < 8; ++y) {
                for (u8 x = 0; x < 8; ++x) {
                    // Extract pixel data from tile_data
                    if (tile_data != NULL) {
                        u16 hhhh = y * 8 + x;
                        u8 color_id = tile_data[hhhh];
                        gui_render_color(color_id, map_x * 8 + x, map_y * 8 + y);
                    }
                }
            }

            free(tile_data);
        }
    }
}

void ppu_tick() {
    if (ctx.oam_src) {
        bus_write(ADDR_OAM + ctx.oam_pos, bus_read(ctx.oam_src + ctx.oam_pos));
        if (++ctx.oam_pos >= OAM_SIZE) {
            ctx.oam_src = 0;
        }
    }

    /* Amount of cycles left over since the last full frame */
    ctx.leftover = ctx.cycles % (456 * 154);

    /* Each scanline is 456 cycles */
    ctx.lcd_line = ctx.leftover / 456;

    if (ctx.cycles % 44 == 0) {
        ppu_display_bg_data();
    }

    ctx.cycles++;
}
