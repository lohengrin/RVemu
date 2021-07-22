#include "Trap.h"
#include "Defines.h"
#include "Cpu.h"

/// Helper method for a trap handler.
void Trap::take_trap(Cpu* cpu, Except e, Interrupt i)
{
    auto exception_pc = cpu->pc - 4;
    auto previous_mode = cpu->mode;

    uint64_t cause = (uint64_t)-1;

    if (e != Except::InvalidExcept)
    {
        cause = (uint64_t)e;
    }
    else if (i != Interrupt::InvalidInterrupt)
    {
        cause = (uint64_t)i;
        cause = ((uint64_t)1 << 63) | cause;
    }
    else
        return;


    if ((previous_mode <= Cpu::Mode::Supervisor) &&
       (((cpu->warppingShr(cpu->load_csr(MEDELEG),cause)) & 1) != 0))
    {
        // Handle the trap in S-mode.
        cpu->mode = Cpu::Mode::Supervisor;

        // Set the program counter to the supervisor trap-handler base address (stvec).
        if (i != Interrupt::InvalidInterrupt)
        {
            uint64_t vector = 0;
            if ((cpu->load_csr(STVEC) & 1) == 1)
                vector = 4 * cause; // vectored mode
            else
                vector = 0; // direct mode
            
            cpu->pc = (cpu->load_csr(STVEC) & ~1) + vector;
        }
        else 
        {
            cpu->pc = cpu->load_csr(STVEC) & ~1;
        }

        // 4.1.9 Supervisor Exception Program Counter (sepc)
        // "The low bit of sepc (sepc[0]) is always zero."
        // "When a trap is taken into S-mode, sepc is written with the virtual address of
        // the instruction that was interrupted or that encountered the exception.
        // Otherwise, sepc is never written by the implementation, though it may be
        // explicitly written by software."
        cpu->store_csr(SEPC, exception_pc & ~1);

        // 4.1.10 Supervisor Cause Register (scause)
        // "When a trap is taken into S-mode, scause is written with a code indicating
        // the event that caused the trap.  Otherwise, scause is never written by the
        // implementation, though it may be explicitly written by software."
        cpu->store_csr(SCAUSE, cause);

        // 4.1.11 Supervisor Trap Value (stval) Register
        // "When a trap is taken into S-mode, stval is written with exception-specific
        // information to assist software in handling the trap. Otherwise, stval is never
        // written by the implementation, though it may be explicitly written by software."
        // "When a hardware breakpoint is triggered, or an instruction-fetch, load, or
        // store address-misaligned, access, or page-fault exception occurs, stval is
        // written with the faulting virtual address. On an illegal instruction trap,
        // stval may be written with the first XLEN or ILEN bits of the faulting
        // instruction as described below. For other exceptions, stval is set to zero."
        cpu->store_csr(STVAL, 0);

        // Set a privious interrupt-enable bit for supervisor mode (SPIE, 5) to the value
        // of a global interrupt-enable bit for supervisor mode (SIE, 1).
        if (((cpu->load_csr(SSTATUS) >> 1) & 1) == 1)
            cpu->store_csr(SSTATUS, cpu->load_csr(SSTATUS) | (1 << 5));
        else
            cpu->store_csr(SSTATUS, cpu->load_csr(SSTATUS) & ~(1 << 5));

        // Set a global interrupt-enable bit for supervisor mode (SIE, 1) to 0.
        cpu->store_csr(SSTATUS, cpu->load_csr(SSTATUS) & ~(1 << 1));
        // 4.1.1 Supervisor Status Register (sstatus)
        // "When a trap is taken, SPP is set to 0 if the trap originated from user mode, or
        // 1 otherwise."
        if (previous_mode == Cpu::Mode::User)
            cpu->store_csr(SSTATUS, cpu->load_csr(SSTATUS) & ~(1 << 8));
        else
            cpu->store_csr(SSTATUS, cpu->load_csr(SSTATUS) | (1 << 8));
    }
    else 
    {
        // Handle the trap in M-mode.
        cpu->mode = Cpu::Mode::Machine;

        // Set the program counter to the machine trap-handler base address (mtvec).
        if (i != Interrupt::InvalidInterrupt)
        {
            uint64_t vector = 0;
            if ((cpu->load_csr(MTVEC) & 1) == 1)
                vector = 4 * cause; // vectored mode
            else
                vector = 0; // direct mode

            cpu->pc = (cpu->load_csr(MTVEC) & ~1) + vector;
        }
        else
        {
            cpu->pc = cpu->load_csr(MTVEC) & ~1;
        }

        // 3.1.15 Machine Exception Program Counter (mepc)
        // "The low bit of mepc (mepc[0]) is always zero."
        // "When a trap is taken into M-mode, mepc is written with the virtual address of
        // the instruction that was interrupted or that encountered the exception.
        // Otherwise, mepc is never written by the implementation, though it may be
        // explicitly written by software."
        cpu->store_csr(MEPC, exception_pc & ~1);

        // 3.1.16 Machine Cause Register (mcause)
        // "When a trap is taken into M-mode, mcause is written with a code indicating
        // the event that caused the trap. Otherwise, mcause is never written by the
        // implementation, though it may be explicitly written by software."
        cpu->store_csr(MCAUSE, cause);

        // 3.1.17 Machine Trap Value (mtval) Register
        // "When a trap is taken into M-mode, mtval is either set to zero or written with
        // exception-specific information to assist software in handling the trap.
        // Otherwise, mtval is never written by the implementation, though it may be
        // explicitly written by software."
        // "When a hardware breakpoint is triggered, or an instruction-fetch, load, or
        // store address-misaligned, access, or page-fault exception occurs, mtval is
        // written with the faulting virtual address. On an illegal instruction trap,
        // mtval may be written with the first XLEN or ILEN bits of the faulting
        // instruction as described below. For other traps, mtval is set to zero."
        cpu->store_csr(MTVAL, 0);

        // Set a privious interrupt-enable bit for supervisor mode (MPIE, 7) to the value
        // of a global interrupt-enable bit for supervisor mode (MIE, 3).
        if (((cpu->load_csr(MSTATUS) >> 3) & 1) == 1) 
            cpu->store_csr(MSTATUS, cpu->load_csr(MSTATUS) | (1 << 7));
        else
            cpu->store_csr(MSTATUS, cpu->load_csr(MSTATUS) & ~(1 << 7));

        // Set a global interrupt-enable bit for supervisor mode (MIE, 3) to 0.
        cpu->store_csr(MSTATUS, cpu->load_csr(MSTATUS) & ~(1 << 3));
        // Set a privious privilege mode for supervisor mode (MPP, 11..13) to 0.
        cpu->store_csr(MSTATUS, cpu->load_csr(MSTATUS) & ~(0b11 << 11));
    }
}
