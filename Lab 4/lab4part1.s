Intro:   .asciz "Rasberry Pi Wiring Pi Blink Test\n"
ErrMsg:  .asciz "ERROR: setup failed\nAborting...\n"

@data Section 
	     .data   
         .balign 4

LedPin:  .int    8 
i:       .int    0
delayMs: .int    1000

OUTPUT   =       1
         .text
         .global main    
         .extern printf
         .extern wiringPiSetup
         .extern delay
         .extern digitalWrite
         .extern analogWrite
         .extern pinMode


main:    push   {ip, lr}   			 @push return address + dummy register for alignment

		 @printf("Blink...\n"); 
         ldr     r0, =Intro
         bl      printf


		 @if setup loop 
		 @if (wiringPiSetup() == -1) { 
		 @   printf("ERROR: Setup failed\nAborting...\n")
		 @   exit(1)
		 @}
         bl     wiringPiSetup
         mov    r1,#-1
         cmp    r0,r1
         bne    init
         ldr    r0, =ErrMsg
         bl     printf
         b      done 

init: 
		 @pinMode(pin, OUTPUT)
         ldr    r0, =LedPin
         ldr    r0, [r0]
         mov    r1, #OUTPUT
         bl     pinMode

		 @for(i=0; i<10; i++)
         ldr    r4, =i
         ldr    r4, [r4]
         mov    r5, #10

forLoop: 
         cmp    r4, r5
         bgt    done

		 @digitalWrite(pin, 1)
         ldr    r0, =LedPin
         ldr    r0, [r0]
         mov    r1, #1
         bl     digitalWrite

		 @delay(1)
         ldr    r0, =delayMs
         ldr    r0, [r0]
         bl     delay

		 @digitalWrite(pin, 0) 
         ldr    r0, =LedPin
         ldr    r0, [r0]
         mov    r1, #0
         bl     digitalWrite

		 @delay(1)
         ldr     r0, =delayMs
         ldr     r0, [r0]
         bl      delay

         add     r4, #1
         b       forLoop

done: 
         pop    {ip, pc}  			@pop return address into PC