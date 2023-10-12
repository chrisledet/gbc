#pragma once

#include <common.h>

// 16-bit address bus
// 0x0000-0x3FFF : ROM BANK 0
// 0x4000-0x7FFF : ROM BANK 1 - Switchable
// 0x8000-0x97FF : CHR RAM / 8kB Video RAM
// 0x9800-0x9BFF : BG  MAP1
// 0x9C00-0x9FFF : BG  MAP2
// 0xA000-0xBFFF : CART RAM
// 0xC000-0xCFFF : RAM BANK 0
// 0xD000-0xDFFF : RAM BANK 1-7 - Switchable - Color
// 0xE000-0xFDFF : RESERVED - Echo Ram
// 0xFE00-0xFE9F : OBJ ATTR MEMORY
// 0xFEA0-0xFEFF : RESERVED - Unusable
// 0xFF00-0xFF7F : IO REGISTERS
// 0xFF80-0xFFFE : Internal RAM

uint8_t bus_read(uint16_t addr);

void bus_write(uint16_t addr, uint8_t val);
