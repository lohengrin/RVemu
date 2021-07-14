#include "Cpu.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

//---------------------------------------------------------
Cpu::Cpu(size_t memorySize) :
	regs{ 0 },
	pc(0),
	dram(0)
{
	dram.resize(memorySize);

	regs[REGSP] = memorySize;
}

//---------------------------------------------------------
Cpu::~Cpu()
{
}

//---------------------------------------------------------
uint32_t Cpu::fetch() const
{
	return (uint32_t)dram[pc] | (uint32_t)dram[pc + 1] << 8 | (uint32_t)dram[pc + 2] << 16 | (uint32_t)dram[pc + 3] << 24;
}

//---------------------------------------------------------
void Cpu::execute(uint32_t inst)
{
	uint8_t opcode = inst & 0x7f;
	uint8_t rd = ((inst >> 7) & 0x1f);
	uint8_t rs1 = ((inst >> 15) & 0x1f);
	uint8_t rs2 = ((inst >> 20) & 0x1f);

	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;

	switch (opcode)
	{
	case 0x13: // addi
	{
		uint64_t imm = (inst & 0xfff00000) >> 20;
		regs[rd] = regs[rs1] + imm;
		break;
	}
	case 0x33: // add
	{
		regs[rd] = regs[rs1] + regs[rs2];
		break;
	}
	default:
		std::cerr << "Opcode not implemented: 0x" << std::hex << opcode << std::endl;
	}
}

//---------------------------------------------------------
void Cpu::printRegisters()
{
	static std::vector<std::string> header =
	{ "zero", " ra ", " sp ", " gp ", " tp ", " t0 ", " t1 ", " t2 ", " s0 ", " s1 ", " a0 ",
		" a1 ", " a2 ", " a3 ", " a4 ", " a5 ", " a6 ", " a7 ", " s2 ", " s3 ", " s4 ", " s5 ",
		" s6 ", " s7 ", " s8 ", " s9 ", " s10", " s11", " t3 ", " t4 ", " t5 ", " t6 " };

	for (int i = 0; i < 32; i++)
	{
		std::cout << std::setfill('0') << std::setw(2) << std::dec << i << ": " << header[i] << ": " << std::hex << "0x" << std::setw(8) << regs[i] << "\t";
		if ((i+1) % 5 == 0)
			std::cout << std::endl;
	}
}
