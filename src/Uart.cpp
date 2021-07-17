#include "Uart.h"
#include "Trap.h"

#include <iostream>
#include <chrono>

#ifdef WIN32
#include <io.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}
int getch(void)
{
    return getchar();
}
#endif

//------------------------------------------------------------------------------
Uart::Uart() :
    interrupting(false),
    uart(UART_SIZE,0),
    quitThread(false),
    uartThread(&Uart::threadFunc, this)
{
    {
        std::lock_guard<std::mutex> lock(uartMutex);
        uart[UART_LSR] |= UART_LSR_TX;
    }
}

//------------------------------------------------------------------------------
Uart::~Uart()
{
    quitThread = true;
    uartThread.join();
}

//------------------------------------------------------------------------------
void Uart::threadFunc()
{
    while (!quitThread)
    {
        if (kbhit())
        {
            uint8_t key = getch();
            //  Wait for previous data to be read
            bool ready = false;
            while (!ready)
            {
                {
                    std::lock_guard<std::mutex> lock(uartMutex);
                    ready = (uart[UART_LSR] & UART_LSR_RX) == 0;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            // Read the char
            {
                std::lock_guard<std::mutex> lock(uartMutex);
                uart[0] = key;
                uart[UART_LSR] |= UART_LSR_RX;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//------------------------------------------------------------------------------
uint64_t Uart::load(uint64_t addr, uint8_t size) const
{
    if (size == 8)
        return load8(addr);
    else
        throw CpuException(Except::LoadAccessFault);
}

//------------------------------------------------------------------------------
void Uart::store(uint64_t addr, uint8_t size, uint64_t value)
{
    if (size == 8)
        store8(addr, value);
    else
        throw CpuException(Except::StoreAMOAccessFault);
}

//------------------------------------------------------------------------------
uint64_t Uart::load8(uint64_t addr) const
{
    std::lock_guard<std::mutex> lock(uartMutex);
    if (addr == UART_RHR)
    {
        uart[UART_LSR] &= ~UART_LSR_RX;
        return ASU64(uart[UART_RHR]);
    }
    else
        return uart[addr];
}

//------------------------------------------------------------------------------
void Uart::store8(uint64_t addr, uint64_t value)
{
    if (addr == UART_THR)
        std::cout << ASU8(value);
    else
    {
        std::lock_guard<std::mutex> lock(uartMutex);
        uart[addr] = ASU8(value);
    }
}
