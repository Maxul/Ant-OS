#include <stddef.h>
#include <stdint.h>

#include "uart.h"
#if 0
void uart_init(void)
{
	unsigned int ra;

	put32(AUX_ENABLES, 1);
	put32(AUX_MU_IER_REG, 0);
	put32(AUX_MU_CNTL_REG, 0);
	put32(AUX_MU_LCR_REG, 3);
	put32(AUX_MU_MCR_REG, 0);
	put32(AUX_MU_IER_REG, 0);
	put32(AUX_MU_IIR_REG, 0xC6);
	put32(AUX_MU_BAUD_REG, 270);
	ra = get32(GPFSEL1);
	ra &= ~(7 << 12);	//gpio14
	ra |= 2 << 12;		//alt5
	ra &= ~(7 << 15);	//gpio15
	ra |= 2 << 15;		//alt5
	put32(GPFSEL1, ra);
	put32(GPPUD, 0);
	delay(150);
	put32(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);
	put32(GPPUDCLK0, 0);
	put32(AUX_MU_CNTL_REG, 3);
}

unsigned int uart_lsr(void)
{
	return get32(AUX_MU_LSR_REG);
}

unsigned int uart_recv(void)
{
	while (1) {
		if (uart_lsr() & 0x01) {
			break;
		}
	}
	return get32(AUX_MU_IO_REG) & 0xFF;
}

void uart_send(unsigned int c)
{
	while (1) {
		if (uart_lsr() & 0x20) {
			break;
		}
	}
	put32(AUX_MU_IO_REG, c);
}

void uart_send_string(char *str)
{
	for (int i = 0; str[i] != '\0'; i++) {
		uart_send((char)str[i]);
	}
}

void uart_putc(char c)
{
	uart_send(c);
}
#endif

#define UART0_DR   ((volatile uint32_t *)(0x3F201000))
#define UART0_FR   ((volatile uint32_t *)(0x3F201018))
#define UART0_IMSC ((volatile uint32_t *)(0x3F201038))
#define UART0_MIS  ((volatile uint32_t *)(0x3F201040))

void uart_putc(char c)
{
    // Wait for UART to become ready to transmit.
    while (*UART0_FR & (1 << 5)) { }
    *UART0_DR = c;
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

