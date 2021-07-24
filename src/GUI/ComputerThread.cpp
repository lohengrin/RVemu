#include "ComputerThread.h"

#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Clint.h"
#include "Plic.h"
#include "Uart.h"
#include "VirtIO.h"
#include "Trap.h"

#include <fstream>
#include <iomanip>

//---------------------------------------------------
ComputerThread::ComputerThread(const QString& programFile, const QString& diskImage, QObject* parent) :
	QThread(parent),
	myCurrentMode(Mode::STARTED),
	myProgramFile(programFile),
	myDiskImage(diskImage),
	mem(nullptr),
	plic(nullptr),
	clint(nullptr),
	uart(nullptr),
	bus(nullptr),
	cpu(nullptr),
	virtio(nullptr)
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
//	for (uint64_t sp = cpu->getRegister(REGSP); sp < DRAM_BASE + DEFAULT_MEMORYSIZE; sp += 8)
//		myState.stack.push_back(std::make_pair(sp, cpu->readMem(sp, 64)));
}


//---------------------------------------------------
void ComputerThread::keypressed(char c)
{
	if (uart)
		uart->putChar(c);
}

void printRegisters(const Cpu* cpu, std::ofstream& out)
{
	for (int i = 0; i < 32; i++)
	{
		out << "x" << std::setfill('0') << std::setw(2) << std::dec << i << "(" << RegisterNames[i] << ")=" << std::setfill('0') << std::hex << "0x" << std::setw(16) << cpu->getRegister(i) << " ";
		if ((i + 1) % 4 == 0)
			out << std::endl;
	}
}

//---------------------------------------------------------
void printCsrs(const Cpu* cpu, std::ofstream& out)
{
	out << "mstatus=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(MSTATUS)
		<< " mtvec=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(MTVEC)
		<< " mepc=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(MEPC)
		<< " mcause=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(MCAUSE) << std::endl;

	out << "sstatus=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(SSTATUS)
		<< " stvec=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(STVEC)
		<< " sepc=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(SEPC)
		<< " scause=0x" << std::hex << std::setfill('0') << std::setw(16) << cpu->getCsr(SCAUSE) << std::endl;
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
		virtio = new VirtIO();

		bus->addDevice(DRAM_BASE, mem);
		bus->addDevice(PLIC_BASE, plic);
		bus->addDevice(CLINT_BASE, clint);
		bus->addDevice(UART_BASE, uart);
		bus->addDevice(VIRTIO_BASE, virtio);

		myCurrentMode = Mode::STARTED;
		Mode previousMode = Mode::STARTED;

		while (!abortFlag)
		{
			switch (myCurrentMode)
			{
			case Mode::STARTED:
			{
				previousMode = Mode::STARTED;
				if (!myDiskImage.isEmpty())
					virtio->loadDisk(myDiskImage.toStdString());
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
					// 1. Fetch.
					uint32_t inst = cpu->fetch();
					// 3. Decode.
					uint8_t opcode, rd, rs1, rs2, funct3, funct7;
					cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

					//- To GUI
					updateState(cpu, inst, opcode, rd, rs1, rs2, funct3, funct7);

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
					// 1. Fetch.
					uint32_t inst = cpu->fetch();

					// 2. Add 4 to the program counter.
					cpu->forwardPC();

					// 3. Decode.
					uint8_t opcode, rd, rs1, rs2, funct3, funct7;
					cpu->decode(inst, opcode, rd, rs1, rs2, funct3, funct7);

#if 0
					if (cpu->record)
					{
						static std::ofstream log("logcpp.txt");
						printRegisters(cpu, log);
						printCsrs(cpu, log);
						log << "0x" << std::hex << std::setw(8) << std::setfill('0') << cpu->getPC()
							<< ": opcode:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)inst
							<< " rd:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)rd
							<< " rs1:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)rs1
							<< " rs2:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)rs2
							<< " f3:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)funct3
							<< " f7:0x" << std::hex << std::setw(8) << std::setfill('0') << (uint64_t)funct7 << std::endl;
					}
					//- To GUI
//					updateState(cpu, inst, opcode, rd, rs1, rs2, funct3, funct7);
#endif

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
		delete virtio;
	}
}