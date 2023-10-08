#include <stdint.h>
#include <stdbool.h>

typedef struct {
	bool paused;
	bool running;
	uint64_t ticks;
} gbc_context;

gbc_context* gbc_get_context();

int gbc_run(const char *rom_filepath);
