#pragma once

#include <common.h>

typedef struct {
	// 0100-0103 main exec address
	u8 entry_point[4];
	// 0104-0133 — Nintendo logo
	u8 logo[0x30];
	// 0134-0143 — game title
	char game_title[16];
	u16 new_license_code;
	u8 sgb_flag;
	u8 type;
	u8 rom_size;
	u8 ram_size;
	u8 dst_code;
	// game publisher
	u8 license_code;
	u8 version;
	u8 checksum;
	u16 gchecksum;
} rom_header;

typedef struct {
	const char *filepath;
	u32 rom_size;
	u8 *rom_data;
	rom_header *header;
} cart_context;

cart_context *get_cart_context();
bool cart_load(const char *cart_filepath);
u8 cart_read(u16 addr);
void cart_debug();
