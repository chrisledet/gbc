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

void gbc_delay(u32 ms) {
    SDL_Delay(ms);
}

u32 gbc_get_ticks() {
    return SDL_GetTicks();
}


int gbc_sys_run(void* data) {
    ctx.debug_mode = true;
    ctx.ticks = 0;
    ctx.running = true;

    cpu_init();
    timer_init();
    ppu_init();    

    while (ctx.running) {
//        if (ctx.debug_mode)
//            cpu_debug();

        cpu_step();
        if (timer_tick())
            cpu_request_interrupt(INTERRUPT_TIMER);
        ppu_tick();
    }
    return 0;
}

int gbc_run(const char *rom_filepath) {
    // Load ROM
    if (!cart_init(rom_filepath))
        return -1;

    fprintf(stderr, "GBC STARTED\n");
    fprintf(stderr, "WAITING FOR ROM...\n");
    fprintf(stderr, "ROM?>: %s\n", rom_filepath);
    cart_debug();

    // System
    bus_init();
    SDL_CreateThread(gbc_sys_run, "gbc cpu", NULL);

    // UI
    gui_init();
    u32 prev_frame = 0;
    while (ctx.running) {
        gbc_delay(1);
        ctx.running = !(gui_handle_input() & GUI_QUIT);

        if (ppu_get_current_frame() != prev_frame) {
            gui_tick();
        }
        prev_frame = ppu_get_current_frame();
    }

    return 0;
}
