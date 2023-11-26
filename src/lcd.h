#include "common.h"

#define LCD_PALETTE_BG 0
#define LCD_PALETTE_OBJ0 1
#define LCD_PALETTE_OBJ1 2

typedef enum {
	MODE_HBLANK,
	MODE_VBLANK,
	MODE_OAM_SCAN,
	MODE_OAM_XFER,
} lcd_mode;

void lcd_init();
void lcd_tick();

lcd_mode lcd_get_mode();
void lcd_set_mode(lcd_mode mode);

bool lcd_is_enabled();
bool lcd_is_bgw_enabled();
bool lcd_is_obj_enabled();
u8 lcd_obj_size();
u16 lcd_bg_map();
u16 lcd_bg_tile();
bool lcd_is_win_enabled();
u16 lcd_win_tile_map();

void lcd_update_palette(u8 palette_index, u8 palette_data);
