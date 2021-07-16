#include "Memory.h"
#include "Trap.h"

#include <iostream>

Memory::Memory(size_t size):
	dram(size, 0x0)
{
}

Memory::~Memory()
{
}

//! Preload memory
void Memory::preload(size_t addr, uint8_t* data, size_t size)
{
	memcpy(&dram[0], data, std::min(size, dram.size() - addr));
}

/// Load bytes from the little-endiam dram.
uint64_t Memory::load(uint64_t addr, uint8_t size) const
{
    if (addr + (size / 8) > dram.size())
        throw CpuException(Except::LoadAccessFault);

    switch (size)
    {
    case 8: return load8(addr);
    case 16: return load16(addr);
    case 32: return load32(addr);
    case 64: return load64(addr);
    default:
        std::cerr << "Memory::LOAD error: unknown size: " << size << std::endl;
        throw CpuException(Except::LoadAccessFault);
    }
}

/// Store bytes to the little-endiam dram.
void Memory::store(uint64_t addr, uint8_t size, uint64_t value)
{
    if (addr + (size/8) > dram.size())
        throw CpuException(Except::StoreAMOAccessFault);

    switch(size)
    {
    case 8: store8(addr, value); break;
    case 16: store16(addr, value); break;
    case 32: store32(addr, value); break;
    case 64: store64(addr, value); break;
    default:
        std::cerr << "Memory::STORE error: unknown size: " << size << std::endl;
        throw CpuException(Except::StoreAMOAccessFault);
    }
}

/// Load a byte from the little-endian dram.
uint64_t Memory::load8(uint64_t addr) const
{
    return static_cast<uint64_t>(dram[addr]);
}

/// Load 2 bytes from the little-endian dram.
uint64_t Memory::load16(uint64_t addr) const
{
    return static_cast<uint64_t>(dram[addr]) | static_cast<uint64_t>(dram[addr + 1]) << 8;
}

/// Load 4 bytes from the little-endian dram.
uint64_t Memory::load32(uint64_t addr) const
{
    return static_cast<uint64_t>(dram[addr])
        | static_cast<uint64_t>(dram[addr + 1]) << 8
        | static_cast<uint64_t>(dram[addr + 2]) << 16
        | static_cast<uint64_t>(dram[addr + 3]) << 24;
}

/// Load 8 bytes from the little-endian dram.
uint64_t Memory::load64(uint64_t addr) const
{
    return static_cast<uint64_t>(dram[addr])
        | static_cast<uint64_t>(dram[addr + 1]) << 8
        | static_cast<uint64_t>(dram[addr + 2]) << 16
        | static_cast<uint64_t>(dram[addr + 3]) << 24
        | static_cast<uint64_t>(dram[addr + 4]) << 32
        | static_cast<uint64_t>(dram[addr + 5]) << 40
        | static_cast<uint64_t>(dram[addr + 6]) << 48
        | static_cast<uint64_t>(dram[addr + 7]) << 56;
}

/// Store a byte to the little-endian dram.
void Memory::store8(uint64_t addr, uint64_t value) 
{
    dram[addr] = static_cast<uint8_t>(value);
}

/// Store 2 bytes to the little-endian dram.
void Memory::store16(uint64_t addr, uint64_t value) 
{
    dram[addr] = static_cast<uint8_t>(value & 0xff);
    dram[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xff);
}

/// Store 4 bytes to the little-endian dram.
void Memory::store32(uint64_t addr, uint64_t value) 
{
    dram[addr] = static_cast<uint8_t>(value & 0xff);
    dram[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xff);
    dram[addr + 2] = static_cast<uint8_t>((value >> 16) & 0xff);
    dram[addr + 3] = static_cast<uint8_t>((value >> 24) & 0xff);
}

/// Store 8 bytes to the little-endian dram.
void Memory::store64(uint64_t addr, uint64_t value)
{
    dram[addr] = static_cast<uint8_t>(value & 0xff);
    dram[addr + 1] = static_cast<uint8_t>((value >> 8) & 0xff);
    dram[addr + 2] = static_cast<uint8_t>((value >> 16) & 0xff);
    dram[addr + 3] = static_cast<uint8_t>((value >> 24) & 0xff);
    dram[addr + 4] = static_cast<uint8_t>((value >> 32) & 0xff);
    dram[addr + 5] = static_cast<uint8_t>((value >> 40) & 0xff);
    dram[addr + 6] = static_cast<uint8_t>((value >> 48) & 0xff);
    dram[addr + 7] = static_cast<uint8_t>((value >> 56) & 0xff);
}