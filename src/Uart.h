#pragma once
//! The uart module contains the implementation of a universal asynchronous receiver-transmitter
//! (UART). The device is 16550a UART, which is used in the QEMU virt machine.
//! See the spec: http://byterunner.com/16550.html
//! 
#include "Device.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <deque>

/// The interrupt request of UART.
const uint64_t UART_IRQ = 10;

/// Receive holding register (for input bytes).
const uint64_t UART_RHR = 0;
/// Transmit holding register (for output bytes).
const uint64_t UART_THR = 0;
/// Line control register.
const uint64_t UART_LCR = 3;
/// Line status register.
/// LSR BIT 0:
///     0 = no data in receive holding register or FIFO.
///     1 = data has been receive and saved in the receive holding register or FIFO.
/// LSR BIT 5:
///     0 = transmit holding register is full. 16550 will not accept any data for transmission.
///     1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
const uint64_t UART_LSR = 5;

/// The receiver (RX) bit.
const uint8_t UART_LSR_RX = 1;
/// The transmitter (TX) bit.
const uint8_t UART_LSR_TX = 1 << 5;

/// The size of UART.
const uint64_t UART_SIZE = 0x100;

class Uart : public Device
{
public:
    Uart(bool useConsole = true);
    virtual ~Uart();

    //! Device Interface
    //!load
    uint64_t load(uint64_t addr, uint8_t size) const;
    //! store
    void store(uint64_t addr, uint8_t size, uint64_t value);
    //! Get address space size of device
    uint64_t size() const { return UART_SIZE; }

    //! return next char available in queue
    char getChar();
    //! Post char to in port
    void putChar(char c);

    bool is_interrupting() {
        if (interrupting) { interrupting = false; return true; }
        return false;
    }

protected:
    uint64_t load8(uint64_t addr) const;
    void store8(uint64_t addr, uint64_t value);

    // Flag to use console or custom terminal
    bool myUseConsole;

    // Uart buffer
    mutable std::vector<uint8_t> uart;

    // Uart thread stuff
    std::atomic<bool> quitThread;
    std::thread uartThread;
    void threadFunc();
    mutable std::mutex uartMutex;

    // FIFO for custom terminal
    mutable std::mutex inputMutex;
    std::deque<char> input;

    mutable std::mutex outputMutex;
    std::deque<char> output;

    /// Bit if an interrupt happens.
    std::atomic<bool> interrupting;
};


