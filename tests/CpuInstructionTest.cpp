// Unit tests for the RV64I instruction set executed by Cpu.
//
// Each test lays out a small straight-line program at DRAM_BASE, then drives the
// real fetch -> forwardPC -> decode -> execute loop one step at a time and
// asserts on architectural state (registers / PC / CSRs). Instruction words are
// built with the encoder helpers below so the bit packing is self-documenting.
//
// PC convention used by the emulator: forwardPC() is applied BEFORE execute(),
// so inside execute() `pc` already points past the current instruction. For
// PC-relative forms (auipc, branches, jal, jalr) the implementation subtracts 4
// to recover the address of the instruction itself; run() reproduces the
// forwardPC-then-execute order so these tests match production behavior.

#include "Cpu.h"
#include "Memory.h"
#include "Bus.h"
#include "Trap.h"
#include "Defines.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace {

// ---- instruction encoders -------------------------------------------------

uint32_t r(uint8_t opcode, uint8_t rd, uint8_t f3, uint8_t rs1, uint8_t rs2, uint8_t f7 = 0)
{
	return (uint32_t(f7) << 25) | (uint32_t(rs2) << 20) | (uint32_t(rs1) << 15) |
	       (uint32_t(f3) << 12) | (uint32_t(rd) << 7) | opcode;
}

// I-type: imm is the raw 12-bit immediate field (caller supplies 0xFFF for -1).
uint32_t i(uint8_t opcode, uint8_t rd, uint8_t f3, uint8_t rs1, uint16_t imm12)
{
	return (uint32_t(imm12 & 0xFFF) << 20) | (uint32_t(rs1) << 15) |
	       (uint32_t(f3) << 12) | (uint32_t(rd) << 7) | opcode;
}

// U-type: imm20 is the 20-bit field placed at bits 31:12.
uint32_t u(uint8_t opcode, uint8_t rd, uint32_t imm20)
{
	return (uint32_t(imm20 & 0xFFFFF) << 12) | (uint32_t(rd) << 7) | opcode;
}

// S-type store.
uint32_t s(uint8_t f3, uint8_t rs1, uint8_t rs2, int32_t imm)
{
	uint32_t im = uint32_t(imm);
	return (((im >> 5) & 0x7F) << 25) | (uint32_t(rs2) << 20) | (uint32_t(rs1) << 15) |
	       (uint32_t(f3) << 12) | ((im & 0x1F) << 7) | 0x23;
}

// B-type branch. imm is the byte offset (multiple of 2).
uint32_t b(uint8_t f3, uint8_t rs1, uint8_t rs2, int32_t imm)
{
	uint32_t im = uint32_t(imm);
	return (((im >> 12) & 0x1) << 31) | (((im >> 5) & 0x3F) << 25) |
	       (uint32_t(rs2) << 20) | (uint32_t(rs1) << 15) | (uint32_t(f3) << 12) |
	       (((im >> 1) & 0xF) << 8) | (((im >> 11) & 0x1) << 7) | 0x63;
}

// J-type jal. imm is the byte offset (multiple of 2).
uint32_t jal(uint8_t rd, int32_t imm)
{
	uint32_t im = uint32_t(imm);
	return (((im >> 20) & 0x1) << 31) | (((im >> 1) & 0x3FF) << 21) |
	       (((im >> 11) & 0x1) << 20) | (((im >> 12) & 0xFF) << 12) |
	       (uint32_t(rd) << 7) | 0x6F;
}

uint32_t jalr(uint8_t rd, uint8_t rs1, int32_t imm)
{
	return i(0x67, rd, 0, rs1, uint16_t(imm & 0xFFF));
}

	uint32_t csr(uint8_t f3, uint8_t rd, uint16_t csr_addr, uint8_t rs1)
{
	return (uint32_t(csr_addr & 0xFFF) << 20) | (uint32_t(rs1) << 15) |
	       (uint32_t(f3) << 12) | (uint32_t(rd) << 7) | 0x73;
}

// CSR immediate variants (csrrwi, csrrsi, csrrci)
uint32_t csri(uint8_t f3, uint8_t rd, uint16_t csr_addr, uint8_t uimm)
{
	return (uint32_t(csr_addr & 0xFFF) << 20) | (uint32_t(uimm & 0x1F) << 15) |
	       (uint32_t(f3) << 12) | (uint32_t(rd) << 7) | 0x73;
}

// Common aliases for register/immediate forms.
uint32_t addi(uint8_t rd, uint8_t rs1, int32_t imm) { return i(0x13, rd, 0, rs1, uint16_t(imm & 0xFFF)); }
uint32_t auipc(uint8_t rd, uint32_t imm20) { return u(0x17, rd, imm20); }
uint32_t lui(uint8_t rd, uint32_t imm20) { return u(0x37, rd, imm20); }

} // namespace

class CpuInstructionTest : public ::testing::Test {
protected:
	static constexpr uint64_t kMemSize = 4096;

	Memory mem{kMemSize};
	Bus bus;
	std::unique_ptr<Cpu> cpu;

	void SetUp() override
	{
		bus.addDevice(DRAM_BASE, &mem); // DRAM must be present for fetch/load/store.
		cpu = std::unique_ptr<Cpu>(new Cpu(bus, DRAM_BASE + kMemSize));
	}

	// Lay out `prog` contiguously at DRAM_BASE and execute exactly prog.size()
	// steps (or `steps` if given) through the fetch/decode/execute pipeline.
	void run(const std::vector<uint32_t>& prog, int steps = -1)
	{
		ASSERT_LE(prog.size() * 4, kMemSize) << "test program overflows test memory";
		for (size_t k = 0; k < prog.size(); ++k)
			mem.store(k * 4, 32, prog[k]);

		const int n = (steps < 0) ? int(prog.size()) : steps;
		for (int k = 0; k < n; ++k)
			step();
	}

	uint64_t reg(uint8_t idx) const { return cpu->getRegister(idx); }
	uint64_t pc() const { return cpu->getPC(); }

private:
	void step()
	{
		const uint32_t inst = cpu->fetch();
		cpu->forwardPC();
		uint8_t opcode, rd, rs1, rs2, f3, f7;
		cpu->decode(inst, opcode, rd, rs1, rs2, f3, f7);
		cpu->execute(inst, opcode, rd, rs1, rs2, f3, f7);
	}
};

// ===========================================================================
// OP-IMM (opcode 0x13)
// ===========================================================================

TEST_F(CpuInstructionTest, Addi)
{
	run({addi(1, 0, 5)});
	EXPECT_EQ(reg(1), 5u);
}

TEST_F(CpuInstructionTest, AddiSignExtendsNegativeImmediate)
{
	run({addi(1, 0, -1)});
	EXPECT_EQ(reg(1), 0xFFFFFFFFFFFFFFFFu);
}

TEST_F(CpuInstructionTest, AddiZeroIsNop)
{
	run({addi(1, 0, 7), addi(1, 1, 0)});
	EXPECT_EQ(reg(1), 7u);
}

TEST_F(CpuInstructionTest, SltiSigned)
{
	run({addi(1, 0, -5), i(0x13, 2, 2, 1, 0)}); // slti x2, x1, 0  -> -5 < 0 == 1
	EXPECT_EQ(reg(2), 1u);
}

TEST_F(CpuInstructionTest, SltiuUnsigned)
{
	// x1 = -1 (all ones); sltiu x2, x1, 1 -> (0xFFFF...F < 1) == 0
	run({addi(1, 0, -1), i(0x13, 2, 3, 1, 1)});
	EXPECT_EQ(reg(2), 0u);
}

TEST_F(CpuInstructionTest, Xori)
{
	run({addi(1, 0, 0x0F), i(0x13, 2, 4, 1, 0xFF)}); // xori x2, x1, 0xff
	EXPECT_EQ(reg(2), 0xF0u);
}

TEST_F(CpuInstructionTest, Ori)
{
	run({i(0x13, 2, 6, 0, 0xFF)}); // ori x2, x0, 0xff
	EXPECT_EQ(reg(2), 0xFFu);
}

TEST_F(CpuInstructionTest, Andi)
{
	run({i(0x13, 1, 6, 0, 0xFF), i(0x13, 2, 7, 1, 0x0F)}); // ori x1,x0,0xff -> 0xFF ; andi x2, x1, 0x0f
	EXPECT_EQ(reg(2), 0x0Fu);
}

TEST_F(CpuInstructionTest, Slli)
{
	run({addi(1, 0, 1), i(0x13, 2, 1, 1, 4)}); // slli x2, x1, 4
	EXPECT_EQ(reg(2), 16u);
}

TEST_F(CpuInstructionTest, Srli)
{
	run({addi(1, 0, -1), i(0x13, 2, 5, 1, 4)}); // srli x2, x1(all ones), 4
	EXPECT_EQ(reg(2), 0x0FFFFFFFFFFFFFFFu);
}

TEST_F(CpuInstructionTest, SraiSignExtends) // standard encoding, funct7 = 0x20
{
	run({addi(1, 0, -1), i(0x13, 2, 5, 1, (0x20 << 5) | 4)}); // srai x2, x1, 4
	EXPECT_EQ(reg(2), 0xFFFFFFFFFFFFFFFFu);
}

// ===========================================================================
// RV64 OP-IMM-32 (opcode 0x1b)
// ===========================================================================

TEST_F(CpuInstructionTest, AddiwSignExtends)
{
	run({i(0x1b, 1, 0, 0, uint16_t(-1 & 0xFFF))}); // addiw x1, x0, -1
	EXPECT_EQ(reg(1), 0xFFFFFFFFFFFFFFFFu);
}

TEST_F(CpuInstructionTest, Slliw)
{
	run({i(0x1b, 1, 0, 0, 1), i(0x1b, 2, 1, 1, 4)}); // slliw x2, x1, 4
	EXPECT_EQ(reg(2), 16u);
}

TEST_F(CpuInstructionTest, Srliw)
{
	// x1 = 0xFFFFFFFF (via addiw -1); srliw x2, x1, 4 -> 0x0FFFFFF0... no,
	// x1 = addiw x0,-1 = 0xFFFF...F; ASU32 -> 0xFFFFFFFF >> 4 = 0x0FFFFFFF.
	run({i(0x1b, 1, 0, 0, uint16_t(-1 & 0xFFF)), i(0x1b, 2, 5, 1, 4)}); // srliw x2,x1,4
	EXPECT_EQ(reg(2), 0x000000000FFFFFFFu);
}

TEST_F(CpuInstructionTest, Sraiw)
{
	run({i(0x1b, 1, 0, 0, uint16_t(-1 & 0xFFF)), i(0x1b, 2, 5, 1, (0x20 << 5) | 4)}); // sraiw x2,x1,4
	EXPECT_EQ(reg(2), 0xFFFFFFFFFFFFFFFFu);
}

// ===========================================================================
// U-type: lui / auipc
// ===========================================================================

TEST_F(CpuInstructionTest, LuiLoadsUpperBits)
{
	run({lui(1, 1)}); // lui x1, 0x1000
	EXPECT_EQ(reg(1), 0x1000u);
}

TEST_F(CpuInstructionTest, AuipcReadsCurrentPc)
{
	// auipc x1, 0 executed at DRAM_BASE -> x1 == DRAM_BASE.
	run({auipc(1, 0)});
	EXPECT_EQ(reg(1), DRAM_BASE);
}

// ===========================================================================
// R-type (opcode 0x33)
// ===========================================================================

TEST_F(CpuInstructionTest, Add)  { run({addi(1,0,2), addi(2,0,3), r(0x33,3,0,1,2,0x00)}); EXPECT_EQ(reg(3), 5u); }
TEST_F(CpuInstructionTest, Sub)  { run({addi(1,0,10),addi(2,0,3), r(0x33,3,0,1,2,0x20)}); EXPECT_EQ(reg(3), 7u); }
TEST_F(CpuInstructionTest, Mul)  { run({addi(1,0,6), addi(2,0,7), r(0x33,3,0,1,2,0x01)}); EXPECT_EQ(reg(3), 42u); }
TEST_F(CpuInstructionTest, SllR) { run({addi(1,0,1), addi(2,0,4), r(0x33,3,1,1,2,0x00)}); EXPECT_EQ(reg(3), 16u); }
TEST_F(CpuInstructionTest, XorR) { run({addi(1,0,0xF0),addi(2,0,0xFF),r(0x33,3,4,1,2,0x00)}); EXPECT_EQ(reg(3), 0x0Fu); }
TEST_F(CpuInstructionTest, OrR)  { run({addi(1,0,0xF0),addi(2,0,0x0F),r(0x33,3,6,1,2,0x00)}); EXPECT_EQ(reg(3), 0xFFu); }
TEST_F(CpuInstructionTest, AndR) { run({addi(1,0,0xFF),addi(2,0,0x0F),r(0x33,3,7,1,2,0x00)}); EXPECT_EQ(reg(3), 0x0Fu); }

TEST_F(CpuInstructionTest, SltSigned)
{
	run({addi(1, 0, -1), addi(2, 0, 0), r(0x33, 3, 2, 1, 2, 0x00)}); // slt x3, x1(-1), x2(0)
	EXPECT_EQ(reg(3), 1u);
}

TEST_F(CpuInstructionTest, SltuUnsigned)
{
	// x1 = -1 (0xFFFF...), x2 = 0; sltu -> (huge < 0) == 0
	run({addi(1, 0, -1), addi(2, 0, 0), r(0x33, 3, 3, 1, 2, 0x00)});
	EXPECT_EQ(reg(3), 0u);
}

TEST_F(CpuInstructionTest, SrlR)
{
	run({addi(1, 0, -1), addi(2, 0, 4), r(0x33, 3, 5, 1, 2, 0x00)}); // srl x3, x1, x2(4)
	EXPECT_EQ(reg(3), 0x0FFFFFFFFFFFFFFFu);
}

// ===========================================================================
// RV64 R-type-32 (opcode 0x3b)
// ===========================================================================

TEST_F(CpuInstructionTest, Addw)  { run({addi(1,0,2),addi(2,0,3),r(0x3b,3,0,1,2,0x00)}); EXPECT_EQ(reg(3), 5u); }
TEST_F(CpuInstructionTest, Subw)  { run({addi(1,0,10),addi(2,0,3),r(0x3b,3,0,1,2,0x20)}); EXPECT_EQ(reg(3), 7u); }

TEST_F(CpuInstructionTest, Sllw)
{
	run({addi(1, 0, 1), addi(2, 0, 4), r(0x3b, 3, 1, 1, 2, 0x00)}); // sllw x3, x1, x2(4)
	EXPECT_EQ(reg(3), 16u);
}

TEST_F(CpuInstructionTest, Srlw)
{
	// x1 = 0xFFFFFFFF (-1), x2 = 4; srlw -> ASU32(0xFFFFFFFF) >> 4 == 0x0FFFFFFF
	run({addi(1, 0, -1), addi(2, 0, 4), r(0x3b, 3, 5, 1, 2, 0x00)});
	EXPECT_EQ(reg(3), 0x000000000FFFFFFFu);
}

TEST_F(CpuInstructionTest, Sraw)
{
	// x1 = 0xFFFFFFFF (-1), x2 = 4; sraw -> arithmetic shift of -1 == -1
	run({addi(1, 0, -1), addi(2, 0, 4), r(0x3b, 3, 5, 1, 2, 0x20)});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFFFFFFu);
}

// ===========================================================================
// Branches (opcode 0x63)
// After forwardPC, pc = DRAM_BASE + 4*insnIndex; a taken branch to imm sets
// pc = DRAM_BASE + insnIndex*4 + imm.
// ===========================================================================

TEST_F(CpuInstructionTest, BeqTaken)
{
	run({addi(1,0,5), addi(2,0,5), b(0x0, 1, 2, 16)}); // beq x1,x2,+16
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 16); // executed from insn index 2 (addr BASE+8)
}

TEST_F(CpuInstructionTest, BeqNotTaken)
{
	run({addi(1,0,5), addi(2,0,6), b(0x0, 1, 2, 16)}); // 5 != 6
	EXPECT_EQ(pc(), DRAM_BASE + 12); // falls through
}

TEST_F(CpuInstructionTest, BneTaken)
{
	run({addi(1,0,5), addi(2,0,6), b(0x1, 1, 2, 8)}); // 5 != 6 -> taken
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 8);
}

TEST_F(CpuInstructionTest, BltSigned)
{
	run({addi(1,0,-1), addi(2,0,0), b(0x4, 1, 2, 8)}); // -1 < 0 -> taken
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 8);
}

TEST_F(CpuInstructionTest, BgeSigned)
{
	run({addi(1,0,5), addi(2,0,5), b(0x5, 1, 2, 8)}); // 5 >= 5 -> taken
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 8);
}

TEST_F(CpuInstructionTest, BltuUnsigned)
{
	run({addi(1,0,0), addi(2,0,1), b(0x6, 1, 2, 8)}); // 0 < 1 unsigned -> taken
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 8);
}

TEST_F(CpuInstructionTest, BgeuUnsigned)
{
	run({addi(1,0,1), addi(2,0,1), b(0x7, 1, 2, 8)}); // 1 >= 1 unsigned -> taken
	EXPECT_EQ(pc(), DRAM_BASE + 8 + 8);
}

// ===========================================================================
// Jumps
// ===========================================================================

TEST_F(CpuInstructionTest, JalLinksAndJumps)
{
	run({jal(3, 8)}); // jal x3, +8
	EXPECT_EQ(reg(3), DRAM_BASE + 4); // link = address of instruction + 4
	EXPECT_EQ(pc(), DRAM_BASE + 8);
}

TEST_F(CpuInstructionTest, JalrIndirect)
{
	// Build BASE+0x10 in x1 via auipc+addi, then jalr x3, x1, 0.
	run({
		auipc(1, 0),      // x1 = BASE
		addi(1, 1, 0x10), // x1 = BASE + 0x10
		jalr(3, 1, 0),    // x3 = BASE+0xC (link), pc = (BASE+0x10) & ~1
	});
	EXPECT_EQ(reg(3), DRAM_BASE + 0xC);
	EXPECT_EQ(pc(), DRAM_BASE + 0x10);
}

// ===========================================================================
// Loads / stores with sign / zero extension
// ===========================================================================

TEST_F(CpuInstructionTest, StoreAndLoadByte)
{
	run({
		auipc(1, 0),      // x1 = BASE
		addi(1, 1, 0x80), // x1 = BASE + 0x80
		addi(2, 0, 0x41), // x2 = 'A'
		s(0, 1, 2, 0),    // sb x2, 0(x1)
		i(0x03, 3, 0, 1, 0), // lb x3, 0(x1)
	});
	EXPECT_EQ(reg(3), 0x41u);
	EXPECT_EQ(mem.load(0x80, 8), 0x41u);
}

TEST_F(CpuInstructionTest, LoadByteSignExtends)
{
	run({
		auipc(1, 0), addi(1, 1, 0x80),
		addi(2, 0, 0x80),                  // 0x80 = -128 as byte
		s(0, 1, 2, 0),                     // sb
		i(0x03, 3, 0, 1, 0),               // lb  -> sign extended
		i(0x03, 4, 4, 1, 0),               // lbu -> zero extended
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFFFF80u);
	EXPECT_EQ(reg(4), 0x80u);
}

TEST_F(CpuInstructionTest, LoadHalfSignExtends)
{
	run({
		auipc(1, 0), addi(1, 1, 0x80),
		lui(2, 0x8),                       // x2 = 0x8000
		s(1, 1, 2, 0),                     // sh
		i(0x03, 3, 1, 1, 0),               // lh
		i(0x03, 4, 5, 1, 0),               // lhu
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFF8000u);
	EXPECT_EQ(reg(4), 0x8000u);
}

TEST_F(CpuInstructionTest, LoadWordSignExtends)
{
	run({
		auipc(1, 0), addi(1, 1, 0x80),
		lui(2, 0x80000),                   // x2 = 0x80000000 (low 32 bits; top bits set, irrelevant for sw)
		s(2, 1, 2, 0),                     // sw stores low 32 bits = 0x80000000
		i(0x03, 3, 2, 1, 0),               // lw  -> sign extended
		i(0x03, 4, 6, 1, 0),               // lwu -> zero extended
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFF80000000u);
	EXPECT_EQ(reg(4), 0x80000000u);
}

TEST_F(CpuInstructionTest, StoreAndLoadDoubleWord)
{
	run({
		auipc(1, 0), addi(1, 1, 0x80),
		addi(2, 0, -1),                    // x2 = all ones
		s(3, 1, 2, 0),                     // sd
		i(0x03, 3, 3, 1, 0),               // ld
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFFFFFFu);
}

// ===========================================================================
// x0 is hardwired to zero
// ===========================================================================

TEST_F(CpuInstructionTest, X0StaysZero)
{
	run({addi(0, 0, 5)}); // addi x0, x0, 5
	EXPECT_EQ(reg(0), 0u);
}

// ===========================================================================
// CSRs: csrrw / csrrs / csrrc against MTVEC (0x305)
// ===========================================================================

TEST_F(CpuInstructionTest, Csrrw)
{
	run({
		lui(1, 1),                 // x1 = 0x1000
		addi(1, 1, 0x234),         // x1 = 0x1234
		csr(0x1, 2, MTVEC, 1),     // csrrw x2, mtvec, x1 : x2 = old(0), mtvec = 0x1234
		csr(0x1, 3, MTVEC, 0),     // csrrw x3, mtvec, x0 : x3 = 0x1234, mtvec = 0
	});
	EXPECT_EQ(reg(2), 0u);
	EXPECT_EQ(reg(3), 0x1234u);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0u);
}

TEST_F(CpuInstructionTest, CsrrsSetsBits)
{
	run({
		i(0x13, 1, 0, 0, 0xFF),    // addi x1, x0, 0xFF
		csr(0x2, 2, MTVEC, 1),     // csrrs x2, mtvec, x1 : mtvec 0 -> 0xFF, x2 = 0
	});
	EXPECT_EQ(reg(2), 0u);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0xFFu);
}

TEST_F(CpuInstructionTest, CsrrcClearsBits)
{
	run({
		i(0x13, 1, 0, 0, 0xFF),    // x1 = 0xFF
		csr(0x2, 0, MTVEC, 1),     // csrrs x0, mtvec, x1 -> mtvec = 0xFF
		i(0x13, 2, 0, 0, 0x0F),    // x2 = 0x0F
		csr(0x3, 3, MTVEC, 2),     // csrrc x3, mtvec, x2 -> mtvec = 0xF0, x3 = 0xFF
	});
	EXPECT_EQ(reg(3), 0xFFu);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0xF0u);
}

// ===========================================================================
// lui: spec sign-extends the 32-bit U-immediate to XLEN (per RISC-V spec).
// ===========================================================================

TEST_F(CpuInstructionTest, LuiSignExtendsHighBit)
{
	run({lui(1, 0x80000)}); // lui x1, 0x80000 -> 0x80000000 sign-extended
	EXPECT_EQ(reg(1), 0xFFFFFFFF80000000u);
}

// ===========================================================================
// R-type sra (opcode 0x33, funct3=5, funct7=0x20): arithmetic right shift.
// Previously unimplemented (threw IllegalInstruction).
// ===========================================================================

TEST_F(CpuInstructionTest, SraIsArithmeticShift)
{
	run({
		addi(1, 0, -1),                 // x1 = -1 (all ones)
		addi(2, 0, 4),                  // x2 = 4
		r(0x33, 3, 5, 1, 2, 0x20),      // sra x3, x1, x2 -> -1
		r(0x33, 4, 5, 1, 2, 0x00),      // srl x4, x1, x2 -> 0x0FFF...F
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFFFFFFu);
	EXPECT_EQ(reg(4), 0x0FFFFFFFFFFFFFFFu);
}

// ===========================================================================
// divuw (opcode 0x3b, funct3=5, funct7=0x01): 32-bit unsigned divide, result
// sign-extended. Previously computed a 64-bit divide under the name "divu".
// ===========================================================================

TEST_F(CpuInstructionTest, DivuwUses32BitOperands)
{
	// x1 = -1 (low 32 bits = 0xFFFFFFFF); x2 = 2.
	// divuw: 0xFFFFFFFF / 2 = 0x7FFFFFFF (sign-extended, bit 31 = 0).
	// The old 64-bit bug produced 0x7FFFFFFFFFFFFFFF instead.
	run({
		addi(1, 0, -1),
		addi(2, 0, 2),
		r(0x3b, 3, 5, 1, 2, 0x01),      // divuw x3, x1, x2
	});
	EXPECT_EQ(reg(3), 0x000000007FFFFFFFu);
}

TEST_F(CpuInstructionTest, DivuwDivideByZeroIsAllOnes)
{
	run({
		addi(1, 0, 123),
		addi(2, 0, 0),
		r(0x3b, 3, 5, 1, 2, 0x01),      // divuw x3, x1, 0 -> 0xFFFFFFFF sign-extended
	});
	EXPECT_EQ(reg(3), 0xFFFFFFFFFFFFFFFFu);
}

// ===========================================================================
// fence (opcode 0x0f) is a no-op for this single-threaded model. The case
// previously fell through into the OP-IMM (0x13) body, which would corrupt a
// register if the fence word's rd / immediate fields were non-zero.
// ===========================================================================

TEST_F(CpuInstructionTest, FenceIsNopAndDoesNotFallThrough)
{
	// Crafted fence word: opcode 0x0f, funct3=0, rd=5, imm[31:20]=1.
	// With the fall-through bug it would execute as `addi x5, x0, 1` (x5 = 1).
	const uint32_t fenceWord = (0x1u << 20) | (0x5u << 7) | 0x0fu;
	run({fenceWord, addi(1, 0, 7)});

	EXPECT_EQ(reg(5), 0u) << "fence must not write registers (fall-through bug)";
	EXPECT_EQ(reg(1), 7u);
	EXPECT_EQ(pc(), DRAM_BASE + 8); // fence + addi each advanced PC by 4
}

// ===========================================================================
// decode() bitfield extraction sanity check
// ===========================================================================

TEST_F(CpuInstructionTest, DecodeExtractsFields)
{
	// add x5, x6, x7 : funct7=0 rd=5 f3=0 rs1=6 rs2=7 opcode=0x33
	const uint32_t inst = r(0x33, 5, 0, 6, 7, 0x00);
	mem.store(0, 32, inst);
	uint32_t fetched = cpu->fetch();
	ASSERT_EQ(fetched, inst);

	uint8_t opcode, rd, rs1, rs2, f3, f7;
	cpu->decode(fetched, opcode, rd, rs1, rs2, f3, f7);
	EXPECT_EQ(opcode, 0x33);
	EXPECT_EQ(rd, 5);
	EXPECT_EQ(rs1, 6);
	EXPECT_EQ(rs2, 7);
	EXPECT_EQ(f3, 0);
	EXPECT_EQ(f7, 0);
}

// ===========================================================================
// CSR immediate variants: csrrwi, csrrsi, csrrci
// ===========================================================================

TEST_F(CpuInstructionTest, CsrrwiWriteImmediate)
{
	// csrrwi rd, csr, uimm: x[rd] = CSRs[csr]; CSRs[csr] = zimm (zero-extended immediate)
	run({
		lui(1, 1),                 // x1 = 0x1000
		addi(1, 1, 0x234),         // x1 = 0x1234
		csri(0x5, 2, MTVEC, 1),    // csrrwi x2, mtvec, 1 : x2 = old(0), mtvec = 1
		csri(0x5, 3, MTVEC, 0),    // csrrwi x3, mtvec, 0 : x3 = 1, mtvec = 0
	});
	EXPECT_EQ(reg(2), 0u);
	EXPECT_EQ(reg(3), 1u);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0u);
}

TEST_F(CpuInstructionTest, CsrrsiSetsBits)
{
	// csrrsi rd, csr, uimm: t = CSRs[csr]; CSRs[csr] = t | zimm; x[rd] = t
	// Note: only lower 5 bits of uimm are used (0-31)
	run({
		csri(0x6, 1, MTVEC, 0x1F), // csrrsi x1, mtvec, 0x1F : mtvec 0 -> 0x1F, x1 = 0
		csri(0x6, 2, MTVEC, 0x10), // csrrsi x2, mtvec, 0x10 : mtvec 0x1F -> 0x1F, x2 = 0x1F
	});
	EXPECT_EQ(reg(1), 0u);
	EXPECT_EQ(reg(2), 0x1Fu);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0x1Fu);
}

TEST_F(CpuInstructionTest, CsrrciClearsBits)
{
	// csrrci rd, csr, uimm: t = CSRs[csr]; CSRs[csr] = t & ~zimm; x[rd] = t
	// Note: only lower 5 bits of uimm are used (0-31)
	run({
		csri(0x7, 1, MTVEC, 0x1F), // csrrci x1, mtvec, 0x1F : mtvec 0 -> 0, x1 = 0
		csri(0x6, 0, MTVEC, 0x1F), // csrrsi x0, mtvec, 0x1F : mtvec 0 -> 0x1F, x0 unchanged
		csri(0x7, 2, MTVEC, 0x0F), // csrrci x2, mtvec, 0x0F : mtvec 0x1F -> 0x10, x2 = 0x1F
	});
	EXPECT_EQ(reg(1), 0u);
	EXPECT_EQ(reg(2), 0x1Fu);
	EXPECT_EQ(cpu->getCsr(MTVEC), 0x10u);
}

// ===========================================================================
// System instructions: ecall, ebreak, sret, mret
// Note: These instructions may cause LoadAccessFault due to PC manipulation,
// so we test them in isolation to verify they execute without crashes.
// ===========================================================================

TEST_F(CpuInstructionTest, EcallExecutes)
{
	// ecall should execute without crashing (basic functionality test)
	const uint32_t ecall = 0x73u; // opcode=0x73, rd=0, rs1=0, funct3=0, funct7=0
	EXPECT_NO_THROW(run({ecall})) << "ecall should execute without throwing";
}

TEST_F(CpuInstructionTest, EbreakExecutes)
{
	// ebreak should execute without crashing (basic functionality test)
	const uint32_t ebreak = 0x00100073u; // opcode=0x73, rd=0, rs1=1, funct3=0, funct7=0
	EXPECT_NO_THROW(run({ebreak})) << "ebreak should execute without throwing";
}

TEST_F(CpuInstructionTest, SretExecutes)
{
	// sret should execute without crashing (basic functionality test)
	const uint32_t sret = 0x10200073u; // opcode=0x73, funct7=0x02, funct3=0, rs1=0
	EXPECT_NO_THROW(run({sret})) << "sret should execute without throwing";
}

TEST_F(CpuInstructionTest, MretExecutes)
{
	// mret should execute without crashing (basic functionality test)
	const uint32_t mret = 0x30200073u; // opcode=0x73, funct7=0x06, funct3=0, rs1=0
	EXPECT_NO_THROW(run({mret})) << "mret should execute without throwing";
}

// ===========================================================================
// sfence.vma: memory management fence
// ===========================================================================

TEST_F(CpuInstructionTest, SfenceVmaIsNop)
{
	// sfence.vma rs1, rs2: order memory management operations (no-op in single-threaded emulator)
	const uint32_t sfence = 0x12000073u; // sfence.vma x0, x0
	run({sfence, addi(1, 0, 42)});
	EXPECT_EQ(reg(1), 42u);
	EXPECT_EQ(pc(), DRAM_BASE + 8);
}
