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
} cpu_r16;

typedef struct {
	struct {
		cpu_r16 AF;
		cpu_r16 BC;
		cpu_r16 DE;
		cpu_r16 HL;
		cpu_r16 PC;
		cpu_r16 SP;
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
