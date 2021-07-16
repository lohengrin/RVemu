#pragma once

#include "Device.h"

//! The clint module contains the core-local interruptor (CLINT). The CLINT
//! block holds memory-mapped control and status registers associated with
//! software and timer interrupts. It generates per-hart software interrupts and timer.

/// The address of a mtimecmp register starts. A mtimecmp is a dram mapped machine mode timer
/// compare register, used to trigger an interrupt when mtimecmp is greater than or equal to mtime.
const uint64_t CLINT_MTIMECMP = 0x4000;
/// The address of a timer register. A mtime is a machine mode timer register which runs at a
/// constant frequency.
const uint64_t CLINT_MTIME = 0xbff8;

/// The size of CLINT.
const uint64_t CLINT_SIZE = 0x10000;

/// The core-local interruptor (CLINT).
struct Clint : public Device
{
public:
    Clint();
    virtual ~Clint();

    //! Device Interface
    //!load
    uint64_t load(uint64_t addr, uint8_t size) const;
    //! store
    void store(uint64_t addr, uint8_t size, uint64_t value);
    //! Get address space size of device
    uint64_t size() const { return CLINT_SIZE; }

protected:
    uint64_t load64(uint64_t addr) const;
    void store64(uint64_t addr, uint64_t value);

    uint64_t mtime;
    uint64_t mtimecmp;
};


