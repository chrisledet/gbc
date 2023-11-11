#include "gbc.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
// #include "mbc.h"

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
        fprintf(stderr, "ERR: cartridge load failure\n");
        return -1;
    }

    cart_debug();

    // set up cpu
    cart_context *cart_ctx = get_cart_context();
    bus_init(cart_ctx);
    cpu_init();
    ctx.running = true;
    ctx.paused = false;
    ctx.ticks = 0;
    const u32 MAX_CYCLES = 69905;

    // gbc_display_window();

    // display
    // SDL_Window *window = SDL_CreateWindow(
    //     "gbc",
    //     160*2,
    //     144*2,
    //     SDL_WINDOW_RESIZABLE
    // );
    // SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_PRESENTVSYNC);
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    // SDL_bool app_quit = SDL_FALSE;
    // while (!app_quit)
    // {

    //     SDL_Event event;
    //     while (SDL_PollEvent(&event))
    //     {
    //         switch (event.type)
    //         {
    //             case SDL_EVENT_QUIT:
    //                 app_quit = SDL_TRUE;
    //                 break;
    //         }
    //     }

        u32 cycles_this_update = 0;
        while (ctx.running) {
            while (cycles_this_update < MAX_CYCLES) {
                if (ctx.paused == false) {
                    cycles_this_update += cpu_step();
                } else {
                    SDL_Delay(10/*ms*/); // delay? are we sure?
                }

                ctx.ticks++;

                // DEBUG exit after 100 ticks
                // if (ctx.ticks > 100) {
                    // app_quit = SDL_TRUE;
                    // return 0;
                // }
            }

            cycles_this_update = 0;
        }


        // SDL_RenderClear(renderer);
        // SDL_RenderPresent(renderer);
    // }

    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);

    // SDL_Quit();
    return 0;
}
