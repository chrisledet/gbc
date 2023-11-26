#include "cpu.h"

#include <stdio.h>

#include "common.h"
#include "bus.h"
#include "cart.h"
#include "timer.h"
#include "interrupt.h"


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

#define CPU_FLAG_Z BIT(CPU_REG_F, 7)
#define CPU_FLAG_N BIT(CPU_REG_F, 6)
#define CPU_FLAG_H BIT(CPU_REG_F, 5)
#define CPU_FLAG_C BIT(CPU_REG_F, 4)

#define CPU_SET_FLAG_Z(x) CPU_REG_F = ((CPU_REG_F & 0x7f) | (x ? 0x80 : 0))
#define CPU_SET_FLAG_N(x) CPU_REG_F = ((CPU_REG_F & 0xbf) | (x ? 0x40 : 0))
#define CPU_SET_FLAG_H(x) CPU_REG_F = ((CPU_REG_F & 0xdf) | (x ? 0x20 : 0))
#define CPU_SET_FLAG_C(x) CPU_REG_F = ((CPU_REG_F & 0xef) | (x ? 0x10 : 0))

static cpu_context ctx;

u16* cpu_reg16_ptr(cpu_register r) {
    switch (r) {
        case REG_AF: return &(CPU_REG_AF);
        case REG_BC: return &(CPU_REG_BC);
        case REG_DE: return &(CPU_REG_DE);
        case REG_HL: return &(CPU_REG_HL);
        case REG_SP: return &ctx.registers.SP;
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
	} else {
		fprintf(stderr, "ERR: invalid 8-bit register: %.02X\n", r);
	}
	return 0;
}

u16 cpu_read_reg16(cpu_register r) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		return *reg16ptr;
	} else {
		return cpu_read_reg(r);
	}
}

void cpu_write_reg16(cpu_register r, u16 v) {
	u16* reg16ptr = cpu_reg16_ptr(r);
	if (reg16ptr != NULL) {
		*reg16ptr = v;
	} else {
		fprintf(stderr, "ERR: invalid 16bit register: %.02X\n", r);
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

void cpu_execute_ld() {
	if (ctx.write_dst) {
		if (ctx.fetched_data > 0xFF) {
			bus_write16(ctx.write_dst, ctx.fetched_data);
		} else {
			bus_write(ctx.write_dst, ctx.fetched_data & 0xFF);
		}
	} else {
		if (ctx.current_instruction.r_target >= REG_AF) {
			cpu_write_reg16(ctx.current_instruction.r_target, ctx.fetched_data);
		} else {
			cpu_write_reg(ctx.current_instruction.r_target, (ctx.fetched_data & 0xFF));
		}
	}
}

u8 cpu_read_n() {
	ctx.cycles += 1;
	return bus_read(ctx.registers.PC++);
}

i8 cpu_read_signed_n() {
	ctx.cycles += 1;
	return bus_read(ctx.registers.PC++);
}

u16 cpu_read_nn() {
	u16 lo = cpu_read_n();
	u16 hi = cpu_read_n() << 8;
	return lo | hi;
}

void cpu_fetch_instruction() {
	ctx.current_opcode = bus_read(ctx.registers.PC);
	ctx.registers.PC += 1;

	if (ctx.current_opcode == 0xCB)
		ctx.current_instruction = instructions[0x100 + bus_read(ctx.registers.PC++)];
	else
		ctx.current_instruction = instructions[ctx.current_opcode];

	ctx.fetched_data = 0;
	ctx.write_dst = 0;
}

void cpu_fetch_data() {
	switch (ctx.current_instruction.mode) {
		case MODE_NONE:
		break;
		case MODE_U8:
			ctx.fetched_data =  cpu_read_n();
		break;
		case MODE_U16:
		case MODE_A16:
			ctx.fetched_data = cpu_read_nn();
		break;
		case MODE_A16_TO_REG: {
			u16 addr = cpu_read_nn();
			ctx.fetched_data = bus_read(addr);
		}
		break;
		case MODE_U8_TO_REG:
			ctx.fetched_data = cpu_read_n();
			if (ctx.current_instruction.r_source)
				ctx.fetched_data += cpu_read_reg16(ctx.current_instruction.r_source);
		break;
		case MODE_D8_TO_ADDR:
			ctx.fetched_data = cpu_read_n();
			if (ctx.current_instruction.r_target)
				ctx.write_dst = cpu_read_reg16(ctx.current_instruction.r_target);
		break;
		case MODE_D8:
			ctx.fetched_data =  cpu_read_signed_n();
		break;
		case MODE_D8_TO_REG:
			ctx.fetched_data = cpu_read_signed_n();
			if (ctx.current_instruction.r_source)
				ctx.fetched_data += cpu_read_reg16(ctx.current_instruction.r_source);
		break;
		case MODE_REG:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_target);
		break;
		case MODE_REG_TO_REG:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
		break;
		case MODE_REG_TO_ADDR:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			if (ctx.current_instruction.r_target) {
				u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
				ctx.write_dst = n;
			} else {
				ctx.write_dst = ctx.current_instruction.byte_length > 2 ? cpu_read_nn() : cpu_read_n();
			}
		break;
		case MODE_REG_TO_IOADDR:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			if (ctx.current_instruction.r_target) {
				ctx.write_dst = 0xFF00 + cpu_read_reg16(ctx.current_instruction.r_target);
			} else {
				ctx.write_dst = ctx.current_instruction.byte_length > 2 ? cpu_read_nn() : cpu_read_n();
			}
		break;
		case MODE_ADDR_TO_REG: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read16(n);
		}
		break;
		case MODE_IOADDR_TO_REG: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(0xFF00+n);
		}
		break;
		case MODE_D16_TO_REG:
			ctx.fetched_data = cpu_read_nn();
		break;
		case MODE_ADDR: {
			u16 addr = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(addr);
			ctx.write_dst = addr;
		}
		break;
		case MODE_A8_TO_REG:
			ctx.fetched_data = bus_read(0xFF00 + cpu_read_n());
		break;
		case MODE_REG_TO_A8: {
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			u8 n = cpu_read_n();
			ctx.write_dst = 0xFF00 + n;
		}
		break;
		case MODE_PARAM:
			ctx.fetched_data =  ctx.current_instruction.parameter;
		break;
		default:
			fprintf(stderr, "ERR: address mode not supported: %02X\n", ctx.current_instruction.mode);
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

		case INSTRUCT_ADD: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u32 r = n + ctx.fetched_data;
			CPU_SET_FLAG_N(0);
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				CPU_SET_FLAG_Z((r & 0xFF) == 0);
				CPU_SET_FLAG_C(r > 0xFF);
				CPU_SET_FLAG_H(((n & 0xF) + (ctx.fetched_data & 0xF)) > 0xF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r & 0xFFFF);
				CPU_SET_FLAG_C(r > 0xFFFF);
				CPU_SET_FLAG_H((r & 0xFFF) < (n & 0xFFF));
				ctx.cycles += 2;
			}
		}
		break;

		case INSTRUCT_ADC: {
			u16 r = cpu_read_reg16(ctx.current_instruction.r_target);
			CPU_SET_FLAG_H(((r & 0xF) + (ctx.fetched_data & 0xF) + CPU_FLAG_C) > 0xF);

			r += ctx.fetched_data + CPU_FLAG_C;
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_C(r > 0xFF);

			if (ctx.current_instruction.r_target < REG_AF) {
				CPU_SET_FLAG_Z((r & 0xFF) == 0);
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				CPU_SET_FLAG_Z(r == 0);
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}
		}
		break;

		case INSTRUCT_SUB: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n - ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(1);
			CPU_SET_FLAG_H((n & 0xF) < (ctx.fetched_data & 0xF));
			CPU_SET_FLAG_C(n < ctx.fetched_data);
		}
		break;

		case INSTRUCT_SBC: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			bool c = CPU_FLAG_C;

			if (ctx.current_instruction.r_target < REG_AF) {
				u8 r = n - ctx.fetched_data - c;
				cpu_write_reg(ctx.current_instruction.r_target, r);
				CPU_SET_FLAG_Z(r == 0);
				ctx.cycles += 1;
			} else {
				u16 r = n - ctx.fetched_data - c;
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				CPU_SET_FLAG_Z(r == 0);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_H(((n & 0xF) - (ctx.fetched_data & 0xF) - c) < 0);
			CPU_SET_FLAG_C((n - ctx.fetched_data - c) < 0);
			CPU_SET_FLAG_N(1);
		}
		break;

		case INSTRUCT_AND: {
			u16 r = cpu_read_reg16(ctx.current_instruction.r_target) & ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(1);
			CPU_SET_FLAG_C(0);
		}
		break;

		case INSTRUCT_XOR: {
			u8 r = (cpu_read_reg(ctx.current_instruction.r_target) ^ ctx.fetched_data) & 0xFF;
			// target is always REG_A
			cpu_write_reg(ctx.current_instruction.r_target, r);
			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			ctx.cycles += 1;
		}
		break;

		case INSTRUCT_OR: {
			u16 r = cpu_read_reg16(ctx.current_instruction.r_target) | ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
		}
		break;

		case INSTRUCT_CP: {
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n - ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF)
				ctx.cycles += 1;
			else
				ctx.cycles += 2;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(1);
			CPU_SET_FLAG_H((n & 0xF) < (ctx.fetched_data & 0xF));
			CPU_SET_FLAG_C(n < ctx.fetched_data);
		}
		break;

		case INSTRUCT_LD:
			ctx.cycles += 1;
			cpu_execute_ld();
		break;

		case INSTRUCT_LDI:
			ctx.cycles += 1;
			cpu_execute_ld();
			cpu_inc_reg(REG_HL);
		break;

		case INSTRUCT_LDD:
			ctx.cycles += 1;
			cpu_execute_ld();
			cpu_dec_reg(REG_HL);
		break;

		case INSTRUCT_JP:
			ctx.cycles += 1;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.cycles += 2;
				ctx.registers.PC = ctx.fetched_data;
			} else {
				ctx.cycles += 1;
			}
		break;

		case INSTRUCT_JR:
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.cycles += 3;
				ctx.registers.PC += ctx.fetched_data;
			} else {
				ctx.cycles += 2;
			}
		break;

		case INSTRUCT_INC: {
			ctx.cycles += 1;
			bool carry = (ctx.fetched_data & 0xF) == 0xF;
			ctx.fetched_data++;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, ctx.fetched_data);
			else
				cpu_inc_reg(ctx.current_instruction.r_target);

			if (ctx.write_dst || ctx.current_instruction.r_target < REG_AF) {
				CPU_SET_FLAG_Z((ctx.fetched_data & 0xFF) == 0);
				CPU_SET_FLAG_N(0);
				CPU_SET_FLAG_H(carry);
			}
		}
		break;

		case INSTRUCT_DEC: {
			ctx.cycles += 1;
			ctx.fetched_data--;
			bool carry = (ctx.fetched_data & 0xF) == 0xF;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, ctx.fetched_data);
			else
				cpu_dec_reg(ctx.current_instruction.r_target);

			if (ctx.write_dst || ctx.current_instruction.r_target < REG_AF) {
				CPU_SET_FLAG_Z((ctx.fetched_data & 0xFF) == 0);
				CPU_SET_FLAG_N(1);
				CPU_SET_FLAG_H(carry);
			}
		}
		break;

		case INSTRUCT_RST:
			ctx.cycles += 4;
			ctx.registers.SP -= 2;
			bus_write16(ctx.registers.SP, ctx.registers.PC);
			ctx.registers.PC = ctx.fetched_data;
		break;

		case INSTRUCT_RET:
			ctx.cycles += 1;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.registers.PC = bus_read16(ctx.registers.SP);
				ctx.registers.SP += 2;
			}
		break;

		case INSTRUCT_RETI:
			ctx.registers.PC = bus_read16(ctx.registers.SP);
			ctx.registers.SP += 2;
			ctx.enable_ime = true;
			ctx.cycles += 1;
		break;

		case INSTRUCT_CALL:
			ctx.cycles += 2;
			if (cpu_check_cond(ctx.current_instruction.flag)) {
				ctx.registers.SP -= 2;
				bus_write16(ctx.registers.SP, ctx.registers.PC);
				ctx.registers.PC = ctx.fetched_data;
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
			if (bus_read(ADDR_KEY1) & 0x1) {
				if (bus_read(ADDR_KEY1) & 0x80) {
					bus_write(ADDR_KEY1, 0);
				} else {
					bus_write(ADDR_KEY1, 0x80);
				}
			}
		break;

		case INSTRUCT_RLCA: {
			u8 c = (ctx.fetched_data & 0x80) >> 7;
			u8 v = (ctx.fetched_data << 1) + c;
			cpu_write_reg(ctx.current_instruction.r_target, v);
			CPU_REG_F = 0;
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_RLA: {
			u8 c = (ctx.fetched_data & 0x80) >> 7;
			cpu_write_reg(ctx.current_instruction.r_target, (ctx.fetched_data << 1) + CPU_FLAG_C);
			CPU_REG_F = 0;
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_RRCA: {
			u8 c = ctx.fetched_data & 0x1;
			u8 v = (c << 7) | (ctx.fetched_data >> 1);
			cpu_write_reg(ctx.current_instruction.r_target, v);
			CPU_REG_F = 0;
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_RRA: {
			u8 c = (ctx.fetched_data & 0x1);
			u8 v = ((CPU_FLAG_C << 7) | (ctx.fetched_data >> 1)) & 0xFF;
			cpu_write_reg(ctx.current_instruction.r_target, v);
			CPU_REG_F = 0;
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_POP: {
			cpu_write_reg16(ctx.current_instruction.r_target, ctx.fetched_data);
			if (ctx.current_instruction.r_target == REG_AF)
				CPU_REG_F = ctx.fetched_data & 0xF0;
			ctx.registers.SP += 2;
			ctx.cycles += 3;
		}
		break;

		case INSTRUCT_PUSH: {
			ctx.registers.SP -= 2;
			bus_write16(ctx.registers.SP, ctx.fetched_data);
			ctx.cycles += 4;
		}
		break;

		case INSTRUCT_DAA: {
			if (!CPU_FLAG_N) {
				if (CPU_FLAG_C || CPU_REG_A > 0x99) {
					CPU_REG_A += 0x60;
					CPU_SET_FLAG_C(1);
				}
				if (CPU_FLAG_H || (CPU_REG_A & 0xF) > 9) CPU_REG_A += 0x06;
			} else {
				if (CPU_FLAG_H) CPU_REG_A = (CPU_REG_A - 0x06) & 0xFF;
				if (CPU_FLAG_C) CPU_REG_A -= 0x60;
			}
			CPU_SET_FLAG_Z(CPU_REG_A == 0);
			CPU_SET_FLAG_H(0);
			ctx.cycles += 1;
		}
		break;

		case INSTRUCT_CPL: {
			CPU_REG_A = ~CPU_REG_A;
			CPU_SET_FLAG_N(1);
			CPU_SET_FLAG_H(1);
			ctx.cycles += 1;
		}
		break;

		case INSTRUCT_SCF: {
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(0);
			CPU_SET_FLAG_C(1);
			ctx.cycles += 1;
		}
		break;

		case INSTRUCT_CCF: {
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(0);
			CPU_SET_FLAG_C(!CPU_FLAG_C);
			ctx.cycles += 1;
		}
		break;

		case INSTRUCT_CB_SET: {
			ctx.fetched_data |= (1 << ctx.current_instruction.parameter);
			if (ctx.write_dst)
				bus_write(ctx.write_dst, ctx.fetched_data & 0xff);
			else
				cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data & 0xff);

			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_RES: {
			ctx.fetched_data &= ~(1 << ctx.current_instruction.parameter);
			if (ctx.write_dst)
				bus_write(ctx.write_dst, ctx.fetched_data & 0xff);
			else
				cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data & 0xff);

			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_BIT: {
			u8 r = (ctx.fetched_data & (1 << ctx.current_instruction.parameter)) & 0xFF;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(1);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_RLC: {
			u8 c = (ctx.fetched_data & 0x80) >> 7;
			u8 r = ((ctx.fetched_data << 1) & 0xFF) + c;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_RL: {
			u8 c = (ctx.fetched_data & 0x80) >> 7;
			u8 r = ((ctx.fetched_data << 1) & 0xFF) + CPU_FLAG_C;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_RR: {
			u8 c = (ctx.fetched_data & 0x1);
			u8 r = ((CPU_FLAG_C << 7) | (ctx.fetched_data >> 1)) & 0xFF;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_RRC: {
			u8 c = (ctx.fetched_data & 0x1);
			u8 r = ((c << 7) | (ctx.fetched_data >> 1)) & 0xFF;

			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_SLA: {
			u8 c = (ctx.fetched_data & 0x80) >> 7;
			u8 r = (ctx.fetched_data << 1) & 0xFF;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_SRA: {
			u8 c = (ctx.fetched_data & 0x1);
			u8 r = ((ctx.fetched_data & 0x80) | (ctx.fetched_data >> 1)) & 0xFF;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_SWAP: {
			u8 r = ((ctx.fetched_data << 4) | (ctx.fetched_data >> 4)) & 0xFF;
			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			ctx.cycles += 2;
		}
		break;

		case INSTRUCT_CB_SRL: {
			u8 c = (ctx.fetched_data & 0x1);
			u8 r = (ctx.fetched_data >> 1) & 0xFF;

			if (ctx.write_dst)
				bus_write16(ctx.write_dst, r);
			else
				cpu_write_reg(ctx.current_instruction.r_target, r);

			CPU_REG_F = 0;
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_C(c);
			ctx.cycles += 2;
		}
		break;

		default:
			// ctx.cycles++;
			fprintf(stderr, "ERR: CPU step not implemented\n");
		break;
	}
}

void cpu_init() {
	cpu_write_reg16(REG_AF, 0x01B0);
	cpu_write_reg16(REG_BC, 0x0013);
	cpu_write_reg16(REG_DE, 0x00D8);
	cpu_write_reg16(REG_HL, 0x014D);

	ctx.registers.PC = 0x100;
	ctx.registers.SP = 0xFFFE;
}

void cpu_debug() {
	// game boy doctor format
	// printf("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
	// 	cpu_read_reg(REG_A), cpu_read_reg(REG_F), cpu_read_reg(REG_B), cpu_read_reg(REG_C), cpu_read_reg(REG_D), cpu_read_reg(REG_E), cpu_read_reg(REG_H), cpu_read_reg(REG_L), ctx.registers.SP, ctx.registers.PC,
	// 	bus_read(ctx.registers.PC), bus_read(ctx.registers.PC + 1), bus_read(ctx.registers.PC + 2), bus_read(ctx.registers.PC + 3)
	// );

    printf("PC: 0x%04X (%02X %02X %02X %02X) | AF: %02X%02X, BC: %02X%02X, DE: %02X%02X, HL: %02X%02X, SP: %04X, cycles: %04d | FLAGS Z=%d N=%d H=%d C=%d | DIV: %02X | TIMA: %02X | TMA: %02X | TAC: %02X\n",
       ctx.registers.PC,
       bus_read(ctx.registers.PC),
       bus_read(ctx.registers.PC+1),
       bus_read(ctx.registers.PC+2),
       bus_read(ctx.registers.PC+3),
       cpu_read_reg(REG_A), cpu_read_reg(REG_F), cpu_read_reg(REG_B), cpu_read_reg(REG_C), cpu_read_reg(REG_D), cpu_read_reg(REG_E), cpu_read_reg(REG_H), cpu_read_reg(REG_L), ctx.registers.SP, ctx.cycles,
       CPU_FLAG_Z, CPU_FLAG_N, CPU_FLAG_H, CPU_FLAG_C,
       timer_read(ADDR_DIV), timer_read(ADDR_TIMA), timer_read(ADDR_TMA), timer_read(ADDR_TAC));
}

u8 cpu_execute_interrupts() {
	if (!ctx.ime)
		return 0;

	u8 interrupt_addr = 0, interrupt_flag = 0, cycles = 0;
	u8 ie = bus_read(ADDR_IE);
	u8 ifs = bus_read(ADDR_IF);

	if ((ie & INTERRUPT_VBLANK) && (ifs & INTERRUPT_VBLANK)) {
		interrupt_addr = 0x40;
		interrupt_flag = INTERRUPT_VBLANK;
	} else if ((ie & INTERRUPT_LCD_STAT) && (ifs & INTERRUPT_LCD_STAT)) {
		interrupt_addr = 0x48;
		interrupt_flag = INTERRUPT_LCD_STAT;
	} else if ((ie & INTERRUPT_TIMER) && (ifs & INTERRUPT_TIMER)) {
		interrupt_addr = 0x50;
		interrupt_flag = INTERRUPT_TIMER;
	} else if ((ie & INTERRUPT_SERIAL) && (ifs & INTERRUPT_SERIAL)) {
		interrupt_addr = 0x58;
		interrupt_flag = INTERRUPT_SERIAL;
	} else if ((ie & INTERRUPT_JOYPAD) && (ifs & INTERRUPT_JOYPAD)) {
		interrupt_addr = 0x60;
		interrupt_flag = INTERRUPT_JOYPAD;
	}

	if (interrupt_addr && interrupt_flag) {
		ctx.ime = false;
		ctx.registers.SP -= 2;
		bus_write16(ctx.registers.SP, ctx.registers.PC);
		ctx.registers.PC = interrupt_addr;
		cycles = 20;
		bus_write(ADDR_IF, ifs & ~interrupt_flag);
	}

	return cycles;
}

u32 cpu_step() {
	ctx.cycles = 0;

	if (ctx.halted) {
		// TODO: handle halt bug
		u8 ifs = bus_read(ADDR_IF);
		u8 ie = bus_read(ADDR_IE);
		if (ctx.ime && ifs && ie)
			ctx.halted = false;
		return ++ctx.cycles;
	}

	if (ctx.ime)
		ctx.cycles += cpu_execute_interrupts();

	if (ctx.enable_ime) {
		ctx.ime = true;
		ctx.enable_ime = false;
	}

	cpu_fetch_instruction();
	cpu_fetch_data();
	cpu_execute_instruction();

	return ctx.cycles;
}

void cpu_request_interrupt(u8 i) {
	u8 r = bus_read(ADDR_IF) | i;
	bus_write(ADDR_IF, r);
}
