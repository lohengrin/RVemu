#pragma once

#include <QThread>

class Cpu;
class Uart;
class Plic;
class Clint;
class Memory;
class Bus;

struct CpuState {
	std::vector<uint64_t> regs;
	std::vector<uint64_t> csrs;
	std::vector<std::pair<uint64_t, uint64_t>> stack;
	struct {
		uint64_t	pc;
		uint32_t	inst;
		uint8_t		opcode;
		uint8_t		rd, rs1, rs2;
		uint8_t		funct3, funct7;
	} nextstep;
};

class ComputerThread : public QThread
{
	Q_OBJECT;
public:
	enum class Mode {
		STARTED, // Initial state after reset
		RUNNING, // Program is running
		PAUSED,	 // Program is pause
		STOPED,  // Program is stoped at end
		RUNSTEP  // Program is running one STEP
	};

	ComputerThread(const QString& programFile, QObject* parent = nullptr);
	~ComputerThread();

	void startProgram() { myCurrentMode = Mode::RUNNING; }
	void stepProgram() { myCurrentMode = Mode::RUNSTEP; }
	void pauseProgram() { myCurrentMode = Mode::PAUSED; }
	void resetProgram() { resetFlag = true; abortFlag = true;  }
	void abort() { resetFlag = false; abortFlag = true; }

public slots:
	void keypressed(char c);

signals:
	void stepFinished(CpuState state);
	void outputChar(char c);
	void paused();
	void stoped();

protected:
	void updateState(const Cpu* cpu, uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7);
	void run() override;

	QString myProgramFile;
	bool abortFlag = false;
	bool resetFlag = true;
	Mode myCurrentMode = Mode::STARTED;
	CpuState myState;

	//! The computer
	Memory* mem;
	Plic* plic;
	Clint* clint;
	Uart* uart;
	Bus* bus;
	Cpu* cpu;
};