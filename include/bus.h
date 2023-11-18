#pragma once

#include <cart.h>
#include <common.h>

// 16-bit address bus
// 0x0000-0x7FFF 	 : PROGRAM DATA
	// 0x0000-0x3FFF 	- ROM BANK 0
	// 0x4000-0x7FFF 	- ROM BANK 1 - Switchable
// 0x8000-0x9FFF	 : LCD RAM
	// 0x8000-0x97FF 	 : CHR RAM / 8kB Video RAM (other 8kB accessed via BG bank switch)
	// 0x9800-0x9BFF 	 : BG  MAP1 / 4 KiB Work RAM (WRAM) / cartridge switchable bank
	// 0x9C00-0x9FFF 	 : BG  MAP2 / 4 KiB Work RAM (WRAM) / cartridge switchable bank
// 0xA000-0xBFFF 	 : CART RAM // external expansion RAM
// 0xC000-0xDFFF 	 : RAM (8KB)
	// 0xC000-0xCFFF 	- RAM BANK 0 (4KB)
	// 0xD000-0xDFFF 	- RAM BANK 1-7 - (Switchable CGB only) (4KB)
// 0xE000-0xFDFF : PROHIBTED
// 0xFE00-0xFFFF : Interanl CPU RAM
	// 0xFE00-0xFE9F : OAM (object attribute memory) RAM (40 objects)
	// 0xFEA0-0xFEFF : RESERVED - Unusable
	// 0xFF00-0xFF7F : IO REGISTERS
	// 0xFF80-0xFFFE : CPU work / stack RAM

#define ADDR_JOYPAD 0xFF00
#define ADDR_SB 0xFF01
#define ADDR_SC 0xFF02
#define ADDR_DIV 0xFF04
#define ADDR_TIMA 0xFF05
#define ADDR_TMA 0xFF06
#define ADDR_TAC 0xFF07
#define ADDR_IF 0xFF0F

#define ADDR_LCD 0xFF40
#define ADDR_STAT 0xFF41
#define ADDR_SCY 0xFF42
#define ADDR_SCX 0xFF43
#define ADDR_LY 0xFF44
#define ADDR_LYC 0xFF45
#define ADDR_WY 0xFF4A
#define ADDR_WX 0xFF48 // window.x - 7

#define ADDR_KEY1 0xFF4D

void bus_init(const cart_context* cart_ctx);
u8 bus_read(u16 addr);
u16 bus_read16(u16 addr);
void bus_write(u16 addr, u8 val);
void bus_write16(u16 addr, u16 val);
