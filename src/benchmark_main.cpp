
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
#include <csignal>
#include <future>

static constexpr int BENCHMARK_TIMEOUT_S = 30;

// Signal handler to catch the interruption
void timeout_handler(int signum) {
    std::cout << "\n[Main Thread] Interrupted by Watchdog! Timeout reached." << std::endl;
    // Clean up and exit, or throw an exception to unwind the stack gracefully
    exit(1); 
}

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


	// Register the signal handler for the main thread
    std::signal(SIGUSR1, timeout_handler);
    pthread_t main_thread = pthread_self();

    // Promise/Future pair to communicate with the watchdog
    std::promise<void> work_done;
    std::future<void> watchdog_future = work_done.get_future();

    // 1. Launch the Watchdog Thread
    std::thread watchdog([&watchdog_future, main_thread]() {
        // Watchdog waits for 3 seconds for the future to be fulfilled
        if (watchdog_future.wait_for(std::chrono::seconds(BENCHMARK_TIMEOUT_S)) == std::future_status::timeout) {
            std::cout << "[Watchdog] Time is up! Sending interrupt to main thread...\n";
            // Fire an OS-level interrupt at the main thread
            pthread_kill(main_thread, SIGUSR1);
        } else {
            std::cout << "[Watchdog] Work finished in time. Shutting down.\n";
        }
    });

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

		}
	}
	catch (const CpuFatal& e)
	{
		std::cerr << "\nFatal Error: " << e.what() << std::endl;
		exit(1);
	}

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = end - start;

	std::cout << "\n--- Benchmark complete ---" << std::endl;
	std::cout << "Boot time: " << std::fixed << std::setprecision(6) << elapsed.count() << " s" << std::endl;
	std::cout << "\n--- Validation passed ---" << std::endl;

	// If work finishes successfully, notify the watchdog to cancel the timeout
    work_done.set_value();
	watchdog.join();

	exit(0);
}
