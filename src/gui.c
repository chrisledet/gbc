#include "gui.h"
#include "bus.h"

#include <stdio.h>
#include <SDL.h>

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;

	SDL_Window *dbgWindow;
	SDL_Renderer *dbgRenderer;
	SDL_Texture *dbgTexture;
	SDL_Surface *dbgSurface;
} gui_context;

static int scale = 2;

static gui_context ctx = {0};

//static unsigned long color_palette[4] = {0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000}; // black + white
static unsigned long color_palette[4] = { 0xFF9BBC0F, 0xFF8BAC0F, 0xFF306230, 0xFF0F380F }; // greenish

SDL_Surface* gui_get_surface() {
	return SDL_GetWindowSurface(ctx.window);
}

void gui_init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	ctx.window = SDL_CreateWindow(
		"gbc",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale,
		SDL_WINDOW_SHOWN);
	ctx.renderer = SDL_CreateRenderer(ctx.window, 0, SDL_RENDERER_ACCELERATED);

	SDL_CreateWindowAndRenderer(16 * 8 * scale, 32 * 8 * scale, 0, &ctx.dbgWindow, &ctx.dbgRenderer);
	ctx.dbgSurface = SDL_CreateRGBSurface(0,
		(16 * 8 * scale) + (16 * scale),
		(32 * 8 * scale) + (64 * scale),
		32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	ctx.dbgTexture = SDL_CreateTexture(ctx.dbgRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
		(16 * 8 * scale) + (16 * scale),
		(32 * 8 * scale) + (64 * scale));
	int x, y;
	SDL_GetWindowPosition(ctx.window, &x, &y);
	SDL_SetWindowPosition(ctx.dbgWindow, x + SCREEN_WIDTH + 25, y + 25);
	SDL_SetWindowTitle(ctx.dbgWindow, "gbc debug view");
}

void gui_render_tile(SDL_Surface *surface, u16 addr, u16 tile_idx, u16 x, u16 y) {
	SDL_Rect rc = {0};
	for (int tile_y = 0; tile_y < TILE_SIZE; tile_y += 2) {
		u8 tile1 = bus_read(addr + tile_idx * TILE_SIZE + tile_y);
		u8 tile2 = bus_read(addr + tile_idx * TILE_SIZE + tile_y + 1);
		for (int bit = 7; bit >= 0; --bit) {
			u8 hi = !!(tile1 & (1 << bit)) << 1;
			u8 lo = !!(tile2 & (1 << bit));
			u8 color = hi | lo;
			rc.x = x + (7 - bit) * scale;
			rc.y = y + (tile_y / 2 * scale);
			rc.w = scale;
			rc.h = scale;
			SDL_FillRect(surface, &rc, color_palette[color]);
		}
	}
}

void gui_dbg_window_tick() {
	SDL_Rect rc = {0};
	rc.x = 0;
	rc.y = 0;
	rc.w = ctx.dbgSurface->w;
	rc.h = ctx.dbgSurface->h;
	SDL_FillRect(ctx.dbgSurface, &rc, 0xFF111222);

	int x_draw = 0;
	int y_draw = 0;
	int tile = 0;

	for (int y = 0; y < 24; ++y) {
		for (int x = 0; x < 16; ++x) {
			gui_render_tile(ctx.dbgSurface, 0x8000, tile, x_draw + (x * scale), y_draw + (y * scale));
			x_draw += (scale * 8);
			tile++;
		}
		y_draw += (scale * 8);
		x_draw = 0;
	}

	SDL_UpdateTexture(ctx.dbgTexture, NULL, ctx.dbgSurface->pixels, ctx.dbgSurface->pitch);
	SDL_RenderClear(ctx.dbgRenderer);
	SDL_RenderCopy(ctx.dbgRenderer, ctx.dbgTexture, NULL, NULL);
	SDL_RenderPresent(ctx.dbgRenderer);
}

void gui_gbc_window_tick() {
	// TODO
}

void gui_tick() {
	gui_dbg_window_tick();
	gui_gbc_window_tick();
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

gui_event gui_handle_input() {
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
			// case SDL_KEYDOWN: printf("GUI: KEY DOWN!\n");
			// break;
			// case SDL_KEYUP: printf("GUI: KEY UP!\n");
			// break;
			case SDL_WINDOWEVENT:
				if (e.window.event == SDL_WINDOWEVENT_CLOSE) return GUI_QUIT;
			break;
			case SDL_QUIT:
				return GUI_QUIT;
			break;
		}
	}
	return GUI_NONE;
}

void gui_shutdown() {
	if (ctx.window)
		SDL_DestroyWindow(ctx.window);
}
