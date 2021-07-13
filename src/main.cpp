#include "Cpu.h"

#include <iostream>


int main(int argc, const char *argv[]) 
{
	Cpu cpu(1000);

	cpu.printRegisters();

	return 0;
}
