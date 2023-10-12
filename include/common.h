#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
#define BIT_SET(a, n, on) (on ? (a) |= (1 << n) : (a) &= ~(1 << n))
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

void delay(uint32_t ms);
