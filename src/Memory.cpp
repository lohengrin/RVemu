#include "Memory.h"
#include "Trap.h"

#include <iostream>
#include <iomanip>
#include <string.h>
#include <fstream>

Memory::Memory(size_t size):
	dram(size, 0x0)
{
}

Memory::~Memory()
{
}

//! Preload memory
bool Memory::preload(const std::string& file)
{
    // Load program
    std::ifstream program(file, std::ios::in | std::ios::binary | std::ios::ate);
    if (!program.is_open())
    {
        std::cerr << "Unable to read: " << file << std::endl;
        return false;
    }

    size_t fsize = program.tellg();
    program.seekg(0, std::ios::beg);
    program.read((char*) &dram[0], std::min(fsize, dram.size()));

    return true;
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
    return ASU64(dram[addr]);
}

/// Load 2 bytes from the little-endian dram.
uint64_t Memory::load16(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    return ASU64(*((uint16_t*)&dram[addr]));
#elif BYTE_ORDER==BIG_ENDIAN
    return ASU64(dram[addr])
        | ASU64(dram[addr + 1]) << 8;
#endif
}

/// Load 4 bytes from the little-endian dram.
uint64_t Memory::load32(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    return ASU64(*((uint32_t*)&dram[addr]));
#elif BYTE_ORDER==BIG_ENDIAN
    return ASU64(dram[addr])
        | ASU64(dram[addr + 1]) << 8
        | ASU64(dram[addr + 2]) << 16
        | ASU64(dram[addr + 3]) << 24;
#endif
}

/// Load 8 bytes from the little-endian dram.
uint64_t Memory::load64(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    return *((uint64_t*)&dram[addr]);
#elif BYTE_ORDER==BIG_ENDIAN
    return ASU64(dram[addr])
        | ASU64(dram[addr + 1]) << 8
        | ASU64(dram[addr + 2]) << 16
        | ASU64(dram[addr + 3]) << 24
        | ASU64(dram[addr + 4]) << 32
        | ASU64(dram[addr + 5]) << 40
        | ASU64(dram[addr + 6]) << 48
        | ASU64(dram[addr + 7]) << 56;
#endif
}

/// Store a byte to the little-endian dram.
void Memory::store8(uint64_t addr, uint64_t value) 
{
    dram[addr] = ASU8(value);
}

/// Store 2 bytes to the little-endian dram.
void Memory::store16(uint64_t addr, uint64_t value) 
{
#if BYTE_ORDER==LITTLE_ENDIAN
    * ((uint16_t*)&dram[addr]) = ASU16(value);
#elif BYTE_ORDER==BIG_ENDIAN
    dram[addr] = ASU8(value & 0xff);
    dram[addr + 1] = ASU8((value >> 8) & 0xff);
#endif
}

/// Store 4 bytes to the little-endian dram.
void Memory::store32(uint64_t addr, uint64_t value) 
{
#if BYTE_ORDER==LITTLE_ENDIAN
    * ((uint32_t*)&dram[addr]) = ASU32(value);
#elif BYTE_ORDER==BIG_ENDIAN
    dram[addr] = ASU8(value & 0xff);
    dram[addr + 1] = ASU8((value >> 8) & 0xff);
    dram[addr + 2] = ASU8((value >> 16) & 0xff);
    dram[addr + 3] = ASU8((value >> 24) & 0xff);
#endif
}

/// Store 8 bytes to the little-endian dram.
void Memory::store64(uint64_t addr, uint64_t value)
{
#if BYTE_ORDER==LITTLE_ENDIAN
    * ((uint64_t*)&dram[addr]) = ASU64(value);
#elif BYTE_ORDER==BIG_ENDIAN
    dram[addr] = ASU8(value & 0xff);
    dram[addr + 1] = ASU8((value >> 8) & 0xff);
    dram[addr + 2] = ASU8((value >> 16) & 0xff);
    dram[addr + 3] = ASU8((value >> 24) & 0xff);
    dram[addr + 4] = ASU8((value >> 32) & 0xff);
    dram[addr + 5] = ASU8((value >> 40) & 0xff);
    dram[addr + 6] = ASU8((value >> 48) & 0xff);
    dram[addr + 7] = ASU8((value >> 56) & 0xff);
#endif
}
