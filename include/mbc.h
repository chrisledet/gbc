#include <stdbool.h>

#include "cart.h"
#include "common.h"

#define MEM_SIZE  0x10000
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000

typedef enum {
	MBC_NOT_SUPPORTED,
	MBC_NONE,
	MBC_MBC1,
	MBC_MBC2,
	MBC_MMC1,
	MBC_MBC3,
	MBC_MBC4,
	MBC_MBC5,
	MBC_MBC6,
	MBC_MBC7,
	MBC_MMM1,
} MBC_TYPE;

typedef struct {
	MBC_TYPE type;
	u8 bank_idx;
	u16* memory;
} mbc_context;

mbc_context* get_mbc_context();
void mbc_init(const cart_context *cart_ctx);
bool mbc_is_valid(u8 idx);
void mbc_switch_rom_bank(u8 num);
