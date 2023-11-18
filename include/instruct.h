#include <stdint.h>
#include <common.h>

typedef enum {
	INSTRUCT_NONE,
	INSTRUCT_NOP,
	INSTRUCT_ADD,
	INSTRUCT_ADC,
	INSTRUCT_SUB,
	INSTRUCT_SBC,
	INSTRUCT_AND,
	INSTRUCT_XOR,
	INSTRUCT_CP,
	INSTRUCT_OR,
	INSTRUCT_LD,
	INSTRUCT_LDI,
	INSTRUCT_LDD,
	INSTRUCT_HALT,
	INSTRUCT_JP,
	INSTRUCT_JR,
	INSTRUCT_RST,
	INSTRUCT_RET,
	INSTRUCT_RETI,
	INSTRUCT_DEC,
	INSTRUCT_DI,
	INSTRUCT_EI,
	INSTRUCT_STOP,
	INSTRUCT_INC,
	INSTRUCT_CALL,
	INSTRUCT_RLCA,
	INSTRUCT_RLA,
	INSTRUCT_RRC,
	INSTRUCT_RRA,
	INSTRUCT_CB,
	INSTRUCT_CB_SET,
	INSTRUCT_CB_RES,
	INSTRUCT_CB_BIT,
	INSTRUCT_CB_SRL,
	INSTRUCT_CB_SWAP,
	INSTRUCT_CB_SRA,
	INSTRUCT_CB_SLA,
	INSTRUCT_CB_RR,
	INSTRUCT_CB_RL,
	INSTRUCT_CB_RRC,
	INSTRUCT_CB_RLC,
	INSTRUCT_PUSH,
	INSTRUCT_POP,
	INSTRUCT_DAA,
	INSTRUCT_CPL,
	INSTRUCT_SCF,
	INSTRUCT_CCF,
} cpu_instruction_type;

typedef enum {
	REG_NONE,
	REG_A,
	REG_F,
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_H,
	REG_L,
	REG_AF,
	REG_BC,
	REG_DE,
	REG_HL,
	REG_SP,
} cpu_register;

typedef enum {
	MODE_NONE,
	MODE_D8,  // 8-bit data
	MODE_D16, // 16-bit data
	MODE_U8, // 8-bit unsigned data
	MODE_U16, // 16-bit unsigned data
	MODE_A16, // 16-bit address
	MODE_REG, // register value
	MODE_REG_TO_REG, // register to register
	MODE_REG_TO_A8, // (a8), reg
	MODE_ADDR_TO_REG, // reg, $addr
	MODE_IOADDR_TO_REG, // reg, 0xff00 + $addr
	MODE_A8_TO_REG, // reg, (0xff00 + a8)
	MODE_D8_TO_REG, // reg, d8
	MODE_U8_TO_REG, // reg, u8
	MODE_D16_TO_REG, // reg, d16
	MODE_REG_TO_ADDR, // $addr, reg
	MODE_REG_TO_IOADDR, // 0xff00 + $addr, reg
	MODE_ADDR, // $(reg)
	MODE_PARAM, // fixed value
} cpu_address_mode;

typedef enum {
	FLAG_NONE,
	FLAG_Z,  // Zero flag
	FLAG_C,  // Carry flag
	FLAG_H,  // Half carry flag
	FLAG_N,  // Subtraction flag
	FLAG_NZ, // Not zero flag
	FLAG_NC, // Not carry flag
} cpu_condition_flag;

typedef struct {
	cpu_instruction_type type;
	cpu_address_mode mode;
	cpu_register r_target;
	cpu_register r_source;
	cpu_condition_flag flag;
	u8 parameter;
	u8 byte_length;
} cpu_instruction;

static const cpu_instruction instructions[0x200] = {
	[0x00] = {INSTRUCT_NOP, MODE_NONE},
	[0x01] = {INSTRUCT_LD, MODE_D16, REG_BC, .byte_length = 3},
	[0x02] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_BC, REG_A},
	[0x03] = {INSTRUCT_INC, MODE_REG, REG_BC},
	[0x04] = {INSTRUCT_INC, MODE_REG, REG_B},
	[0x05] = {INSTRUCT_DEC, MODE_REG, REG_B},
	[0x06] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_B},
	[0x07] = {INSTRUCT_RLCA},
	[0x08] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_NONE, REG_SP, .byte_length = 3},
	[0x09] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_HL, REG_BC},
	[0x0A] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_A, REG_BC},
	[0x0B] = {INSTRUCT_DEC, MODE_REG, REG_BC},
	[0x0C] = {INSTRUCT_INC, MODE_REG, REG_C},
	[0x0D] = {INSTRUCT_DEC, MODE_REG, REG_C},
	[0x0E] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_C},
	[0x0F] = {INSTRUCT_RRC},

	[0x10] = {INSTRUCT_STOP},
	[0x11] = {INSTRUCT_LD, MODE_D16, REG_DE, .byte_length = 3},
	[0x12] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_DE, REG_A},
	[0x13] = {INSTRUCT_INC, MODE_REG, REG_DE},
	[0x14] = {INSTRUCT_INC, MODE_REG, REG_D},
	[0x15] = {INSTRUCT_DEC, MODE_REG, REG_D},
	[0x16] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_D},
	[0x17] = {INSTRUCT_RLA},
	[0x18] = {INSTRUCT_JR, MODE_D8},
	[0x19] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_HL, REG_DE},
	[0x1A] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_A, REG_DE},
	[0x1B] = {INSTRUCT_DEC, MODE_REG, REG_DE},
	[0x1C] = {INSTRUCT_INC, MODE_REG, REG_E},
	[0x1D] = {INSTRUCT_DEC, MODE_REG, REG_E},
	[0x1E] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_E},
	[0x1F] = {INSTRUCT_RRA},

	[0x20] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_NZ},
	[0x21] = {INSTRUCT_LD, MODE_D16, REG_HL, .byte_length = 3},
	[0x22] = {INSTRUCT_LDI, MODE_REG_TO_ADDR, REG_HL, REG_A},
	[0x23] = {INSTRUCT_INC, MODE_REG, REG_HL},
	[0x24] = {INSTRUCT_INC, MODE_REG, REG_H},
	[0x25] = {INSTRUCT_DEC, MODE_REG, REG_H},
	[0x26] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_H},
	[0x27] = {INSTRUCT_DAA},
	[0x28] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_Z},
	[0x29] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_HL, REG_HL},
	[0x2A] = {INSTRUCT_LDI, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x2B] = {INSTRUCT_DEC, MODE_REG, REG_HL},
	[0x2C] = {INSTRUCT_INC, MODE_REG, REG_L},
	[0x2D] = {INSTRUCT_DEC, MODE_REG, REG_L},
	[0x2E] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_L},
	[0x2F] = {INSTRUCT_CPL},

	[0x30] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_NC},
	[0x31] = {INSTRUCT_LD, MODE_D16_TO_REG, REG_SP, .byte_length = 3},
	[0x32] = {INSTRUCT_LDD, MODE_REG_TO_ADDR, REG_HL, REG_A},
	[0x33] = {INSTRUCT_INC, MODE_REG, REG_SP},
	[0x34] = {INSTRUCT_INC, MODE_ADDR, REG_NONE, REG_HL},
	[0x35] = {INSTRUCT_DEC, MODE_ADDR, REG_NONE, REG_HL},
	[0x36] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_HL},
	[0x37] = {INSTRUCT_SCF},
	[0x38] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_C},
	[0x39] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_HL, REG_SP},
	[0x3A] = {INSTRUCT_LDD, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x3B] = {INSTRUCT_DEC, MODE_REG, REG_SP},
	[0x3C] = {INSTRUCT_INC, MODE_REG, REG_A},
	[0x3D] = {INSTRUCT_DEC, MODE_REG, REG_A},
	[0x3E] = {INSTRUCT_LD, MODE_U8_TO_REG, REG_A},
	[0x3F] = {INSTRUCT_CCF},

	[0x40] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_B},
	[0x41] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_C},
	[0x42] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_D},
	[0x43] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_E},
	[0x44] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_H},
	[0x45] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_L},
	[0x46] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_HL},
	[0x47] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_B, REG_A},
	[0x48] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_B},
	[0x49] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_C},
	[0x4A] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_D},
	[0x4B] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_E},
	[0x4C] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_H},
	[0x4D] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_L},
	[0x4E] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_HL},
	[0x4F] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_C, REG_A},

	[0x50] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_B},
	[0x51] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_C},
	[0x52] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_D},
	[0x53] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_E},
	[0x54] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_H},
	[0x55] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_L},
	[0x56] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_HL},
	[0x57] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_D, REG_A},
	[0x58] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_B},
	[0x59] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_C},
	[0x5A] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_D},
	[0x5B] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_E},
	[0x5C] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_H},
	[0x5D] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_L},
	[0x5E] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_HL},
	[0x5F] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_E, REG_A},

	[0x60] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_B},
	[0x61] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_C},
	[0x62] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_D},
	[0x63] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_E},
	[0x64] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_H},
	[0x65] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_L},
	[0x66] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_HL},
	[0x67] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_H, REG_A},
	[0x68] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_B},
	[0x69] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_C},
	[0x6A] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_D},
	[0x6B] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_E},
	[0x6C] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_H},
	[0x6D] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_L},
	[0x6E] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_HL},
	[0x6F] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_L, REG_A},

	[0x70] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_B},
	[0x71] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_C},
	[0x72] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_D},
	[0x73] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_E},
	[0x74] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_H},
	[0x75] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_L},
	[0x76] = {INSTRUCT_HALT},
	[0x77] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_HL, REG_A},
	[0x78] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_B},
	[0x79] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_C},
	[0x7A] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_D},
	[0x7B] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_E},
	[0x7C] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_H},
	[0x7D] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_L},
	[0x7E] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x7F] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_A, REG_A},

	[0x80] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_B},
	[0x81] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_C},
	[0x82] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_D},
	[0x83] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_E},
	[0x84] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_H},
	[0x85] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_L},
	[0x86] = {INSTRUCT_ADD, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x87] = {INSTRUCT_ADD, MODE_REG_TO_REG, REG_A, REG_A},
	[0x88] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_B},
	[0x89] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_C},
	[0x8A] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_D},
	[0x8B] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_E},
	[0x8C] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_H},
	[0x8D] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_L},
	[0x8E] = {INSTRUCT_ADC, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x8F] = {INSTRUCT_ADC, MODE_REG_TO_REG, REG_A, REG_A},

	[0x90] = {INSTRUCT_SUB, MODE_REG, REG_B},
	[0x91] = {INSTRUCT_SUB, MODE_REG, REG_C},
	[0x92] = {INSTRUCT_SUB, MODE_REG, REG_D},
	[0x93] = {INSTRUCT_SUB, MODE_REG, REG_E},
	[0x94] = {INSTRUCT_SUB, MODE_REG, REG_H},
	[0x95] = {INSTRUCT_SUB, MODE_REG, REG_L},
	[0x96] = {INSTRUCT_SUB, MODE_ADDR, REG_HL},
	[0x97] = {INSTRUCT_SUB, MODE_REG, REG_A},
	[0x98] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_B},
	[0x99] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_C},
	[0x9A] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_D},
	[0x9B] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_E},
	[0x9C] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_H},
	[0x9D] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_L},
	[0x9E] = {INSTRUCT_SBC, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0x9F] = {INSTRUCT_SBC, MODE_REG_TO_REG, REG_A, REG_A},

	[0xA0] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_B},
	[0xA1] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_C},
	[0xA2] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_D},
	[0xA3] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_E},
	[0xA4] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_H},
	[0xA5] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_L},
	[0xA6] = {INSTRUCT_AND, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0xA7] = {INSTRUCT_AND, MODE_REG_TO_REG, REG_A, REG_A},
	[0xA8] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_B},
	[0xA9] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_C},
	[0xAA] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_D},
	[0xAB] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_E},
	[0xAC] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_H},
	[0xAD] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_L},
	[0xAE] = {INSTRUCT_XOR, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0xAF] = {INSTRUCT_XOR, MODE_REG_TO_REG, REG_A, REG_A},

	[0xB0] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_B},
	[0xB1] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_C},
	[0xB2] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_D},
	[0xB3] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_E},
	[0xB4] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_H},
	[0xB5] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_L},
	[0xB6] = {INSTRUCT_OR, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0xB7] = {INSTRUCT_OR, MODE_REG_TO_REG, REG_A, REG_A},
	[0xB8] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_B},
	[0xB9] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_C},
	[0xBA] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_D},
	[0xBB] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_E},
	[0xBC] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_H},
	[0xBD] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_L},
	[0xBE] = {INSTRUCT_CP, MODE_ADDR_TO_REG, REG_A, REG_HL},
	[0xBF] = {INSTRUCT_CP, MODE_REG_TO_REG, REG_A, REG_A},

	[0xC0] = {INSTRUCT_RET, .flag = FLAG_NZ},
	[0xC1] = {INSTRUCT_POP, MODE_ADDR_TO_REG, REG_SP},
	[0xC2] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_NZ, .byte_length = 3},
	[0xC3] = {INSTRUCT_JP, MODE_A16, .byte_length = 3},
	[0xC4] = {INSTRUCT_CALL, MODE_A16, .flag = FLAG_Z, .byte_length = 3},
	[0xC5] = {INSTRUCT_PUSH, MODE_REG_TO_ADDR, REG_SP, REG_BC},
	[0xC6] = {INSTRUCT_ADD, MODE_U8_TO_REG, REG_A},
	[0xC7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x0},
	[0xC8] = {INSTRUCT_RET, .flag = FLAG_Z},
	[0xC9] = {INSTRUCT_RET},
	[0xCA] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_Z, .byte_length = 3},
	[0xCB] = {INSTRUCT_CB},
	[0xCC] = {INSTRUCT_CALL, MODE_A16, .flag = FLAG_Z, .byte_length = 3},
	[0xCD] = {INSTRUCT_CALL, MODE_A16, .byte_length = 3},
	[0xCE] = {INSTRUCT_ADC, MODE_U8_TO_REG, REG_A},
	[0xCF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x8},

	[0xD0] = {INSTRUCT_RET, .flag = FLAG_NC},
	[0xD1] = {INSTRUCT_POP, MODE_ADDR_TO_REG, REG_DE, REG_SP},
	[0xD2] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_NC, .byte_length = 3},
	[0xD3] = {INSTRUCT_NONE},
	[0xD4] = {INSTRUCT_CALL, MODE_A16, .flag = FLAG_Z, .byte_length = 3},
	[0xD5] = {INSTRUCT_PUSH, MODE_REG_TO_ADDR, REG_SP, REG_DE},
	[0xD6] = {INSTRUCT_SUB, MODE_U8},
	[0xD7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x10},
	[0xD8] = {INSTRUCT_RET, .flag = FLAG_C},
	[0xD9] = {INSTRUCT_RETI},
	[0xDA] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_C, .byte_length = 3},
	[0xDB] = {INSTRUCT_NONE},
	[0xDC] = {INSTRUCT_CALL, MODE_A16, .flag = FLAG_C, .byte_length = 3},
	[0xDD] = {INSTRUCT_NONE},
	[0xDE] = {INSTRUCT_SBC, MODE_U8_TO_REG, REG_A},
	[0xDF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x18},

	[0xE0] = {INSTRUCT_LD, MODE_REG_TO_A8, REG_NONE, REG_A, .byte_length = 2},
	[0xE1] = {INSTRUCT_POP, MODE_ADDR_TO_REG, REG_HL, REG_SP},
	[0xE2] = {INSTRUCT_LD, MODE_REG_TO_IOADDR, REG_C, REG_A},
	[0xE3] = {INSTRUCT_NONE},
	[0xE4] = {INSTRUCT_NONE},
	[0xE5] = {INSTRUCT_PUSH, MODE_REG_TO_ADDR, REG_SP, REG_HL},
	[0xE6] = {INSTRUCT_AND, MODE_U8},
	[0xE7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x20},
	[0xE8] = {INSTRUCT_ADD, MODE_D8_TO_REG, REG_SP},
	[0xE9] = {INSTRUCT_JP, MODE_REG, REG_HL},
	[0xEA] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_NONE, REG_A, .byte_length = 3},
	[0xEB] = {INSTRUCT_NONE},
	[0xEC] = {INSTRUCT_NONE},
	[0xED] = {INSTRUCT_NONE},
	[0xEE] = {INSTRUCT_XOR, MODE_U8, REG_A},
	[0xEF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x28},

	[0xF0] = {INSTRUCT_LD, MODE_A8_TO_REG, REG_A, .byte_length = 2},
	[0xF1] = {INSTRUCT_POP, MODE_ADDR_TO_REG, REG_AF, REG_SP},
	[0xF2] = {INSTRUCT_LD, MODE_IOADDR_TO_REG, REG_A, REG_C},
	[0xF3] = {INSTRUCT_DI},
	[0xF4] = {INSTRUCT_NONE},
	[0xF5] = {INSTRUCT_PUSH, MODE_REG_TO_ADDR, REG_SP, REG_AF},
	[0xF6] = {INSTRUCT_OR, MODE_U8, REG_A},
	[0xF7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x30},
	[0xF8] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_HL, REG_SP},
	[0xF9] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_HL, REG_SP},
	[0xFA] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_NONE, REG_A, .byte_length = 3},
	[0xFB] = {INSTRUCT_EI},
	[0xFC] = {INSTRUCT_NONE},
	[0xFD] = {INSTRUCT_NONE},
	[0xFE] = {INSTRUCT_CP, MODE_U8, REG_A, .byte_length = 2},
	[0xFF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x38},

	// 0xCB extension

	[0x100] = {INSTRUCT_CB_RLC, MODE_REG, REG_B},
	[0x101] = {INSTRUCT_CB_RLC, MODE_REG, REG_C},
	[0x102] = {INSTRUCT_CB_RLC, MODE_REG, REG_D},
	[0x103] = {INSTRUCT_CB_RLC, MODE_REG, REG_E},
	[0x104] = {INSTRUCT_CB_RLC, MODE_REG, REG_H},
	[0x105] = {INSTRUCT_CB_RLC, MODE_REG, REG_L},
	[0x106] = {INSTRUCT_CB_RLC, MODE_ADDR, REG_HL},
	[0x107] = {INSTRUCT_CB_RLC, MODE_REG, REG_A},
	[0x108] = {INSTRUCT_CB_RR, MODE_REG, REG_B},
	[0x109] = {INSTRUCT_CB_RR, MODE_REG, REG_C},
	[0x10A] = {INSTRUCT_CB_RR, MODE_REG, REG_D},
	[0x10B] = {INSTRUCT_CB_RR, MODE_REG, REG_E},
	[0x10C] = {INSTRUCT_CB_RR, MODE_REG, REG_H},
	[0x10D] = {INSTRUCT_CB_RR, MODE_REG, REG_L},
	[0x10E] = {INSTRUCT_CB_RR, MODE_ADDR, REG_HL},
	[0x10F] = {INSTRUCT_CB_RR, MODE_REG, REG_A},

	[0x110] = {INSTRUCT_CB_RL, MODE_REG, REG_B},
	[0x111] = {INSTRUCT_CB_RL, MODE_REG, REG_C},
	[0x112] = {INSTRUCT_CB_RL, MODE_REG, REG_D},
	[0x113] = {INSTRUCT_CB_RL, MODE_REG, REG_E},
	[0x114] = {INSTRUCT_CB_RL, MODE_REG, REG_H},
	[0x115] = {INSTRUCT_CB_RL, MODE_REG, REG_L},
	[0x116] = {INSTRUCT_CB_RL, MODE_ADDR, REG_HL},
	[0x117] = {INSTRUCT_CB_RL, MODE_REG, REG_A},
	[0x118] = {INSTRUCT_CB_RR, MODE_REG, REG_B},
	[0x119] = {INSTRUCT_CB_RR, MODE_REG, REG_C},
	[0x11A] = {INSTRUCT_CB_RR, MODE_REG, REG_D},
	[0x11B] = {INSTRUCT_CB_RR, MODE_REG, REG_E},
	[0x11C] = {INSTRUCT_CB_RR, MODE_REG, REG_H},
	[0x11D] = {INSTRUCT_CB_RR, MODE_REG, REG_L},
	[0x11E] = {INSTRUCT_CB_RR, MODE_ADDR, REG_HL},
	[0x11F] = {INSTRUCT_CB_RR, MODE_REG, REG_A},

	[0x120] = {INSTRUCT_CB_SLA, MODE_REG, REG_B},
	[0x121] = {INSTRUCT_CB_SLA, MODE_REG, REG_C},
	[0x122] = {INSTRUCT_CB_SLA, MODE_REG, REG_D},
	[0x123] = {INSTRUCT_CB_SLA, MODE_REG, REG_E},
	[0x124] = {INSTRUCT_CB_SLA, MODE_REG, REG_H},
	[0x125] = {INSTRUCT_CB_SLA, MODE_REG, REG_L},
	[0x126] = {INSTRUCT_CB_SLA, MODE_ADDR, REG_HL},
	[0x127] = {INSTRUCT_CB_SLA, MODE_REG, REG_A},
	[0x128] = {INSTRUCT_CB_SRA, MODE_REG, REG_B},
	[0x129] = {INSTRUCT_CB_SRA, MODE_REG, REG_C},
	[0x12A] = {INSTRUCT_CB_SRA, MODE_REG, REG_D},
	[0x12B] = {INSTRUCT_CB_SRA, MODE_REG, REG_E},
	[0x12C] = {INSTRUCT_CB_SRA, MODE_REG, REG_H},
	[0x12D] = {INSTRUCT_CB_SRA, MODE_REG, REG_L},
	[0x12E] = {INSTRUCT_CB_SRA, MODE_ADDR, REG_HL},
	[0x12F] = {INSTRUCT_CB_SRA, MODE_REG, REG_A},

	[0x130] = {INSTRUCT_CB_SWAP, MODE_REG, REG_B},
	[0x131] = {INSTRUCT_CB_SWAP, MODE_REG, REG_C},
	[0x132] = {INSTRUCT_CB_SWAP, MODE_REG, REG_D},
	[0x133] = {INSTRUCT_CB_SWAP, MODE_REG, REG_E},
	[0x134] = {INSTRUCT_CB_SWAP, MODE_REG, REG_H},
	[0x135] = {INSTRUCT_CB_SWAP, MODE_REG, REG_L},
	[0x136] = {INSTRUCT_CB_SWAP, MODE_ADDR, REG_HL},
	[0x137] = {INSTRUCT_CB_SWAP, MODE_REG, REG_A},
	[0x138] = {INSTRUCT_CB_SRL, MODE_REG, REG_B},
	[0x139] = {INSTRUCT_CB_SRL, MODE_REG, REG_C},
	[0x13A] = {INSTRUCT_CB_SRL, MODE_REG, REG_D},
	[0x13B] = {INSTRUCT_CB_SRL, MODE_REG, REG_E},
	[0x13C] = {INSTRUCT_CB_SRL, MODE_REG, REG_H},
	[0x13D] = {INSTRUCT_CB_SRL, MODE_REG, REG_L},
	[0x13E] = {INSTRUCT_CB_SRL, MODE_ADDR, REG_HL},
	[0x13F] = {INSTRUCT_CB_SRL, MODE_REG, REG_A},

	[0x140] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 0},
	[0x141] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 0},
	[0x142] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 0},
	[0x143] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 0},
	[0x144] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 0},
	[0x145] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 0},
	[0x146] = {INSTRUCT_CB_BIT, MODE_ADDR, REG_HL, REG_HL, .parameter = 0},
	[0x147] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 0},
	[0x148] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 1},
	[0x149] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 1},
	[0x14A] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 1},
	[0x14B] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 1},
	[0x14C] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 1},
	[0x14D] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 1},
	[0x14E] = {INSTRUCT_CB_BIT, MODE_REG, REG_HL, REG_HL, .parameter = 1},
	[0x14F] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 1},

	[0x150] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 2},
	[0x151] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 2},
	[0x152] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 2},
	[0x153] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 2},
	[0x154] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 2},
	[0x155] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 2},
	[0x156] = {INSTRUCT_CB_BIT, MODE_ADDR, REG_HL, REG_HL, .parameter = 2},
	[0x157] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 2},
	[0x158] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 3},
	[0x159] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 3},
	[0x15A] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 3},
	[0x15B] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 3},
	[0x15C] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 3},
	[0x15D] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 3},
	[0x15E] = {INSTRUCT_CB_BIT, MODE_REG, REG_HL, REG_HL, .parameter = 3},
	[0x15F] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 3},

	[0x160] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 4},
	[0x161] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 4},
	[0x162] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 4},
	[0x163] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 4},
	[0x164] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 4},
	[0x165] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 4},
	[0x166] = {INSTRUCT_CB_BIT, MODE_ADDR, REG_HL, REG_HL, .parameter = 4},
	[0x167] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 4},
	[0x168] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 5},
	[0x169] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 5},
	[0x16A] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 5},
	[0x16B] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 5},
	[0x16C] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 5},
	[0x16D] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 5},
	[0x16E] = {INSTRUCT_CB_BIT, MODE_REG, REG_HL, REG_HL, .parameter = 5},
	[0x16F] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 5},

	[0x170] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 6},
	[0x171] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 6},
	[0x172] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 6},
	[0x173] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 6},
	[0x174] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 6},
	[0x175] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 6},
	[0x176] = {INSTRUCT_CB_BIT, MODE_ADDR, REG_HL, REG_HL, .parameter = 6},
	[0x177] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 6},
	[0x178] = {INSTRUCT_CB_BIT, MODE_REG, REG_B, .parameter = 7},
	[0x179] = {INSTRUCT_CB_BIT, MODE_REG, REG_C, .parameter = 7},
	[0x17A] = {INSTRUCT_CB_BIT, MODE_REG, REG_D, .parameter = 7},
	[0x17B] = {INSTRUCT_CB_BIT, MODE_REG, REG_E, .parameter = 7},
	[0x17C] = {INSTRUCT_CB_BIT, MODE_REG, REG_H, .parameter = 7},
	[0x17D] = {INSTRUCT_CB_BIT, MODE_REG, REG_L, .parameter = 7},
	[0x17E] = {INSTRUCT_CB_BIT, MODE_REG, REG_HL, REG_HL, .parameter = 7},
	[0x17F] = {INSTRUCT_CB_BIT, MODE_REG, REG_A, .parameter = 7},

	[0x180] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 0},
	[0x181] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 0},
	[0x182] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 0},
	[0x183] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 0},
	[0x184] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 0},
	[0x185] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 0},
	[0x186] = {INSTRUCT_CB_RES, MODE_ADDR, REG_HL, REG_HL, .parameter = 0},
	[0x187] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 0},
	[0x188] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 1},
	[0x189] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 1},
	[0x18A] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 1},
	[0x18B] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 1},
	[0x18C] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 1},
	[0x18D] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 1},
	[0x18E] = {INSTRUCT_CB_RES, MODE_REG, REG_HL, REG_HL, .parameter = 1},
	[0x18F] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 1},

	[0x190] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 2},
	[0x191] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 2},
	[0x192] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 2},
	[0x193] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 2},
	[0x194] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 2},
	[0x195] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 2},
	[0x196] = {INSTRUCT_CB_RES, MODE_ADDR, REG_HL, REG_HL, .parameter = 2},
	[0x197] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 2},
	[0x198] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 3},
	[0x199] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 3},
	[0x19A] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 3},
	[0x19B] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 3},
	[0x19C] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 3},
	[0x19D] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 3},
	[0x19E] = {INSTRUCT_CB_RES, MODE_REG, REG_HL, REG_HL, .parameter = 3},
	[0x19F] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 3},

	[0x1A0] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 4},
	[0x1A1] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 4},
	[0x1A2] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 4},
	[0x1A3] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 4},
	[0x1A4] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 4},
	[0x1A5] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 4},
	[0x1A6] = {INSTRUCT_CB_RES, MODE_ADDR, REG_HL, REG_HL, .parameter = 4},
	[0x1A7] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 4},
	[0x1A8] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 5},
	[0x1A9] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 5},
	[0x1AA] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 5},
	[0x1AB] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 5},
	[0x1AC] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 5},
	[0x1AD] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 5},
	[0x1AE] = {INSTRUCT_CB_RES, MODE_REG, REG_HL, REG_HL, .parameter = 5},
	[0x1AF] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 5},

	[0x1B0] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 6},
	[0x1B1] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 6},
	[0x1B2] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 6},
	[0x1B3] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 6},
	[0x1B4] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 6},
	[0x1B5] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 6},
	[0x1B6] = {INSTRUCT_CB_RES, MODE_ADDR, REG_HL, REG_HL, .parameter = 6},
	[0x1B7] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 6},
	[0x1B8] = {INSTRUCT_CB_RES, MODE_REG, REG_B, .parameter = 7},
	[0x1B9] = {INSTRUCT_CB_RES, MODE_REG, REG_C, .parameter = 7},
	[0x1BA] = {INSTRUCT_CB_RES, MODE_REG, REG_D, .parameter = 7},
	[0x1BB] = {INSTRUCT_CB_RES, MODE_REG, REG_E, .parameter = 7},
	[0x1BC] = {INSTRUCT_CB_RES, MODE_REG, REG_H, .parameter = 7},
	[0x1BD] = {INSTRUCT_CB_RES, MODE_REG, REG_L, .parameter = 7},
	[0x1BE] = {INSTRUCT_CB_RES, MODE_REG, REG_HL, REG_HL, .parameter = 7},
	[0x1BF] = {INSTRUCT_CB_RES, MODE_REG, REG_A, .parameter = 7},

	[0x1C0] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 0},
	[0x1C1] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 0},
	[0x1C2] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 0},
	[0x1C3] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 0},
	[0x1C4] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 0},
	[0x1C5] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 0},
	[0x1C6] = {INSTRUCT_CB_SET, MODE_ADDR, REG_HL, REG_HL, .parameter = 0},
	[0x1C7] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 0},
	[0x1C8] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 1},
	[0x1C9] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 1},
	[0x1CA] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 1},
	[0x1CB] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 1},
	[0x1CC] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 1},
	[0x1CD] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 1},
	[0x1CE] = {INSTRUCT_CB_SET, MODE_REG, REG_HL, REG_HL, .parameter = 1},
	[0x1CF] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 1},

	[0x1D0] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 2},
	[0x1D1] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 2},
	[0x1D2] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 2},
	[0x1D3] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 2},
	[0x1D4] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 2},
	[0x1D5] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 2},
	[0x1D6] = {INSTRUCT_CB_SET, MODE_ADDR, REG_HL, REG_HL, .parameter = 2},
	[0x1D7] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 2},
	[0x1D8] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 3},
	[0x1D9] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 3},
	[0x1DA] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 3},
	[0x1DB] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 3},
	[0x1DC] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 3},
	[0x1DD] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 3},
	[0x1DE] = {INSTRUCT_CB_SET, MODE_REG, REG_HL, REG_HL, .parameter = 3},
	[0x1DF] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 3},

	[0x1E0] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 4},
	[0x1E1] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 4},
	[0x1E2] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 4},
	[0x1E3] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 4},
	[0x1E4] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 4},
	[0x1E5] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 4},
	[0x1E6] = {INSTRUCT_CB_SET, MODE_ADDR, REG_HL, REG_HL, .parameter = 4},
	[0x1E7] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 4},
	[0x1E8] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 5},
	[0x1E9] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 5},
	[0x1EA] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 5},
	[0x1EB] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 5},
	[0x1EC] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 5},
	[0x1ED] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 5},
	[0x1EE] = {INSTRUCT_CB_SET, MODE_REG, REG_HL, REG_HL, .parameter = 5},
	[0x1EF] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 5},

	[0x1F0] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 6},
	[0x1F1] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 6},
	[0x1F2] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 6},
	[0x1F3] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 6},
	[0x1F4] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 6},
	[0x1F5] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 6},
	[0x1F6] = {INSTRUCT_CB_SET, MODE_ADDR, REG_HL, REG_HL, .parameter = 6},
	[0x1F7] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 6},
	[0x1F8] = {INSTRUCT_CB_SET, MODE_REG, REG_B, .parameter = 7},
	[0x1F9] = {INSTRUCT_CB_SET, MODE_REG, REG_C, .parameter = 7},
	[0x1FA] = {INSTRUCT_CB_SET, MODE_REG, REG_D, .parameter = 7},
	[0x1FB] = {INSTRUCT_CB_SET, MODE_REG, REG_E, .parameter = 7},
	[0x1FC] = {INSTRUCT_CB_SET, MODE_REG, REG_H, .parameter = 7},
	[0x1FD] = {INSTRUCT_CB_SET, MODE_REG, REG_L, .parameter = 7},
	[0x1FE] = {INSTRUCT_CB_SET, MODE_REG, REG_HL, REG_HL, .parameter = 7},
	[0x1FF] = {INSTRUCT_CB_SET, MODE_REG, REG_A, .parameter = 7},
};
