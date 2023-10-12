#include "cpu.h"
#include "bus.h"
#include "cart.h"

#include <stdio.h>



static cpu_context ctx = {0};

#define CPU_FLAG_Z BIT(ctx.registers.f, 7)
#define CPU_FLAG_C BIT(ctx.registers.f, 4)
#define CPU_LD_RR(x, y) do { ctx.registers.x = ctx.registers.y; } while(0)
#define CPU_SET_REG_PAIR(high, low, v) do { ctx.registers.high = (v) >> 8; ctx.registers.low = (v) & 0xFF; } while(0)
#define REVERSE(n) ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8)

u16 cpu_read_reg(cpu_register r) {
	switch (r) {
		case REG_A: return ctx.registers.a;
		case REG_F: return ctx.registers.f;
		case REG_B: return ctx.registers.b;
		case REG_C: return ctx.registers.c;
		case REG_D: return ctx.registers.d;
		case REG_E: return ctx.registers.e;
		case REG_H: return ctx.registers.h;
		case REG_L: return ctx.registers.l;
		case REG_AF: return REVERSE(*((u16 *)&ctx.registers.a));
		case REG_BC: return REVERSE(*((u16 *)&ctx.registers.b));
		case REG_DE: return REVERSE(*((u16 *)&ctx.registers.d));
		case REG_HL: return REVERSE(*((u16 *)&ctx.registers.h));
		case REG_PC: return ctx.registers.pc;
		case REG_SP: return ctx.registers.sp;
		default: return 0;
	}
}

bool _cpu_check_cond(cpu_condition_flag flag) {
	bool z = CPU_FLAG_Z;
	bool c = CPU_FLAG_C;

	switch (flag) {
		case FLAG_NONE: return true;
		case FLAG_Z: return z;
		case FLAG_NZ: return !z;
		case FLAG_C: return c;
		case FLAG_NC: return !c;
		default: return false;
	}
}


u8 _cpu_read_n() {
	ctx.cycles += 1;
	return bus_read(ctx.registers.pc++);
}

i8 _cpu_read_signed_n() {
	ctx.cycles += 1;
	return bus_read(ctx.registers.pc++);
}

u16 _cpu_read_nn() {
	ctx.cycles += 2;
	u16 nn = bus_read(ctx.registers.pc) | (bus_read(ctx.registers.pc+1) << 8);
	ctx.registers.pc += 2;
	return nn;
}

void _cpu_fetch_instruction() {
	ctx.current_opcode = bus_read(ctx.registers.pc);
	ctx.current_instruction = instructions[ctx.current_opcode];
	ctx.registers.pc += 1;
}

void _cpu_fetch_data() {
	switch (ctx.current_instruction.mode) {
		case MODE_A16:
			ctx.fetched_data =  _cpu_read_nn();
		break;
		case MODE_D8:
			ctx.fetched_data =  _cpu_read_signed_n();
		break;
		case MODE_REG:
			ctx.fetched_data = cpu_read_reg(ctx.current_instruction.register_1);
		break;
		case MODE_REG_TO_REG:
			ctx.fetched_data = cpu_read_reg(ctx.current_instruction.register_2);
		break;
		case MODE_PARAM:
			ctx.fetched_data =  ctx.current_instruction.parameter;
		break;
	}
}

void _cpu_execute_instruction() {
	switch (ctx.current_instruction.type) {
		case INSTRUCT_NOP:
			ctx.cycles += 1;
		break;

		case INSTRUCT_HALT:
			ctx.cycles += 1;
			ctx.halted = true;
		break;

		case INSTRUCT_LD:
			ctx.cycles += 1;
			ctx.current_instruction.register_1 = ctx.fetched_data;
		break;
		// case INSTRUCT_LD:
			// ctx.cycles += 1;
			// ctx.current_instruction.register_1
		// break;

		case INSTRUCT_RST:
			ctx.cycles += 4;
			ctx.registers.sp -= 2;
			ctx.registers.pc = ctx.fetched_data;
		break;

		case INSTRUCT_JP:
			ctx.cycles += 1;
			if (_cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.cycles += 1;
				ctx.registers.pc = ctx.fetched_data;
			} else {
				ctx.cycles += 2;
				ctx.registers.pc += 2;
			}
		break;

		case INSTRUCT_JR:
			ctx.cycles += 1;
			if (_cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.registers.pc += ctx.fetched_data;
				ctx.cycles += 1;
			} else {
				ctx.cycles += 2;
				ctx.registers.pc += 2;
			}
		break;

		default:
			// ctx.cycles++;
			fprintf(stderr, "ERR: CPU step not implemented\n");
		break;
	}
}

void _cpu_set_AF(u16 v) {
	CPU_SET_REG_PAIR(a, f, v);
}

void _cpu_set_BC(u16 v) {
	CPU_SET_REG_PAIR(b, c, v);
}

void _cpu_set_HL(u16 v) {
	CPU_SET_REG_PAIR(h, l, v);
}

void cpu_init() {
	_cpu_set_AF(0x11); // gbc
	_cpu_set_BC(0x13);
	_cpu_set_HL(0x014D);
	ctx.registers.pc = 0x100;
	ctx.registers.sp = 0xFFFE;
}

void _cpu_debug() {
	printf("PC: %2.2X | OPCODE: %2.2X NN: %2.2X | CYCLES: %d\n",
		ctx.registers.pc-1,
		ctx.current_opcode,
		(bus_read(ctx.registers.pc) | (bus_read(ctx.registers.pc+1) << 8)),
		ctx.cycles
	);
}

bool cpu_step() {
	if (ctx.halted) {
		ctx.cycles += 1;
		return true;
	}

	_cpu_fetch_instruction();
	_cpu_fetch_data();
	_cpu_debug();
	_cpu_execute_instruction();

	// if () {

	// }
	return ctx.cycles < 100;
	return true;
}
