#pragma once

#include "Memory.h"

class Bus {
public:
	Bus(Memory& m) : mem(m)  {}
	virtual ~Bus() {}

	//! Bus functions
	uint64_t load(uint64_t addr, uint8_t size);
	void store(uint64_t addr, uint8_t size, uint64_t value);

protected:
	Memory& mem;
};