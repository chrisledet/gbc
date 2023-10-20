#pragma once

#include "common.h"
#include "instruct.h"

#include <stdbool.h>
#include <stdint.h>


typedef union {
	u16 val;
	struct {
		u8 l;
		u8 h;
	} bytes;
} r16;

typedef struct {
	struct {
		r16 AF;
		r16 BC;
		r16 DE;
		r16 HL;
		u16 PC;
		u16 SP;
	} registers;
	u8 current_opcode;
	cpu_instruction current_instruction;
	u16 fetched_data;
	u32 cycles;
	bool halted;
	bool stopped;
	bool write_bus;
	u16 write_dst; // bus address to write to

	bool ime;
	bool enable_ime;
	u8 interupt_flags;
} cpu_context;

void cpu_init();
bool cpu_step();
