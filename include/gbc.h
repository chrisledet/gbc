#include <stdint.h>
#include <stdbool.h>

#include <common.h>

typedef struct {
	bool paused;
	bool running;
	bool quit;
	u64 ticks;
} gbc_context;

gbc_context* gbc_get_context();

int gbc_run(const char *rom_filepath);
