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
	uint64_t load(uint64_t addr, uint8_t size) const;
	void store(uint64_t addr, uint8_t size, uint64_t value);
	uint64_t load_csr(uint64_t addr) const;
	void store_csr(uint64_t addr, uint64_t value);

	uint32_t fetch() const;
	void execute(uint32_t inst);


	//! Utility
	void printRegisters();
	void printInstruction(uint32_t inst) const;
	void printStack() const;

	uint64_t	pc;

protected:
	uint64_t warppingAdd(uint64_t a, uint64_t b) const { return a + b; }
	uint64_t warppingAdd(uint64_t a, int64_t b) const { return a + b; }
	uint64_t warppingSub(uint64_t a, uint64_t b) const { return a - b; }
	uint64_t warppingMul(uint64_t a, uint64_t b) const { return a * b; }
	uint64_t warppingShr(uint64_t a, uint64_t b) const { return a >> b; }
	uint32_t warppingShr(uint32_t a, uint64_t b) const { return a >> b; }
	int32_t warppingShr(int32_t a, uint64_t b) const { return a >> b; }
	int64_t warppingShr(int64_t a, uint64_t b) const { return a >> b; }
	uint64_t warppingShl(uint64_t a, uint64_t b) const { return a << b; }
	uint32_t warppingShl(uint32_t a, uint64_t b) const { return a << b; }
	int64_t warppingShl(int64_t a, uint64_t b) const { return a << b; }

	void printExecuteError(uint8_t opcode, uint8_t funct3, uint8_t funct7) const;
	


	//! internals
	//! Registers
	uint64_t	regs[32];
	//! Control and Status registers
	uint64_t	csrs[4096];
	Bus& bus;
};