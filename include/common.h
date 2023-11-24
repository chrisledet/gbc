#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))
#define REVERSE(n) ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8)

#define ADDR_OAM 0xFE00

#define ADDR_JOYPAD 0xFF00
#define ADDR_SB 0xFF01
#define ADDR_SC 0xFF02
#define ADDR_DIV 0xFF04
#define ADDR_TIMA 0xFF05
#define ADDR_TMA 0xFF06
#define ADDR_TAC 0xFF07
#define ADDR_IF 0xFF0F


#define ADDR_LCDC 0xFF40
#define ADDR_STAT 0xFF41
#define ADDR_SCY 0xFF42
#define ADDR_SCX 0xFF43
#define ADDR_LY 0xFF44
#define ADDR_LYC 0xFF45
#define ADDR_DMA_TRANSFER 0xFF46
#define ADDR_WY 0xFF4A
#define ADDR_WX 0xFF48
#define ADDR_KEY1 0xFF4D

#define ADDR_IE 0xFFFF
