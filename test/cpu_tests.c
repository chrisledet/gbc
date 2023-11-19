#include <stdio.h>
#include <stdio.h>
#include <assert.h>

#include <cpu.h>
#include <bus.h>

void test_cpu_step() {
    cpu_context *ctx = cpu_get_context();
    cpu_init(); // Assuming cpu_init takes a pointer to cpu_context

    bus_write(ctx->registers.PC, 0x0C);
    ctx->registers.BC.bytes.l = 0x01;

    cpu_fetch_instruction();
    u8 cycles_used = cpu_step();

    assert(ctx->registers.BC.bytes.l == 0x2);

    // Check the number of cycles after the step
    // assert(cycles_used == 1);

    // // Check if the CPU's halted and stopped states are as expected
    // assert(cpu.halted == /* expected boolean value */);
    // assert(cpu.stopped == /* expected boolean value */);

    // // Check if interrupts are managed correctly
    // assert(cpu.ime == /* expected boolean value */);
    // assert(cpu.enable_ime == /* expected boolean value */);
    // assert(cpu.IE == /* expected value */);
    // assert(cpu.IF == /* expected value */);

    // // You can also verify the fetched data, write bus status, and the write destination.
    // assert(cpu.fetched_data == /* expected value */);
    // assert(cpu.write_bus == /* expected boolean value */);
    // assert(cpu.write_dst == /* expected address */);

    printf("Test passed: cpu_step function works as expected.\n");
}

int main(int argc, char *argv[]) {
    test_cpu_step();
    return 0;
}
