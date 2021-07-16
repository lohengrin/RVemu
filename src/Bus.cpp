#include "Bus.h"
#include "Trap.h"

#include <iostream>

uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	if (DRAM_BASE <= addr)
		return mem.load(addr-DRAM_BASE, size);

	throw(CpuException(Except::LoadAccessFault));
	return 0;
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	if (DRAM_BASE <= addr)
		mem.store(addr - DRAM_BASE, size, value);
	else
		throw(CpuException(Except::StoreAMOAccessFault));
}
