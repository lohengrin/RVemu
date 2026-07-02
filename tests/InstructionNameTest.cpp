// Regression tests for the instruction-name table in CpuUtils.cpp.
//
// getInstructionName() must agree with what the executor actually implements
// for a given (opcode, funct3, funct7). The cases below lock in the encodings
// that previously had mismatched names or funct fields.

#include "Defines.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

static std::string name(uint8_t op, uint8_t f3, uint8_t f7)
{
	return getInstructionName(op, f3, f7);
}

// LOAD: funct3=5 is lhu (load half unsigned), funct3=6 is lwu (load word unsigned).
// The table previously had these two swapped.
TEST(InstructionNameTest, LoadHalfAndWordUnsigned)
{
	EXPECT_EQ(name(0x03, 0x5, 0x00), "lhu");
	EXPECT_EQ(name(0x03, 0x6, 0x00), "lwu");
}

// OP-IMM shift: srai uses the standard funct7 = 0x20 encoding (the executor
// matches funct7>>1 == 0x10). The table previously listed funct7 = 0x10.
TEST(InstructionNameTest, ShiftImmediateImmediate)
{
	EXPECT_EQ(name(0x13, 0x5, 0x00), "srli");
	EXPECT_EQ(name(0x13, 0x5, 0x20), "srai");
}

// AMO: the executor decodes funct5 = funct7>>2, so amoswap (funct5 = 1) lives at
// funct7 = 0x04, not 0x01 (which decodes as funct5 = 0 = amoadd).
TEST(InstructionNameTest, AtomicSwapFunct7)
{
	EXPECT_EQ(name(0x2f, 0x2, 0x00), "amoadd.w");
	EXPECT_EQ(name(0x2f, 0x2, 0x04), "amoswap.w");
	EXPECT_EQ(name(0x2f, 0x3, 0x00), "amoadd.d");
	EXPECT_EQ(name(0x2f, 0x3, 0x04), "amoswap.d");
}

// RV64M word ops live at opcode 0x3b with funct7 = 0000001; the table previously
// named the (0x3b, funct3=5, funct7=1) slot "divu" -- per spec that is divuw.
TEST(InstructionNameTest, DivUwName)
{
	EXPECT_EQ(name(0x3b, 0x5, 0x01), "divuw");
}

// R-type sra (opcode 0x33, funct3=5, funct7=0x20) is now implemented, so the
// name table must identify it.
TEST(InstructionNameTest, SraName)
{
	EXPECT_EQ(name(0x33, 0x5, 0x00), "srl");
	EXPECT_EQ(name(0x33, 0x5, 0x20), "sra");
}

// A few stable entries as a sanity check that nothing else regressed.
TEST(InstructionNameTest, StableEntries)
{
	EXPECT_EQ(name(0x33, 0x0, 0x00), "add");
	EXPECT_EQ(name(0x33, 0x0, 0x20), "sub");
	EXPECT_EQ(name(0x13, 0x0, 0x00), "addi");
	EXPECT_EQ(name(0x63, 0x0, 0x00), "beq");
	EXPECT_EQ(name(0x6f, 0x0, 0x00), "jal");
}
