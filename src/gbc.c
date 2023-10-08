#include "gbc.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "cart.h"
#include "cpu.h"

#include "SDL.h"

/*
    gbc:
        - cart
        - cpu
        - addr bus
        - ppu
        - timer
*/

static gbc_context ctx;

gbc_context* gbc_get_context() {
    return &ctx;
}


void delay(uint32_t ms) {
    SDL_Delay(ms);
}

void gbc_display_window() {
    SDL_Window *window = SDL_CreateWindow(
        "gbc",
        640,
        480,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_bool app_quit = SDL_FALSE;
    while (!app_quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    app_quit = SDL_TRUE;
                    break;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

/*

`gbc_run` Return Codes:

| Code | Name                  | Description                   |
|------|-----------------------|-------------------------------|
|  0   | EMULATOR_SUCCESS      | Successfully executed.        |
| -1   | CARTRIDGE_LOAD_ERR    | Problem with cartridge file.  |
| -3   | CPU_ERR               | Failure during CPU execution. |
| ...  | ...                   | ...                           |
*/
int gbc_run(const char *rom_filepath) {
    if (!cart_load(rom_filepath)) {
        fprintf(stderr, "\n");
        return -1;
    }

    cpu_init();
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    while (ctx.running) {
        if (ctx.paused == false) {
            if (!cpu_step()) {
                return -3;
            }
        } else {
            delay(10);
        }

        ctx.ticks++;
    }

    cpu_step();

    // gbc_display_window();

    return 0;
}
