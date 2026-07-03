
#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Clint.h"
#include "Plic.h"
#include "Uart.h"
#include "VirtIO.h"
#include "Trap.h"
#ifdef WITH_ELFIO
#include "ElfLoader.h"
#endif

#include <iostream>
#include <iomanip>
#include <chrono>
#include <string>

static constexpr int VALIDATION_TIMEOUT_S = 240;

//---------------------------------------------------------
void printUsage(const char* name)
{
	std::cout << "RVemuValidate: RISC-V boot validation" << std::endl;
	std::cout << "Usage: " << name << " <file.bin|elf> [disk.img]" << std::endl;
}

//---------------------------------------------------------
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage(argv[0]);
		return 1;
	}

	// Instantiate computer
	std::unique_ptr<Memory> mem(new Memory());
	std::unique_ptr<Plic> plic(new Plic());
	std::unique_ptr<Clint> clint(new Clint());
	std::unique_ptr<Uart> uart(new Uart());
	std::unique_ptr<Bus> bus(new Bus());
	std::unique_ptr<Cpu> cpu(new Cpu(*bus, DRAM_BASE+mem->size()));
	std::unique_ptr<VirtIO> virtio(new VirtIO());

	bus->addDevice(DRAM_BASE, mem.get());
	bus->addDevice(PLIC_BASE, plic.get());
	bus->addDevice(CLINT_BASE, clint.get());
	bus->addDevice(UART_BASE, uart.get());
	bus->addDevice(VIRTIO_BASE, virtio.get());

	bool isElf = false;
#ifdef WITH_ELFIO
	ElfLoader eloader;
	isElf = eloader.load(argv[1], mem.get());
#endif

	if (!isElf)
	{
		if (!isElf && !mem->preload(argv[1]))
		{
			std::cerr << "Error while loading: " << argv[1] << std::endl;
			return 1;
		}
	}
	else
	{
		if (eloader.start != 0)
			cpu->setPC(eloader.start);

		cpu->store_csr(MTVEC, eloader.mtvec);
	}

	if (argc == 3)
	{
		if (!virtio->loadDisk(argv[2]))
		{
			std::cerr << "Error while loading: " << argv[2] << std::endl;
			return 1;
		}
	}

	// Validation state
	std::string buf;
	bool found = false;
	auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(VALIDATION_TIMEOUT_S);

	uart->setOutputHook([&](uint8_t ch)
	{
		buf.push_back(static_cast<char>(ch));
		if (buf.size() > 80)
			buf.erase(0, buf.size() - 40);
		if (!found && buf.find("init: starting sh") != std::string::npos)
			found = true;
	});

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
			if (i != Interrupt::InvalidInterrupt) [[unlikely]]
				Trap::take_trap(cpu.get(), Except::InvalidExcept, i);

			if (found)
				break;

			if (std::chrono::steady_clock::now() >= deadline)
			{
				std::cerr << "\nValidation TIMEOUT after " << VALIDATION_TIMEOUT_S << " s" << std::endl;
				return 1;
			}
		}
	}
	catch (const CpuFatal& e)
	{
		std::cerr << "\nFatal Error: " << e.what() << std::endl;
		return 1;
	}

	std::cout << "\n--- Validation passed ---" << std::endl;
	return 0;
}
