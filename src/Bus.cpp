#include "Bus.h"
#include "Trap.h"

#include <iostream>

uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	if (CLINT_BASE <= addr && addr < CLINT_BASE + CLINT_SIZE)
		return clint.load(addr - CLINT_BASE, size);
	else if (PLIC_BASE <= addr && addr < PLIC_BASE + PLIC_SIZE)
		return plic.load(addr - CLINT_BASE, size);
	else if (DRAM_BASE <= addr)
		return mem.load(addr-DRAM_BASE, size);

	throw(CpuException(Except::LoadAccessFault));
	return 0;
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	if (CLINT_BASE <= addr && addr < CLINT_BASE + CLINT_SIZE)
		clint.store(addr - CLINT_BASE, size, value);
	else if (PLIC_BASE <= addr && addr < PLIC_BASE + PLIC_SIZE)
		plic.store(addr - CLINT_BASE, size, value);
	else if (DRAM_BASE <= addr)
		mem.store(addr - DRAM_BASE, size, value);
	else
		throw(CpuException(Except::StoreAMOAccessFault));
}
