#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

#define LITTLE_ENDIAN 0x41424344UL 
#define BIG_ENDIAN    0x44434241UL
#define PDP_ENDIAN    0x42414443UL
#define ENDIAN_ORDER  ('ABCD') 

#define ASI8(V) static_cast<int8_t>(V)
#define ASI16(V) static_cast<int16_t>(V)
#define ASI32(V) static_cast<int32_t>(V)
#define ASI64(V) static_cast<int64_t>(V)
#define ASU8(V) static_cast<uint8_t>(V)
#define ASU16(V) static_cast<uint16_t>(V)
#define ASU32(V) static_cast<uint32_t>(V)
#define ASU64(V) static_cast<uint64_t>(V)

# define TARGET_LONG_BITS 64
#define RV32 ((uint64_t)1 << (TARGET_LONG_BITS - 2))
#define RV64 ((uint64_t)2 << (TARGET_LONG_BITS - 2))

#define RV(x) ((uint64_t)1 << (x - 'A'))

#define RVI RV('I')
#define RVE RV('E') /* E and I are mutually exclusive */
#define RVM RV('M')
#define RVA RV('A')
#define RVF RV('F')
#define RVD RV('D')
#define RVV RV('V')
#define RVC RV('C')
#define RVS RV('S')
#define RVU RV('U')
#define RVH RV('H')
#define RVB RV('B')

/// The page size (4 KiB) for the virtual dram system.
const uint64_t PAGE_SIZE = 4096;

// Machine-level CSRs.
/// Machine status register.
const uint64_t MSTATUS = 0x300;
/// ISA and extensions register.
const uint64_t MISA = 0x301;
/// Machine exception delefation register.
const uint64_t MEDELEG = 0x302;
/// Machine interrupt delefation register.
const uint64_t MIDELEG = 0x303;
/// Machine interrupt-enable register.
const uint64_t MIE = 0x304;
/// Machine trap-handler base address.
const uint64_t MTVEC = 0x305;
/// Machine exception program counter.
const uint64_t MEPC = 0x341;
/// Machine trap cause.
const uint64_t MCAUSE = 0x342;
/// Machine bad address or instruction.
const uint64_t MTVAL = 0x343;
/// Machine interrupt pending.
const uint64_t MIP = 0x344;

// MIP fields.
const uint64_t MIP_SSIP = 1 << 1;
const uint64_t MIP_MSIP = 1 << 3;
const uint64_t MIP_STIP = 1 << 5;
const uint64_t MIP_MTIP = 1 << 7;
const uint64_t MIP_SEIP = 1 << 9;
const uint64_t MIP_MEIP = 1 << 11;

// Supervisor-level CSRs.
/// Supervisor status register.
const uint64_t SSTATUS = 0x100;
/// Supervisor interrupt-enable register.
const uint64_t SIE = 0x104;
/// Supervisor trap handler base address.
const uint64_t STVEC = 0x105;
/// Supervisor exception program counter.
const uint64_t SEPC = 0x141;
/// Supervisor trap cause.
const uint64_t SCAUSE = 0x142;
/// Supervisor bad address or instruction.
const uint64_t STVAL = 0x143;
/// Supervisor interrupt pending.
const uint64_t SIP = 0x144;
/// Supervisor address translation and protection.
const uint64_t SATP = 0x180;

/// The address which the core-local interruptor (CLINT) starts. It contains the timer and
/// generates per-hart software interrupts and timer
/// interrupts.
const uint64_t CLINT_BASE = 0x2000000;

/// The address which the platform-level interrupt controller (PLIC) starts. The PLIC connects all external interrupts in the
/// system to all hart contexts in the system, via the external interrupt source in each hart.
const uint64_t PLIC_BASE = 0xc000000;

/// The address which dram starts, same as QEMU virt machine.
const uint64_t DRAM_BASE = 0x80000000;

/// The address which UART starts, same as QEMU virt machine.
const uint64_t UART_BASE = 0x10000000;

/// The address which virtio starts.
const uint64_t VIRTIO_BASE = 0x10001000;

struct Instruction {
	uint8_t opcode;
	uint8_t funct3;
	uint8_t funct7;
	std::string name;
};

extern std::vector<Instruction> InstructionSet;
extern const char* getInstructionName(uint8_t opcode, uint8_t funct3, uint8_t funct7);

extern std::vector<std::string> RegisterNames;
