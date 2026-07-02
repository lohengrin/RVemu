#include "Bus.h"
#include "Memory.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace {
// Minimal Device for deterministic routing checks (avoids the 128 MiB Memory default).
class FakeDevice : public Device {
public:
	explicit FakeDevice(uint64_t sz) : m_size(sz) {}

	uint64_t load(uint64_t addr, uint8_t /*size*/) const override { return addr; }
	void store(uint64_t addr, uint8_t /*size*/, uint64_t value) override { lastOffset = addr; lastValue = value; }
	uint64_t size() const override { return m_size; }

	uint64_t lastOffset = 0;
	uint64_t lastValue = 0;

private:
	uint64_t m_size;
};
} // namespace

// A load hits the device whose [base, base+size) range covers the address,
// and the device receives the address relative to its own base.
TEST(BusTest, LoadRoutesToOwnerWithOffset)
{
	Bus bus;
	FakeDevice dev(0x100);
	ASSERT_TRUE(bus.addDevice(0x80000000, &dev));

	EXPECT_EQ(bus.load(0x80000000, 8), 0u);      // base -> offset 0
	EXPECT_EQ(bus.load(0x80000040, 8), 0x40u);   // mid  -> offset 0x40
	EXPECT_EQ(bus.load(0x800000FF, 8), 0xFFu);   // last byte of range
}

TEST(BusTest, StoreRoutesToOwnerWithOffset)
{
	Bus bus;
	FakeDevice dev(0x100);
	ASSERT_TRUE(bus.addDevice(0x80000000, &dev));

	bus.store(0x80000010, 8, 0xABCD);
	EXPECT_EQ(dev.lastOffset, 0x10u);
	EXPECT_EQ(dev.lastValue, 0xABCDu);
}

// Addresses not covered by any device fault.
TEST(BusTest, UnmappedLoadFaults)
{
	Bus bus;
	FakeDevice dev(0x100);
	bus.addDevice(0x80000000, &dev);

	EXPECT_THROW(bus.load(0x70000000, 8), CpuException);
	EXPECT_THROW(bus.load(0x80000100, 8), CpuException); // first byte after the range
}

TEST(BusTest, UnmappedStoreFaults)
{
	Bus bus;
	EXPECT_THROW(bus.store(0x1234, 8, 0), CpuException);
}

// Multiple devices coexist; the bus hands each access to the right one.
TEST(BusTest, MultipleDevicesSelectCorrectly)
{
	Bus bus;
	FakeDevice a(0x100);
	FakeDevice b(0x100);
	bus.addDevice(0x80000000, &a);
	bus.addDevice(0x10000000, &b);

	EXPECT_EQ(bus.load(0x80000010, 8), 0x10u);
	EXPECT_EQ(bus.load(0x10000020, 8), 0x20u);

	bus.store(0x10000030, 8, 1);
	EXPECT_EQ(b.lastOffset, 0x30u);
	EXPECT_EQ(a.lastValue, 0u); // untouched
}

TEST(BusTest, GetDeviceByBase)
{
	Bus bus;
	FakeDevice a(0x100);
	FakeDevice b(0x100);
	bus.addDevice(0x80000000, &a);
	bus.addDevice(0x10000000, &b);

	EXPECT_EQ(bus.getDevice(0x80000000), &a);
	EXPECT_EQ(bus.getDevice(0x10000000), &b);
	EXPECT_EQ(bus.getDevice(0xDEADBEEF), nullptr);
}

TEST(BusTest, AddNullDeviceFails)
{
	Bus bus;
	EXPECT_FALSE(bus.addDevice(0x1000, nullptr));
}

// Routing also works against the real Memory device end-to-end.
TEST(BusTest, RoutesToRealMemory)
{
	Bus bus;
	Memory mem(4096);
	bus.addDevice(0x80000000, &mem);

	bus.store(0x80000100, 32, 0xCAFEBABE);
	EXPECT_EQ(mem.load(0x100, 32), 0xCAFEBABEu);
	EXPECT_EQ(bus.load(0x80000100, 32), 0xCAFEBABEu);
}
