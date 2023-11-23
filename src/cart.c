#include "cart.h"

#include <stdio.h>
#include <string.h>

static uint8_t scrolling_logo[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

static cart_context ctx;

cart_context *get_cart_context() {
    return &ctx;
}

u8 cart_read(u16 addr) {
    return ctx.rom_data[addr];
}

bool cart_init(const char *cart_filepath) {
	ctx.filepath = cart_filepath;

    FILE *file = fopen(cart_filepath, "rb");
    if (!file) {
        perror("Failed to open file.");
        return false;
    }

    fseek(file, 0, SEEK_END);
    ctx.rom_size = ftell(file);

    // TODO: enforce max rom size
    ctx.rom_data = malloc(ctx.rom_size);
	if (ctx.rom_data == NULL) {
		fprintf(stderr, "memory allocation failed!\n");
        fclose(file);
		return false;
	}

    rewind(file);
    fread(ctx.rom_data, ctx.rom_size, 1, file);
    const u32 last_read = ftell(file);
    fclose(file);

    if (ctx.rom_size != last_read) {
    	fprintf(stderr, "ERROR: failed to read cartridge data from %s\n", cart_filepath);
        free(ctx.rom_data);
        return false;
    }

    // rom actually starts at 0x100
    ctx.header = (rom_header *)(ctx.rom_data + 0x100);
    ctx.header->game_title[15] = 0; // ensure str termination

    if (memcmp(&ctx.rom_data[0x104], scrolling_logo, sizeof(scrolling_logo)) != 0) {
        fprintf(stderr, "ERROR: cartridge missing logo header\n");
        free(ctx.rom_data);
        return false;
    }

    // run checksum
    u16 checksum  = 0;
    for (int i = 0x0134; i <= 0x014C; i++)
    	checksum = checksum - (ctx.rom_data[i] - 1);
    bool r_checksum = (checksum & 0xFF);
    // printf("DEBUG: CHECKSUM: %2.2X (%s)\n", ctx.header->checksum, r_checksum ? "PASSED" : "FAILED");
    return r_checksum;
}

void cart_debug() {
    printf("CARTRIDGE LOADED:\n");
    printf("\tPATH     : %s\n",    ctx.filepath);
    printf("\tTITLE    : %s\n",    ctx.header->game_title);
    printf("\tTYPE     : %2.2X\n", ctx.header->type);
    printf("\tROM SIZE : %d KB\n", 32 << ctx.header->rom_size);
    printf("\tRAM SIZE : %2.2X\n", ctx.header->ram_size);
    printf("\tLIC CODE : %2.2X\n", ctx.header->license_code);
    printf("\tROM VERS : %2.2X\n", ctx.header->version);
    printf("\tPC       : 0x%02X%02X%02X%02X\n",
        ctx.header->entry_point[0],
        ctx.header->entry_point[1],
        ctx.header->entry_point[2],
        ctx.header->entry_point[3]
    );
}
