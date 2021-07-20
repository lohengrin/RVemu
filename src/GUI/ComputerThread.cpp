#include "ComputerThread.h"

#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Clint.h"
#include "Plic.h"
#include "Uart.h"
#include "Trap.h"


//---------------------------------------------------
ComputerThread::ComputerThread(const QString& programFile, QObject* parent) :
	QThread(parent),
	myCurrentMode(Mode::STARTED),
	myProgramFile(programFile),
	mem(nullptr),
	plic(nullptr),
	clint(nullptr),
	uart(nullptr),
	bus(nullptr),
	cpu(nullptr)
{
	qRegisterMetaType<CpuState>("CpuState");

}

//---------------------------------------------------
ComputerThread::~ComputerThread()
{

}

//---------------------------------------------------
void ComputerThread::updateState(const Cpu* cpu, uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7)
{
	myState.nextstep.pc = cpu->getPC();

	myState.regs = cpu->getRegs();
	myState.csrs = cpu->getCsrs();
	myState.nextstep.inst = inst;
	myState.nextstep.opcode = opcode;
	myState.nextstep.rd = rd;
	myState.nextstep.rs1 = rs1;
	myState.nextstep.rs2 = rs2;
	myState.nextstep.funct3 = funct3;
	myState.nextstep.funct7 = funct7;

	myState.stack.clear();
	for (uint64_t sp = cpu->getRegister(REGSP); sp < DRAM_BASE + DEFAULT_MEMORYSIZE; sp += 8)
		myState.stack.push_back(std::make_pair(sp, cpu->readMem(sp, 64)));
}


//---------------------------------------------------
void ComputerThread::keypressed(char c)
{
	if (uart)
		uart->putChar(c);
}

//---------------------------------------------------
void ComputerThread::run()
{
	while (resetFlag)
	{
		resetFlag = false; 
		abortFlag = false;

		// Instanciate Computer
		mem = new Memory();
		plic = new Plic();
		clint = new Clint();
		uart = new Uart(false);
		bus = new Bus();
		cpu = new Cpu(*bus, DRAM_BASE + mem->size());

		bus->addDevice(DRAM_BASE, mem);
		bus->addDevice(PLIC_BASE, plic);
		bus->addDevice(CLINT_BASE, clint);
		bus->addDevice(UART_BASE, uart);

		myCurrentMode = Mode::STARTED;
		Mode previousMode = Mode::STARTED;

		while (!abortFlag)
		{
			switch (myCurrentMode)
			{
			case Mode::STARTED:
			{
				previousMode = Mode::STARTED;
				mem->preload(myProgramFile.toStdString());
				myCurrentMode = Mode::PAUSED;

				uint32_t inst = cpu->fetch();
				uint8_t opcode, rd, rs1, rs2, funct3, funct7;
				cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

				updateState(cpu, inst, opcode, rd, rs1, rs2, funct3, funct7);
			}
			break;
			case Mode::STOPED:
				if (previousMode != Mode::STOPED)
				{
					emit stepFinished(myState);
					emit stoped();
				}
				previousMode = Mode::STOPED;
				// Do nothing
				msleep(100);
				break;
			case Mode::PAUSED:
				if (previousMode != Mode::PAUSED)
				{
					emit stepFinished(myState);
					emit paused();
				}
				previousMode = Mode::PAUSED;
				// Do nothing
				msleep(100);
				break;
			case Mode::RUNSTEP:
			{
				myCurrentMode = Mode::PAUSED;
			}
			case Mode::RUNNING:
			{
				previousMode = Mode::RUNNING;
				// Run program
				try {

					if (cpu->getPC() == 0)
					{
						myCurrentMode = Mode::STOPED;
						break;
					}

					// 1. Fetch.
					uint32_t inst = cpu->fetch();

					// 2. Add 4 to the program counter.
					cpu->forwardPC();

					// 3. Decode.
					uint8_t opcode, rd, rs1, rs2, funct3, funct7;
					cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

					//- To GUI
					updateState(cpu, inst, opcode, rd, rs1, rs2, funct3, funct7);

					// 4. Execute.
					cpu->execute(inst, opcode, rd, rs1, rs2, funct3, funct7);

					// 5. check interrupt
					Interrupt i = cpu->check_pending_interrupt();
					if (i != Interrupt::InvalidInterrupt)
						Trap::take_trap(cpu, Except::InvalidExcept, i);

					// 6. Read Uart
					char key = uart->getChar();
					if (key)
						emit outputChar(key);
				}
				catch (const CpuFatal&)
				{
					//std::cerr << "Fatal Error: " << e.what() << std::endl;
					myCurrentMode = Mode::STOPED;
				}
			}
			break;
			}

			
		}

		delete cpu;
		delete bus;
		delete uart;
		delete clint;
		delete plic;
		delete mem;
	}
}