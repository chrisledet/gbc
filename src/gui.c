#include <gui.h>

#include <stdio.h>
#include <windows.h>
#include <SDL.h>

u32 SCREEN_WIDTH = 1024;
u32 SCREEN_HEIGHT = 768;

static unsigned long tile_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};

typedef struct {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *surface;
	SDL_Texture *texture;
} gui_context;

static gui_context ctx = {0};

void gui_init() {
	SDL_Init(SDL_INIT_VIDEO);

	ctx.window = SDL_CreateWindow("gbc", 160*2, 144*2, SDL_WINDOW_RESIZABLE);
	ctx.renderer = SDL_CreateRenderer(ctx.window, NULL, SDL_RENDERER_PRESENTVSYNC);
	ctx.surface = SDL_CreateSurface(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA4444);
	ctx.texture = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

void gui_tick() {
	SDL_UpdateTexture(ctx.texture, NULL, ctx.surface->pixels, ctx.surface->pitch);
	SDL_RenderClear(ctx.renderer);
	SDL_RenderTexture(ctx.renderer, ctx.texture, NULL, NULL);
	SDL_RenderPresent(ctx.renderer);
}

gui_event gui_handle_input() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_KEY_DOWN: {
				printf("GUI: KEY DOWN!\n");
			}
			break;

			case SDL_EVENT_KEY_UP: {
				printf("GUI: KEY UP!\n");
			}
			break;

			case SDL_EVENT_QUIT: {
				return GUI_QUIT;
			}
			break;
		}
	}
	return GUI_NONE;
}
