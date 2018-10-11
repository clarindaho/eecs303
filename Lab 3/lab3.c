// import libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

// variable declaration
const char *dataFilePath = "lab3data.txt";
FILE *dataFile;
int newFileFlag = 1;

const int sensorPin = 29;
double temperature;
double humidity;
int errorCount = 0;

const double temperatureThreshold = 27;

int backLightI2CAddress = 0x62;
int textDisplayI2CAddress = 0x3E;
int backLight;
int textDisplay;

// helper methods declaration
void setBackgroundColor(int red, int green, int blue);
void textCommand(char command);
int setText(char *text);
void resetDisplay(int resetFlag);

int readDHTSensor();
int readDataByte();
void printResult();

// main method
int main(){ 
	// catch signal (Ctrl + C)
	struct sigaction sigHandler;
	sigHandler.sa_handler = resetDisplay;
	sigemptyset(&sigHandler.sa_mask);
	sigHandler.sa_flags = 0;
	sigaction(SIGINT, &sigHandler, NULL);

	// initialize wiringPi using wiringPi numbering system
	if(wiringPiSetup() == -1){
		fprintf(stderr, "ERROR: Unable to start wiringPi\n");
		return 1;
	}
	
	// setup I2C bus
	backLight = wiringPiI2CSetup(backLightI2CAddress);
	textDisplay = wiringPiI2CSetup(textDisplayI2CAddress);
	if((backLight < 0) || (textDisplay < 0)){
		fprintf(stderr, "ERROR: Unable to initialize I2C device\n");
		return 1;
	}
	
	// reset display
	textCommand(0x01);
	setBackgroundColor(0, 0, 0);
	
	//
	// Show the Raspberry Pi transaction data on the LCD display
	//
 	
	int i = 0, maxReadings = 20;
	for (i = 0; i < maxReadings; i++){
		int maxErrors = 1000;
		while(readDHTSensor() == 0){				// if data is wrong
			errorCount++;
			delayMicroseconds(1);

			if (errorCount >= maxErrors){
				fprintf(stderr, "ERROR: Failed 1000 times. Terminating program.\n");
				
				setText("ERROR: Failed 1000 times.");
				setBackgroundColor(0, 0, 0);
				
				return 2;
			}
		}
		
		if (temperature > temperatureThreshold)		// temperature is above the threshold
			setBackgroundColor(255, 0, 0);
		else										// temperature is below the threshold
			setBackgroundColor(0, 0, 255);
			
		delayMicroseconds(1);						// delay between each reading
		errorCount = 0;
	}
	
	serialClose(sensorPin);
 
	return 0;
}

// helper methods

void setBackgroundColor(int red, int green, int blue){
	// initialize the backlight
	wiringPiI2CWriteReg8(backLight, 0x00, 0x00);
	wiringPiI2CWriteReg8(backLight, 0x01, 0x00);
	
	// use PWM to control all LED
	wiringPiI2CWriteReg8(backLight, 0x08, 0xAA);
	
	// set RGB color
	wiringPiI2CWriteReg8(backLight, 0x04, red);
	wiringPiI2CWriteReg8(backLight, 0x03, green);
	wiringPiI2CWriteReg8(backLight, 0x02, blue);
}

void textCommand(char command){
	wiringPiI2CWriteReg8(textDisplay, 0x80, command);
}

int setText(char *text){
	// clear display
	textCommand(0x01);
	delay(50);
	
	// setup display formatting
	textCommand(0x08 | 0x04);			// turn display on with no cursor
	textCommand(0x28);					// display two lines
	delay(50);
	
	// format and display text
	int i = 0, flag = 0;
	int lineCharCount = 0, row = 0;
	for (i = 0; (text[i] != '\0') && (flag == 0); i++){
		if ((text[i] == '\n') || (lineCharCount >= 16)){		// new line character encountered or character exceeds space allowed per line
			// go to next row
			lineCharCount = 0;
			row++;
			
			if (row == 2)				// maximum number of lines reached
				flag = 1;				// truncate any remaining characters
			else {						// maximum number of lines not reached
				textCommand(0xc0);		// move cursor to next line
				
				if(text[i] != '\n'){	// character exceeds space allowed per line
					lineCharCount++;
					wiringPiI2CWriteReg8(textDisplay, 0x40, text[i]);
				}
			}
		} else {
			lineCharCount++;
			wiringPiI2CWriteReg8(textDisplay, 0x40, text[i]);
		}
	}
}

void resetDisplay(int resetFlag){
	// clear display
	textCommand(0x01);
	
	// disable backlight
	setBackgroundColor(0, 0, 0);
	
	fprintf(stderr, "NOTE: Termination of program\n");
	exit(3);
}

int readDHTSensor(){
	//
	// use the polling I/O technique to implement one RPi-to-DHT11 transaction
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

	// wait for sensor response to request signal
	pinMode(sensorPin, INPUT);
	while (digitalRead(sensorPin) == 1){
		errorReadingCount++;
		if (errorReadingCount >= 255)
			return 0;

		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// sensor sends response signal by pulling down voltage for 80 us
	delayMicroseconds(80);
	while (digitalRead(sensorPin) == 0){
		errorReadingCount++;
		if (errorReadingCount >= 255)
			return 0;

		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// sensor pulls up voltage for 80 us
	delayMicroseconds(77);
	while (digitalRead(sensorPin) == 1){
		errorReadingCount++;
		if (errorReadingCount >= 255)
			return 0;
		
		delayMicroseconds(1);
	}
	errorReadingCount = 0;

	// start data transmission
	int i = 0,  dataSum = 0;
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

	int i = 0, dataByte = 0;
	for (i = 0; i < 8; i++){
		// bitwise left shift
		if (i != 0)
			dataByte = dataByte << 1;
		
		// wait for 50 us low voltage separation to end
		delayMicroseconds(45);
		
		int errorReadingCount = 0;
		while(digitalRead(sensorPin) == 0){
			errorReadingCount++;
			if (errorReadingCount >= 255)
				return -1;

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
	
	// display temperature and humidity on LCD screen
	char displayText[50];
	sprintf(displayText, "T: %.3f\nH: %.3f", temperature, humidity);
	setText(displayText);

	// close data file
	fclose(dataFile);
}