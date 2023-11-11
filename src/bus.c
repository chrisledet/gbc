#include "bus.h"

#include <stdio.h>
#include <string.h>

#include <cart.h>
#include <cpu.h>
#include <timer.h>

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

#define MEM_SIZE 0x10000
#define ROM_BANK_SIZE 0x4000
#define RAM_BANK_SIZE 0x2000
#define VRAM_BANK_SIZE 0x2000

typedef struct {
	u8 rom_bank;
	u8 ram_bank;
	u8 vram_bank;
	u8* rom;
	u8* ram; // switchable ram
	u8* wram; // work ram
	u8* vram;
	u8* hram; // high ram
	u8* hw; // io registers and ports
	u8 ie; // interrupt enable
	bool ram_enabled;
} bus_ctx;

static bus_ctx ctx;

void bus_init(const cart_context* cart_ctx) {
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}

	u8 rom_bank_count = 1 << (cart_ctx->header->rom_size + 1);
	ctx.rom = malloc(ROM_BANK_SIZE * rom_bank_count);
	if (ctx.rom != NULL) {
		for (u8 i = 0; i < rom_bank_count; i += 1) {
			memcpy(&ctx.rom[ROM_BANK_SIZE * i], &cart_ctx->rom_data[ROM_BANK_SIZE * i], ROM_BANK_SIZE);
		}
	}

	u8 ram_bank_count = 0;
	switch (cart_ctx->header->ram_size) {
		case 0x2:
			ram_bank_count = 1;
		break;
		case 0x3:
			ram_bank_count = 4;
		break;
		case 0x4:
			ram_bank_count = 16;
		break;
		case 0x5:
			ram_bank_count = 8;
		break;
		default:
			ram_bank_count = 0;
		break;
	}
	ctx.ram = malloc(RAM_BANK_SIZE * ram_bank_count);

	ctx.wram = malloc(RAM_BANK_SIZE);
	ctx.vram = malloc(VRAM_BANK_SIZE * 2);
	ctx.hram = malloc(0x7F);
	ctx.hw = malloc(0xFF);
	ctx.rom_bank = 0;
	ctx.ram_bank = 0;
	ctx.vram_bank = 0;
}

u8 bus_read(u16 addr) {
	if (addr < 0x4000) {
		// ROM DATA / BANK
		return ctx.rom[addr];
	} else if (addr >= 0x4000 && addr < 0x8000) {
		// ROM SWITCHABLE BANK
		return ctx.rom[((ctx.rom_bank+1) * ROM_BANK_SIZE) + (addr - 0x4000)];
	} else if (addr >= 0xA000 && addr < 0xC000) {
		// CART RAM
		return ctx.ram_enabled ? ctx.ram[(ctx.ram_bank * RAM_BANK_SIZE) + (addr - 0xA000)] : 0xff;
	} else if (addr >= 0xC000 && addr < 0xE000) {
		// WORK RAM
		return ctx.wram[addr - 0xC000];
	} else if (addr >= 0xE000 && addr < 0xFF00) {
		// check echo ram access
		return ctx.wram[addr-0x2000];
	} else if (addr >= 0xFF00 && addr < 0xFF80) {
		switch (addr) {
			case ADDR_DIV:
				return timer_read(ADDR_DIV);
			case ADDR_TIMA:
				return timer_read(ADDR_TIMA);
			case ADDR_TMA:
				return timer_read(ADDR_TMA);
			case ADDR_TAC:
				return timer_read(ADDR_TAC);
			default:
				return ctx.hw[addr - 0xFF00];
		}
	} else if (addr >= 0xFF80 && addr < 0xFFFF) {
		// high ram
		return ctx.hram[addr - 0xFF80];
	} else if (addr == 0xFFFF) {
		return ctx.ie;
	} else {
		printf("ERR: bus_read not supported at address: %02X\n", addr);
	}

	return 0;
}

u16 bus_read16(u16 addr) {
	return bus_read(addr) | (bus_read(addr+1) << 8);
}

void bus_write(u16 addr, u8 val) {
	if (addr < 0x1FFF) {
		// mbc1 logic
		if (val == 0xA)
			ctx.ram_enabled = true;
		else if (!val)
			ctx.ram_enabled = false;
	} else if (0x2000 <= addr && addr < 0x8000) {
		// TODO: ROM / RAM switch
	} else if (0x8000 <= addr && addr < 0xA000) {
		ctx.vram[(ctx.vram_bank + VRAM_BANK_SIZE) + (addr - 0x8000)] = val;
	} else if (0xA000 <= addr && addr < 0xC000) {
		// cart ram (save progress)
		ctx.ram[(ctx.ram_bank * RAM_BANK_SIZE) + (addr - 0xA000)] = val;
	} else if (0xC000 <= addr && addr < 0xE000) {
		ctx.ram[addr] = val;
	} else if (addr >= 0xFF00 && addr < 0xFF80) {
		switch (addr) {
			case ADDR_DIV:
				timer_write(ADDR_DIV, val);
				break;
			case ADDR_TIMA:
				timer_write(ADDR_TIMA, val);
				break;
			case ADDR_TMA:
				timer_write(ADDR_TMA, val);
				break;
			case ADDR_TAC:
				timer_write(ADDR_TAC, val);
				break;
			default:
				ctx.hw[addr - 0xFF00] = val;
				break;
		}
	} else if (addr >= 0xFF80 && addr < 0xFFFF) {
		ctx.hram[addr - 0xFF80] = val;
	} else if (addr == 0xFFFF) {
		ctx.ie = val;
	} else {
		printf("ERR: bus_write at address: %02X NOT IMPLEMENTED\n", addr);
	}
}

void bus_write16(u16 addr, u16 val) {
	bus_write(addr, val & 0xff);
	bus_write(addr+1, (val >> 8) & 0xFF);
}
