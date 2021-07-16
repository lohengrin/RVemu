#include "Bus.h"
#include "Trap.h"

#include <iostream>

uint64_t Bus::load(uint64_t addr, uint8_t size) const
{
	if (CLINT_BASE <= addr && addr < CLINT_BASE + clint.size())
		return clint.load(addr - CLINT_BASE, size);
	else if (PLIC_BASE <= addr && addr < PLIC_BASE + plic.size())
		return plic.load(addr - CLINT_BASE, size);
	else if (UART_BASE <= addr && addr < UART_BASE + uart.size())
		return uart.load(addr - UART_BASE, size);
	else if (DRAM_BASE <= addr)
		return mem.load(addr-DRAM_BASE, size);

	throw(CpuException(Except::LoadAccessFault));
	return 0;
}

void Bus::store(uint64_t addr, uint8_t size, uint64_t value)
{
	if (CLINT_BASE <= addr && addr < CLINT_BASE + clint.size())
		clint.store(addr - CLINT_BASE, size, value);
	else if (PLIC_BASE <= addr && addr < PLIC_BASE + plic.size())
		plic.store(addr - CLINT_BASE, size, value);
	else if (UART_BASE <= addr && addr < UART_BASE + uart.size())
		uart.store(addr - UART_BASE, size, value);
	else if (DRAM_BASE <= addr)
		mem.store(addr - DRAM_BASE, size, value);
	else
		throw(CpuException(Except::StoreAMOAccessFault));
}
