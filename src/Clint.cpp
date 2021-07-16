#include "Clint.h"
#include "Trap.h"

//------------------------------------------------------------------------------
Clint::Clint() :
    mtime(0),
    mtimecmp(0)
{
}

//------------------------------------------------------------------------------
Clint::~Clint()
{
}

//------------------------------------------------------------------------------
uint64_t Clint::load(uint64_t addr, uint8_t size) const
{
    if (size == 64)
        return load64(addr);
    else
        throw CpuException(Except::LoadAccessFault);
}

//------------------------------------------------------------------------------
void Clint::store(uint64_t addr, uint8_t size, uint64_t value)
{
    if (size == 64)
        store64(addr, value);
    else
        throw CpuException(Except::StoreAMOAccessFault);
}

//------------------------------------------------------------------------------
uint64_t Clint::load64(uint64_t addr) const
{
    switch (addr)
    {
    case CLINT_MTIMECMP: return mtimecmp;
    case CLINT_MTIME: return mtime;
    default: return 0;
    }
}
 
//------------------------------------------------------------------------------
void Clint::store64(uint64_t addr, uint64_t value)
{
    switch (addr)
    {
    case CLINT_MTIMECMP: mtimecmp = value; break;
    case CLINT_MTIME: mtime = value; break;
    }
}
