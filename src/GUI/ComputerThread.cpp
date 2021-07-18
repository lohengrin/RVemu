#include "ComputerThread.h"

#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Trap.h"

//---------------------------------------------------
ComputerThread::ComputerThread(const QString& programFile, QObject* parent) :
	QThread(parent),
	myCurrentMode(Mode::STARTED),
	myProgramFile(programFile)
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
	myState.nextstep.rs1 = rs2;
	myState.nextstep.funct3 = funct7;

	myState.stack.clear();
	for (uint64_t sp = cpu->getRegister(REGSP); sp < DRAM_BASE + DEFAULT_MEMORYSIZE; sp += 8)
		myState.stack.push_back(std::make_pair(sp, cpu->readMem(sp, 64)));
}

//---------------------------------------------------
void ComputerThread::run()
{
	while (resetFlag)
	{
		resetFlag = false; 
		abortFlag = false;
		// Instanciate Computer
		std::unique_ptr<Memory> mem(new Memory);
		std::unique_ptr<Bus> bus(new Bus(*mem));
		std::unique_ptr<Cpu> cpu(new Cpu(*bus));

		myCurrentMode = Mode::STARTED;

		while (!abortFlag)
		{
			Mode previousMode = Mode::STARTED;
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

				updateState(cpu.get(), inst, opcode, rd, rs1, rs2, funct3, funct7);
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
					updateState(cpu.get(), inst, opcode, rd, rs1, rs2, funct3, funct7);
					//emit stepFinished(myState);

					// 4. Execute.
					cpu->execute(inst, opcode, rd, rs1, rs2, funct3, funct7);
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
	}
}