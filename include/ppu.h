#include "common.h"

typedef struct {
	u8 y;
	u8 x;
	u8 tile_idx;
	u8 flags;
} oam_entry;

void ppu_init();
void ppu_tick();
void ppu_dma_oam_transfer(u8 addr);
