#include "gbc.h"

#include "common.h"
#include "cart.h"
#include "cpu.h"
#include "bus.h"
#include "gui.h"
#include "ppu.h"
#include "timer.h"
#include "interrupt.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>
#include <SDL_thread.h>

/*
    gbc:
        - bus
        - cart
        - gui
        - cpu
        - ppu
        - timer
*/

static gbc_context ctx = {0};

gbc_context* gbc_get_context() {
    return &ctx;
}

int gbc_sys_run(void* data) {
    ctx.debug_mode = true;
    ctx.ticks = 0;
    ctx.running = true;

    cpu_init();
    timer_init();
    ppu_init();    

    while (ctx.running) {
        int cycles = 0;

        if (ctx.debug_mode)
            cpu_debug();
        
        ppu_tick();
        cycles += cpu_step();
        if (timer_tick())
            cpu_request_interrupt(INTERRUPT_TIMER);
    }
    return 0;
}

int gbc_run(const char *rom_filepath) {
    // load cartridge / rom
    if (!cart_init(rom_filepath)) {
        fprintf(stderr, "ERR: cartridge load failure\n");
        return -1;
    }
    // cart_debug();
    cart_context *cart_ctx = get_cart_context();
    bus_init(cart_ctx);

    // System
    SDL_CreateThread(gbc_sys_run, "gbc cpu", NULL);

    // UI
    gui_init();
    while (ctx.running) {
        SDL_Delay(1);
        gui_tick();
        ctx.running = !(gui_handle_input() & GUI_QUIT);
    }

    return 0;
}
