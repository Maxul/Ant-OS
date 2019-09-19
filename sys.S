#include "sys.h"

.macro syscall_entry name id
    .global \name
    \name:
        mov w8, \id
        svc #0
        ret
.endm

syscall_gates:
    syscall_entry call_sys_write,  #SYS_WRITE_NUMBER
    syscall_entry call_sys_malloc, #SYS_MALLOC_NUMBER
    syscall_entry call_sys_exit,   #SYS_EXIT_NUMBER


.globl call_sys_clone
call_sys_clone:
	/* Save args for the child.  */
	mov	x10, x0					/*func*/
	mov	x11, x1					/*args*/
	mov	x12, x2					/*stack*/

	/* Do the system call.  */
	mov 	x0, x2					/* stack  */
	mov	x8, #SYS_CLONE_NUMBER
	svc	#0

	cmp	x0, #0
	beq	thread_start
	ret

thread_start:
	mov	x29, #0

	/* Pick the function args and execute.  */
	mov	x0, x11
	blr	x10

	/* We are done, pass the return value through x0.  */
	mov	x8, #SYS_EXIT_NUMBER
	svc	#0
