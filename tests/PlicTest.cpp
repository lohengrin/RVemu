#include "Plic.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

// PLIC only accepts 32-bit accesses.
TEST(PlicTest, RejectsNon32BitAccess)
{
	Plic plic;
	EXPECT_THROW(plic.load(PLIC_SENABLE, 64), CpuException);
	EXPECT_THROW(plic.store(PLIC_SENABLE, 8, 0), CpuException);
	EXPECT_NO_THROW(plic.load(PLIC_SENABLE, 32));
}

TEST(PlicTest, PendingRoundTrip)
{
	Plic plic;
	EXPECT_EQ(plic.load(PLIC_PENDING, 32), 0u);
	plic.store(PLIC_PENDING, 32, 0xDEADBEEF);
	EXPECT_EQ(plic.load(PLIC_PENDING, 32), 0xDEADBEEFu);
}

TEST(PlicTest, SenableRoundTrip)
{
	Plic plic;
	plic.store(PLIC_SENABLE, 32, 0xABCD1234);
	EXPECT_EQ(plic.load(PLIC_SENABLE, 32), 0xABCD1234u);
}

TEST(PlicTest, SpriorityRoundTrip)
{
	Plic plic;
	plic.store(PLIC_SPRIORITY, 32, 7);
	EXPECT_EQ(plic.load(PLIC_SPRIORITY, 32), 7u);
}

TEST(PlicTest, SclaimRoundTrip)
{
	Plic plic;
	plic.store(PLIC_SCLAIM, 32, 42);
	EXPECT_EQ(plic.load(PLIC_SCLAIM, 32), 42u);
}

TEST(PlicTest, RegistersAreIndependent)
{
	Plic plic;
	plic.store(PLIC_PENDING, 32, 1);
	plic.store(PLIC_SENABLE, 32, 2);
	plic.store(PLIC_SPRIORITY, 32, 3);
	plic.store(PLIC_SCLAIM, 32, 4);
	EXPECT_EQ(plic.load(PLIC_PENDING, 32), 1u);
	EXPECT_EQ(plic.load(PLIC_SENABLE, 32), 2u);
	EXPECT_EQ(plic.load(PLIC_SPRIORITY, 32), 3u);
	EXPECT_EQ(plic.load(PLIC_SCLAIM, 32), 4u);
}
