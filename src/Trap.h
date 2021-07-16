#pragma once

#include "Cpu.h"

#include <stdexcept>
#include <inttypes.h>

enum class Except : uint32_t {
	InstructionAddressMisaligned = 0,
	InstructionAccessFault = 1,
	IllegalInstruction = 2,
	Breakpoint = 3,
	LoadAddressMisaligned = 4,
	LoadAccessFault = 5,
	StoreAMOAddressMisaligned = 6,
	StoreAMOAccessFault = 7,
	EnvironmentCallFromUMode = 8,
	EnvironmentCallFromSMode = 9,
	EnvironmentCallFromMMode = 11,
	InstructionPageFault = 12,
	LoadPageFault = 13,
	StoreAMOPageFault = 15
};

struct Trap {
	/// Helper method for a trap handler.
	static void take_trap(Except e, Cpu* cpu);
};

struct CpuFatal : public std::runtime_error
{
	CpuFatal(const std::string& msg) : std::runtime_error(msg) {}
};

struct CpuException : public std::exception
{
	CpuException(Except e) : ex(e) {}
	const char* what() const throw ()
	{
		switch (ex) {
		case Except::InstructionAddressMisaligned: return "InstructionAddressMisaligned";
		case Except::InstructionAccessFault: return "InstructionAccessFault";
		case Except::IllegalInstruction: return "IllegalInstruction";
		case Except::Breakpoint: return "Breakpoint";
		case Except::LoadAddressMisaligned: return "LoadAddressMisaligned";
		case Except::LoadAccessFault: return "LoadAccessFault";
		case Except::StoreAMOAddressMisaligned: return "StoreAMOAddressMisaligned";
		case Except::StoreAMOAccessFault: return "StoreAMOAccessFault";
		case Except::EnvironmentCallFromUMode: return "EnvironmentCallFromUMode";
		case Except::EnvironmentCallFromSMode: return "EnvironmentCallFromSMode";
		case Except::EnvironmentCallFromMMode: return "EnvironmentCallFromMMode";
		case Except::InstructionPageFault: return "InstructionPageFault";
		case Except::LoadPageFault: return "LoadPageFault";
		case Except::StoreAMOPageFault: return "StoreAMOPageFault";
		default: return "Unkown Cpu Exception";
		}
	}

	void take_trap(Cpu* cpu) const { Trap::take_trap(ex, cpu); }

	bool is_fatal() const {
		return (ex == Except::InstructionAddressMisaligned ||
			ex == Except::InstructionAccessFault ||
			ex == Except::LoadAccessFault ||
			ex == Except::StoreAMOAddressMisaligned ||
			ex == Except::StoreAMOAccessFault);
	};


	Except ex;
};
