#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define BIT(a, n) ((a & (1 << n)) ? 1 : 0)
#define BIT_SET(a, n, on) (on ? (a) |= (1 << n) : (a) &= ~(1 << n))
#define BETWEEN(a, b, c) ((a >= b) && (a <= c))

void delay(uint32_t ms);
