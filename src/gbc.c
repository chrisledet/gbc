#include "gbc.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
#include "mbc.h"

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
        400,
        600,
        SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC);
    //SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

   // Body of the Game Boy
    //SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Light gray
    //SDL_Rect body = {50, 50, 300, 500};
    //SDL_RenderFillRect(renderer, &body);

    // Screen
    //SDL_SetRenderDrawColor(renderer, 0, 128, 64, 255); // Greenish color like Game Boy's screen
    //SDL_FRect screen = {75, 100, 250, 200};
    //SDL_RenderFillRect(renderer, &screen);

    //// D-pad
    //SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); // Dark gray
    //SDL_Rect dPad = {75, 350, 100, 100};
    //SDL_RenderFillRect(renderer, &dPad);
    //SDL_SetRenderDrawColor(renderer, 192, 192, 192, 255); // Light gray
    //SDL_Rect dPadH = {95, 375, 60, 50};
    //SDL_RenderFillRect(renderer, &dPadH);
    //SDL_Rect dPadV = {100, 360, 50, 80};
    //SDL_RenderFillRect(renderer, &dPadV);

    //// Buttons
    //SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255); // Dark gray
    //SDL_Rect btnA = {250, 375, 50, 50};
    //SDL_Rect btnB = {200, 415, 50, 50};
    //SDL_RenderFillRect(renderer, &btnA);
    //SDL_RenderFillRect(renderer, &btnB);

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
Return Codes:
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

    cart_context *cart_ctx = get_cart_context();
    mbc_init(cart_ctx);
    bus_init(cart_ctx);
    cpu_init();
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;

    // gbc_display_window();

    while (ctx.running) {
        if (ctx.paused == false) {
            if (!cpu_step()) {
                return -3;
            }
        } else {
            delay(10);
        }

        ctx.ticks++;

        // DEBUG exit after 100 ticks
        if (ctx.ticks > 100) return 0;
    }


    return 0;
}
