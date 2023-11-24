#pragma once

#include <common.h>
#include <instruct.h>
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
	bool halted;
	bool stopped;
	u32 cycles;
	// instruction state
	u8 current_opcode;
	cpu_instruction current_instruction;
	u16 fetched_data;
	bool write_bus;
	u16 write_dst;
	// interrupts
	bool ime;
	bool enable_ime;
} cpu_context;

void cpu_init();
u32 cpu_step();
void cpu_request_interrupt(u8 interrupt);
