#pragma once

#include "Device.h"

//! The plic module contains the platform-level interrupt controller (PLIC).
//! The plic connects all external interrupts in the system to all hart
//! contexts in the system, via the external interrupt source in each hart.
//! It's the global interrupt controller in a RISC-V system.

/// The address of interrupt pending bits.
const uint64_t PLIC_PENDING = 0x1000;
/// The address of the regsiters to enable interrupts for S-mode.
const uint64_t PLIC_SENABLE = 0x2080;
/// The address of the registers to set a priority for S-mode.
const uint64_t PLIC_SPRIORITY = 0x201000;
/// The address of the claim/complete registers for S-mode.
const uint64_t PLIC_SCLAIM = 0x201004;

/// The size of PLIC.
const uint64_t PLIC_SIZE = 0x4000000;

/// The platform-level-interrupt controller (PLIC).
class Plic : public Device
{
public:
    Plic();
    virtual ~Plic();

    //! Device Interface
    //!load
    uint64_t load(uint64_t addr, uint8_t size) const;
    //! store
    void store(uint64_t addr, uint8_t size, uint64_t value);
    //! Get address space size of device
    uint64_t size() const { return PLIC_SIZE; }

protected:
    uint64_t load32(uint64_t addr) const;
    void store32(uint64_t addr, uint64_t value);

    uint64_t pending;
    uint64_t senable;
    uint64_t spriority;
    uint64_t sclaim;
};


