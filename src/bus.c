#include "bus.h"

#include <stdio.h>
#include <string.h>

#include "cart.h"
#include "mbc.h"


void bus_init(const cart_context* cart_ctx) {
	//cart_context *cart_ctx = get_cart_context();
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}
}

u8 bus_read(u16 addr) {
	mbc_context *mbc_ctx = get_mbc_context();

	if (addr < 0x8000) {
		return mbc_ctx->memory[addr] & 0xff;
	}

	// check echo ram access
	if (0xDFFF < addr && addr < 0xFF00) {
		addr -= 2000;
	}

	printf("ERR: mapper NOT IMPLEMENTED\n");

	return 0;
}

u16 bus_read16(u16 addr) {
	return bus_read(addr) | (bus_read(addr+1) << 8);
}

void bus_write(u16 addr, u8 val) {

	if (0x2000 <= addr && addr < 0x4000) {
		printf("DEBUG: ROM bank switched: %d\n", val);
		mbc_switch_rom_bank(val);
		return;
	}

	printf("ERR: bus_write NOT IMPLEMENTED\n");
	if (addr < 0x8000) {
		// write to MBCs registers
	}
}
