#include "Cpu.h"
#include "Defines.h"
#include "Trap.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

//---------------------------------------------------------
Cpu::Cpu(Bus& b) :
	regs(32,0),
	csrs(4096,0),
	mode(Mode::Machine),
	pc(DRAM_BASE),
	bus(b)
{
	regs[REGSP] = DRAM_BASE + DEFAULT_MEMORYSIZE;
}

//---------------------------------------------------------
Cpu::~Cpu()
{
}

//---------------------------------------------------------
uint64_t Cpu::readMem(uint64_t addr, uint8_t size) const
{
	try {
		return bus.load(addr, size);
	}
	catch (...)
	{
		return 0;
	}
}


//---------------------------------------------------------
uint64_t Cpu::load(uint64_t addr, uint8_t size)
{
	try {
		return bus.load(addr, size);
	}
	catch (const CpuException& e)
	{
		e.take_trap(this);
		if (e.is_fatal())
			throw CpuFatal(e.what());
		return 0;
	}
}

//---------------------------------------------------------
void Cpu::store(uint64_t addr, uint8_t size, uint64_t value)
{
	try {
		bus.store(addr, size, value);
	}
	catch (const CpuException& e)
	{
		e.take_trap(this);
		if (e.is_fatal())
			throw CpuFatal(e.what());
	}
}

//---------------------------------------------------------
uint64_t Cpu::load_csr(uint64_t addr) const
{
	if (addr == SIE)
		return csrs[MIE] & csrs[MIDELEG];
	else
		return csrs[addr];
}

//---------------------------------------------------------
void Cpu::store_csr(uint64_t addr, uint64_t value)
{
	if (addr == SIE)
		csrs[MIE] = (csrs[MIE] & !csrs[MIDELEG]) | (value & csrs[MIDELEG]);
	else
		csrs[addr] = value;
}


//---------------------------------------------------------
uint32_t Cpu::fetch() const
{
	return ASU32(bus.load(pc, 32));
}

//---------------------------------------------------------
void Cpu::decode(uint32_t inst, uint8_t& opcode, uint8_t& rd, uint8_t& rs1, uint8_t& rs2, uint8_t& funct3, uint8_t& funct7) const
{
	opcode = inst & 0x0000007f;
	rd = (inst & 0x00000f80) >> 7;
	rs1 = (inst & 0x000f8000) >> 15;
	rs2 = (inst & 0x01f00000) >> 20;
	funct3 = (inst & 0x00007000) >> 12;
	funct7 = (inst & 0xfe000000) >> 25;
}


//---------------------------------------------------------
void Cpu::execute(uint32_t inst, uint8_t opcode, uint8_t rd, uint8_t rs1, uint8_t rs2, uint8_t funct3, uint8_t funct7)
{
	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;

	try {

		switch (opcode) {
		case 0x03: //..................................................................
		{
			// imm[11:0] = inst[31:20]
			int64_t imm = ASI64(ASI32(inst & 0xfff00000) >> 20);
			uint64_t addr = warppingAdd(regs[rs1], imm);
			switch (funct3) {
			case 0x0: regs[rd] = ASU64(ASI64(ASI8(load(addr, 8)))); break;		// lb
			case 0x1: regs[rd] = ASU64(ASI64(ASI16(load(addr, 16)))); break;	// lh
			case 0x2: regs[rd] = ASU64(ASI64(ASI32(load(addr, 32)))); break;	// lw
			case 0x3: regs[rd] = load(addr, 64); break;							// ld
			case 0x4: regs[rd] = load(addr, 8); break;							// lbu
			case 0x5: regs[rd] = load(addr, 16); break;							// lhu
			case 0x6: regs[rd] = load(addr, 32); break;							// lwu
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x0f:
		{
			// A fence instruction does nothing because this emulator executes an
			// instruction sequentially on a single thread.
			if (funct3 == 0x0)
			{
				// Fence: Do nothing
			}
			else
				executeError(opcode, funct3, funct7);
		}

		case 0x13: //..................................................................
		{
			// imm[11:0] = inst[31:20]
			int64_t imm = ASI64(ASI32(inst & 0xfff00000) >> 20);
			// "The shift amount is encoded in the lower 6 bits of the I-immediate field for RV64I."
			uint32_t shamt = (imm & 0x3f);
			switch (funct3) {
			case 0x0: regs[rd] = warppingAdd(regs[rs1], imm); break;			// addi
			case 0x1: regs[rd] = regs[rs1] << shamt; break;						// slli
			case 0x2: regs[rd] = (ASI64(regs[rs1]) < (ASI64(imm))) ? 1 : 0;	break; // slti
			case 0x3: regs[rd] = (regs[rs1] < ASU64(imm)) ? 1 : 0; break;		// sltiu
			case 0x4: regs[rd] = regs[rs1] ^ ASU64(imm); break;					// xori
			case 0x5: // lhu
				switch (funct7) {
				case 0x00: regs[rd] = warppingShr(regs[rs1], shamt); break;		// srli
				case 0x10: regs[rd] = ASU64(warppingShr(ASI64(regs[rs1]), shamt)); break;// srli
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x6: regs[rd] = regs[rs1] | imm; break;						// ori
			case 0x7: regs[rd] = regs[rs1] & imm; break;						// andi
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x17: // auipc //..................................................................
		{
			int64_t imm = ASI64(ASI32(inst & 0xfff00000) >> 20);
			regs[rd] = warppingSub(warppingAdd(pc, imm), 4);
		}
		break;
		case 0x1b: //..................................................................
		{
			// imm[11:0] = inst[31:20]
			int64_t imm = ASI64(ASI32(inst & 0xfff00000) >> 20);
			// "SLLIW, SRLIW, and SRAIW encodings with imm[5] ̸= 0 are reserved."
			// "The shift amount is encoded in the lower 6 bits of the I-immediate field for RV64I."
			uint32_t shamt = (imm & 0x1f);
			switch (funct3) {
			case 0x0: regs[rd] = ASU64(ASI64(ASI32(warppingAdd(regs[rs1], imm)))); break;	// addiw
			case 0x1: regs[rd] = ASU64(ASI64(ASI32(warppingShl(regs[rs1], shamt)))); break;	// slliw
			case 0x5:
				switch (funct7) {
				case 0x00: regs[rd] = ASI32(warppingShl(ASU32(regs[rs1]), shamt)); break;	// srliw
				case 0x20: regs[rd] = ASU64(ASI64(warppingShr(ASI32(regs[rs1]), shamt))); break; // sraiw
				default: executeError(opcode, funct3, funct7);
				} break;
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x23: // store //..................................................................
		{
			int64_t imm = (ASI64(ASI32(inst & 0xfe000000)) >> 20) | ((inst >> 7) & 0x1f);
			auto addr = warppingAdd(regs[rs1], imm);
			switch (funct3) {
			case 0x0: store(addr, 8, regs[rs2]); break;		// sb
			case 0x1: store(addr, 16, regs[rs2]); break;	// sh
			case 0x2: store(addr, 32, regs[rs2]); break;	// sw
			case 0x3: store(addr, 64, regs[rs2]); break;	// sd
			default: executeError(opcode, funct3, funct7);
			};
		} break;
		case 0x2f: //..................................................................
		{
			// RV64A: "A" standard extension for atomic instructions
			uint8_t funct5 = (funct7 & 0b1111100) >> 2;
			uint8_t _aq = (funct7 & 0b0000010) >> 1; // acquire access
			uint8_t _rl = funct7 & 0b0000001; // release access
			if (funct3 == 0x02 && funct5 == 0x00) {
				// amoadd.w
				uint64_t t = load(regs[rs1], 32);
				store(regs[rs1], 32, warppingAdd(t, regs[rs2]));
				regs[rd] = t;
			}
			else if (funct3 == 0x3 && funct5 == 0x00) {
				// amoadd.d
				uint64_t t = load(regs[rs1], 64);
				store(regs[rs1], 64, warppingAdd(t, regs[rs2]));
				regs[rd] = t;
			}
			else if (funct3 == 0x2 && funct5 == 0x01) {
				// amoswap.w
				uint64_t t = load(regs[rs1], 32);
				store(regs[rs1], 32, regs[rs2]);
				regs[rd] = t;
			}
			else if (funct3 == 0x3 && funct5 == 0x01) {
				// amoswap.d
				uint64_t t = load(regs[rs1], 64);
				store(regs[rs1], 64, regs[rs2]);
				regs[rd] = t;
			}
			else
			{
				executeError(opcode, funct3, funct7);
			}
		} break;
		case 0x33: //..................................................................
		{
			// "SLL, SRL, and SRA perform logical left, logical right, and arithmetic right
			// shifts on the value in register rs1 by the shift amount held in register rs2.
			// In RV64I, only the low 6 bits of rs2 are considered for the shift amount."
			uint32_t shamt = regs[rs2] & 0x3f;
			switch (funct3) {
			case 0x0:
				switch (funct7) {
				case 0x00: regs[rd] = warppingAdd(regs[rs1], regs[rs2]); break;	// add
				case 0x01: regs[rd] = warppingMul(regs[rs1], regs[rs2]); break;	// mul
				case 0x20: regs[rd] = warppingSub(regs[rs1], regs[rs2]); break;	// sub
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x1:
				switch (funct7) {
				case 0x00: regs[rd] = warppingShl(regs[rs1], shamt); break;		// sll
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x2:
				switch (funct7) {
				case 0x00: regs[rd] = (ASI64(regs[rs1]) < ASI64(regs[rs2])) ? 1 : 0; break;	// slt
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x3:
				switch (funct7) {
				case 0x00: regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0; break;	// sltu
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x4:
				switch (funct7) {
				case 0x00: regs[rd] = regs[rs1] ^ regs[rs2]; break;				// xor
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x5:
				switch (funct7) {
				case 0x00: regs[rd] = warppingShr(regs[rs1], shamt); break;		// srl
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x6:
				switch (funct7) {
				case 0x00: regs[rd] = regs[rs1] | regs[rs2]; break;				// or
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x7:
				switch (funct7) {
				case 0x00: regs[rd] = regs[rs1] & regs[rs2]; break;				// and
				default: executeError(opcode, funct3, funct7);
				}
				break;
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x37: //..................................................................
			regs[rd] = ASU64(ASI64(ASI32(inst & 0xfffff000))); break;			// lui
		case 0x3b: //..................................................................
		{
			// "The shift amount is given by rs2[4:0]."
			uint32_t shamt = regs[rs2] & 0x1f;
			switch (funct3) {
			case 0x0:
				switch (funct7) {
				case 0x00: regs[rd] = ASU64(ASI64(ASI32(warppingAdd(regs[rs1], regs[rs2])))); break;	// addw
				case 0x20: regs[rd] = ASU64(ASI32(warppingSub(regs[rs1], regs[rs2]))); break;			// subw
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x1:
				switch (funct7) {
				case 0x00: regs[rd] = ASU64(ASI32(warppingShl(ASU32(regs[rs1]), shamt))); break;		// sllw
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x5:
				switch (funct7) {
				case 0x00: regs[rd] = ASU64(ASI32(warppingShr(ASU32(regs[rs1]), shamt))); break;		// srlw
				case 0x01: regs[rd] = (regs[rs2] == 0) ? 0xffffffffffffffff : warppingDiv(regs[rs1], regs[rs2]); break;		// divu      // TODO: Set DZ (Divide by Zero) in the FCSR csr flag to 1.
				case 0x20: regs[rd] = ASU64(ASI32(regs[rs1]) >> ASI32(shamt)); break;				// sraw
				default: executeError(opcode, funct3, funct7);
				} break;
			case 0x7:
				switch (funct7) {
				case 0x01: regs[rd] = (regs[rs2] == 0) ? regs[rs1] : warppingRem(ASU32(regs[rs1]), ASU32(regs[rs2])); break;		// remuw
				default: executeError(opcode, funct3, funct7);
				} break;
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x63: //..................................................................
		{
			// imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
			uint64_t imm = ASU64(ASI64(ASI32(inst & 0x80000000)) >> 19)
				| ((inst & 0x00000080) << 4) // imm[11]
				| ((inst >> 20) & 0x7e0) // imm[10:5]
				| ((inst >> 7) & 0x1e); // imm[4:1]

			switch (funct3) {
			case 0x0: // beq
				if (regs[rs1] == regs[rs2])
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			case 0x1: // bne
				if (regs[rs1] != regs[rs2])
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			case 0x4: // blt
				if (ASI64(regs[rs1]) < ASI64(regs[rs2]))
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			case 0x5: // bge
				if (ASI64(regs[rs1]) >= ASI64(regs[rs2]))
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			case 0x6: // bltu
				if (regs[rs1] < regs[rs2])
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			case 0x7: // bgeu
				if (regs[rs1] >= regs[rs2])
					pc = warppingSub(warppingAdd(pc, imm), 4);
				break;
			default: executeError(opcode, funct3, funct7);
			};
		}
		break;
		case 0x67: // jalr  //..................................................................
		{
			// Note: Don't add 4 because the pc already moved on.
			auto t = pc;
			int64_t imm = ASI64(ASI32(inst & 0xfff00000)) >> 20;
			pc = warppingAdd(regs[rs1], imm) & ~1;
			regs[rd] = t;
		}
		break;
		case 0x6f: // jal //..................................................................
		{
			regs[rd] = pc;

			// imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
			int64_t imm = ASI64(ASI32(inst & 0x80000000) >> 11) // imm[20]
				| (inst & 0xff000) // imm[19:12]
				| ((inst >> 9) & 0x800) // imm[11]
				| ((inst >> 20) & 0x7fe); // imm[10:1]

			pc = warppingSub(warppingAdd(pc, imm), 4);
		}
		break;
		case 0x73:	//CSRS  //..................................................................
		{
			uint64_t csr_addr = ASU64((inst & 0xfff00000) >> 20);
			switch (funct3) {
			case 0x0: {
				if (rs2 == 0x0 && funct7 == 0x0) {
					// ecall
					// Makes a request of the execution environment by raising an
					// environment call exception.
					switch (mode) {
					case Mode::User:
						throw CpuException(Except::EnvironmentCallFromUMode);
					case Mode::Supervisor:
						throw CpuException(Except::EnvironmentCallFromSMode);
					case Mode::Machine:
						throw CpuException(Except::EnvironmentCallFromMMode);
					};

				}
				else if (rs2 == 0x1 && funct7 == 0x0) {
					// ebreak
					// Makes a request of the debugger bu raising a Breakpoint
					// exception.
					throw CpuException(Except::Breakpoint);
				}
				else if (rs2 == 0x2 && funct7 == 0x8) {
					// sret
					// The SRET instruction returns from a supervisor-mode exception
					// handler. It does the following operations:
					// - Sets the pc to CSRs[sepc].
					// - Sets the privilege mode to CSRs[sstatus].SPP.
					// - Sets CSRs[sstatus].SIE to CSRs[sstatus].SPIE.
					// - Sets CSRs[sstatus].SPIE to 1.
					// - Sets CSRs[sstatus].SPP to 0.
					pc = load_csr(SEPC);
					// When the SRET instruction is executed to return from the trap
					// handler, the privilege level is set to user mode if the SPP
					// bit is 0, or supervisor mode if the SPP bit is 1. The SPP bit
					// is the 8th of the SSTATUS csr.
					mode = (((load_csr(SSTATUS) >> 8) & 1) == 1) ? Mode::Supervisor : Mode::User;
					// The SPIE bit is the 5th and the SIE bit is the 1st of the
					// SSTATUS csr.
					if (((load_csr(SSTATUS) >> 5) & 1) == 1)
						store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 1));
					else
						store_csr(SSTATUS, load_csr(SSTATUS) & !(1 << 1));
					store_csr(SSTATUS, load_csr(SSTATUS) | (1 << 5));
					store_csr(SSTATUS, load_csr(SSTATUS) & !(1 << 8));
				}
				else if (rs2 == 0x2 && funct7 == 0x18)
				{
					// mret
					// The MRET instruction returns from a machine-mode exception
					// handler. It does the following operations:
					// - Sets the pc to CSRs[mepc].
					// - Sets the privilege mode to CSRs[mstatus].MPP.
					// - Sets CSRs[mstatus].MIE to CSRs[mstatus].MPIE.
					// - Sets CSRs[mstatus].MPIE to 1.
					// - Sets CSRs[mstatus].MPP to 0.
					pc = load_csr(MEPC);
					// MPP is two bits wide at [11..12] of the MSTATUS csr.
					switch ((load_csr(MSTATUS) >> 11) & 0b11) {
					case 2: mode = Mode::Machine; break;
					case 1: mode = Mode::Supervisor; break;
					default: mode = Mode::User; break;
					};
					// The MPIE bit is the 7th and the MIE bit is the 3rd of the
					// MSTATUS csr.

					if (((load_csr(MSTATUS) >> 7) & 1) == 1)
						store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 3));
					else
						store_csr(MSTATUS, load_csr(MSTATUS) & !(1 << 3));

					store_csr(MSTATUS, load_csr(MSTATUS) | (1 << 7));
					store_csr(MSTATUS, load_csr(MSTATUS) & !(0b11 << 11));
				}
				else if (funct7 == 0x9)
				{
					// sfence.vma
							// Do nothing.
				}
				else
				{
					executeError(opcode, funct3, funct7);
				}
			} break;
			case 0x1: // csrrw
			{
				auto t = load_csr(csr_addr);
				store_csr(csr_addr, regs[rs1]);
				regs[rd] = t;
			} break;
			case 0x2: // csrrs
			{
				auto t = load_csr(csr_addr);
				store_csr(csr_addr, t | regs[rs1]);
				regs[rd] = t;
			} break;
			case 0x3: // csrrc
			{
				auto t = load_csr(csr_addr);
				store_csr(csr_addr, t & (!regs[rs1]));
				regs[rd] = t;
			} break;
			case 0x5: // csrrwi
			{
				auto zimm = ASU64(rs1);
				regs[rd] = load_csr(csr_addr);
				store_csr(csr_addr, zimm);
			} break;
			case 0x6: // csrrsi
			{
				auto zimm = ASU64(rs1);
				auto t = load_csr(csr_addr);
				store_csr(csr_addr, t | zimm);
				regs[rd] = t;
			} break;
			case 0x7: // csrrci
			{
				auto zimm = ASU64(rs1);
				auto t = load_csr(csr_addr);
				store_csr(csr_addr, t & (!zimm));
				regs[rd] = t;
			} break;
			default: executeError(opcode, funct3, funct7);
			}
		} break;
		default: executeError(opcode, funct3, funct7);
		}
	}
	catch (const CpuException& e)
	{
		e.take_trap(this);
		if (e.is_fatal())
			throw CpuFatal(e.what());
	}

	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;
}

//---------------------------------------------------------
void Cpu::executeError(uint8_t opcode, uint8_t funct3, uint8_t funct7) const
{
	std::cerr << "Cpu::execute not yet implemented: opcode 0x" << std::hex << (int)opcode << " funct3 0x" << std::hex << (int)funct3 << " funct7 0x" << std::hex << (int)funct7 << std::endl;
	throw CpuException(Except::IllegalInstruction);
}

