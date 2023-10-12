#pragma once

#include "common.h"
#include "instruct.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t a; // AF
	uint8_t f; // ZNHC0000
	uint8_t b; // bc
	uint8_t c;
	uint8_t d; // de
	uint8_t e;
	uint8_t h; // hl
	uint8_t l;
	uint16_t pc; // program counter
	uint16_t sp; // stack pointer
} cpu_registers;

typedef struct {
	cpu_registers registers;
	u8 current_opcode;
	cpu_instruction current_instruction;
	u16 fetched_data;
	u32 cycles;
	bool halted;
	bool stopped;
} cpu_context;

// prep cpu
void cpu_init();

// perform next execution step
bool cpu_step();
