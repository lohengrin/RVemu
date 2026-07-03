
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

static constexpr int BENCHMARK_TIMEOUT_S = 240;

//---------------------------------------------------------
void printUsage(const char* name)
{
	std::cout << "RVemuBench: RISC-V boot benchmark" << std::endl;
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
		if (!mem->preload(argv[1]))
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

	// Benchmark state
	std::string buf;
	bool found = false;
	auto start = std::chrono::high_resolution_clock::now();
	auto deadline = start + std::chrono::seconds(BENCHMARK_TIMEOUT_S);

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
			inst = cpu->fetch();
			cpu->forwardPC();

			uint8_t opcode, rd, rs1, rs2, funct3, funct7;
			cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

			auto exec_result = cpu->execute(inst, opcode, rd, rs1, rs2, funct3, funct7);
			if (exec_result.is_error()) [[unlikely]] {
				Except e = exec_result.error();
				Trap::take_trap(cpu.get(), e);
				if (Trap::is_fatal_except(e))
					throw CpuFatal("Fatal exception during execute");
				continue;
			}

			Interrupt i = cpu->check_pending_interrupt();
			if (i != Interrupt::InvalidInterrupt) [[unlikely]]
				Trap::take_trap(cpu.get(), Except::InvalidExcept, i);

			if (found)
				break;

			if (std::chrono::high_resolution_clock::now() >= deadline)
			{
				std::cerr << "\nBenchmark TIMEOUT after " << BENCHMARK_TIMEOUT_S << " s" << std::endl;
				return 1;
			}
		}
	}
	catch (const CpuFatal& e)
	{
		std::cerr << "\nFatal Error: " << e.what() << std::endl;
		return 1;
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	std::cout << "\n--- Benchmark complete ---" << std::endl;
	std::cout << "Boot time: " << std::fixed << std::setprecision(6) << elapsed.count() << " s" << std::endl;

	return 0;
}
