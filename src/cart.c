#include <stdio.h>
#include "cart.h"

typedef struct {
	const char *filepath;
	uint32_t rom_size;
	uint8_t *rom_data;
	rom_header *header;
} cart_context;

static cart_context ctx;

bool cart_load(const char *cart_filepath) {
	ctx.filepath = cart_filepath;

    FILE *file;
    errno_t ferr = fopen_s(&file, cart_filepath, "r");
    if (ferr != 0 && !file) {
        perror("Failed to open file.");
        return false;
    }

    fseek(file, 0, SEEK_END);
    ctx.rom_size = ftell(file);

    // TODO: enforce max rom size
    ctx.rom_data = malloc(ctx.rom_size);
	if (ctx.rom_data == NULL) {
		fprintf(stderr, "memory allocation failed!\n");
		return false;
	}

    rewind(file);
    const size_t fread_rc = fread(ctx.rom_data, ctx.rom_size, 1, file);
    // still confused if rc is supppose to be 1 or not
    // if (fread_rc == 1) {
    if (ctx.rom_size != ftell(file)) {
    	fprintf(stderr, "WARN: cart read wasnt successful: %zu, failed at %lu\n", fread_rc, ftell(file));
        return false;
    }

    fclose(file);

    // rom actually starts at 0x100
    ctx.header = (rom_header *)(ctx.rom_data + 0x100);
    ctx.header->game_title[15] = 0; // ensure str termination

    // run checksum
    uint16_t checksum  = 0;
    for (uint16_t i = 0x0134; i <= 0x014C; i++) {
    	checksum = checksum - ctx.rom_data[i] - 1;
    }
    bool r_checksum = (checksum & 0xFF);

    // debug result
    printf("CARTRIDGE LOADED:\n");
    printf("\tPATH     : %s\n",    ctx.filepath);
    printf("\tTITLE    : %s\n",    ctx.header->game_title);
    printf("\tTYPE     : %2.2X\n", ctx.header->type);
    printf("\tROM SIZE : %d KB\n", 32 << ctx.header->rom_size);
    printf("\tRAM SIZE : %2.2X\n", ctx.header->ram_size);
    printf("\tLIC CODE : %2.2X\n", ctx.header->license_code);
    printf("\tROM VERS : %2.2X\n", ctx.header->version);
    printf("\tCHECKSUM : %2.2X (%s)\n", ctx.header->checksum, r_checksum ? "PASSED" : "FAILED");
    return r_checksum;
}
