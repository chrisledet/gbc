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

typedef struct {
	u32 bg_colors[4];
	u32 sp1_colors[4];
	u32 sp2_colors[4];
} lcd_context;

void lcd_init();
lcd_context *lcd_get_context();
void lcd_tick();

lcd_mode lcd_get_mode();
void lcd_set_mode(lcd_mode mode);

bool lcd_is_enabled();
bool lcd_is_bgw_enabled();
bool lcd_is_obj_enabled();
u8 lcd_obj_size();
u16 lcd_bg_map_addr();
u16 lcd_bgw_data_addr();
bool lcd_is_win_enabled();
u16 lcd_win_tile_map_addr();

void lcd_update_palette(u8 palette_index, u8 palette_data);
