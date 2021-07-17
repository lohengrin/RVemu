#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Trap.h"

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
    try {
        uint32_t inst = 0;
        while(cpu.getPC() != 0)
        {
            // 1. Fetch.
            inst = cpu.fetch();
            if (inst == 0)
                break;

            //std::cout << "=== PC: 0x" << std::setfill('0') << std::setw(16) << cpu.pc << std::endl;

            // 2. Add 4 to the program counter.
            cpu.forwardPC();

            // 3. Decode.
            uint8_t opcode, rd, rs1, rs2, funct3, funct7;
            cpu.decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

            // Debug
            //cpu.printInstruction(inst, opcode, rd, rs1, rs2, funct3, funct7);
            //cpu.printCsrs();
            //cpu.printStack();

            // 4. Execute.
            cpu.execute(inst, opcode, rd, rs1, rs2, funct3, funct7);
        }

        std::cout << "Normal End of program" << std::endl;
        cpu.printRegisters();
    }
    catch (const CpuFatal& e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        cpu.printRegisters();
        return 1;
    }



	return 0;
}
