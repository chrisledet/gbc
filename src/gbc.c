#include <gbc.h>

#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <cart.h>
#include <cpu.h>
#include <bus.h>
#include <gui.h>
#include <SDL.h>
#include <SDL_thread.h>

/*
    gbc:
        - cart
        - cpu
        - addr bus
        - ppu
        - timer
*/

// HANDLE cpu_handle;

SDL_Thread *cpu_thread;

static gbc_context ctx;

gbc_context* gbc_get_context() {
    return &ctx;
}

int gbc_run_cpu(void*) {
    ctx.running = true;
    ctx.ticks = 0;
    cpu_init();

    u32 MAX_CYCLES = 69905;
    u32 cycles = 0;
    while (ctx.running) {
        while (cycles < MAX_CYCLES) {
            if (ctx.paused == false) {
                cycles += cpu_step();
            } else {
                SDL_Delay(100/*ms*/); // delay? are we sure?
            }

            ctx.ticks++;
        }

        cycles -= MAX_CYCLES;
    }

    return 0;
}

int gbc_run(const char *rom_filepath) {
    // load cartridge / rom
    if (!cart_load(rom_filepath)) {
        fprintf(stderr, "ERR: cartridge load failure\n");
        return -1;
    }
    cart_debug();
    cart_context *cart_ctx = get_cart_context();
    bus_init(cart_ctx);

    // load ui
    gui_init();

    // start cpu
    cpu_thread = SDL_CreateThread(gbc_run_cpu, "cpu", (void*)NULL);

    // input loop
    while (!ctx.quit) {
        ctx.quit = gui_get_quit();
        SDL_Delay(10/*ms*/);
    }

    // shutdown
    gui_shutdown();
    return 0;
}
