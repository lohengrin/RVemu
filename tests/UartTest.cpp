#include "Uart.h"
#include "Trap.h"

#include <gtest/gtest.h>

#include <cstdint>

// NOTE: Uart(false) is mandatory in unit tests. The default constructor spawns
// a background thread that blocks on stdin when useConsole == true, which would
// hang the test process. With useConsole == false the thread only drains an
// in-memory input queue and is safe to construct/destroy in a test.

// On construction the LSR "transmitter empty" bit is set, so the CPU can
// immediately push the next character.
TEST(UartTest, LsrTxReadyOnConstruction)
{
	Uart uart(false);
	EXPECT_NE(uart.load(UART_LSR, 8) & UART_LSR_TX, 0u);
}

// A store to THR in non-console mode buffers the byte for later retrieval via
// getChar() (the deterministic output path, independent of the RX thread).
TEST(UartTest, TransmitHoldingRegisterBuffersOutput)
{
	Uart uart(false);
	uart.store(UART_THR, 8, 'H');
	uart.store(UART_THR, 8, 'i');
	EXPECT_EQ(uart.getChar(), 'H');
	EXPECT_EQ(uart.getChar(), 'i');
}

// Generic registers (e.g. LCR) are plain read/write storage.
TEST(UartTest, LineControlRegisterRoundTrip)
{
	Uart uart(false);
	uart.store(UART_LCR, 8, 0x03);
	EXPECT_EQ(uart.load(UART_LCR, 8), 0x03u);
}

// UART only accepts byte accesses.
TEST(UartTest, RejectsNonByteAccess)
{
	Uart uart(false);
	EXPECT_THROW(uart.load(UART_LSR, 32), CpuException);
	EXPECT_THROW(uart.store(UART_THR, 16, 0), CpuException);
}
