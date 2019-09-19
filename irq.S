#include "sysregs.h"
#include "entry.h"
#include "sys.h"

.globl hang
hang: b hang

.macro handler_entry el, type
    kernel_entry \el
    
    mov x0, #\type
    mrs x1, esr_el1
    mrs x2, elr_el1
    b hang
.endm

.macro	kernel_entry, el
	sub	sp, sp, #S_FRAME_SIZE
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	.if	\el == 0
	mrs	x21, sp_el0
	.else
	add	x21, sp, #S_FRAME_SIZE
	.endif /* \el == 0 */

	mrs	x22, elr_el1
	mrs	x23, spsr_el1

	stp	x30, x21, [sp, #16 * 15] 
	stp	x22, x23, [sp, #16 * 16]
.endm


.macro	kernel_exit, el
	ldp	x22, x23, [sp, #16 * 16]
	ldp	x30, x21, [sp, #16 * 15] 

	.if	\el == 0
	msr	sp_el0, x21
	.endif /* \el == 0 */

	msr	elr_el1, x22			
	msr	spsr_el1, x23

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	add	sp, sp, #S_FRAME_SIZE		
	eret
.endm


/*
 * Exception vectors.
 */

.macro vector_entry handler
    .align 7
    b \handler
.endm


.align	11
.globl vectors 
vectors:
	vector_entry	dummy_el1
	vector_entry	irq_el1
	vector_entry	dummy_el1
	vector_entry	dummy_el1

	vector_entry	dummy_el1
	vector_entry	irq_el1
	vector_entry	dummy_el1
	vector_entry	dummy_el1

    // 64-bit
	vector_entry	sync_el0
	vector_entry	irq_el0
	vector_entry	dummy_el0
	vector_entry	dummy_el0

    // 32-bit
	vector_entry	dummy_el0
	vector_entry	dummy_el0
	vector_entry	dummy_el0
	vector_entry	dummy_el0

dummy_el0:
    handler_entry 0, 0

dummy_el1:
    handler_entry 1, 0

irq_el1:
	kernel_entry 1 
	bl c_irq_handler
	kernel_exit 1 

irq_el0:
	kernel_entry 0 
	bl c_irq_handler
	kernel_exit 0 

sync_el0:
	kernel_entry 0
	mrs	x25, esr_el1				    // read the syndrome register
	lsr	x24, x25, #ESR_ELx_EC_SHIFT		// exception class
	cmp	x24, #ESR_ELx_EC_SVC64			// SVC in 64-bit state
	b.eq	svc_el0
	handler_entry 0, SYNC_ERROR

sc_nr	.req	x25					// number of system calls
scno	.req	x26					// syscall number
stbl	.req	x27					// syscall table pointer

svc_el0:
	adr	stbl, syscall_tbl			// load syscall table pointer
	uxtw	scno, w8				// syscall number in w8
	mov	sc_nr, #__NR_syscalls
	bl	enable_irq
	cmp     scno, sc_nr             // check upper syscall limit
	b.hs	ni_sys

	ldr	x16, [stbl, scno, lsl #3]  // address in the syscall table
	blr	x16					       // call sys_* routine
	b	ret_from_syscall
ni_sys:
	handler_entry 0, SYSCALL_ERROR
ret_from_syscall:
	bl	disable_irq				
	str	x0, [sp, #S_X0]				// returned x0
	kernel_exit 0

.globl ret_from_fork
ret_from_fork:
	cbz	x19, ret_to_user            // not a kernel thread
	mov	x0, x20
	blr	x19
ret_to_user:
	bl disable_irq				
	kernel_exit 0 


