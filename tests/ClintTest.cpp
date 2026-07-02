#include "Clint.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

// CLINT only accepts 64-bit accesses.
TEST(ClintTest, RejectsNon64BitAccess)
{
	Clint clint;
	EXPECT_THROW(clint.load(CLINT_MTIME, 32), CpuException);
	EXPECT_THROW(clint.store(CLINT_MTIME, 32, 0), CpuException);
	EXPECT_NO_THROW(clint.load(CLINT_MTIME, 64));
}

TEST(ClintTest, MtimeRoundTrip)
{
	Clint clint;
	EXPECT_EQ(clint.load(CLINT_MTIME, 64), 0u);
	clint.store(CLINT_MTIME, 64, 0x0123456789ABCDEFULL);
	EXPECT_EQ(clint.load(CLINT_MTIME, 64), 0x0123456789ABCDEFULL);
}

TEST(ClintTest, MtimecmpRoundTrip)
{
	Clint clint;
	EXPECT_EQ(clint.load(CLINT_MTIMECMP, 64), 0u);
	clint.store(CLINT_MTIMECMP, 64, 0xFEDCBA9876543210ULL);
	EXPECT_EQ(clint.load(CLINT_MTIMECMP, 64), 0xFEDCBA9876543210ULL);
}

// Writes to mtime and mtimecmp are independent.
TEST(ClintTest, RegistersAreIndependent)
{
	Clint clint;
	clint.store(CLINT_MTIME, 64, 100);
	clint.store(CLINT_MTIMECMP, 64, 200);
	EXPECT_EQ(clint.load(CLINT_MTIME, 64), 100u);
	EXPECT_EQ(clint.load(CLINT_MTIMECMP, 64), 200u);
}

// Unknown offsets read as 0 and silently ignore writes.
TEST(ClintTest, UnknownOffsetIsZero)
{
	Clint clint;
	EXPECT_EQ(clint.load(0x0000, 64), 0u);
	clint.store(0x0000, 64, 0x1234);
	EXPECT_EQ(clint.load(0x0000, 64), 0u);
}
