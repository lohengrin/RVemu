#pragma once

#include "Bus.h"

#include <stdint.h>

#define REGX0 0
#define REGX1 1
#define REGSP 2

class Cpu {
	friend struct Trap;
public:
	enum class Mode {
		User = 0x00,
		Supervisor = 0x01,
		Machine = 0x03,
	};
	
	//! Object 
	Cpu(Bus& b);
	virtual ~Cpu();

	//! Main loop functions
	uint32_t fetch() const;
	void decode(uint32_t inst, uint8_t& opcode, uint8_t& rd, uint8_t& rs1, uint8_t& rs2, uint8_t& funct3, uint8_t& funct7) const;
	void execute(uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7);
	void forwardPC() { pc += 4; }
	uint64_t getPC() { return pc; }


	//! Utility
	void printRegisters() const;
	void printCsrs() const;
	void printInstruction(uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7) const;
	void printStack();

protected:
	//! Cpu functions
	uint64_t load(uint64_t addr, uint8_t size);
	void store(uint64_t addr, uint8_t size, uint64_t value);
	uint64_t load_csr(uint64_t addr) const;
	void store_csr(uint64_t addr, uint64_t value);

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
	uint64_t warppingDiv(uint64_t a, uint64_t b) const { return a / b; }
	uint32_t warppingRem(uint32_t a, uint32_t b) const { return a % b; }


	void executeError(uint8_t opcode, uint8_t funct3, uint8_t funct7) const;

	//! internals
	//! Registers
	uint64_t	regs[32];
	//! Control and Status registers
	uint64_t	csrs[4096];
	//! Current mode
	Mode		mode;
	//! program counter
	uint64_t	pc;

	Bus& bus;
};