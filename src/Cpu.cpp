#include "Cpu.h"
#include "Defines.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>

//---------------------------------------------------------
Cpu::Cpu(Bus& b) :
	regs{ 0 },
	csrs{ 0 },
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
uint64_t Cpu::load(uint64_t addr, uint8_t size) const
{
	return bus.load(addr, size);
}

//---------------------------------------------------------
void Cpu::store(uint64_t addr, uint8_t size, uint64_t value)
{
	bus.store(addr, size, value);
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
void Cpu::execute(uint32_t inst)
{
	uint8_t opcode = inst & 0x7f;
	uint8_t rd = (inst >> 7) & 0x1f;
	uint8_t rs1 = (inst >> 15) & 0x1f;
	uint8_t rs2 = (inst >> 20) & 0x1f;
	uint8_t funct3 = (inst >> 12) & 0x7;
	uint8_t funct7 = (inst >> 25) & 0x7f;

	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;

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
		default: printExecuteError(opcode, funct3, funct7);
		};
	}
	break;
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
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x6: regs[rd] = regs[rs1] | imm; break;						// ori
		case 0x7: regs[rd] = regs[rs1] & imm; break;						// andi
		default: printExecuteError(opcode, funct3, funct7);
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
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		default: printExecuteError(opcode, funct3, funct7);
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
		default: printExecuteError(opcode, funct3, funct7);
		};
	}
	break;
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
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x1:
			switch (funct7) {
			case 0x00: regs[rd] = warppingShl(regs[rs1], shamt); break;		// sll
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x2:
			switch (funct7) {
			case 0x00: regs[rd] = (ASI64(regs[rs1]) < ASI64(regs[rs2])) ? 1 : 0; break;	// slt
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x3:
			switch (funct7) {
			case 0x00: regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0; break;	// sltu
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x4:
			switch (funct7) {
			case 0x00: regs[rd] = regs[rs1] ^ regs[rs2]; break;				// xor
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x5:
			switch (funct7) {
			case 0x00: regs[rd] = warppingShr(regs[rs1], shamt); break;		// srl
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x6:
			switch (funct7) {
			case 0x00: regs[rd] = regs[rs1] | regs[rs2]; break;				// or
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x7:
			switch (funct7) {
			case 0x00: regs[rd] = regs[rs1] & regs[rs2]; break;				// and
			default: printExecuteError(opcode, funct3, funct7);
			}
			break;
		default: printExecuteError(opcode, funct3, funct7);
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
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x1:
			switch (funct7) { 
			case 0x00: regs[rd] = ASU64(ASI32(warppingShl(ASU32(regs[rs1]), shamt))); break;		// sllw
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		case 0x5:
			switch (funct7) {
			case 0x00: regs[rd] = ASU64(ASI32(warppingShr(ASU32(regs[rs1]), shamt))); break;		// srlw
			case 0x20: regs[rd] = ASU64( ASI32(regs[rs1]) >>  ASI32(shamt) ); break;				// sraw
			default: printExecuteError(opcode, funct3, funct7);
			} break;
		default: printExecuteError(opcode, funct3, funct7);
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
		default: printExecuteError(opcode, funct3, funct7);
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

		pc = warppingSub(warppingAdd(pc,imm),4);
	}
	break;
	default: printExecuteError(opcode, funct3, funct7);
	}

	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;
}

//---------------------------------------------------------
void Cpu::printExecuteError(uint8_t opcode, uint8_t funct3, uint8_t funct7) const
{
	std::cerr << "Cpu::execute not yet implemented: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << " funct7 0x" << std::hex << funct7 << std::endl;
}

//---------------------------------------------------------
void Cpu::printRegisters()
{
	for (int i = 0; i < 32; i++)
	{
		std::cout << std::setfill('0') << std::setw(2) << std::dec << i << ": " << RegisterNames[i] << ": " << std::hex << "0x" << std::setw(16) << regs[i] << "\t";
		if ((i + 1) % 5 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

//---------------------------------------------------------
void Cpu::printInstruction(uint32_t inst) const
{
	uint8_t opcode = inst & 0x7f;
	uint8_t rd = (inst >> 7) & 0x1f;
	uint8_t rs1 = (inst >> 15) & 0x1f;
	uint8_t rs2 = (inst >> 20) & 0x1f;
	auto funct3 = (inst >> 12) & 0x7;
	auto funct7 = (inst >> 25) & 0x7f;

	for (const auto& i : InstructionSet)
	{
		if (opcode == i.opcode)
		{
			if (i.funct3 == (uint8_t)-1 || i.funct3 == funct3)
			{
				if (i.funct7 == (uint8_t)-1 || i.funct7 == funct7)
				{
					std::cout << i.name << 
						" rd=0x" << std::hex << ASU32(rd) << " (" << RegisterNames[rd] << ") " <<
						" rs1=0x" << std::hex << ASU32(rs1) << " (" << RegisterNames[rs1] << ") " <<
						" rs2=0x" << std::hex << ASU32(rs2) << " (" << RegisterNames[rs2] << ") " << std::endl;
				}
			}
		}
	}
}

//---------------------------------------------------------
void Cpu::printStack() const
{
	std::cout << "==== Stack ====" << std::endl;
	for (uint64_t sp = regs[REGSP]; sp < DRAM_BASE + DEFAULT_MEMORYSIZE; sp += 8)
	{
		std::cout << std::hex << "0x" << std::setfill('0') << std::setw(16) << sp
			<< std::hex << "   0x" << std::setfill('0') << std::setw(16) << load(sp, 64) << std::endl;
	}
	std::cout << "===============" << std::endl;
}
