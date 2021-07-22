#include "Uart.h"
#include "Trap.h"

#include <iostream>
#include <chrono>

#ifdef WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
int getch(void)
{
    return getchar();
}
#endif

//------------------------------------------------------------------------------
Uart::Uart(bool useConsole) :
    interrupting(false),
    uart(UART_SIZE,0),
    quitThread(false),
    myUseConsole(useConsole),
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

//! return next char available in queue
//------------------------------------------------------------------------------
char Uart::getChar()
{
    if (myUseConsole)
        return 0;

    char res = 0;
    outputMutex.lock();
    if (!output.empty())
    {
        res = output.front();
        output.pop_front();
    }
    outputMutex.unlock();

    return res;
}

//! Post char to in port
//------------------------------------------------------------------------------
void Uart::putChar(char c)
{
    if (myUseConsole)
        return;

    inputMutex.lock();
    input.push_back(c);
    inputMutex.unlock();
}


//------------------------------------------------------------------------------
void Uart::threadFunc()
{
    while (!quitThread)
    {
        char key = 0;
        if (myUseConsole)
        {
#ifdef WIN32
            key = _getch();
#else
            key = getch();
#endif
        }
        else
        {
            inputMutex.lock();
            if (!input.empty())
            {
                key = input.front();
                input.pop_front();
            }
            inputMutex.unlock();
        }

        if (key != 0)
        {
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
                interrupting = true;
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
    {
        if (myUseConsole)
            std::cout << ASU8(value);
        else
        {
            outputMutex.lock();
            output.push_back(ASU8(value));
            outputMutex.unlock();
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(uartMutex);
        uart[addr] = ASU8(value);
    }
}
