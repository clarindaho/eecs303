// import libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <softPwm.h>

// global variables
const int ledPin = 26;

// helper methods

extern void setup(){
	if(wiringPiSetup() < 0){
		printf("ERROR: cannot setup Wiring Pi\n");
		exit(1);
	}

	pinMode(ledPin, OUTPUT);

	if(softPwmCreate(ledPin, 0, 100) != 0){
		printf("ERROR: cannot setup PWM pin\n");
		exit(1);
	}
}

extern void delayHalfSec(){
	delay(500); 					// half a second 
}

extern void dim100(){
	softPwmWrite(ledPin, 100);
}

extern void dim75(){
	softPwmWrite(ledPin, 75);
}

extern void dim50(){
	softPwmWrite(ledPin, 50);
}

extern void dim25(){
	softPwmWrite(ledPin, 25);
}

extern void dim0(){
	softPwmWrite(ledPin, 0);
}

extern void printHere(){ 
	printf("HERE\n"); 
}