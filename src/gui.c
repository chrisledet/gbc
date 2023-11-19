#include <gui.h>
#include <stdio.h>
#include <SDL.h>


typedef struct {
	SDL_Window* window;
	SDL_Surface* surface;
	bool quit;
} gui_context;

static gui_context ctx = {0};
// static u32 SCREEN_WIDTH = 1024;
// static u32 SCREEN_HEIGHT = 768;
// static unsigned long tile_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

static bool key_state[322]; // number of sdlk_down events

void gui_init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	ctx.window = SDL_CreateWindow("gbc", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*2, 144*2, 0);
	ctx.surface = SDL_GetWindowSurface(ctx.window);
	gui_clear();
}

void gui_tick() {
	SDL_UpdateWindowSurface(ctx.window);
}

void gui_clear() {
	// SDL_LockSurface(ctx.surface);
	// SDL_memset(ctx.surface->pixels, 255, (ctx.surface->h * ctx.surface->pitch));
	// SDL_UnlockSurface(ctx.surface);
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

u32 *gui_get_frame_buffer() {
	return ctx.surface->pixels;
}

void gui_handle_input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				printf("GUI: KEY DOWN!\n");
				key_state[event.key.keysym.sym] = true;
			}
			break;

			case SDL_KEYUP: {
				printf("GUI: KEY UP!\n");
				key_state[event.key.keysym.sym] = false;
			}
			break;

			case SDL_QUIT: {
				ctx.quit = true;
			}
			break;
		}
	}
}

bool gui_get_quit() {
	gui_handle_input();
	ctx.quit = key_state[SDLK_ESCAPE];
	return ctx.quit;
}

void gui_shutdown() {
	SDL_DestroyWindow(ctx.window);
}
