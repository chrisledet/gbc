#include "lcd.h"
#include "bus.h"

static lcd_context ctx = {0};
static unsigned long default_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

void lcd_init() {
	for (int i = 0; i < 4; ++i) {
		ctx.bg_colors[i] = default_colors[i];
		ctx.sp1_colors[i] = default_colors[i];
		ctx.sp2_colors[i] = default_colors[i];
	}
}

void lcd_tick() {

}

lcd_mode lcd_get_mode() {
	return (lcd_mode)(bus_read(ADDR_STAT) & 0x3);
}

void lcd_set_mode(lcd_mode mode) {
	u8 m = (bus_read(ADDR_STAT) & ~0x3) | mode;
	bus_io_write(ADDR_STAT, m);
}

bool lcd_is_bgw_enabled() {
	return (bus_read(ADDR_LCDC) & 0x01); 
}

bool lcd_is_obj_enabled() {
	return (bus_read(ADDR_LCDC) & 0x02); 
}

u8 lcd_obj_size() {
	return (bus_read(ADDR_LCDC) & 0x04) ? 0x10 : 0x8; 
}

u16 lcd_bg_map_addr() {
	return (bus_read(ADDR_LCDC) & 0x08) ? 0x9C00 : 0x9800; 
}

u16 lcd_bgw_data_addr() {
	return (bus_read(ADDR_LCDC) & 0x10) ? 0x8000 : 0x8800; 
}

bool lcd_is_win_enabled() {
	return (bus_read(ADDR_LCDC) & 0x20); 
}

u16 lcd_win_tile_map_addr() {
	return (bus_read(ADDR_LCDC) & 0x40) ? 0x9C00 : 0x9800; 
}

bool lcd_is_enabled() {
	return (bus_read(ADDR_LCDC) & 0x80); 
}

lcd_context *lcd_get_context() {
	return &ctx;
}

void lcd_update_palette(u8 palette_idx, u8 palette_data) {
	u32 *bg_colors = ctx.bg_colors;
	if (palette_idx == 0x1)
		bg_colors = ctx.sp1_colors;
	else if (palette_idx == 0x2)
		bg_colors = ctx.sp1_colors;

	bg_colors[0] = default_colors[(palette_data >> 0) & 0x3];
	bg_colors[1] = default_colors[(palette_data >> 2) & 0x3];
	bg_colors[2] = default_colors[(palette_data >> 4) & 0x3];
	bg_colors[3] = default_colors[(palette_data >> 6) & 0x3];
}
