#include "mbc.h"

#include <stdio.h>
#include <string.h>

#include "cart.h"


static mbc_context ctx;

mbc_context *get_mbc_context() {
	return &ctx;
}

bool mbc_is_valid(u8 idx) {
	return (idx < 0 || idx > 32);
}

void mbc_init(const cart_context *cart_ctx) {
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}

	ctx.memory = malloc(MEM_SIZE);
	if (ctx.memory != NULL) {
		memcpy(&ctx.memory[0], &cart_ctx->rom_data[0], ROM_BANK_SIZE);
		memcpy(&ctx.memory[ROM_BANK_SIZE], &cart_ctx->rom_data[ROM_BANK_SIZE], ROM_BANK_SIZE);
	}

	// 0x01 - 
	u8 ram_bank_count = cart_ctx->header->ram_size;
	if (ram_bank_count > 0) {
		if (ram_bank_count >= 3) {
			ram_bank_count = 4;
		}
		ctx.ram_banks = malloc(0x2000 * ram_bank_count);
	}

	switch(cart_ctx->header->type) {
		case 0x00:
		case 0x08:
		case 0x09:
			ctx.type = MBC_NONE;
		break;
		case 0x01:
		case 0x02:
		case 0x03:
			ctx.type = MBC_MBC1;
		break;
		case 0x05:
		case 0x06:
			ctx.type = MBC_MBC2;
		break;
		// case 0x0B:
		// case 0x0C:
		// 	ctx.type = MBC_MMM1;
		// break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			ctx.type = MBC_MBC3;
		break;
		// case 0x15:
		// case 0x16:
		// case 0x17:
		// 	ctx.type = MBC_MBC4;
		// break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			ctx.type = MBC_MBC5;
		break;
		case 0x20:
			ctx.type = MBC_MBC6;
		break;
		case 0x22:
			ctx.type = MBC_MBC7;
		break;
		default:
			ctx.type = MBC_NOT_SUPPORTED;
		break;
	}
}

void mbc_switch_rom_bank(u8 n) {
	if (n == 0) {
		n = 1;
	}
	else if (n >= 32) {
		printf("ERR: INVALID ROM BANK: %d\n", n);
	}

	cart_context* cart_ctx = get_cart_context();
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}

	memcpy(&ctx.memory[0x4000], &cart_ctx->rom_data[n * ROM_BANK_SIZE], ROM_BANK_SIZE);
}

void mbc_switch_ram_bank(u8 n) {
	if (n == 0) {
		n = 1;
	}
	else if (n >= 32) {
		printf("ERR: INVALID ROM BANK: %d\n", n);
	}

	cart_context* cart_ctx = get_cart_context();
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}

	printf("ERR: ram bank switch NOT IMPLEMENTED\n");
}
