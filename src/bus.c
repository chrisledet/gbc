#include "bus.h"
#include "cart.h"

uint8_t bus_read(uint16_t addr) {
	if (addr < 0x8000) {
		return cart_read(addr);
	}

	return 0;
}

void bus_write(uint16_t addr, uint8_t val) {

}
