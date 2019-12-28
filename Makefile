all:
	gcc -g *.c -o run -lm -Wall
access:
	sudo chmod 666 /dev/i2c-*
