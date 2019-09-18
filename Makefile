CROSS = aarch64-linux-gnu
CC = ${CROSS}-gcc
AS = ${CROSS}-as
OBJDUMP = ${CROSS}-objdump
CFLAGS = -nostdlib -nostartfiles -ffreestanding -mgeneral-regs-only -std=gnu99 -O2 -w # -Wall -Wextra
OBJ = startup.o kernel.o switch.o utils.o irq.o mm.o sched.o uart.o timer.c printf.o sys.o clone.o syscall.o string.o

kernel.elf: ${OBJ}
	${CC} -Wl,--build-id=none -T linker.ld -o $@ -ffreestanding -O2 -nostdlib ${OBJ}
	${OBJDUMP} -D kernel.elf > kernel.list

%.o : %.c Makefile
	$(CC) ${CFLAGS} -c -o $*.o $*.c

run :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -serial stdio -kernel kernel.elf
	#qemu-system-aarch64 -machine raspi3 -nographic -serial null -serial mon:stdio -kernel kernel.elf

runasm :
	$(MAKE) kernel.elf
	qemu-system-aarch64 -M raspi3 -m 128 -serial mon:stdio -nographic -kernel kernel.elf -d in_asm

clean:
	rm -f *.o *.elf *.list

.PHONY: clean
