// import libraries
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

// variable declaration
const int ledPin = 8;

int main(){
	// intialize wiringPi using Broadcom pin numbers
	wiringPiSetup();

	// set pins to input/output
	pinMode(ledPin, OUTPUT);

	int i = 0;
	for (i = 0; i < 10; i++){
		digitalWrite(ledPin, HIGH);	// turn LED on
		delay(5000);			// wait 5 seconds
		digitalWrite(ledPin, LOW);	// turn LED off
		delay(5000);			// wait 5 seconds
	}
}
