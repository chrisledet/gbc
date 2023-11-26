#include "common.h"

typedef struct {
	u8 y;
	u8 x;
	u8 tile_idx;
	u8 flags;
} oam_entry;

typedef enum {
	FIFO_MODE_TILE,
	FIFO_MODE_DATA0,
	FIFO_MODE_DATA1,
	FIFO_MODE_IDLE,
	FIFO_MODE_PUSH
} fifo_fetch_mode;

typedef struct _fifo_entry {
	struct _fifo_entry *next;
	u32 color;
} fifo_entry;

typedef struct {
	fifo_entry *head;
	fifo_entry *tail;
	u32 size;
} fifo;

typedef struct {
	fifo_fetch_mode mode;
	fifo pixel_fifo;
	u8 line_x;
	u8 pushed_x;
	u8 fetch_x;
	u8 bgw_fetch[3];
	u8 fetch_entry[6];
	u8 map_y;
	u8 map_x;
	u8 tile_y;
	u8 fifo_x;
} pixel_fifo_context;

typedef struct {
	u8 dma_delay;
	u16 oam_src;
	u8 oam_pos;
	u32 current_frame;
	u32 ticks;
	u32 *vbuffer;
} ppu_context;

void ppu_init();
ppu_context *ppu_get_context();
void ppu_tick();
void ppu_dma_start(u8 addr);
bool ppu_dma_is_transferring();
u32 ppu_get_current_frame();
