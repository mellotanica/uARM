# Stripped-down version of libuarm, suitable for user-mode processes

 	# Constants
	
	# BIOS BREAK codes

	PANICBCODE	= 1
	HALTBCODE	= 2
	LDSTBCODE	= 3
	SYSBCODE	= 8
	
	# Code start

################################################

	# this function cause a system call trap
	# system call code is in $4 (a0) register
	# return value in $2 (v0) register
	# it is programmer's task to load the return value into 
 	# state register, and to set PC correctly for returning _after_
	# syscall

	.global	SYSCALL

SYSCALL:
	SWI SYSBCODE
	BX lr
	

