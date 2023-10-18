#include "cpu.h"

#include <stdio.h>

#include "bus.h"
#include "cart.h"
#include "mbc.h"
#include "common.h"


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
#define CPU_SET_FLAG_N(x) CPU_REG_F = ((CPU_REG_F & 0xbf) | (x << 6))
#define CPU_SET_FLAG_H(x) CPU_REG_F = ((CPU_REG_F & 0xdf) | (x << 5))
#define CPU_SET_FLAG_C(x) CPU_REG_F = ((CPU_REG_F & 0xef) | (x << 4))

#define REGISTER_ADDR_KEY1 0xFF4D

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

u8 cpu_read_reg(cpu_register r) {
	u8* reg8ptr = cpu_reg8_ptr(r);
	if (reg8ptr != NULL) {
		return *reg8ptr;
	}
	return 0;
}

u16 cpu_read_reg16(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		return REVERSE(*reg16ptr);
	}
	return cpu_read_reg(r);
}

void cpu_write_reg16(cpu_register r, u16 v) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		*reg16ptr = v;
	} else {
		printf("ERR: invalid 16bit register: %0.02X\n", r);
	}
}

void cpu_write_reg(cpu_register r, u8 v) {
	u8* reg8ptr = cpu_reg8_ptr(r);
	if (reg8ptr != NULL) {
		*reg8ptr = v;
	} else {
		cpu_write_reg16(r, v);
	}
}

void cpu_inc_reg(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		(*reg16ptr)++;
	} else {
		u8* reg8ptr = cpu_reg8_ptr(r);
		if (reg8ptr != NULL) {
			(*reg8ptr)++;
		}
	}
}


void cpu_dec_reg(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		(*reg16ptr)--;
	} else {
		u8* reg8ptr = cpu_reg8_ptr(r);
		if (reg8ptr != NULL) {
			(*reg8ptr)--;
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
	ctx.write_bus = false;
}

void cpu_fetch_data() {
	ctx.fetched_data = 0;
	ctx.write_bus = false;
	ctx.write_dst = 0;

	switch (ctx.current_instruction.mode) {
		case MODE_NONE:
		break;
		case MODE_A16:
			ctx.fetched_data =  cpu_read_nn();
		break;
		case MODE_D8:
			ctx.fetched_data =  cpu_read_signed_n();
		break;
		case MODE_D8_TO_REG:
			ctx.fetched_data = cpu_read_signed_n();
		break;
		case MODE_D16:
			ctx.fetched_data =  cpu_read_nn();
		break;
		case MODE_REG:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
		break;
		case MODE_REG_TO_REG:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
		break;
		case MODE_REG_TO_ADDR:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.write_bus = true;
			if (ctx.current_instruction.r_target > REG_NONE) {
				u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
				ctx.write_dst = n;
			} else {
				ctx.write_dst = cpu_read_nn();
			}
		break;
		case MODE_REG_TO_IOADDR:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.write_bus = true;
			if (ctx.current_instruction.r_target > REG_NONE) {
				u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
				ctx.write_dst = 0xFF00+n;
			} else {
				ctx.write_dst = cpu_read_nn();
			}
		break;
		case MODE_ADDR_TO_REG:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(n);
		}
		case MODE_IOADDR_TO_REG:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(0xFF00+n);
		}
		break;
		case MODE_D16_TO_REG:
			ctx.fetched_data = cpu_read_nn();
		break;
		case MODE_IOADDR:
		{
			u16 addr = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(addr);
			ctx.write_bus = true;
			ctx.write_dst = addr;
		}
		break;
		case MODE_PARAM:
			ctx.fetched_data =  ctx.current_instruction.parameter;
		break;
		default:
			printf("ERR: address mode not supported.\n");
		break;
	}
}

void cpu_execute_ld(cpu_context *ctx) {
	if (ctx->fetched_data > 0xFF) {
		if (ctx->write_bus) {
			bus_write16(ctx->write_dst, ctx->fetched_data);
		} else {
			cpu_write_reg16(ctx->current_instruction.r_target, ctx->fetched_data);
		}
	} else {
		if (ctx->write_bus) {
			bus_write(ctx->write_dst, ctx->fetched_data & 0xff);
		} else {
			cpu_write_reg(ctx->current_instruction.r_target, ctx->fetched_data & 0xff);
		}
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

		case INSTRUCT_ADD:
			ctx.cycles += 1;
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			cpu_write_reg(ctx.current_instruction.r_target, n + ctx.fetched_data);
		break;

		case INSTRUCT_LD:
			ctx.cycles += 1;
			cpu_execute_ld(&ctx);
		break;

		case INSTRUCT_LDI:
			ctx.cycles += 1;
			cpu_execute_ld(&ctx);
			cpu_inc_reg(ctx.current_instruction.r_target);
		break;

		case INSTRUCT_LDD:
			ctx.cycles += 1;
			cpu_execute_ld(&ctx);
			cpu_dec_reg(ctx.current_instruction.r_target);
		break;

		case INSTRUCT_RST:
			ctx.cycles += 4;
			CPU_REG_SP -= 2;
			cpu_write_reg16(REG_SP, CPU_REG_PC);
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

		case INSTRUCT_INC:
			ctx.cycles += 1;
			if (ctx.write_bus) {
				bus_write(ctx.write_dst, ctx.fetched_data+1);
			} else {
				cpu_inc_reg(ctx.current_instruction.r_source);
			}
			if (ctx.current_instruction.r_target > REG_AF) {
				// TODO: set flags
				CPU_SET_FLAG_N(0);
			}
		break;

		case INSTRUCT_DEC:
			ctx.cycles += 1;
			if (ctx.write_bus) {
				bus_write16(ctx.write_dst, ctx.fetched_data-1);
			} else {
				cpu_dec_reg(ctx.current_instruction.r_source);
			}
			if (ctx.current_instruction.r_target > REG_AF) {
				// TODO: set flags
				CPU_SET_FLAG_N(1);
			}
		break;

		case INSTRUCT_CP:
		{
			ctx.cycles += 1;
			u16 a = cpu_read_reg(ctx.current_instruction.r_source);
			u16 r = (a - ctx.fetched_data) & 0xf;
			CPU_SET_FLAG_Z(a == ctx.fetched_data);
			CPU_SET_FLAG_N(1);
			CPU_SET_FLAG_H((r > a) & 0xf);
			CPU_SET_FLAG_C(a < ctx.fetched_data);
		}
		break;

		case INSTRUCT_RET:
			ctx.cycles += 1;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				cpu_write_reg16(REG_PC, CPU_REG_SP);
				CPU_REG_SP += 2;
			} else {
				cpu_write_reg16(REG_PC, CPU_REG_SP);
				CPU_REG_SP += 2;
			}
		break;

		case INSTRUCT_DI:
			ctx.cycles += 1;
			ctx.ime = false;
		break;

		case INSTRUCT_EI:
			ctx.cycles += 1;
			ctx.enable_ime = true;
		break;

		case INSTRUCT_STOP:
			ctx.cycles += 2;
			// only supported with CGB
			if (bus_read(REGISTER_ADDR_KEY1) & 0x1) {
				if (bus_read(REGISTER_ADDR_KEY1) & 0x80) {
					bus_write(REGISTER_ADDR_KEY1, 0);
				} else {
					bus_write(REGISTER_ADDR_KEY1, 0x80);
				}
			}
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

	printf("PC: %04X OPCODE: %2.2X | AF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X, SP: %04X, cycles: %04d | FLAGS Z=%d N=%d H=%d C=%d\n",
		CPU_REG_PC,
		ctx.current_opcode,
		CPU_REG_A, CPU_REG_F, CPU_REG_B, CPU_REG_C, CPU_REG_D, CPU_REG_E, CPU_REG_H, CPU_REG_L, CPU_REG_SP, ctx.cycles,
		CPU_FLAG_Z, CPU_FLAG_N, CPU_FLAG_H, CPU_FLAG_C);
}

void cpu_init() {
	cpu_write_reg16(REG_AF, 0x01B0);
	cpu_write_reg16(REG_BC, 0x0013);
	cpu_write_reg16(REG_DE, 0x00D8);
	cpu_write_reg16(REG_HL, 0x014D);
	cpu_write_reg16(REG_PC, 0x100);
	cpu_write_reg16(REG_SP, 0xFFFE);
}

bool cpu_step() {
	if (ctx.halted) {
		ctx.cycles += 1;
		if (ctx.interupt_flags) {
			ctx.halted = false;
		}
		return true;
	}

	cpu_fetch_instruction();
	_cpu_debug();
	CPU_REG_PC += 1; // ++ here because we want to print pc for debug
	cpu_fetch_data();
	cpu_execute_instruction();

	// DEBUG exit after 100 ticks
	return ctx.cycles < 100;
	return true;
}
