/*
	int_handler.S


*/

#define ASM 1

# Keyboard, RTC, Mouse (not implemented) and PIT / System Call interrupt handlers
.globl 	INT_HANDLER_33, INT_HANDLER_40, INT_HANDLER_44, INT_HANDLER_32

#define INT_HANDLER(handler_idx, handler_id)	 \
	handler_idx:								;\
		pushl %eax								;\
		pushl %ecx 								;\
		pushl %edx 								;\
		pushl %ebx 								;\
		pushl %esi 								;\
		pushl %edi 								;\
		pushfl 										;\
		call handler_id 					;\
		popfl 										;\
		popl %edi 								;\
		popl %esi 								;\
		popl %ebx 								;\
		popl %edx									;\
		popl %ecx 								;\
		popl %eax 								;\
		iret 											;\

# keyboard handler
INT_HANDLER(INT_HANDLER_33, keyboard_handler);
# RTC handler
INT_HANDLER(INT_HANDLER_40, RTC_handler);
# mouse handler
INT_HANDLER(INT_HANDLER_44, mouse_handler);
# pit handler
INT_HANDLER(INT_HANDLER_32, pit_handler);

#
# System Call Interrupt Handler
#

# 1. halt
# 2. execute
# 3. read
# 4. write
# 5. open
# 6. close
# 7. getargs
# 8. vidmap
# 9. set_handler
# 10. sigreturn

.globl SYSCALL_INTERRUPT

# NOTE: EAX assumed to hold a value which will be used to jump to correct syscall
SYSCALL_INTERRUPT:
	cli

	pushfl
	pushl %ebp
	pushl %edi
	pushl %esi
	# push args
	pushl %edx
	pushl %ecx
	pushl %ebx

	# checking if integer is between 1 - 10
	cmpl $10, %eax
	jg error_handle
	cmpl $1, %eax
	jl error_handle

	# perform jumptable call
	sti
	call *jumptable(,%eax, 4)
	cli
	jmp clean_up

error_handle:
	movl $-1, %eax

clean_up:
	# pop args
	popl %ebx
	popl %ecx
	popl %edx

	popl %esi
	popl %edi
	popl %ebp
	popfl

	sti

	iret

# jumptable for the sys calls (first value is a dummy number, since indices are 1 - 10)
jumptable:
	.long 0xDEADECEB, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn
