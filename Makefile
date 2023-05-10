CC = gcc
CFLAGS = -g -Wall


all: main

main: main.o queue.o
	$(CC) $(CFLAGS) -o main main.o queue.o -L. ./libCodec.so -pthread

q: queue.o queue.o
	$(CC) $(CFLAGS) -o q queue.o queue.o

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

main.o: main.c queue.h codec.h
	$(CC) $(CFLAGS) -c main.c

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c


.PHONY: all clean

clean:
	-rm main *.o 2>/dev/null