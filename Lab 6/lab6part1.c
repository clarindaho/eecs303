// import libraries

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>

// module information

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Clarinda Ho and Emmeline Pearson");
MODULE_DESCRIPTION("Blink an LED Using Kernel Module");
MODULE_VERSION("1.0");

// global variables

#define LED_PIN		20

#define HIGH		1
#define LOW			0

// kernel methods

static void blink(void){
	printk(KERN_INFO "lab6part1: Blinking LED...\n");
	
	int i = 0;
	for (i = 0; i < 10; i++){
		msleep(2000);
		gpio_set_value(LED_PIN, LOW);
		msleep(2000);
		gpio_set_value(LED_PIN, HIGH);
	}
}

static int __init initialization(void){	
	// check whether GPIO pin is valid
	if (!gpio_is_valid(LED_PIN)){
		printk(KERN_INFO "lab6part1: Initialization failed. Invalid LED GPIO.\n");
		return -ENODEV;
	}
	
	// request GPIO pin
	gpio_request(LED_PIN, "sysfs");
	
	// set direction of GPIO pin as output with its value at high voltage
	gpio_direction_output(LED_PIN, HIGH);
	
	// make LED GPIO visible in /sys/class/gpio
	// prevent others from changing direction of the pin
	gpio_export(LED_PIN, false);
	
	printk(KERN_INFO "lab6part1: Initialization succeeded.\n");

	blink();
	
	return 0;
}

static void __exit finalization(void){
	// turn off LED
	gpio_set_value(LED_PIN, 0);
	
	// unexport and free LED GPIO pin
	gpio_unexport(LED_PIN);
	gpio_free(LED_PIN);
	
	// update kernel log (kern.log)
	printk(KERN_INFO "lab6part1: Exit.\n");
}

// specify initialization and finalization function
module_init(initialization);
module_exit(finalization);