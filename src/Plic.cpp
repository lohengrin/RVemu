#include "Plic.h"
#include "Trap.h"

//------------------------------------------------------------------------------
Plic::Plic() :
    pending(0),
    senable(0),
    spriority(0),
    sclaim(0)
{
}

//------------------------------------------------------------------------------
Plic::~Plic()
{
}

//------------------------------------------------------------------------------
uint64_t Plic::load(uint64_t addr, uint8_t size) const
{
    if (size == 32)
        return load32(addr);
    else
        throw CpuException(Except::LoadAccessFault);
}

//------------------------------------------------------------------------------
void Plic::store(uint64_t addr, uint8_t size, uint64_t value)
{
    if (size == 32)
        store32(addr, value);
    else
        throw CpuException(Except::StoreAMOAccessFault);
}

//------------------------------------------------------------------------------
uint64_t Plic::load32(uint64_t addr) const
{
    switch (addr)
    {
    case PLIC_PENDING: return pending;
    case PLIC_SENABLE: return senable;
    case PLIC_SPRIORITY: return spriority;
    case PLIC_SCLAIM: return sclaim;
    default: return 0;
    }
}
 
//------------------------------------------------------------------------------
void Plic::store32(uint64_t addr, uint64_t value)
{
    switch (addr)
    {
    case PLIC_PENDING: pending = value; break;
    case PLIC_SENABLE: senable = value; break;
    case PLIC_SPRIORITY: spriority = value; break;
    case PLIC_SCLAIM: sclaim = value; break;
    }
}
