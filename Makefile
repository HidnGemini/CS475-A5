all: main.c sobel.o rtclock.o
	gcc -O3 -Wall -g -o sobel main.c sobel.o rtclock.o -lpthread -lm

sobel.o: sobel.h sobel.c
	gcc -O3 -Wall -g -c sobel.c

rtclock.o: rtclock.h rtclock.c
	gcc -O3 -Wall -g -c rtclock.c

clean:
	rm -f sobel *.o
