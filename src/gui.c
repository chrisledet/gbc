#include "gui.h"
#include "common.h"
#include "bus.h"

#include <stdio.h>
#include <SDL.h>

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	SDL_Window *vramWindow;
	SDL_Renderer *vramRenderer;
	SDL_Texture *vramTexture;
	SDL_Surface *vramSurface;
} gui_context;

static gui_context ctx = {0};
// static u32 SCREEN_WIDTH = 1024;
// static u32 SCREEN_HEIGHT = 768;
// static unsigned long color_palette[4] = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000}; // black + white
static unsigned long color_palette[4] = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F }; // greenish

static int scale = 2;

SDL_Surface* gui_get_surface() {
	return SDL_GetWindowSurface(ctx.window);
}

void gui_init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	//ctx.window
	ctx.window = SDL_CreateWindow("gbc", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*2, 144*2, SDL_WINDOW_SHOWN);
	ctx.renderer = SDL_CreateRenderer(ctx.window, 0, SDL_RENDERER_ACCELERATED);

	SDL_CreateWindowAndRenderer(16 * 8 * scale, 32 * 8 * scale, 0, &ctx.vramWindow, &ctx.vramRenderer);
	ctx.vramSurface = SDL_CreateRGBSurface(0,
		(16 * 8 * scale) + (16 * scale),
		(32 * 8 * scale) + (64 * scale),
		32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	ctx.vramTexture = SDL_CreateTexture(ctx.vramRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		(16 * 8 * scale) + (16 * scale),
		(32 * 8 * scale) + (64 * scale));
	SDL_SetWindowTitle(ctx.vramWindow, "gbc - vram");
	// SDL_SetRenderDrawColor(ctx.renderer, 0xFF, 0xAA, 0xAA, 0xFF);
}

void display_tile(u16 addr, u16 tile_idx, u16 x, u16 y) {
	static u16 tile_size = 16;

	SDL_Rect rc;
	for (int tile_y = 0; tile_y < tile_size; tile_y += 2) {
		u8 tile1 = bus_read(addr + tile_idx * tile_size + tile_y);
		u8 tile2 = bus_read(addr + tile_idx * tile_size + tile_y + 1);
		for (int bit = 7; bit >= 0; --bit) {
			u8 hi = !!(tile1 * (1 << bit)) << 1;
			u8 lo = !!(tile2 * (1 << bit));
			u8 color = hi | lo;

			rc.x = x + (7 - bit) * scale;
			rc.y = y + (tile_y / 2) * scale;
			rc.w = scale;
			rc.h = scale;

			SDL_FillRect(ctx.vramSurface, &rc, color_palette[color]);
		}
	}
}

void vram_window_tick() {
	u16 x_draw = 0;
	u16 y_draw = 0;
	u16 tile = 0;

	SDL_Rect rc = {0};
	rc.x = 0;
	rc.y = 0;
	rc.w = ctx.vramSurface->w;
	rc.h = ctx.vramSurface->h;
	SDL_FillRect(ctx.vramSurface, &rc, 0xFF111111);

	// 24 * 16
	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 16; ++x) {
			display_tile(0x8000, tile, x_draw + (x * scale), y_draw + (y * scale));
			x_draw += (scale * 8);
			tile++;
		}
		y_draw += (scale * 8);
		x_draw = 0;
	}

	SDL_UpdateTexture(ctx.vramTexture, NULL, ctx.vramSurface->pixels, ctx.vramSurface->pitch);
	SDL_RenderClear(ctx.vramRenderer);
	SDL_RenderCopy(ctx.vramRenderer, ctx.vramTexture, NULL, NULL);
	SDL_RenderPresent(ctx.vramRenderer);
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

void gui_tick() {
	vram_window_tick();
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
			// case SDL_KEYDOWN: printf("GUI: KEY DOWN!\n");
			// break;
			// case SDL_KEYUP: printf("GUI: KEY UP!\n");
			// break;
			case SDL_WINDOWEVENT_CLOSE:
			case SDL_QUIT: return GUI_QUIT;
			break;
		}
	}
	return GUI_NONE;
}

void gui_shutdown() {
	if (ctx.window)
		SDL_DestroyWindow(ctx.window);
}
