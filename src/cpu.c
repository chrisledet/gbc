#include "cpu.h"

#include <stdio.h>

#include <common.h>
#include <bus.h>
#include <cart.h>
#include <timer.h>


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

#define CPU_SET_FLAG_Z(x) CPU_REG_F = ((CPU_REG_F & 0x7f) | (x << 7))
#define CPU_SET_FLAG_N(x) CPU_REG_F = ((CPU_REG_F & 0xbf) | (x << 6))
#define CPU_SET_FLAG_H(x) CPU_REG_F = ((CPU_REG_F & 0xdf) | (x << 5))
#define CPU_SET_FLAG_C(x) CPU_REG_F = ((CPU_REG_F & 0xef) | (x << 4))

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
		printf("ERR: invalid 8-bit register: %0.02X\n", r);
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

void cpu_execute_ld() {
	if (ctx.write_bus) {
		if (ctx.fetched_data > 0xFF) {
			bus_write16(ctx.write_dst, ctx.fetched_data);
		} else {
			bus_write(ctx.write_dst, ctx.fetched_data & 0xFF);
		}
	} else {
		if (ctx.current_instruction.r_target >= REG_AF) {
			cpu_write_reg16(ctx.current_instruction.r_target, ctx.fetched_data);
		} else {
			cpu_write_reg(ctx.current_instruction.r_target, ctx.fetched_data & 0xFF);
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
	ctx.cycles += 2;
	u16 nn = bus_read(ctx.registers.PC) | (bus_read(ctx.registers.PC+1) << 8);
	ctx.registers.PC += 2;
	return nn;
}

void cpu_fetch_instruction() {
	ctx.current_opcode = bus_read(ctx.registers.PC);
	ctx.current_instruction = instructions[ctx.current_opcode];
	ctx.registers.PC += 1;

	ctx.fetched_data = 0;
	ctx.write_bus = false;
	ctx.write_dst = 0;
}

void cpu_fetch_data() {
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
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_target);
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
				ctx.write_dst = ctx.current_instruction.byte_length > 2 ? cpu_read_nn() : cpu_read_n();
			}
		break;
		case MODE_REG_TO_IOADDR:
			ctx.fetched_data = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.write_bus = true;
			if (ctx.current_instruction.r_target > REG_NONE) {
				ctx.write_dst = 0xFF00 + cpu_read_reg16(ctx.current_instruction.r_target);
			} else {
				ctx.write_dst = ctx.current_instruction.byte_length > 2 ? cpu_read_nn() : cpu_read_n();
			}
		break;
		case MODE_ADDR_TO_REG:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(n);
		}
		break;
		case MODE_IOADDR_TO_REG:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(0xFF00+n);
		}
		break;
		case MODE_D16_TO_REG:
			ctx.fetched_data = cpu_read_nn();
		break;
		case MODE_ADDR:
		{
			u16 addr = cpu_read_reg16(ctx.current_instruction.r_source);
			ctx.fetched_data = bus_read(addr);
			ctx.write_bus = true;
			ctx.write_dst = addr;
		}
		break;
		case MODE_A8_TO_REG:
			ctx.fetched_data = bus_read(0xFF00 + cpu_read_n());
		break;
		case MODE_REG_TO_A8:
			ctx.fetched_data = cpu_read_n();
			ctx.write_bus = true;
			ctx.write_dst = 0xFF00 + ctx.fetched_data;
		break;
		case MODE_PARAM:
			ctx.fetched_data =  ctx.current_instruction.parameter;
		break;
		default:
			printf("ERR: address mode not supported.\n");
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

		case INSTRUCT_ADD:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n + ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}
			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H((n & 0xF) + (ctx.fetched_data & 0xF) > 0xF);
			CPU_SET_FLAG_C(r > 0xFF);
		}
		break;

		case INSTRUCT_ADC:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n + ctx.fetched_data + CPU_FLAG_Z;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(((n & 0xF) + (ctx.fetched_data & 0xF) + (CPU_FLAG_Z & 0xF)) > 0xF);
			CPU_SET_FLAG_C(r > 0xFF);
		}
		break;

		case INSTRUCT_SUB:
		{
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

		case INSTRUCT_SBC:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n - ctx.fetched_data - CPU_FLAG_Z;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(1);
			CPU_SET_FLAG_H((n & 0xF) < ((ctx.fetched_data & 0xF) + CPU_FLAG_Z));
			CPU_SET_FLAG_C(r < ctx.fetched_data);
		}
		break;

		case INSTRUCT_AND:
		{
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

		case INSTRUCT_XOR:
		{
			u16 r = cpu_read_reg16(ctx.current_instruction.r_target) ^ ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(0);
			CPU_SET_FLAG_C(0);
		}
		break;

		case INSTRUCT_OR:
		{
			u16 r = cpu_read_reg16(ctx.current_instruction.r_target) | ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				cpu_write_reg(ctx.current_instruction.r_target, r & 0xFF);
				ctx.cycles += 1;
			} else {
				cpu_write_reg16(ctx.current_instruction.r_target, r);
				ctx.cycles += 2;
			}

			CPU_SET_FLAG_Z(r == 0);
			CPU_SET_FLAG_N(0);
			CPU_SET_FLAG_H(0);
			CPU_SET_FLAG_C(0);
		}
		break;

		case INSTRUCT_CP:
		{
			u16 n = cpu_read_reg16(ctx.current_instruction.r_target);
			u16 r = n - ctx.fetched_data;
			if (ctx.current_instruction.r_target < REG_AF) {
				ctx.cycles += 1;
			} else {
				ctx.cycles += 2;
			}
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
				ctx.cycles += 1;
				ctx.registers.PC = ctx.fetched_data;
			} else {
				ctx.cycles += 2;
				ctx.registers.PC += 2;
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

		case INSTRUCT_INC:
		{
			ctx.cycles += 1;
			bool carry = (ctx.fetched_data & 0xF) == 0xF;
			ctx.fetched_data++;
			if (ctx.write_bus) {
				bus_write16(ctx.write_dst, ctx.fetched_data);
			}
			else {
				cpu_inc_reg(ctx.current_instruction.r_target);
			}

			if (ctx.write_bus || ctx.current_instruction.r_target < REG_AF) {
				CPU_SET_FLAG_Z(ctx.fetched_data == 0);
				CPU_SET_FLAG_N(0);
				CPU_SET_FLAG_H(carry);
			}
		}
		break;

		case INSTRUCT_DEC:
		{
			ctx.cycles += 1;
			ctx.fetched_data--;
			bool carry = (ctx.fetched_data & 0x0f) == 0x0f;
			if (ctx.write_bus) {
				bus_write16(ctx.write_dst, ctx.fetched_data);
			}
			else {
				cpu_dec_reg(ctx.current_instruction.r_target);
			}

			if (ctx.write_bus || ctx.current_instruction.r_target < REG_AF) {
				CPU_SET_FLAG_Z(ctx.fetched_data == 0);
				CPU_SET_FLAG_N(0);
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
    // printf("PC: %2.2X | OPCODE: %2.2X NN: %2.2X | CYCLES: %d\n",
    //  CPU_REG_PC-1,
    //  ctx.current_opcode,
    //  (bus_read(CPU_REG_PC) | (bus_read(CPU_REG_PC+1) << 8)),
    //  ctx.cycles
    // );
    // cpu_context *cpu_ctx = get_cpu_context()

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

void cpu_execute_interupts() {
	if (ctx.ime) {
		u8 ifs = bus_read(ADDR_IF);

		if ((ctx.IE & INTERRUPT_VBLANK) && (ifs & INTERRUPT_VBLANK)) {
			ctx.registers.PC -= 2;
			ctx.registers.SP = ctx.registers.PC;
			ctx.registers.PC = 0x40;

			ctx.ime = false;
			ctx.cycles += 20;
			bus_write(ADDR_IF, ifs & ~INTERRUPT_VBLANK);
		}
		if ((ctx.IE & INTERRUPT_LCD_STAT) && (ifs & INTERRUPT_LCD_STAT)) {
			ctx.registers.PC -= 2;
			ctx.registers.SP = ctx.registers.PC;
			ctx.registers.PC = 0x48;

			ctx.ime = false;
			ctx.cycles += 20;
			bus_write(ADDR_IF, ifs & ~INTERRUPT_LCD_STAT);
		}
		if ((ctx.IE & INTERRUPT_TIMER) && (ifs & INTERRUPT_TIMER)) {
			ctx.registers.PC -= 2;
			ctx.registers.SP = ctx.registers.PC;
			ctx.registers.PC = 0x50;

			ctx.ime = false;
			ctx.cycles += 20;
			bus_write(ADDR_IF, ifs & ~INTERRUPT_TIMER);
		}
		if ((ctx.IE & INTERRUPT_SERIAL) && (ifs & INTERRUPT_SERIAL)) {
			ctx.registers.PC -= 2;
			ctx.registers.SP = ctx.registers.PC;
			ctx.registers.PC = 0x58;

			ctx.ime = false;
			ctx.cycles += 20;
			bus_write(ADDR_IF, ifs & ~INTERRUPT_SERIAL);
		}
		if ((ctx.IE & INTERRUPT_JOYPAD) && (ifs & INTERRUPT_JOYPAD)) {
			ctx.registers.PC -= 2;
			ctx.registers.SP = ctx.registers.PC;
			ctx.registers.PC = 0x60;

			ctx.ime = false;
			ctx.cycles += 20;
			bus_write(ADDR_IF, ifs & ~INTERRUPT_SERIAL);
		}
	}
}

void cpu_update_graphics() {
	// TODO
}

u32 cpu_step() {
	u32 current_cycles = ctx.cycles;

	if (ctx.halted) {
		ctx.cycles += 1;
		if (ctx.IF) {
			ctx.halted = false;
		}
		return ctx.cycles - current_cycles;
	}

	if (/*ctx.debug_mode*/ 1) {
		cpu_debug();
	}

	if (ctx.enable_ime) {
		ctx.ime = true;
		ctx.enable_ime = false;
	}

	cpu_fetch_instruction();
	cpu_fetch_data();
	cpu_execute_instruction();

	if (ctx.ime) {
		cpu_execute_interupts();
		ctx.ime = false;
	}

	timer_tick(current_cycles * 4);
	cpu_update_graphics();

	return ctx.cycles - current_cycles;
}

void cpu_request_interrupt(u8 interrupt) {
	ctx.IF |= interrupt;
}

u8 cpu_get_ie_register() {
	return ctx.IE;
}

void cpu_set_ie_register(u8 value) {
	ctx.IE = value;
}
