
#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Clint.h"
#include "Plic.h"
#include "Uart.h"
#include "VirtIO.h"
#include "Trap.h"

#include <iostream>
#include <iomanip>

//---------------------------------------------------------
void printUsage(const char* name)
{
	std::cout << "RVemu: a simple RISC-V emulator" << std::endl;
	std::cout << "Usage: " << name << " <file.bin> <disk.img>" << std::endl;
}

//---------------------------------------------------------
void printRegisters(const Cpu* cpu)
{
	for (int i = 0; i < 32; i++)
	{
		std::cout << std::setfill('0') << std::setw(2) << std::dec << i << ": " << RegisterNames[i] << ": " << std::hex << "0x" << std::setw(16) << cpu->getRegister(i) << "\t";
		if ((i + 1) % 5 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

//---------------------------------------------------------
void printCsrs(const Cpu* cpu)
{
	std::cout << "==== CSRS ========================================" << std::endl;
	std::cout << "mstatus=0x" << std::hex << std::setw(16) << cpu->getCsr(MSTATUS)
		<< "\tmtvec=0x" << std::hex << std::setw(16) << cpu->getCsr(MTVEC)
		<< "\tmepc=0x" << std::hex << std::setw(16) << cpu->getCsr(MEPC)
		<< "\tmcause=0x" << std::hex << std::setw(16) << cpu->getCsr(MCAUSE) << std::endl;

	std::cout << "sstatus=0x" << std::hex << std::setw(16) << cpu->getCsr(SSTATUS)
		<< "\tstvec=0x" << std::hex << std::setw(16) << cpu->getCsr(STVEC)
		<< "\tsepc=0x" << std::hex << std::setw(16) << cpu->getCsr(SEPC)
		<< "\tscause=0x" << std::hex << std::setw(16) << cpu->getCsr(SCAUSE) << std::endl;
	std::cout << "==================================================" << std::endl;
}

//---------------------------------------------------------
void printInstruction(uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7)
{
	std::cout << getInstructionName(opcode, funct3, funct7) <<
		" rd=0x" << std::hex << ASU32(rd) << " (" << RegisterNames[rd] << ") " <<
		" rs1=0x" << std::hex << ASU32(rs1) << " (" << RegisterNames[rs1] << ") " <<
		" rs2=0x" << std::hex << ASU32(rs2) << " (" << RegisterNames[rs2] << ") " << std::endl;
}

//---------------------------------------------------------
void printStack(const Cpu* cpu)
{
	std::cout << "==== Stack ====" << std::endl;
	for (uint64_t sp = cpu->getRegister(REGSP); sp < DRAM_BASE + DEFAULT_MEMORYSIZE; sp += 8)
	{
		std::cout << std::hex << "0x" << std::setfill('0') << std::setw(16) << sp
			<< std::hex << "   0x" << std::setfill('0') << std::setw(16) << cpu->readMem(sp, 64) << std::endl;
	}
	std::cout << "===============" << std::endl;
}


int main(int argc, char** argv)
{
	// Check args
	if (argc != 3)
	{
		printUsage(argv[0]);
		return 1;
	}

	// Instanciate Computer
	std::unique_ptr<Memory> mem(new Memory());
	std::unique_ptr<Plic> plic(new Plic());
	std::unique_ptr<Clint> clint(new Clint());
	std::unique_ptr<Uart> uart(new Uart());
	std::unique_ptr<Bus> bus(new Bus());
	std::unique_ptr<Cpu> cpu(new Cpu(*bus, DRAM_BASE+mem->size()));
	std::unique_ptr<VirtIO> virtio(new VirtIO());

	bus->addDevice(DRAM_BASE, mem.get()); // Add first for faster access
	bus->addDevice(PLIC_BASE, plic.get());
	bus->addDevice(CLINT_BASE, clint.get());
	bus->addDevice(UART_BASE, uart.get());
	bus->addDevice(VIRTIO_BASE, virtio.get());
		
	if (!mem->preload(argv[1]))
	{
		std::cerr << "Error while loading: " << argv[1] << std::endl;
		return 1;
	}

	if (!virtio->loadDisk(argv[2]))
	{
		std::cerr << "Error while loading: " << argv[2] << std::endl;
		return 1;
	}

	// Run program
	try {
		uint32_t inst = 0;
		while (true)
		{
			// 1. Fetch.
			inst = cpu->fetch();

			// 2. Add 4 to the program counter.
			cpu->forwardPC();

			// 3. Decode.
			uint8_t opcode, rd, rs1, rs2, funct3, funct7;
			cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

			// Debug
			//printInstruction(inst, opcode, rd, rs1, rs2, funct3, funct7);
			//printCsrs(cpu.get());
			//printStack(cpu.get());

			// 4. Execute.
			cpu->execute(inst, opcode, rd, rs1, rs2, funct3, funct7);

			// 5. check interrupt
			Interrupt i = cpu->check_pending_interrupt();
			if (i != Interrupt::InvalidInterrupt)
				Trap::take_trap(cpu.get(), Except::InvalidExcept, i);
		}
	}
	catch (const CpuFatal& e)
	{
		std::cerr << "Fatal Error: " << e.what() << std::endl;
		printRegisters(cpu.get());
		return 1;
	}

	std::cout << "Normal End of program" << std::endl;
	printRegisters(cpu.get());

	return 0;
}
