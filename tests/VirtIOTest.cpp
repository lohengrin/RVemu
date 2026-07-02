#include "VirtIO.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

// Legacy virtio "magic value" identifies the device to the driver.
TEST(VirtIOTest, MagicValue)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_MAGIC, 32), 0x74726976u); // "virt"
}

TEST(VirtIOTest, VersionIsLegacy1)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_VERSION, 32), 1u);
}

TEST(VirtIOTest, DeviceIdIsBlock)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_DEVICE_ID, 32), 2u); // 2 == block device
}

TEST(VirtIOTest, VendorId)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_VENDOR_ID, 32), 0x554d4551u); // "QEMU"
}

// QUEUE_NUM_MAX is fixed at 8 (the DESC_NUM constant).
TEST(VirtIOTest, QueueNumMax)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_QUEUE_NUM_MAX, 32), 8u);
}

TEST(VirtIOTest, StatusRoundTripAndReset)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_STATUS, 32), 0u);
	v.store(VIRTIO_STATUS, 32, 0xF);
	EXPECT_EQ(v.load(VIRTIO_STATUS, 32), 0xFu);
	// Writing 0 resets the device status.
	v.store(VIRTIO_STATUS, 32, 0);
	EXPECT_EQ(v.load(VIRTIO_STATUS, 32), 0u);
}

TEST(VirtIOTest, QueuePfnRoundTrip)
{
	VirtIO v;
	EXPECT_EQ(v.load(VIRTIO_QUEUE_PFN, 32), 0u);
	v.store(VIRTIO_QUEUE_PFN, 32, 0x1234);
	EXPECT_EQ(v.load(VIRTIO_QUEUE_PFN, 32), 0x1234u);
}

// VirtIO only accepts 32-bit accesses.
TEST(VirtIOTest, RejectsNon32BitAccess)
{
	VirtIO v;
	EXPECT_THROW(v.load(VIRTIO_MAGIC, 64), CpuException);
	EXPECT_THROW(v.store(VIRTIO_STATUS, 8, 0), CpuException);
}
