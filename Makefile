all:
	gcc -g *.c -o accel -lm -Wall
access:
	sudo chmod 666 /dev/i2c-*
