//

.globl put32
put32:
	str w1,[x0]
	ret

.globl get32
get32:
	ldr w0,[x0]
	ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret

.globl memcpy
memcpy:
	ldr x3, [x0], #8
	str x3, [x1], #8
	subs x2, x2, #8
	b.gt memcpy
	ret

.globl memzero
memzero:
	str xzr, [x0], #8
	subs x1, x1, #8
	b.gt memzero
	ret

.globl get_el
get_el:
	mrs x0, CurrentEL
	lsr x0, x0, #2
	ret

.globl set_irq_vector
set_irq_vector:
    // set vector address in EL1.
    ldr x0, =vectors
    msr vbar_el1, x0 
    ret

.globl enable_irq
enable_irq:
    msr daifclr, #2
    ret

.globl disable_irq
disable_irq:
    msr daifset, #2
    ret

