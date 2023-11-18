#include <gui.h>
#include <stdio.h>
#include <SDL.h>


typedef struct {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Surface* surface;
	SDL_Texture* texture;
} gui_context;

static gui_context ctx = {0};
static u32 SCREEN_WIDTH = 1024;
static u32 SCREEN_HEIGHT = 768;
static unsigned long tile_colors[4] = {0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000};


void gui_init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	ctx.window = SDL_CreateWindow("gbc", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*2, 144*2, 0);
	ctx.renderer = SDL_CreateRenderer(ctx.window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	ctx.surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
	ctx.texture = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	gui_display_clear();
}

u64 gui_get_ticks() {
	return SDL_GetTicks();
}

void gui_tick() {
	//SDL_UpdateTexture(ctx.texture, NULL, ctx.surface->pixels, ctx.surface->pitch);
	//SDL_RenderClear(ctx.renderer);
	//SDL_RenderTexture(ctx.renderer, ctx.texture, NULL, NULL);
	//SDL_RenderPresent(ctx.renderer);
}

void gui_display_clear() {
	SDL_LockSurface(ctx.surface);
	SDL_memset(ctx.surface->pixels, 0, (ctx.surface->h * ctx.surface->pitch));
	SDL_UnlockSurface(ctx.surface);
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
	SDL_DestroyWindow(ctx.window);
}
