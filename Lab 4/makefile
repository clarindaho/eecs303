all: part1 part2

part1:
	gcc -o lab4part1 lab4part1.s -Wall -lwiringPi

part2:
	gcc -c lab4part2.c
	gcc lab4part2.s lab4part2.o -lwiringPi -lpthread -o lab4part2
	
clean:
	rm lab4part1
	rm lab4part2.o
	rm lab4part2
