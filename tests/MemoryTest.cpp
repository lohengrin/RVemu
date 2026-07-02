#include "Memory.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

// Little-endian load/store round-trips at every supported width.
TEST(MemoryTest, StoreLoadRoundTripAllWidths)
{
	Memory mem(4096);

	mem.store(0x00, 8, 0xA5);
	EXPECT_EQ(mem.load(0x00, 8), 0xA5);

	mem.store(0x10, 16, 0x1234);
	EXPECT_EQ(mem.load(0x10, 16), 0x1234);

	mem.store(0x20, 32, 0xDEADBEEF);
	EXPECT_EQ(mem.load(0x20, 32), 0xDEADBEEF);

	mem.store(0x30, 64, 0x0123456789ABCDEFULL);
	EXPECT_EQ(mem.load(0x30, 64), 0x0123456789ABCDEFULL);
}

// dram is little-endian: least-significant byte at the lowest address.
TEST(MemoryTest, LittleEndianByteOrder)
{
	Memory mem(4096);
	mem.store(0, 32, 0x12345678);
	EXPECT_EQ(mem.load(0, 8), 0x78);
	EXPECT_EQ(mem.load(1, 8), 0x56);
	EXPECT_EQ(mem.load(2, 8), 0x34);
	EXPECT_EQ(mem.load(3, 8), 0x12);
}

// Writes never spill across the end of the device's address space.
TEST(MemoryTest, OutOfBoundsLoadThrows)
{
	Memory mem(4096);

	// Last fully addressable byte is index 4095.
	EXPECT_NO_THROW(mem.load(4095, 8));

	// A 64-bit load at 4096 is entirely out of range.
	EXPECT_THROW(mem.load(4096, 64), CpuException);
	// A 64-bit load at 4089 would run past the end (4089 + 8 > 4096).
	EXPECT_THROW(mem.load(4089, 64), CpuException);
}

TEST(MemoryTest, OutOfBoundsStoreThrows)
{
	Memory mem(4096);
	EXPECT_NO_THROW(mem.store(4088, 64, 0)); // 4088 + 8 == 4096, ok
	EXPECT_THROW(mem.store(4089, 64, 0), CpuException);
}

// Sizes other than 8/16/32/64 are rejected.
TEST(MemoryTest, UnknownSizeThrows)
{
	Memory mem(4096);
	EXPECT_THROW(mem.load(0, 24), CpuException);
	EXPECT_THROW(mem.store(0, 24, 0), CpuException);
}

// Out-of-range access must raise the specific exception kind.
TEST(MemoryTest, RaisesLoadAccessFault)
{
	Memory mem(4096);
	try {
		mem.load(4096, 8);
		FAIL() << "expected CpuException";
	}
	catch (const CpuException& e) {
		EXPECT_EQ(e.ex, Except::LoadAccessFault);
	}
}
