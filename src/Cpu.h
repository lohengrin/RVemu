#pragma once

#include <vector>
#include <stdint.h>

#define REGX0 0
#define REGX1 1
#define REGSP 2

class Cpu {
public:
	//! Object 
	Cpu(size_t memorySize = 1000);
	virtual ~Cpu();

	//! Cpu functions
	uint32_t fetch() const;
	void execute(uint32_t inst);


	//! Utility
	void printRegisters();

	//! internals
	uint64_t regs[32];
	uint64_t pc;
	std::vector<uint8_t> dram;
};