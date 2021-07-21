#pragma once

#include "Device.h"

//! The virtio module contains a virtualization standard for network and disk device drivers.
//! This is the "legacy" virtio interface.
//!
//! The virtio spec:
//! https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.pdf


/// The interrupt request of virtio.
const uint64_t VIRTIO_IRQ  = 1;

const uint64_t VRING_DESC_SIZE = 16;
/// The number of virtio descriptors. It must be a power of two.
const uint64_t DESC_NUM = 8;

/// Always return 0x74726976.
const uint64_t VIRTIO_MAGIC  = 0x000;
/// The version. 1 is legacy.
const uint64_t VIRTIO_VERSION  = 0x004;
/// device type; 1 is net, 2 is disk.
const uint64_t VIRTIO_DEVICE_ID  = 0x008;
/// Always return 0x554d4551
const uint64_t VIRTIO_VENDOR_ID  = 0x00c;
/// Device features.
const uint64_t VIRTIO_DEVICE_FEATURES  = 0x010;
/// Driver features.
const uint64_t VIRTIO_DRIVER_FEATURES  = 0x020;
/// Page size for PFN, write-only.
const uint64_t VIRTIO_GUEST_PAGE_SIZE  = 0x028;
/// Select queue, write-only.
const uint64_t VIRTIO_QUEUE_SEL  = 0x030;
/// Max size of current queue, read-only. In QEMU, `VIRTIO_COUNT = 8`.
const uint64_t VIRTIO_QUEUE_NUM_MAX  = 0x034;
/// Size of current queue, write-only.
const uint64_t VIRTIO_QUEUE_NUM  = 0x038;
/// Physical page number for queue, read and write.
const uint64_t VIRTIO_QUEUE_PFN  = 0x040;
/// Notify the queue number, write-only.
const uint64_t VIRTIO_QUEUE_NOTIFY  = 0x050;
/// Device status, read and write. Reading from this register returns the current device status flags.
/// Writing non-zero values to this register sets the status flags, indicating the OS/driver
/// progress. Writing zero (0x0) to this register triggers a device reset.
const uint64_t VIRTIO_STATUS  = 0x070;
/// The size of virtio.
const uint64_t VIRTIO_SIZE = 0x1000;

class Cpu;

/// The core-local interruptor (CLINT).
class VirtIO : public Device
{
public:
    VirtIO();
    virtual ~VirtIO();

    //! Device Interface
    //!load
    uint64_t load(uint64_t addr, uint8_t size) const;
    //! store
    void store(uint64_t addr, uint8_t size, uint64_t value);
    //! Get address space size of device
    uint64_t size() const { return VIRTIO_SIZE; }

    /// Return true if an interrupt is pending.
    bool is_interrupting()
    {
        if (queue_notify != 9999) {
            queue_notify = 9999;
            return true;
        }
        return false;
    }

    bool loadDisk(const std::string& imageFile);

protected:
    uint64_t load32(uint64_t addr) const;
    void store32(uint64_t addr, uint64_t value);

    uint64_t get_new_id() { return ++id; } //wrapping_add
    uint64_t desc_addr() const { return ASU64(queue_pfn) * ASU64(page_size); }

    uint64_t read_disk(uint64_t addr) const { return ASU64(disk[addr]); }
    void write_disk(uint64_t addr, uint64_t value) { disk[addr] = ASU8(value); }

    /// Access the disk via virtio. This is an associated function which takes a `cpu` object to
    /// read and write with a dram directly (DMA).
    void disk_access(Cpu* cpu);

    uint64_t id;
    uint32_t driver_features;
    uint32_t page_size;
    uint32_t queue_sel;
    uint32_t queue_num;
    uint32_t queue_pfn;
    uint32_t queue_notify;
    uint32_t status;
    std::vector<uint8_t> disk;
};


