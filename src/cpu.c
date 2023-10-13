#include "cpu.h"
#include "bus.h"
#include "cart.h"

#include <stdio.h>


#define CPU_REG_A ctx.registers.AF.bytes.h
#define CPU_REG_F ctx.registers.AF.bytes.l
#define CPU_REG_B ctx.registers.BC.bytes.h
#define CPU_REG_C ctx.registers.BC.bytes.l
#define CPU_REG_D ctx.registers.DE.bytes.h
#define CPU_REG_E ctx.registers.DE.bytes.l
#define CPU_REG_H ctx.registers.HL.bytes.h
#define CPU_REG_L ctx.registers.HL.bytes.l

#define CPU_REG_AF ctx.registers.AF.val
#define CPU_REG_BC ctx.registers.BC.val
#define CPU_REG_DE ctx.registers.DE.val
#define CPU_REG_HL ctx.registers.HL.val
#define CPU_REG_PC ctx.registers.PC.val
#define CPU_REG_SP ctx.registers.SP.val

#define CPU_FLAG_Z BIT(CPU_REG_F, 7)
#define CPU_FLAG_N BIT(CPU_REG_F, 6)
#define CPU_FLAG_H BIT(CPU_REG_F, 5)
#define CPU_FLAG_C BIT(CPU_REG_F, 4)

#define CPU_SET_FLAG_Z(x) CPU_REG_F = ((CPU_REG_F & 0x7f) | (x << 7))
#define CPU_SET_FLAG_N(x) CPU_REG_N = ((CPU_REG_N & 0xbf) | (x << 6))
#define CPU_SET_FLAG_H(x) CPU_REG_H = ((CPU_REG_H & 0xdf) | (x << 5))
#define CPU_SET_FLAG_C(x) CPU_REG_C = ((CPU_REG_C & 0xef) | (x << 4))

static cpu_context ctx;

u16* cpu_reg16_ptr(cpu_register r) {
    switch (r) {
        case REG_AF: return &(CPU_REG_AF);
        case REG_BC: return &(CPU_REG_BC);
        case REG_DE: return &(CPU_REG_DE);
        case REG_HL: return &(CPU_REG_HL);
        case REG_PC: return &(CPU_REG_PC);
        case REG_SP: return &(CPU_REG_SP);
        default: return NULL;
    }
}

u8* cpu_reg8_ptr(cpu_register r) {
    switch (r) {
        case REG_A: return &(CPU_REG_A);
        case REG_F: return &(CPU_REG_F);
        case REG_B: return &(CPU_REG_B);
        case REG_C: return &(CPU_REG_C);
        case REG_D: return &(CPU_REG_D);
        case REG_E: return &(CPU_REG_E);
        case REG_H: return &(CPU_REG_H);
        case REG_L: return &(CPU_REG_L);
        default: return NULL;
    }
}

u16 cpu_read_reg(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		return *reg16ptr;
	}
	u8* reg8ptr = cpu_reg8_ptr(r);
	if (reg8ptr != NULL) {
		return *reg8ptr;
	}
	return 0;
}

void cpu_write_reg(cpu_register r, u16 v) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		*reg16ptr = v;
	} else {
		u8* reg8ptr = cpu_reg8_ptr(r);
		if (reg8ptr != NULL) {
			*reg8ptr = (u8)v;
		}
	}
}

void cpu_inc_reg(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		*reg16ptr++;
	} else {
		u8* reg8ptr = cpu_reg8_ptr(r);
		if (reg8ptr != NULL) {
			*reg8ptr++;
		}
	}
}


void cpu_dec_reg(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		*reg16ptr--;
	} else {
		u8* reg8ptr = cpu_reg8_ptr(r);
		if (reg8ptr != NULL) {
			*reg8ptr--;
		}
	}
}

bool cpu_check_cond(cpu_condition_flag flag) {
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

u8 cpu_read_n() {
	ctx.cycles += 1;
	return bus_read(CPU_REG_PC++);
}

i8 cpu_read_signed_n() {
	ctx.cycles += 1;
	return bus_read(CPU_REG_PC++);
}

u16 cpu_read_nn() {
	ctx.cycles += 2;
	u16 nn = bus_read(CPU_REG_PC) | (bus_read(CPU_REG_PC+1) << 8);
	CPU_REG_PC += 2;
	return nn;
}

void cpu_fetch_instruction() {
	ctx.current_opcode = bus_read(CPU_REG_PC);
	ctx.current_instruction = instructions[ctx.current_opcode];
}

void cpu_fetch_data() {
	switch (ctx.current_instruction.mode) {
		case MODE_A16:
			ctx.fetched_data =  cpu_read_nn();
		break;
		case MODE_D8:
			ctx.fetched_data =  cpu_read_signed_n();
		break;
		case MODE_D16:
			ctx.fetched_data =  cpu_read_nn();
		break;
		case MODE_REG:
			ctx.fetched_data = cpu_read_reg(ctx.current_instruction.r_target);
		break;
		case MODE_REG_TO_REG:
			ctx.fetched_data = cpu_read_reg(ctx.current_instruction.r_source);
		break;
		case MODE_PARAM:
			ctx.fetched_data =  ctx.current_instruction.parameter;
		break;
	}
}

void cpu_execute_instruction() {
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
			cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data);
		break;

		case INSTRUCT_LDI:
			ctx.cycles += 1;
			cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data);
			cpu_inc_reg(ctx.current_instruction.r_target);
		break;

		case INSTRUCT_LDD:
			ctx.cycles += 1;
			cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data);
			cpu_dec_reg(ctx.current_instruction.r_target);
		break;

		case INSTRUCT_RST:
			ctx.cycles += 4;
			CPU_REG_SP -= 2;
			CPU_REG_PC = ctx.fetched_data;
		break;

		case INSTRUCT_JP:
			ctx.cycles += 1;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.cycles += 1;
				CPU_REG_PC = ctx.fetched_data;
			} else {
				ctx.cycles += 2;
				CPU_REG_PC += 2;
			}
		break;

		case INSTRUCT_JR:
			ctx.cycles += 1;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.cycles += 1;
				CPU_REG_PC += ctx.fetched_data;
			} else {
				ctx.cycles += 2;
				CPU_REG_PC += 2;
			}
		break;

		case INSTRUCT_CP:
			ctx.cycles += 1;
			u16 nn = cpu_read_reg(ctx.current_instruction.r_source);
			u16 r = nn - ctx.fetched_data;
			CPU_SET_FLAG_Z(nn == ctx.fetched_data);
		break;

		default:
			// ctx.cycles++;
			fprintf(stderr, "ERR: CPU step not implemented\n");
		break;
	}
}

void _cpu_debug() {
	// printf("PC: %2.2X | OPCODE: %2.2X NN: %2.2X | CYCLES: %d\n",
	// 	CPU_REG_PC-1,
	// 	ctx.current_opcode,
	// 	(bus_read(CPU_REG_PC) | (bus_read(CPU_REG_PC+1) << 8)),
	// 	ctx.cycles
	// );

	printf("PC: %04X OPCODE: %2.2X | AF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X, SP: %04X, cycles: %d\n",
		CPU_REG_PC,
		ctx.current_opcode,
		CPU_REG_A, CPU_REG_F, CPU_REG_B, CPU_REG_C, CPU_REG_D, CPU_REG_E, CPU_REG_H, CPU_REG_L, CPU_REG_SP, ctx.cycles);
}

void cpu_init() {
	cpu_write_reg(REG_AF, 0x13);
	cpu_write_reg(REG_HL, 0x014D);
	cpu_write_reg(REG_AF, 0x13);
	cpu_write_reg(REG_PC, 0x100);
	cpu_write_reg(REG_SP, 0xFFFE);
}

bool cpu_step() {
	if (ctx.halted) {
		ctx.cycles += 1;
		return true;
	}

	cpu_fetch_instruction();
	_cpu_debug();
	CPU_REG_PC += 1;
	cpu_fetch_data();
	cpu_execute_instruction();

	// if () {

	// }
	return ctx.cycles < 100;
	return true;
}
