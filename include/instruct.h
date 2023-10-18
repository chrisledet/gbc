#include <stdint.h>
#include "common.h"

typedef enum {
	INSTRUCT_NONE,
	INSTRUCT_NOP,
	INSTRUCT_ADD,
	INSTRUCT_LD,
	INSTRUCT_LDI,
	INSTRUCT_LDD,
	INSTRUCT_HALT,
	INSTRUCT_JP,
	INSTRUCT_JR,
	INSTRUCT_RST,
	INSTRUCT_CP,
	INSTRUCT_RET,
	INSTRUCT_DEC,
	INSTRUCT_DI,
	INSTRUCT_EI,
	INSTRUCT_STOP,
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
	REG_PC,
} cpu_register;

typedef enum {
	MODE_NONE,
	MODE_D8,  // 8-bit data
	MODE_D16, // 16-bit data
	MODE_A16, // 16-bit address
	MODE_REG, // register value
	MODE_REG_TO_REG, // register to register
	MODE_ADDR_TO_REG, // reg, $addr
	MODE_IOADDR_TO_REG, // reg, $addr
	MODE_D8_TO_REG, // reg, d8
	MODE_D16_TO_REG, // reg, d8
	MODE_REG_TO_ADDR, // $addr, reg
	MODE_REG_TO_IOADDR, // $addr, reg
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
} cpu_instruction;

static cpu_instruction instructions[0x100] = {
	[0x00] = {INSTRUCT_NOP, MODE_NONE},
	[0x01] = {INSTRUCT_LD, MODE_D16, REG_BC},
	[0x02] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_BC, REG_A},
	[0x03] = {INSTRUCT_NONE},
	[0x04] = {INSTRUCT_NONE},
	[0x05] = {INSTRUCT_DEC, MODE_REG, REG_B},
	[0x06] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_B},
	[0x07] = {INSTRUCT_NONE},
	[0x08] = {INSTRUCT_NONE},
	[0x09] = {INSTRUCT_NONE},
	[0x0A] = {INSTRUCT_NONE},
	[0x0B] = {INSTRUCT_NONE},
	[0x0C] = {INSTRUCT_NONE},
	[0x0D] = {INSTRUCT_NONE},
	[0x0E] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_C},
	[0x0F] = {INSTRUCT_NONE},

	[0x10] = {INSTRUCT_STOP},
	[0x11] = {INSTRUCT_LD, MODE_D16, REG_DE},
	[0x12] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_DE, REG_A},
	[0x13] = {INSTRUCT_NONE},
	[0x14] = {INSTRUCT_NONE},
	[0x15] = {INSTRUCT_NONE},
	[0x16] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_D},
	[0x17] = {INSTRUCT_NONE},
	[0x18] = {INSTRUCT_JR, MODE_D8},
	[0x19] = {INSTRUCT_NONE},
	[0x1A] = {INSTRUCT_NONE},
	[0x1B] = {INSTRUCT_NONE},
	[0x1C] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_NZ},
	[0x1D] = {INSTRUCT_NONE},
	[0x1E] = {INSTRUCT_NONE},
	[0x1F] = {INSTRUCT_NONE},

	[0x20] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_NZ},
	[0x21] = {INSTRUCT_LD, MODE_D16, REG_HL},
	[0x22] = {INSTRUCT_LDI, MODE_REG_TO_ADDR, REG_HL, REG_A},
	[0x23] = {INSTRUCT_NONE},
	[0x24] = {INSTRUCT_NONE},
	[0x25] = {INSTRUCT_NONE},
	[0x26] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_H},
	[0x27] = {INSTRUCT_NONE},
	[0x28] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_Z},
	[0x29] = {INSTRUCT_NONE},
	[0x2A] = {INSTRUCT_NONE},
	[0x2B] = {INSTRUCT_NONE},
	[0x2C] = {INSTRUCT_NONE},
	[0x2D] = {INSTRUCT_NONE},
	[0x2E] = {INSTRUCT_NONE},
	[0x2F] = {INSTRUCT_NONE},

	[0x30] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_NC},
	[0x31] = {INSTRUCT_LD, MODE_D16_TO_REG, REG_SP},
	[0x32] = {INSTRUCT_LDD, MODE_REG_TO_ADDR, REG_HL, REG_A},
	[0x33] = {INSTRUCT_NONE},
	[0x34] = {INSTRUCT_NONE},
	[0x35] = {INSTRUCT_NONE},
	[0x36] = {INSTRUCT_LD, MODE_D8_TO_REG, REG_HL},
	[0x37] = {INSTRUCT_NONE},
	[0x38] = {INSTRUCT_JR, MODE_D8, .flag = FLAG_C},
	[0x39] = {INSTRUCT_NONE},
	[0x3A] = {INSTRUCT_NONE},
	[0x3B] = {INSTRUCT_NONE},
	[0x3C] = {INSTRUCT_NONE},
	[0x3D] = {INSTRUCT_NONE},
	[0x3E] = {INSTRUCT_LD, MODE_D8},
	[0x3F] = {INSTRUCT_NONE},

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
	[0x81] = {INSTRUCT_NONE},
	[0x82] = {INSTRUCT_NONE},
	[0x83] = {INSTRUCT_NONE},
	[0x84] = {INSTRUCT_NONE},
	[0x85] = {INSTRUCT_NONE},
	[0x86] = {INSTRUCT_NONE},
	[0x87] = {INSTRUCT_NONE},
	[0x88] = {INSTRUCT_NONE},
	[0x89] = {INSTRUCT_NONE},
	[0x8A] = {INSTRUCT_NONE},
	[0x8B] = {INSTRUCT_NONE},
	[0x8C] = {INSTRUCT_NONE},
	[0x8D] = {INSTRUCT_NONE},
	[0x8E] = {INSTRUCT_NONE},
	[0x8F] = {INSTRUCT_NONE},

	[0x90] = {INSTRUCT_NONE},
	[0x91] = {INSTRUCT_NONE},
	[0x92] = {INSTRUCT_NONE},
	[0x93] = {INSTRUCT_NONE},
	[0x94] = {INSTRUCT_NONE},
	[0x95] = {INSTRUCT_NONE},
	[0x96] = {INSTRUCT_NONE},
	[0x97] = {INSTRUCT_NONE},
	[0x98] = {INSTRUCT_NONE},
	[0x99] = {INSTRUCT_NONE},
	[0x9A] = {INSTRUCT_NONE},
	[0x9B] = {INSTRUCT_NONE},
	[0x9C] = {INSTRUCT_NONE},
	[0x9D] = {INSTRUCT_NONE},
	[0x9E] = {INSTRUCT_NONE},
	[0x9F] = {INSTRUCT_NONE},

	[0xA0] = {INSTRUCT_NONE},
	[0xA1] = {INSTRUCT_NONE},
	[0xA2] = {INSTRUCT_NONE},
	[0xA3] = {INSTRUCT_NONE},
	[0xA4] = {INSTRUCT_NONE},
	[0xA5] = {INSTRUCT_NONE},
	[0xA6] = {INSTRUCT_NONE},
	[0xA7] = {INSTRUCT_NONE},
	[0xA8] = {INSTRUCT_NONE},
	[0xA9] = {INSTRUCT_NONE},
	[0xAA] = {INSTRUCT_NONE},
	[0xAB] = {INSTRUCT_NONE},
	[0xAC] = {INSTRUCT_NONE},
	[0xAD] = {INSTRUCT_NONE},
	[0xAE] = {INSTRUCT_NONE},
	[0xAF] = {INSTRUCT_NONE},

	[0xB0] = {INSTRUCT_NONE},
	[0xB1] = {INSTRUCT_NONE},
	[0xB2] = {INSTRUCT_NONE},
	[0xB3] = {INSTRUCT_NONE},
	[0xB4] = {INSTRUCT_NONE},
	[0xB5] = {INSTRUCT_NONE},
	[0xB6] = {INSTRUCT_NONE},
	[0xB7] = {INSTRUCT_NONE},
	[0xB8] = {INSTRUCT_NONE},
	[0xB9] = {INSTRUCT_NONE},
	[0xBA] = {INSTRUCT_NONE},
	[0xBB] = {INSTRUCT_NONE},
	[0xBC] = {INSTRUCT_NONE},
	[0xBD] = {INSTRUCT_NONE},
	[0xBE] = {INSTRUCT_NONE},
	[0xBF] = {INSTRUCT_NONE},

	[0xC0] = {INSTRUCT_RET, MODE_NONE},
	[0xC1] = {INSTRUCT_NONE},
	[0xC2] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_NZ},
	[0xC3] = {INSTRUCT_JP, MODE_A16},
	[0xC4] = {INSTRUCT_NONE},
	[0xC5] = {INSTRUCT_NONE},
	[0xC6] = {INSTRUCT_NONE},
	[0xC7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x0},
	[0xC8] = {INSTRUCT_NONE},
	[0xC9] = {INSTRUCT_NONE},
	[0xCA] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_Z},
	[0xCB] = {INSTRUCT_NONE},
	[0xCC] = {INSTRUCT_NONE},
	[0xCD] = {INSTRUCT_NONE},
	[0xCE] = {INSTRUCT_NONE},
	[0xCF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x8},

	[0xD0] = {INSTRUCT_NONE},
	[0xD1] = {INSTRUCT_NONE},
	[0xD2] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_NC},
	[0xD3] = {INSTRUCT_NONE},
	[0xD4] = {INSTRUCT_NONE},
	[0xD5] = {INSTRUCT_NONE},
	[0xD6] = {INSTRUCT_NONE},
	[0xD7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x10},
	[0xD8] = {INSTRUCT_NONE},
	[0xD9] = {INSTRUCT_NONE},
	[0xDA] = {INSTRUCT_JP, MODE_A16, .flag = FLAG_C},
	[0xDB] = {INSTRUCT_NONE},
	[0xDC] = {INSTRUCT_NONE},
	[0xDD] = {INSTRUCT_NONE},
	[0xDE] = {INSTRUCT_NONE},
	[0xDF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x18},

	[0xE0] = {INSTRUCT_LD, MODE_REG_TO_IOADDR, REG_NONE, REG_A},
	[0xE1] = {INSTRUCT_NONE},
	[0xE2] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_C, REG_A},
	[0xE3] = {INSTRUCT_NONE},
	[0xE4] = {INSTRUCT_NONE},
	[0xE5] = {INSTRUCT_NONE},
	[0xE6] = {INSTRUCT_NONE},
	[0xE7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x20},
	[0xE8] = {INSTRUCT_NONE},
	[0xE9] = {INSTRUCT_JP, MODE_REG, REG_NONE, REG_HL},
	[0xEA] = {INSTRUCT_LD, MODE_REG_TO_ADDR, REG_NONE, REG_A},
	[0xEB] = {INSTRUCT_NONE},
	[0xEC] = {INSTRUCT_NONE},
	[0xED] = {INSTRUCT_NONE},
	[0xEE] = {INSTRUCT_NONE},
	[0xEF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x28},

	[0xF0] = {INSTRUCT_LD, MODE_IOADDR_TO_REG, REG_A},
	[0xF1] = {INSTRUCT_NONE},
	[0xF2] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_A, REG_C},
	[0xF3] = {INSTRUCT_DI, MODE_NONE},
	[0xF4] = {INSTRUCT_NONE},
	[0xF5] = {INSTRUCT_NONE},
	[0xF6] = {INSTRUCT_NONE},
	[0xF7] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x30},
	[0xF8] = {INSTRUCT_NONE},
	[0xF9] = {INSTRUCT_LD, MODE_REG_TO_REG, REG_HL, REG_SP},
	[0xFA] = {INSTRUCT_LD, MODE_ADDR_TO_REG, REG_NONE, REG_A},
	[0xFB] = {INSTRUCT_NONE},
	[0xFC] = {INSTRUCT_NONE},
	[0xFD] = {INSTRUCT_NONE},
	[0xFE] = {INSTRUCT_CP, MODE_D8},
	[0xFF] = {INSTRUCT_RST, MODE_PARAM, .parameter = 0x38},
};
