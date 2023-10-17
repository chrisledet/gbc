#include "bus.h"

#include <stdio.h>
#include <string.h>

#include "cart.h"
#include "mbc.h"

// 16-bit address bus
// 0x0000-0x7FFF 	 : PROGRAM DATA
	// 0x0000-0x3FFF 	- ROM BANK 0
	// 0x4000-0x7FFF 	- ROM BANK 1 - Switchable
// 0x8000-0x9FFF	 : LCD RAM
	// 0x8000-0x97FF 	 : CHR RAM / 8kB Video RAM (other 8kB accessed via BG bank switch)
	// 0x9800-0x9BFF 	 : BG  MAP1 / 4 KiB Work RAM (WRAM) / cartridge switchable bank
	// 0x9C00-0x9FFF 	 : BG  MAP2 / 4 KiB Work RAM (WRAM) / cartridge switchable bank
// 0xA000-0xBFFF 	 : CART RAM // external expansion RAM (battery backup ram)
// 0xC000-0xDFFF 	 : RAM (8KB)
	// 0xC000-0xCFFF 	- RAM BANK 0 (4KB)
	// 0xD000-0xDFFF 	- RAM BANK 1-7 - (Switchable CGB only) (4KB)
// 0xE000-0xFDFF : PROHIBTED
// 0xFE00-0xFFFF : Interanl CPU RAM
	// 0xFE00-0xFE9F : OAM (object attribute memory) RAM (40 objects)
	// 0xFEA0-0xFEFF : RESERVED - Unusable
	// 0xFF00-0xFF7F : IO REGISTERS
	// 0xFF80-0xFFFE : CPU work / stack RAM

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
		// ROM DATA
		return mbc_ctx->memory[addr];
	} else if (addr >= 0xA000 && addr <= 0xBFFF) {
		// CART RAM
		return mbc_ctx->memory[addr];
	} else if (addr >= 0xC000 && addr <= 0xDFFF) {
		// WORK RAM
		return mbc_ctx->memory[addr];
	} else if (addr >= 0xE000 && addr < 0xFF00) {
		// check echo ram access
		addr -= 2000;
	} else {
		printf("ERR: bus_read not supported at address: %02X\n", addr);
	}

	return 0;
}

u16 bus_read16(u16 addr) {
	return bus_read(addr) | (bus_read(addr+1) << 8);
}

void bus_write(u16 addr, u8 val) {
	mbc_context* mbc_ctx = get_mbc_context();

	if (0x0000 <= addr && addr < 0x2000) {
		// RAMCS gate data
		printf("ERR: RAM GATE NOT IMPL addr=%02X\n", addr);
	} else if (0x2000 <= addr && addr < 0x4000) {
		mbc_switch_rom_bank(val);
	// } else if (0x4000 <= addr && addr < 0x6000) {
		// UPPER ROM BANK CODE
		// printf("WARN: TODO enable ram / rom bank switch\n");
	// } else if (0x6000 <= addr && addr < 0x8000) {
		// ROM / RAM change
		// printf("WARN: TODO enable ram / rom bank switch\n");
	} else if (0xA000 <= addr && addr < 0xC000) {
		// cart ram (save progress)
		mbc_ctx->memory[addr] = val;
	} else if (0xC000 <= addr && addr < 0xE000) {
		// work ram
		mbc_ctx->memory[addr] = val;
	} else {
		printf("ERR: bus_write at address: %02X NOT IMPLEMENTED\n", addr);
	}
}

void bus_write16(u16 addr, u16 val) {
	bus_write(addr, val & 0xff);
	bus_write(addr+1, (val << 8));
}
