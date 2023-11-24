#include <gbc.h>

#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <cart.h>
#include <cpu.h>
#include <bus.h>
#include <gui.h>
#include <ppu.h>
#include <SDL.h>
#include <SDL_thread.h>

/*
    gbc:
        - gui
        - cart
        - cpu
        - bus
        - ppu
        - timer
*/

static gbc_context ctx;

gbc_context* gbc_get_context() {
    return &ctx;
}

int gbc_run(const char *rom_filepath) {
    // load ui
    gui_init();

    // load cartridge / rom
    if (!cart_init(rom_filepath)) {
        fprintf(stderr, "ERR: cartridge load failure\n");
        return -1;
    }

    // cart_debug();

    cart_context *cart_ctx = get_cart_context();
    bus_init(cart_ctx);

    cpu_init();

    ctx.ticks = 0;
    ctx.running = true;
    while (ctx.running) {
        cpu_step();

        // ppu_tick();
        // gui_tick();
        ctx.ticks++;
        ctx.running = !(gui_handle_input() & GUI_QUIT);
    }

    return 0;
}
