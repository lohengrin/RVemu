#pragma once

#include "Bus.h"

#include <stdint.h>

#define REGX0 0
#define REGX1 1
#define REGSP 2

class Cpu {
public:
	//! Object 
	Cpu(Bus& b);
	virtual ~Cpu();

	//! Cpu functions
	uint64_t load(uint64_t addr, uint8_t size);
	void store(uint64_t addr, uint8_t size, uint64_t value);
	uint32_t fetch() const;
	void execute(uint32_t inst);


	//! Utility
	void printRegisters();

	uint64_t	pc;

protected:
	uint64_t warppingAdd(uint64_t a, uint64_t b) const { return a + b; }
	uint64_t warppingSub(uint64_t a, uint64_t b) const { return a - b; }
	uint64_t warppingMul(uint64_t a, uint64_t b) const { return a * b; }
	uint64_t warppingShr(uint64_t a, uint64_t b) const { return a >> b; }
	uint32_t warppingShr(uint32_t a, uint64_t b) const { return a >> b; }
	int32_t warppingShr(int32_t a, uint64_t b) const { return a >> b; }
	int64_t warppingShr(int64_t a, uint64_t b) const { return a >> b; }
	uint64_t warppingShl(uint64_t a, uint64_t b) const { return a << b; }
	uint32_t warppingShl(uint32_t a, uint64_t b) const { return a << b; }
	int64_t warppingShl(int64_t a, uint64_t b) const { return a << b; }

	//! internals
	uint64_t	regs[32];
	Bus&		bus;
};