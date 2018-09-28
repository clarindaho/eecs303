# import libraries
import time
import RPi.GPIO as GPIO

# define variables
ledPin =  3

def main():
	GPIO.setmode(GPIO.BOARD)
	GPIO.setup(ledPin, GPIO.OUT)
	GPIO.output(ledPin, GPIO.HIGH)

	for i in range(10):
		GPIO.output(ledPin, GPIO.HIGH)	# turn LED on
		time.sleep(5)			# wait 5 seconds
		GPIO.output(ledPin, GPIO.LOW)	# turn LED off
		time.sleep(5)			# wait 5 seconds

if __name__ == "__main__": main()
