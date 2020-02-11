#include <stddef.h>
#include <stdint.h>

#include "uart.h"

void uart_init(void)
{
	unsigned long base = PL011_UART6_BASE;

	/* Clear all errors */
	put32(base + UART_RSR_ECR, 0);
	/* Disable everything */
	put32(base + UART_CR, 0);
	if (CONSOLE_BAUDRATE) {
		unsigned int divisor =
		    (CONSOLE_UART_CLK_IN_HZ * 4) / CONSOLE_BAUDRATE;
		put32(base + UART_IBRD, divisor >> 6);
		put32(base + UART_FBRD, divisor & 0x3f);
	}
	/* Configure TX to 8 bits, 1 stop bit, no parity, fifo disabled. */
	put32(base + UART_LCR_H, UART_LCRH_WLEN_8);

	/* Enable interrupts for receive and receive timeout */
	put32(base + UART_IMSC, UART_IMSC_RXIM | UART_IMSC_RTIM);

	/* Enable UART and RX/TX */
	put32(base + UART_CR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}

void uart_putc(char ch)
{
	/* Wait until there is space in the FIFO or device is disabled */
	while (get32((unsigned long)PL011_UART6_BASE + UART_FR)
	       & UART_FR_TXFF) {
	}
	/* Send the character */
	put32((unsigned long)PL011_UART6_BASE + UART_DR, (unsigned int)ch);
}

void uart_puts(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
        uart_putc((unsigned char)str[i]);
}

void uart_puthex(uint64_t n)
{
    const char* hexdigits = "0123456789ABSDEF";
    for (int i = 60; i >= 0; i -= 4)
        uart_putc(hexdigits[(n >> i) & 0xf]);
}

void uart_dump(void* ptr)
{
    uint64_t a, b, d;
    uint8_t c;
    for (a = (uint64_t)ptr; a < (uint64_t)ptr + 256 + 16; a += 16) {
        uart_puthex(a);
        uart_puts(": ");
        for (b = 0; b < 16; b++) {
            c = *((uint8_t*)(a + b));
            d = (uint64_t)c;
            d >>= 4;
            d &= 0xF;
            d += d > 9 ? 0x37 : 0x30;
            uart_putc(d);
            d = (uint64_t)c;
            d &= 0xF;
            d += d > 9 ? 0x37 : 0x30;
            uart_putc(d);
            uart_putc(' ');
            if (b % 4 == 3)
                uart_putc(' ');
        }
        for (b = 0; b < 16; b++) {
            c = *((uint8_t*)(a + b));
            uart_putc(c < 32 || c >= 127 ? '.' : c);
        }
        uart_putc('\n');
    }
}

