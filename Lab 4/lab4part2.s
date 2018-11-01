.global main

mov r5, #0

main: 
	cmp r5, #10
	bgt exit
	bl setup
	bl printHere
	bl dim0
	bl delayHalfSec
	bl dim25
	bl delayHalfSec
	bl dim50
	bl delayHalfSec
	bl dim75
	bl delayHalfSec
	bl dim100
	bl delayHalfSec
	bl delayHalfSec 		@extra long on last
	bl dim0
	add r5, r5, #1
	b main

exit: 