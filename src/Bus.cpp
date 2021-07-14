#include "Bus.h"

#include <iostream>

uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	if (DRAM_BASE <= addr)
		return mem.load(addr, size);

	std::cerr << "Bus::load: Invalid address" << std::endl;
	return 0;
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	if (DRAM_BASE <= addr)
		mem.store(addr, size, value);
	else
		std::cerr << "Bus::store: Invalid address" << std::endl;
}
