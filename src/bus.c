#include "bus.h"

#include <stdio.h>
#include <string.h>

#include <cart.h>
#include <ppu.h>
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
	u32 rom_bank;
	u32 ram_bank;
	u32 vram_bank;
	u8 *mem;
	u8 *rom;  // banked rom
	u8 *ram;  // banked ram
	u8 *vram; // banked vram
	bool ram_enabled;
	bool dma_transfer;
} bus_ctx;

static bus_ctx ctx = { 0 };

void bus_init(const cart_context* cart_ctx) {
	if (cart_ctx == NULL) {
		printf("ERR: NO CART LOADED!\n");
		return;
	}

	int rom_bank_count = 1 << (cart_ctx->header->rom_size + 1);
	ctx.rom = calloc(1, ROM_BANK_SIZE * rom_bank_count);
	memcpy(ctx.rom, &cart_ctx->rom_data[0], cart_ctx->rom_size);

	ctx.mem = calloc(1, MEM_SIZE);
	memcpy(ctx.mem, &cart_ctx->rom_data[0], ROM_BANK_SIZE * 2);

	int ram_bank_count = 0;
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
	ctx.ram = calloc(1, (ram_bank_count ? RAM_BANK_SIZE * ram_bank_count : RAM_BANK_SIZE));

	ctx.rom_bank = 0;
	ctx.ram_bank = 0;
	ctx.vram_bank = 0;
}

u8 bus_read(u16 addr) {
	if (addr < 0x8000) {
		// 0x0000 - 0x7FFF
		// ROM SWITCHABLE BANK 0x4000
		return ctx.rom[addr];
	} else if (addr < 0xA000) {
		// 0x8000 - 0x9FFF
		// LCD RAM
		// bankable?
		//return ctx.mem[ctx.vram_bank * VRAM_BANK_SIZE + addr];
		return ctx.mem[addr];
	} else if (addr < 0xC000) {
		// 0xA000 - 0xBFFF
		// CART RAM
		// if (!ctx.ram_enabled)
		// 	return 0xFF;
		return ctx.mem[addr];
	} else if (addr < 0xE000) {
		// 0xC000 - 0xDFFF
		// bankable ram
		return ctx.mem[addr];
	} else if (addr < 0xFE00) {
		// 0xE000 - 0xFDFF
		// check echo ram access
		return ctx.mem[addr-0x2000];
	} else if (addr < 0xFE9F) {
		// 0xFE00 - 0xFE9F
		// when OAM blocked return 0xFF;
		// if (ctx.dma_transfer)
			// return 0xFF;
		return ctx.mem[addr];
	} else if (addr < 0xFEFF) {
		// 0xFEA0 - 0xFE9F
		// RESERVED / UNUSABLE
		return 0x0;
	} else if (addr < 0xFF80) {
		switch (addr) {
			case ADDR_DIV:
				return timer_read(ADDR_DIV);
			case ADDR_TIMA:
				return timer_read(ADDR_TIMA);
			case ADDR_TMA:
				return timer_read(ADDR_TMA);
			case ADDR_TAC:
				return timer_read(ADDR_TAC);
			case ADDR_IF:
				return ctx.mem[addr];
			break;
			case ADDR_LCDC:
				return ctx.mem[addr];
			break;
			case ADDR_STAT:
				return ctx.mem[addr];
			break;
			case ADDR_LY:
				return 0x90;
			break;
			default:
				return ctx.mem[addr];
		}
	} else if (addr < 0xFFFF) {
		// high ram
		return ctx.mem[addr];
	} else if (addr == 0xFFFF) {
		return ctx.mem[addr]; // ie
	} else {
		printf("ERR: bus_read not supported at address: %02X\n", addr);
	}

	return 0x0;
}

u16 bus_read16(u16 addr) {
	return bus_read(addr) | (bus_read(addr+1) << 8);
}

void bus_write(u16 addr, u8 val) {
	if (addr <= 0x1FFF) {
		// ROM SPACE
		// mbc1 logic
		// ctx.ram_enabled = (val == 0xA);
	} else if (0x2000 <= addr && addr < 0x8000) {
		// TODO: ROM / RAM switch
	} else if (0x8000 <= addr && addr < 0xA000) {
		//ctx.vram[(ctx.vram_bank + VRAM_BANK_SIZE) + (addr - 0x8000)] = val;
		ctx.mem[addr] = val;
	} else if (0xA000 <= addr && addr < 0xC000) {
		// cart ram (save progress)
		// TODO banking
		//ctx.ram[(ctx.ram_bank * RAM_BANK_SIZE) + (addr - 0xA000)] = val;
		ctx.mem[addr] = val;
	} else if (0xC000 <= addr && addr < 0xE000) {
		// bankable ram
		// if (ctx.ram_enabled)
		ctx.mem[addr] = val;
	} else if (addr >= 0xFE00 && addr < 0xFE9F) {
		// OAM
		// ctx.mem[addr] = val;
	} else if (addr >= 0xFEA0 && addr < 0xFEFF) {
		// NOT USABLE
	} else if (addr >= 0xFF00 && addr < 0xFF80) {
		switch (addr) {
			case ADDR_JOYPAD:
				// TODO
				ctx.mem[ADDR_JOYPAD] = val;
			break;
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
			case ADDR_IF:
				ctx.mem[addr] = 0xE0 | val;
			break;
			case ADDR_LCDC:
				ctx.mem[addr] = val;
			break;
			case ADDR_STAT:
				ctx.mem[addr] = val & 0xFC;
			break;
			case ADDR_LY:
				ctx.mem[addr] = val;
			break;
			case ADDR_DMA_TRANSFER:
				ctx.dma_transfer = true;
				ppu_dma_oam_transfer(val);
			break;
			default:
				ctx.mem[addr] = val;
			break;
		}
	} else if (addr >= 0xFF80 && addr < 0xFFFF) {
		ctx.mem[addr] = val;
	} else if (addr == 0xFFFF) {
		ctx.mem[addr] = val;
	} else {
		printf("ERR: bus_write not supported at address: %02X\n", addr);
	}
}

void bus_write16(u16 addr, u16 val) {
	bus_write(addr, val & 0xFF);
	bus_write(addr+1, (val >> 8) & 0xFF);
}
