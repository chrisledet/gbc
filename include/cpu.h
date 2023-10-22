#pragma once

#include <common.h>
#include <instruct.h>

#include <stdbool.h>
#include <stdint.h>


enum {
	INTERRUPT_VBLANK 	= 0x1,
	INTERRUPT_LCD_STAT 	= 0x2,
	INTERRUPT_TIMER 	= 0x4,
	INTERRUPT_SERIAL 	= 0x8,
	INTERRUPT_JOYPAD 	= 0x10,
};

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
	u8 current_opcode;
	u32 cycles;
	cpu_instruction current_instruction;

	u16 fetched_data;
	bool write_bus;
	u16 write_dst; // bus address to write to

	bool ime;
	bool enable_ime;
	u8 IE; // which interrupts _can_ be called
	u8 IF; // which interrupts _want_ to be called
} cpu_context;

void cpu_init();
u32 cpu_step();
void cpu_request_interrupt(u8 interrupt);
u8 cpu_get_ie_register();
void cpu_set_ie_register(u8 value);
