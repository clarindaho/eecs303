// import libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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
	// Repeat Part 1 using the edge-triggered interrupt technique
	//
 	
	int maxErrors = 1000;
	while(readDHTSensor() == 0){		// if data is wrong
		errorCount++;
		delayMicroseconds(1);

		if (errorCount >= maxErrors){
			fprintf(stderr, "ERROR: Failed 1000 times. Terminating program.\n");
			return 2;
		}
	}
	
	serialClose(sensorPin);
 
	return 0;
}

// helper methods
 
int readDHTSensor(){
	//
	// use the edge-triggered interrupt technique to implement one RPi-to-DHT11 transaction
	//

	int errorReadingCount = 0;
	
	// check if sensor data bus is at high voltage
	pinMode(sensorPin, INPUT);
	while(digitalRead(sensorPin) == 0){
		errorReadingCount++;
		if (errorReadingCount >= 255)
			return 0;

		delayMicroseconds(1);
	}
	errorReadingCount = 0;
	
	// send start request to sensor
	pinMode(sensorPin, OUTPUT);
	digitalWrite(sensorPin, LOW);
	delayMicroseconds(20000);			// hold low level at least 18 ms

	// pull up voltage
	digitalWrite(sensorPin, HIGH);

	// i=0: wait for sensor response to request signal
	// i=0: sensor sends response signal by pulling down voltage for 80 us
	// i=1: sensor pulls up voltage for 80 us
	// i=2: sensor starts data transmission
	pinMode(sensorPin, INPUT);
	
	int j = 0;
	for (j = 0; j < 3; j++){ 
		while (digitalRead(sensorPin) == lastState){
			errorReadingCount++;
			if (errorReadingCount >= 255)
				return 0;

			delayMicroseconds(1);
		}
		lastState == digitalRead(sensorPin);
		errorReadingCount = 0;
		
		if (j == 2)
			gettimeofday(&startTime, NULL);
	}
	
	// start data transmission
	int i = 0, dataSum = 0;
	int data[5];
	for (i = 0; i < 5; i++){
		data[i] = readDataByte();
		dataSum += data[i];
		
		if (data[i] == -1)
			return 0;
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
	} else								// data does not match with checksum
		return 0;
}

int readDataByte(){	
	//
	// Data bits are separated by 50 us low voltage.
	// Length of high voltage represent bit values interpreted as:
	// 	Bit '0': 26-28 us
	// 	Bit '1': 70 us
	//
	
	struct timeval stopTime;
	int i = 0, dataByte = 0;
	for (i = 0; i < 16; i++){
		int errorReadingCount = 0;
		while(digitalRead(sensorPin) == lastState){
			errorReadingCount++;
			if (errorReadingCount >= 255)
				return -1;

			delayMicroseconds(1);
		}
		lastState = digitalRead(sensorPin);

		if (lastState == LOW){
			// determine if the bit is 0 or 1
			gettimeofday(&stopTime, NULL);
			if ((stopTime.tv_usec - startTime.tv_usec - 50) > 40)	// bit '1'
				dataByte += 1;
			
			if(i < 14){
				// bitwise left shift
				dataByte = dataByte << 1;
			}
		
			gettimeofday(&startTime, NULL);
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