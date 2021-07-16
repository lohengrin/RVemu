#pragma once

#include "Memory.h"
#include "Plic.h"
#include "Clint.h"
#include "Uart.h"

/// The address which the core-local interruptor (CLINT) starts. It contains the timer and
/// generates per-hart software interrupts and timer
/// interrupts.
const uint64_t CLINT_BASE = 0x2000000;

/// The address which the platform-level interrupt controller (PLIC) starts. The PLIC connects all external interrupts in the
/// system to all hart contexts in the system, via the external interrupt source in each hart.
const uint64_t PLIC_BASE = 0xc000000;

/// The address which dram starts, same as QEMU virt machine.
const uint64_t DRAM_BASE = 0x80000000;

/// The address which UART starts, same as QEMU virt machine.
const uint64_t UART_BASE = 0x10000000;

class Bus {
public:
	Bus(Memory& m) : mem(m)  {}
	virtual ~Bus() {}

	uint64_t load(uint64_t addr, uint8_t size) const;
	void store(uint64_t addr, uint8_t size, uint64_t value);

protected:
	Memory& mem;
	Plic	plic;
	Clint	clint;
	Uart	uart;
};