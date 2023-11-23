#include "gui.h"
#include "common.h"
#include <stdio.h>
#include <SDL.h>

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
} gui_context;

static gui_context ctx = {0};
// static u32 SCREEN_WIDTH = 1024;
// static u32 SCREEN_HEIGHT = 768;
//static unsigned long tile_colors[4] = {0xFFFFFFFF, 0xAAAAAAFF, 0x555555FF, 0x000000FF}; // black + white
static unsigned long tile_colors[4] = { 0xF4FFF4FF, 0xC0D0C0FF, 0x80A080FF, 0x001000FF }; // greenish

SDL_Surface* gui_get_surface() {
	return SDL_GetWindowSurface(ctx.window);
}

void gui_init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	//ctx.window
	ctx.window = SDL_CreateWindow("gbc", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*2, 144*2, SDL_WINDOW_SHOWN);
	ctx.renderer = SDL_CreateRenderer(ctx.window, 0, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawColor(ctx.renderer, 0xFF, 0xAA, 0xAA, 0xFF);
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

void gui_set_color(u8 color_id) {
	if (color_id > 3) color_id = 3;
	long color = tile_colors[color_id];
	SDL_SetRenderDrawColor(ctx.renderer, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

void gui_render_color(u8 color_id, int x, int y) {
	//gui_set_color(color_id);
	//SDL_RenderDrawPoint(ctx.renderer, x, y);
}

void gui_tick() {
	// static int x = 0, y = 0;

	//SDL_RenderPresent(ctx.renderer);
	//SDL_RenderClear(ctx.renderer);
}

void gui_display_clear() {
	//SDL_Surface* surface = gui_get_surface();
	//SDL_LockSurface(surface);
	//SDL_memset(surface->pixels, 0, (surface->h * surface->pitch));
	//SDL_UnlockSurface(surface);
	//SDL_SetRenderDrawColor(ctx.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(ctx.renderer);
}

gui_event gui_handle_input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN: {
				printf("GUI: KEY DOWN!\n");
			}
			break;

			case SDL_KEYUP: {
				printf("GUI: KEY UP!\n");
			}
			break;

			case SDL_QUIT: {
				return GUI_QUIT;
			}
			break;
		}
	}
	return GUI_NONE;
}

void gui_shutdown() {
	if (ctx.window)
		SDL_DestroyWindow(ctx.window);
}
