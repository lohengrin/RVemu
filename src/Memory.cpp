#include "Memory.h"
#include "Trap.h"

#include <iostream>
#include <iomanip>
#include <string.h>
#include <fstream>
#include <cstring>

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
    std::cout << "Loading as raw bin: " << file << std::endl;
    return loadbin(file);
}

// Load in raw binary format
bool Memory::loadbin(const std::string& file)     // Load program
{
    std::ifstream program(file, std::ios::in | std::ios::binary | std::ios::ate);
    if (!program.is_open())
    {
        std::cerr << "Unable to read: " << file << std::endl;
        return false;
    }

    size_t fsize = program.tellg();
    program.seekg(0, std::ios::beg);
    program.read((char*)&dram[0], std::min(fsize, dram.size()));

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
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from little-endian dram to big-endian host
uint64_t Memory::load16(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    uint16_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    return ASU64(result);
#elif BYTE_ORDER==BIG_ENDIAN
    uint16_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    // Swap bytes to convert from little-endian dram to big-endian host
    result = ((result & 0xFF) << 8) | ((result >> 8) & 0xFF);
    return ASU64(result);
#endif
}

/// Load 4 bytes from the little-endian dram.
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from little-endian dram to big-endian host
uint64_t Memory::load32(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    uint32_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    return ASU64(result);
#elif BYTE_ORDER==BIG_ENDIAN
    uint32_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    // Swap bytes to convert from little-endian dram to big-endian host
    result = ((result & 0xFF) << 24) | 
             ((result & 0xFF00) << 8) | 
             ((result >> 8) & 0xFF00) | 
             ((result >> 24) & 0xFF);
    return ASU64(result);
#endif
}

/// Load 8 bytes from the little-endian dram.
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from little-endian dram to big-endian host
uint64_t Memory::load64(uint64_t addr) const
{
#if BYTE_ORDER==LITTLE_ENDIAN
    uint64_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    return result;
#elif BYTE_ORDER==BIG_ENDIAN
    uint64_t result;
    std::memcpy(&result, &dram[addr], sizeof(result));
    // Swap bytes to convert from little-endian dram to big-endian host
    result = ((result & 0xFF) << 56) | 
             ((result & 0xFF00ULL) << 40) | 
             ((result & 0xFF0000ULL) << 24) | 
             ((result & 0xFF000000ULL) << 8) | 
             ((result >> 8) & 0xFF000000ULL) | 
             ((result >> 24) & 0xFF0000ULL) | 
             ((result >> 40) & 0xFF00ULL) | 
             ((result >> 56) & 0xFF);
    return result;
#endif
}

/// Store a byte to the little-endian dram.
void Memory::store8(uint64_t addr, uint64_t value) 
{
    dram[addr] = ASU8(value);
}

/// Store 2 bytes to the little-endian dram.
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from big-endian host to little-endian dram
void Memory::store16(uint64_t addr, uint64_t value) 
{
#if BYTE_ORDER==LITTLE_ENDIAN
    uint16_t val = ASU16(value);
    std::memcpy(&dram[addr], &val, sizeof(val));
#elif BYTE_ORDER==BIG_ENDIAN
    uint16_t val = ASU16(value);
    // Swap bytes to convert from big-endian host to little-endian dram
    val = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
    std::memcpy(&dram[addr], &val, sizeof(val));
#endif
}

/// Store 4 bytes to the little-endian dram.
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from big-endian host to little-endian dram
void Memory::store32(uint64_t addr, uint64_t value) 
{
#if BYTE_ORDER==LITTLE_ENDIAN
    uint32_t val = ASU32(value);
    std::memcpy(&dram[addr], &val, sizeof(val));
#elif BYTE_ORDER==BIG_ENDIAN
    uint32_t val = ASU32(value);
    // Swap bytes to convert from big-endian host to little-endian dram
    val = ((val & 0xFF) << 24) | 
          ((val & 0xFF00) << 8) | 
          ((val >> 8) & 0xFF00) | 
          ((val >> 24) & 0xFF);
    std::memcpy(&dram[addr], &val, sizeof(val));
#endif
}

/// Store 8 bytes to the little-endian dram.
// The DRAM stores data in little-endian format regardless of host endianness.
// On little-endian hosts: direct copy works (endianness matches)
// On big-endian hosts: need to swap bytes to convert from big-endian host to little-endian dram
void Memory::store64(uint64_t addr, uint64_t value)
{
#if BYTE_ORDER==LITTLE_ENDIAN
    std::memcpy(&dram[addr], &value, sizeof(value));
#elif BYTE_ORDER==BIG_ENDIAN
    uint64_t val = value;
    // Swap bytes to convert from big-endian host to little-endian dram
    val = ((val & 0xFF) << 56) | 
          ((val & 0xFF00ULL) << 40) | 
          ((val & 0xFF0000ULL) << 24) | 
          ((val & 0xFF000000ULL) << 8) | 
          ((val >> 8) & 0xFF000000ULL) | 
          ((val >> 24) & 0xFF0000ULL) | 
          ((val >> 40) & 0xFF00ULL) | 
          ((val >> 56) & 0xFF);
    std::memcpy(&dram[addr], &val, sizeof(val));
#endif
}
