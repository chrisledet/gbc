#include <stdint.h>
#include <stdbool.h>

#include <common.h>

typedef struct {
	bool debug_mode;
	bool paused;
	bool running;
	bool quit;
	u64 ticks;
	u64 cycles;
} gbc_context;

gbc_context* gbc_get_context();

int gbc_run(const char *rom_filepath);
