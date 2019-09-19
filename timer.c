#include <stddef.h>
#include <stdint.h>

#define TICKS_PER_SECOND (25)

extern void enable_irq(void);
extern void disable_irq(void);

static uint32_t cntfrq = 0;

static inline void enable_cntv(void)
{
    asm volatile("msr cntv_ctl_el0, %0" ::"r"(1));
}

static inline void disable_cntv(void)
{
    asm volatile("msr cntv_ctl_el0, %0" ::"r"(0));
}

static inline uint64_t read_cntvct(void)
{
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r"(val));
    return (val);
}

static inline uint32_t read_cntv_tval(void)
{
    uint32_t val;
    asm volatile("mrs %0, cntv_tval_el0" : "=r"(val));
    return val;
}

static inline void write_cntv_tval(uint32_t val)
{
    asm volatile("msr cntv_tval_el0, %0" ::"r"(val));
    return;
}

uint32_t read_cntfrq(void)
{
    uint32_t val;
    asm volatile("mrs %0, cntfrq_el0" : "=r"(val));
    return val;
}

#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t*)(0x40000060))


#define UART0_DR   ((volatile uint32_t *)(0x3F201000))
#define UART0_FR   ((volatile uint32_t *)(0x3F201018))
#define UART0_IMSC ((volatile uint32_t *)(0x3F201038))
#define UART0_MIS  ((volatile uint32_t *)(0x3F201040))

#define IRQ_BASIC   ((volatile uint32_t *)(0x3F00B200))
#define IRQ_PEND2   ((volatile uint32_t *)(0x3F00B208))
#define IRQ_ENABLE2 ((volatile uint32_t *)(0x3F00B214))
#define GPU_INTERRUPTS_ROUTING ((volatile uint32_t *)(0x4000000C))
#define CORE0_INTERRUPT_SOURCE ((volatile uint32_t *)(0x40000060))

/* ARM Generic Timer */
#define CORE0_TIMER_IRQCNTL    ((volatile uint32_t *)(0x40000040))

///
#define GPFSEL1     ((volatile uint32_t *)(0x3f200004))
#define GPPUDCLK0   ((volatile uint32_t *)(0x3f200098))

#define AUX_ENABLES	((volatile uint32_t *)(0x3F215004))
#define AUX_MU_IO	((volatile uint32_t *)(0x3F215040))
#define AUX_MU_IER	((volatile uint32_t *)(0x3F215044))
#define AUX_MU_IIR	((volatile uint32_t *)(0x3F215048))
#define AUX_MU_LCR	((volatile uint32_t *)(0x3F215048))
#define AUX_MU_LSR	((volatile uint32_t *)(0x3F215054))
#define AUX_MU_BAUD	((volatile uint32_t *)(0x3F215068))

typedef void (*INTERRUPT_HANDLER) (void);
typedef struct {
	INTERRUPT_HANDLER fn;
} INTERRUPT_VECTOR;

static INTERRUPT_VECTOR g_vector_table[64];

#define IRQ_ENABLE_1		((volatile uint32_t *)(0x3F00B210))
static void uart_isr_register(void (*fn)(void))
{
	g_vector_table[57].fn = fn;

    // enable UART RX interrupt.
    *UART0_IMSC = 1 << 4;
    // UART interrupt routing.
    *IRQ_ENABLE2 = 1 << 25;
    // IRQ routeing to CORE0.
    *GPU_INTERRUPTS_ROUTING = 0x00;

}
/*-----------------------------------------------------------*/

void uart_isr(void)
{
    if (*UART0_MIS & (1 << 4)) {
        printf("%c", (unsigned char) *UART0_DR); // read for clear tx interrupt.
    }
}

#define IRQ_BASIC_PENDING	((volatile uint32_t *)(0x3F00B200))
#define IRQ_PENDING_1		((volatile uint32_t *)(0x3F00B204))
#define IRQ_PENDING_2		((volatile uint32_t *)(0x3F00B208))

static void handle_range(uint32_t pending, const uint32_t base)
{
	while (pending) {
		/* get index of first set_bit */
		uint32_t bit = 31 - __builtin_clz(pending);
		uint32_t irq = base + bit;
		
		/* call handler */
		if(g_vector_table[irq].fn)
			g_vector_table[irq].fn();

		/* clear bit */
		pending &= ~(1UL << bit);
	}
}

void irq_handler(void)
{
	uint32_t basic = *IRQ_BASIC_PENDING & 0x00000300;

	if (basic & 0x100) {
		handle_range(*IRQ_PENDING_1, 0);
    }
	if (basic & 0x200) {
		handle_range(*IRQ_PENDING_2, 32);
    }
}

///

void set_irq_vector(void);

void interrupt_init(void)
{
    set_irq_vector();

    /* init uart */
    uart_isr_register(uart_isr);

    /* init timer */
    cntfrq = read_cntfrq();
    write_cntv_tval(cntfrq / TICKS_PER_SECOND); // clear cntv interrupt and set next 1 sec timer.

	/* timer interrupt routing. */
	*CORE0_TIMER_IRQCNTL = 1 << 3; /* nCNTVIRQ routing to CORE0.*/

	/* start & enable interrupts in the timer. */
	enable_cntv();
}

void c_irq_handler(void)
{
    disable_irq();

	uint32_t InterruptID;
	InterruptID = (*CORE0_INTERRUPT_SOURCE) & 0x0007FFFFUL;

    // check timer
    if (InterruptID & (1 << 3)) {
        /* Generic Timer */
        write_cntv_tval(cntfrq / TICKS_PER_SECOND); // clear cntv interrupt and set next 1sec timer.
        timer_tick();
    }

    // check interrupt source
    if (InterruptID & (1 << 8)) {
        /* Peripherals */
        irq_handler();
    }

    enable_irq();
    return;
}

