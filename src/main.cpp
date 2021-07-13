#include "Cpu.h"

#include <iostream>
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
    Cpu cpu(1024);
    cpu.printRegisters();

    // Load program
    {
        std::ifstream program(argv[1], std::ios::in | std::ios::binary);
        if (!program.is_open())
        {
            std::cerr << "Unable to read: " << argv[1] << std::endl;
            return 1;
        }

        program.read((char*)&cpu.dram[0], 1024);
    }

    // Run program
    while( cpu.pc < cpu.dram.size())
    {
        // 1. Fetch.
        auto inst = cpu.fetch();

        // 2. Add 4 to the program counter.
        cpu.pc = cpu.pc + 4;

        // 3. Decode.
        // 4. Execute.
        cpu.execute(inst);
    }

    cpu.printRegisters();


	return 0;
}
