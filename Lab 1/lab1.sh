#!/bin/sh

# echo "2" > /sys/class/gpio/export
# echo "out" > /sys/class/gpio/gpio2/direction

# NOTE: The direction node is owned by the root
# so the direction of a GPIO pin cannot be changed
# by the user using echo without sudo permission.
# Thereby, to get around this problem, we used
# the /sys/class/gpio mode commands to export
# the GPIO pin and set the direction.

gpio export 2 out

for i in {1..50}
do
	echo "1" > /sys/class/gpio/gpio2/value
	sleep 5
	echo "0" > /sys/class/gpio/gpio2/value
	sleep 5
done

echo "2" > /sys/class/gpio/unexport
