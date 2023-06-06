# Initial process that execs /init.
# This code runs in user space.

#include "include/sysnum.h"

# exec(init, argv)
.global start
start:
	li a0, 2
	li a1, 1
	li a2, 0
	li a7, SYS_dev
	ecall	# dev(O_RDWR, CONSOLE, 0);
	
	li a0, 0
	li a7, SYS_dup
	ecall	# dup(0);  // stdout

	li a0, 0
	li a7, SYS_dup
	ecall	# dup(0);  // stderr
	
	la t1, test_file
	la t2, test_file_end
test:
	li a0, 0
	li a7, SYS_fork
	ecall
	bne a0, zero, parent 
	lw a0, 0(t1)
	la a1, argv
	li a7, SYS_exec
	ecall
	
parent:
	li a0, 0
	li a7, SYS_wait
	ecall
	addi t1, t1, 4
	bne t1, t2, test

# for(;;) exit()
end:
	li a7,SYS_exit
	ecall
	jal end
getcwd:
	.string "/getcwd\0"
pipe2:
	.string "/pipe\0"
dup:
	.string "/dup\0"
dup3:
	.string "/dup2\0"
chdir:
	.string "/chdir\0"
openat:
	.string "/openat\0"
close:
	.string "/close\0"
getdents64:
	.string "/getdents\0"
read:
	.string "/read\0"
write:
	.string "/write\0"
unlink:
	.string "/unlink\0"
mkdirat:
	.string "/mkdir_\0"
umount2:
	.string "/umount\0"
mount:
	.string "/mount\0"
fstat:
	.string "/fstat\0"
clone:
	.string "/clone\0"
execve:
	.string "/execve\0"
wait4:
	.string "/waitpid\0"
exit:
	.string "/exit\0"
getppid:
	.string "/getppid\0"
getpid:
	.string "/getpid\0"
brk:
	.string "/brk\0"
mmap:
	.string "/mmap\0"
munmap:
	.string "/munmap\0"
times:
	.string "/times\0"
uname:
	.string "/uname\0"
schedyield:
	.string "/yield\0"
gettimeofday:
	.string "/gettimeofday\0"
nanosleep:
	.string "/sleep\0"

fork:
        .string "/fork\0"
open:
        .string "/open\0"
wait:
        .string "/wait\0"
test_echo:
        .string "/test_echo\0"


test_file:
	
	.long fstat
	.long mmap
    .long munmap
	.long unlink
	.long close
	.long nanosleep
    .long getcwd
    .long pipe2
    .long dup
    .long dup3
    .long chdir
    .long openat
    .long getdents64
    .long read
    .long write
    .long mkdirat
    .long umount2
    .long mount
    .long clone
    .long execve
    .long wait4
    .long exit
    .long getppid
    .long getpid
    .long brk
    .long times
    .long uname
    .long schedyield
    .long gettimeofday
    .long fork
    .long open
    .long wait
    .long test_echo

 test_file_end:

argv:
	.long 0
