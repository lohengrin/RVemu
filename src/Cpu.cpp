#include "Cpu.h"

#include <vector>
#include <string>
#include <iostream>

//---------------------------------------------------------
Cpu::Cpu(size_t memorySize) :
	regs{0},
	pc(0),
	dram(nullptr)
{
	dram = new uint8_t[memorySize];

	regs[REGSP] = memorySize;
}

//---------------------------------------------------------
Cpu::~Cpu()
{
	delete[] dram;
}

//---------------------------------------------------------
void Cpu::printRegisters()
{
	static std::vector<std::string> header =
	{ "zero", " ra ", " sp ", " gp ", " tp ", " t0 ", " t1 ", " t2 ", " s0 ", " s1 ", " a0 ",
		" a1 ", " a2 ", " a3 ", " a4 ", " a5 ", " a6 ", " a7 ", " s2 ", " s3 ", " s4 ", " s5 ",
		" s6 ", " s7 ", " s8 ", " s9 ", " s10", " s11", " t3 ", " t4 ", " t5 ", " t6 " };

	for (int i = 0; i < 32; i++)
		std::cout << header[i] << "\t";
	std::cout << std::endl;
	for (int i = 0; i < 32; i++)
		std::cout << std::hex << "0x" << regs[i] << "\t";
	std::cout << std::endl;
}
