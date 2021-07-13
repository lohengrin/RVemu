#pragma once

#include <stdint.h>

#define REGX0 0
#define REGX1 1
#define REGSP 2

class Cpu {
public:
	Cpu(size_t memorySize = 1000);
	virtual ~Cpu();

	void printRegisters();

protected:
	uint64_t regs[32];
	uint64_t pc;
	uint8_t* dram;
};