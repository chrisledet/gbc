#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))
#define REVERSE(n) ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8)

void delay(uint32_t ms);
