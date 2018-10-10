// import libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <wiringPi.h>

// variable declaration
const char *dataFilePath = "lab2part2data.txt";
FILE *dataFile;
int newFileFlag = 1;

const int sensorPin = 29;
double temperature;
double humidity;
int errorCount = 0;

const int ledPin = 14;
const double temperatureThreshold = 27;

int lastState = HIGH;
struct timeval startTime;

// helper methods declaration
int readDHTSensor();
int readDataByte();
void printResult();

// main method
int main(){ 
	// initialize wiringPi using wiringPi numbering system
	if(wiringPiSetup() == -1){
		fprintf(stderr, "ERROR: Unable to start wiringPi\n");
		return 1;
	}
	
	//
	// Write a loop of your C code to record temperature data for a short time
	// If any of the temperature goes above a threshold, RPi will signal the LED to turn on
	//
 	
	int i = 0, maxReadings = 20;
	for (i = 0; i < maxReadings; i++){
		int maxErrors = 1000;
		while(readDHTSensor() == 0){	// if data is wrong
			errorCount++;
			delayMicroseconds(1);

			if (errorCount >= maxErrors){
				fprintf(stderr, "ERROR: Failed 1000 times. Terminating program.\n");
				return 2;
			}
		}
		
		if (temperature > temperatureThreshold)		// if temperature is above the threshold
			digitalWrite(ledPin, HIGH);
		else										// if temperature is below the threshold
			digitalWrite(ledPin, LOW);
			
		delayMicroseconds(1);		// delay between each reading
		errorCount = 0;
	}
	
	serialClose(sensorPin);
 
	return 0;
}

// helper methods

int readDHTSensor(){
	//
	// use the polling I/O technique to implement one RPi-to-DHT11 transaction
	//

	int errorReadingCount = 0;

	// check if sensor data bus is at high voltage
	pinMode(sensorPin, INPUT);
	while(digitalRead(sensorPin) == 0){
		errorReadingCount++;
		if (errorReadingCount >= 255){
			fprintf(stderr, "ERROR: Sensor busy\n");
			return 0;
		}

		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// send start request to sensor
	pinMode(sensorPin, OUTPUT);
	digitalWrite(sensorPin, LOW);
	delayMicroseconds(20000);			// hold low level at least 18 ms

	// pull up voltage
	digitalWrite(sensorPin, HIGH);

	// wait for sensor response to request signal
	pinMode(sensorPin, INPUT);
	while (digitalRead(sensorPin) == 1){
		errorReadingCount++;
		if (errorReadingCount >= 255){
			fprintf(stderr, "ERROR: No response from sensor\n");
			return 0;
		}

		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// sensor sends response signal by pulling down voltage for 80 us
	delayMicroseconds(80);
	while (digitalRead(sensorPin) == 0){
		errorReadingCount++;
		if (errorReadingCount >= 255){
			return 0;
		}

		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// sensor pulls up voltage for 80 us
	delayMicroseconds(77);
	while (digitalRead(sensorPin) == 1){
		errorReadingCount++;
		if (errorReadingCount >= 255){
			return 0;
		}
		
		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// start data transmission
	int i = 0,  dataSum = 0;
	int data[5];
	for (i = 0; i < 5; i++){
		data[i] = readDataByte();
		dataSum += data[i];
		
		if (data[i] == -1){
			fprintf(stderr, "ERROR: Error reading data from sensor\n");
			return 0;
		}
	}

	humidity = data[0] + (data[1] * pow(10, -3));
	temperature = data[2] + (data[3] * pow(10, -3));
	
	// check if temperature is reasonable (within measurement range)
	if (temperature < 0 || temperature > 50)
		return 0;

	// verify the transaction output using its checksum data
	unsigned dataChecksum = dataSum & 0xFF;
	if (dataChecksum == data[5]){		// data matches with checksum
		// return the temperature and humidity output
		printResult();

		return 1;
	} else {				// data does not match with checksum
		fprintf(stderr, "ERROR: Data does not match with checksum\n");
		return 0;
	}
}

int readDataByte(){
	//
	// Data bits are separated by 50 us low voltage.
	// Length of high voltage represent bit values interpreted as:
	// 	Bit '0': 26-28 us
	// 	Bit '1': 70 us
	//

	int i = 0, dataByte = 0;
	for (i = 0; i < 8; i++){
		// bitwise left shift
		if (i != 0){
			dataByte = dataByte << 1;
		}
		
		// wait for 50 us low voltage separation to end
		delayMicroseconds(45);
		
		int errorReadingCount = 0;
		while(digitalRead(sensorPin) == 0){
			errorReadingCount++;
			if (errorReadingCount >= 255){
				return -1;
			}

			delayMicroseconds(1);
		}

		// wait for 40 us to see if the bit is 0 or 1
		delayMicroseconds(40);
		if (digitalRead(sensorPin) != 0){		// bit '1'
			dataByte += 1;	
			delayMicroseconds(30);
		}
	}

	return dataByte;
}

void printResult(){
	// create data file
	if (newFileFlag == 1){
		dataFile = fopen(dataFilePath, "w+");
		newFileFlag = 0;
	}
	else if (newFileFlag == 0){
		dataFile = fopen(dataFilePath, "a+");
	}
	
	// print and save error count
	char errorCountData[50];
	sprintf(errorCountData, "Failed %d times\n", errorCount);
	printf("%s", errorCountData);
	fprintf(dataFile, "%s", errorCountData);

	// print and save temperature output
	char temperatureData[50];
	sprintf(temperatureData, "Temperature: %.3f\n", temperature);
	printf("%s", temperatureData);
	fprintf(dataFile, "%s", temperatureData);

	// print and save humidity output
	char humidityData[50];
	sprintf(humidityData, "Humidity: %.3f\n\n", humidity);
	printf("%s", humidityData);
	fprintf(dataFile, "%s", humidityData);

	// close data file
	fclose(dataFile);
}