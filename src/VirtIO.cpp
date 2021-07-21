#include "VirtIO.h"
#include "Trap.h"
#include "Cpu.h"

#include <fstream>
#include <iostream>

//------------------------------------------------------------------------------
VirtIO::VirtIO() :
    id(0), driver_features(0), page_size(0),
    queue_sel(0), queue_num(0), queue_pfn(0),
    queue_notify(9999), status(0)
{
}

//------------------------------------------------------------------------------
VirtIO::~VirtIO()
{
}

//------------------------------------------------------------------------------
bool VirtIO::loadDisk(const std::string& imageFile)
{
    // Load program
    std::ifstream image(imageFile, std::ios::in | std::ios::binary | std::ios::ate);
    if (!image.is_open())
    {
        std::cerr << "Unable to read: " << imageFile << std::endl;
        return false;
    }

    size_t fsize = image.tellg();
    image.seekg(0, std::ios::beg);

    disk.resize(fsize, 0);

    image.read((char*)&disk[0], std::min(fsize, disk.size()));

    return true;
}


//------------------------------------------------------------------------------
uint64_t VirtIO::load(uint64_t addr, uint8_t size) const
{
    if (size == 32)
        return load32(addr);
    else
        throw CpuException(Except::LoadAccessFault);
}

//------------------------------------------------------------------------------
void VirtIO::store(uint64_t addr, uint8_t size, uint64_t value)
{
    if (size == 32)
        store32(addr, value);
    else
        throw CpuException(Except::StoreAMOAccessFault);
}

//------------------------------------------------------------------------------
uint64_t VirtIO::load32(uint64_t addr) const
{
    switch (addr)
    {
    case VIRTIO_MAGIC: return 0x74726976;
    case VIRTIO_VERSION: return 0x1;
    case VIRTIO_DEVICE_ID: return 0x2;
    case VIRTIO_VENDOR_ID: return 0x554d4551;
    case VIRTIO_DEVICE_FEATURES: return 0; // TODO: what should it return?
    case VIRTIO_DRIVER_FEATURES: return ASU64(driver_features);
    case VIRTIO_QUEUE_NUM_MAX: return 8;
    case VIRTIO_QUEUE_PFN: return ASU64(queue_pfn);
    case VIRTIO_STATUS: return ASU64(status);
    default: return 0;
    }
}
 
//------------------------------------------------------------------------------
void VirtIO::store32(uint64_t addr, uint64_t value)
{
    switch (addr)
    {
    case VIRTIO_DEVICE_FEATURES: driver_features = ASU32(value); break;
    case VIRTIO_GUEST_PAGE_SIZE: page_size = ASU32(value); break;
    case VIRTIO_QUEUE_SEL: queue_sel = ASU32(value); break;
    case VIRTIO_QUEUE_NUM: queue_num = ASU32(value); break;
    case VIRTIO_QUEUE_PFN: queue_pfn = ASU32(value); break;
    case VIRTIO_QUEUE_NOTIFY: queue_notify = ASU32(value); break;
    case VIRTIO_STATUS: status = ASU32(value); break;
    }
}

/// Access the disk via virtio. This is an associated function which takes a `cpu` object to
/// read and write with a dram directly (DMA).
void VirtIO::disk_access(Cpu * cpu)
{
    // See more information in
    // https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/virtio_disk.c

    // the spec says that legacy block operations use three
    // descriptors: one for type/reserved/sector, one for
    // the data, one for a 1-byte status result.

    // desc = pages -- num * VRingDesc
    // avail = pages + 0x40 -- 2 * uint16, then num * uint16
    // used = pages + 4096 -- 2 * uint16, then num * vRingUsedElem
    uint64_t desc_add = desc_addr();
    uint64_t avail_add = desc_addr() + 0x40;
    uint64_t used_add = desc_addr() + 4096;

    // avail[0] is flags
    // avail[1] tells the device how far to look in avail[2...].
    uint64_t offset = cpu->bus.load(avail_add + 1, 16); //.wrapping_add(1)
                                                           // .expect("failed to read offset");
    // avail[2...] are desc[] indices the device should process.
    // we only tell device the first index in our chain of descriptors.
    uint64_t index = cpu->bus.load(avail_add + (offset % DESC_NUM) + 2, 16);
        //.expect("failed to read index");

    // Read `VRingDesc`, virtio descriptors.
    uint64_t desc_addr0 = desc_add + VRING_DESC_SIZE * index;
    uint64_t addr0 = cpu->bus.load(desc_addr0, 64);
        //.expect("failed to read an address field in a descriptor");
    // Add 14 because of `VRingDesc` structure.
    // struct VRingDesc {
    //   uint64 addr;
    //   uint32 len;
    //   uint16 flags;
    //   uint16 next
    // };
    // The `next` field can be accessed by offset 14 (8 + 4 + 2) bytes.
    uint64_t next0 = cpu->bus.load(desc_addr0 + 14, 16);
        //.expect("failed to read a next field in a descripor");

    // Read `VRingDesc` again, virtio descriptors.
    uint64_t desc_addr1 = desc_add + VRING_DESC_SIZE * next0;
    uint64_t addr1 = cpu->bus.load(desc_addr1, 64);
        //.expect("failed to read an address field in a descriptor");
    uint64_t len1 = cpu->bus.load(desc_addr1 + 8, 32);
        //.expect("failed to read a length field in a descriptor");
    uint64_t flags1 = cpu->bus.load(desc_addr1 + 12, 16);
        //.expect("failed to read a flags field in a descriptor");

    // Read `virtio_blk_outhdr`. Add 8 because of its structure.
    // struct virtio_blk_outhdr {
    //   uint32 type;
    //   uint32 reserved;
    //   uint64 sector;
    // } buf0;
    uint64_t blk_sector = cpu->bus.load(addr0 + 8, 64);
        //.expect("failed to read a sector field in a virtio_blk_outhdr");

    // Write to a device if the second bit `flag1` is set.
    if ((flags1 & 2) == 0)
    {
        // Read dram data and write it to a disk directly (DMA).
        for (uint64_t i = 0; i < len1; i++)
        {
            uint64_t data = cpu->bus.load(addr1 + i, 8);
                //.expect("failed to read from dram");
            write_disk(blk_sector * 512 + i, data);
        }
    }
    else
    {
        // Read disk data and write it to dram directly (DMA).
        for (uint64_t i = 0; i < len1; i++)
        {
            uint64_t data = read_disk(blk_sector * 512 + i);
            cpu->bus.store(addr1 + i, 8, data);
                //.expect("failed to write to dram");
        }
    }

    // Write id to `UsedArea`. Add 2 because of its structure.
    // struct UsedArea {
    //   uint16 flags;
    //   uint16 id;
    //   struct VRingUsedElem elems[NUM];
    // };
    uint64_t new_id = get_new_id();
    cpu->bus.store(used_add + 2, 16, new_id % 8);
        //.expect("failed to write to dram");
}

