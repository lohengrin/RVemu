#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"

#include <iostream>
#include <iomanip>
#include <fstream>


void printUsage(const char* name)
{
    std::cout << "RVemu: a simple RISC-V emulator" << std::endl;
    std::cout << "Usage: " << name << " <file.bin>" << std::endl;
}

int main(int argc, const char *argv[]) 
{
    // Check args
    if (argc != 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    // Instanciate cpu
    Memory mem;
    Bus bus(mem);
    Cpu cpu(bus);

    cpu.printRegisters();
    // Load program
    {
        std::ifstream program(argv[1], std::ios::in | std::ios::binary | std::ios::ate);
        if (!program.is_open())
        {
            std::cerr << "Unable to read: " << argv[1] << std::endl;
            return 1;
        }

        size_t fsize = program.tellg();
        program.seekg(0, std::ios::beg);

        char* buffer = new char[fsize];
        program.read(buffer, fsize);

        mem.preload(0, (uint8_t*)buffer, fsize);
        delete[] buffer;
    }

    // Run program
    uint32_t inst = 0;
    while(cpu.pc != 0)
    {
        // 1. Fetch.
        inst = cpu.fetch();
        if (inst == 0)
            break;

        //std::cout << "=== PC: 0x" << std::setfill('0') << std::setw(16) << cpu.pc << std::endl;

        // 2. Add 4 to the program counter.
        cpu.pc = cpu.pc + 4;

        // 3. Decode.

        // 4. Execute.
        cpu.printInstruction(inst);
        cpu.printCsrs();
        //cpu.printStack();
        cpu.execute(inst);
    }
    std::cout << "End of program" << std::endl;

    cpu.printRegisters();


	return 0;
}
