#pragma once

#include <common.h>

typedef struct {
	// 0100-0103 main exec address
	uint8_t entry_point[4];
	// 0104-0133 — Nintendo logo
	uint8_t logo[0x30];
	// 0134-0143 — game title
	char game_title[16];
	uint16_t new_license_code;
	uint8_t sgb_flag;
	uint8_t type;
	uint8_t rom_size;
	uint8_t ram_size;
	uint8_t dst_code;
	// game publisher
	uint8_t license_code;
	uint8_t version;
	uint8_t checksum;
	uint16_t gchecksum;
} rom_header;

bool cart_load(const char *cart_filepath);
