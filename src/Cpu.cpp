﻿#include "Cpu.h"

#include <vector>
#include <string>
#include <iostream>
#include <iomanip>


#define ASI8(V) static_cast<int8_t>(V)
#define ASI16(V) static_cast<int16_t>(V)
#define ASI32(V) static_cast<int32_t>(V)
#define ASI64(V) static_cast<int64_t>(V)
#define ASU8(V) static_cast<uint8_t>(V)
#define ASU16(V) static_cast<uint16_t>(V)
#define ASU32(V) static_cast<uint32_t>(V)
#define ASU64(V) static_cast<uint64_t>(V)

//---------------------------------------------------------
Cpu::Cpu(Bus& b) :
	regs{ 0 },
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
uint64_t Cpu::load(uint64_t addr, uint8_t size)
{
	return bus.load(addr, size);
}

//---------------------------------------------------------
void Cpu::store(uint64_t addr, uint8_t size, uint64_t value)
{
	bus.store(addr, size, value);
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
	auto funct3 = (inst >> 12) & 0x7;
	auto funct7 = (inst >> 25) & 0x7f;

	// Emulate that register x0 is hardwired with all bits equal to 0.
	regs[0] = 0;

	switch (opcode) {
	case 0x03:
	{
		// imm[11:0] = inst[31:20]
		uint64_t imm = ASU64((ASI64(ASI32(inst)) & 0xfff00000) >> 20);
		uint64_t addr = warppingAdd(regs[rs1], imm);
		switch (funct3) {
		case 0x0: // lb
			regs[rd] = ASU64(ASI64(ASI8(load(addr, 8))));
			break;
		case 0x1: // lh
			regs[rd] = ASU64(ASI64(ASI16(load(addr, 16))));
			break;
		case 0x2: // lw
			regs[rd] = ASU64(ASI64(ASI32(load(addr, 32))));
			break;
		case 0x3: // ld
			regs[rd] = load(addr, 64);
			break;
		case 0x4: // lbu
			regs[rd] = load(addr, 8);
			break;
		case 0x5: // lhu
			regs[rd] = load(addr, 16);
			break;
		case 0x6: // lwu
			regs[rd] = load(addr, 32);
			break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x13: // addi
	{
		// imm[11:0] = inst[31:20]
		uint64_t imm = ASU64((ASI64(ASI32(inst)) & 0xfff00000) >> 20);
		// "The shift amount is encoded in the lower 6 bits of the I-immediate field for RV64I."
		uint32_t shamt = (imm & 0x3f);
		switch (funct3) {
		case 0x0: // addi
			regs[rd] = warppingAdd(regs[rs1], imm);
			break;
		case 0x1: // slli
			regs[rd] = regs[rs1] << shamt;
			break;
		case 0x2: // slti
			regs[rd] = (ASI64(regs[rs1]) < (ASI64(imm))) ? 1 : 0;
			break;
		case 0x3: // sltiu
			regs[rd] = (regs[rs1] < imm) ? 1 : 0;
			break;
		case 0x4: // xori
			regs[rd] = regs[rs1] ^ imm;
			break;
		case 0x5: // lhu
			switch (funct7) {
			case 0x00: // srli
				regs[rd] = warppingShr(regs[rs1], shamt);
				break;
			case 0x10: // srli
				regs[rd] = ASU64(warppingShr(ASI64(regs[rs1]), shamt));
				break;
			}
			break;
		case 0x6: // ori
			regs[rd] = regs[rs1] | imm;
			break;
		case 0x7: // andi
			regs[rd] = regs[rs1] & imm;
			break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x17: // auipc
	{
		uint64_t imm = ASU64((ASI64(ASI32(inst)) & 0xfff00000) >> 20);
		pc = warppingSub(warppingAdd(pc, imm), 4);
		regs[rd] = pc;
	}
	break;
	case 0x1b:
	{
		// imm[11:0] = inst[31:20]
		uint64_t imm = ASU64((ASI64(ASI32(inst)) & 0xfff00000) >> 20);
		// "SLLIW, SRLIW, and SRAIW encodings with imm[5] ̸= 0 are reserved."
		// "The shift amount is encoded in the lower 6 bits of the I-immediate field for RV64I."
		uint32_t shamt = (imm & 0x1f);
		switch (funct3) {
		case 0x0: // addiw
			regs[rd] = ASU64(ASI64(ASI32(imm)));
			break;
		case 0x1: // slliw
			regs[rd] = ASU64(ASI64(ASI32(warppingShl(regs[rs1], shamt))));
			break;
		case 0x5:
			switch (funct7) {
			case 0x00: // srliw
				regs[rd] = ASI32(warppingShl(ASU32(regs[rs1]), shamt));
				break;
			case 0x20: // sraiw
				regs[rd] = ASU64(ASI64(warppingShr(ASI32(regs[rs1]), shamt)));
				break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x23: // store
	{
		auto imm = ASU64(ASI64(ASI32(inst & 0xfe000000)) >> 20) | ((inst >> 7) & 0x1f);
		auto addr = warppingAdd(regs[rs1], imm);
		switch (funct3) {
		case 0x0: // sb
			store(addr, 8, regs[rs2]); break;
		case 0x1: // sh
			store(addr, 16, regs[rs2]);	break;
		case 0x2: // sw
			store(addr, 32, regs[rs2]);	break;
		case 0x3: // sd
			store(addr, 64, regs[rs2]);	break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x33:
	{
		// "SLL, SRL, and SRA perform logical left, logical right, and arithmetic right
		// shifts on the value in register rs1 by the shift amount held in register rs2.
		// In RV64I, only the low 6 bits of rs2 are considered for the shift amount."
		uint32_t shamt = regs[rs2] & 0x3f;
		switch (funct3) {
		case 0x0:
			switch (funct7) {
			case 0x00: // add
				regs[rd] = warppingAdd(regs[rs1], regs[rs2]); break;
			case 0x01: // mul
				regs[rd] = warppingMul(regs[rs1], regs[rs2]); break;
			case 0x20: // sub
				regs[rd] = warppingSub(regs[rs1], regs[rs2]); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x1:
			switch (funct7) {
			case 0x00: // sll
				regs[rd] = warppingShl(regs[rs1], shamt); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x2:
			switch (funct7) {
			case 0x00: // slt
				regs[rd] = (ASI64(regs[rs1]) < ASI64(regs[rs2])) ? 1 : 0; break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x3:
			switch (funct7) {
			case 0x00: // sltu
				regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0; break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x4:
			switch (funct7) {
			case 0x00: // xor
				regs[rd] = regs[rs1] ^ regs[rs2]; break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x5:
			switch (funct7) {
			case 0x00: // srl
				regs[rd] = warppingShr(regs[rs1], shamt); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x6:
			switch (funct7) {
			case 0x00: // or
				regs[rd] = regs[rs1] | regs[rs2]; break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x7:
			switch (funct7) {
			case 0x00: // and
				regs[rd] = regs[rs1] & regs[rs2]; break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x37: // lui
	{
		regs[rd] = ASU64(ASI64(ASI32(inst & 0xfffff000)));
	}
	break;
	case 0x3b:
	{
		// "The shift amount is given by rs2[4:0]."
		uint32_t shamt = regs[rs2] & 0x1f;
		switch (funct3) {
		case 0x0:
			switch (funct7) {
			case 0x00: // addw
				regs[rd] = ASU64(ASI64(ASI32(warppingAdd(regs[rs1], regs[rs2])))); break;
			case 0x20: // subw
				regs[rd] = ASU64(ASI32(warppingSub(regs[rs1], regs[rs2]))); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x1:
			switch (funct7) { 
			case 0x00: // sllw
				regs[rd] = ASU64(ASI32(warppingShl(ASU32(regs[rs1]), shamt))); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		case 0x5:
			switch (funct7) {
			case 0x00: // srlw
				regs[rd] = ASU64(ASI32(warppingShr(ASU32(regs[rs1]), shamt))); break;
			case 0x20: // sraw
				regs[rd] = ASU64( ASI32(regs[rs1]) >>  ASI32(shamt) ); break;
			default:
				std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct7 0x" << std::hex << funct7 << std::endl;
			}
			break;
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x63:
	{
		// imm[12|10:5|4:1|11] = inst[31|30:25|11:8|7]
		uint64_t imm = ASU64(ASI64(ASI32(inst & 0x80000000)) >> 19)
			| ((inst & 0x80) << 4) // imm[11]
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
		default:
			std::cerr << "Cpu: not implemented yet: opcode 0x" << std::hex << opcode << " funct3 0x" << std::hex << funct3 << std::endl;
		};
	}
	break;
	case 0x67: // jalr
	{
		// Note: Don't add 4 because the pc already moved on.
		auto t = pc;

		auto imm = ASU64(ASI64(ASI32(inst & 0xfff00000)) >> 20);
		pc = warppingAdd(regs[rs1], imm) & !1;
		regs[rd] = t;
	}
	break;
	case 0x6f: // jal
	{
		regs[rd] = pc;

		// imm[20|10:1|11|19:12] = inst[31|30:21|20|19:12]
		auto imm = ASU64(ASI64(ASI32(inst & 0x80000000)) >> 11) // imm[20]
			| (inst & 0xff000) // imm[19:12]
			| ((inst >> 9) & 0x800) // imm[11]
			| ((inst >> 20) & 0x7fe); // imm[10:1]

		pc = warppingSub(warppingAdd(pc,imm),4);
	}
	break;

	default:
		std::cerr << "Opcode not implemented: 0x" << std::hex << opcode << std::endl;
	}
}

//---------------------------------------------------------
void Cpu::printRegisters()
{
	static std::vector<std::string> header =
	{ "zero", " ra ", " sp ", " gp ", " tp ", " t0 ", " t1 ", " t2 ", " s0 ", " s1 ", " a0 ",
		" a1 ", " a2 ", " a3 ", " a4 ", " a5 ", " a6 ", " a7 ", " s2 ", " s3 ", " s4 ", " s5 ",
		" s6 ", " s7 ", " s8 ", " s9 ", " s10", " s11", " t3 ", " t4 ", " t5 ", " t6 " };

	for (int i = 0; i < 32; i++)
	{
		std::cout << std::setfill('0') << std::setw(2) << std::dec << i << ": " << header[i] << ": " << std::hex << "0x" << std::setw(8) << regs[i] << "\t";
		if ((i + 1) % 5 == 0)
			std::cout << std::endl;
	}
}
