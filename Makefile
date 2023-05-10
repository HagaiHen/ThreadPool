CC = gcc
CFLAGS = -g -Wall


all: main

main: main.o queue.o thread_pool.o
	$(CC) $(CFLAGS) -o coder main.o queue.o thread_pool.o  -L. ./libCodec.so -pthread

queue.o: queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c

main.o: main.c queue.h codec.h thread_pool.h
	$(CC) $(CFLAGS) -c main.c -pthread


thread_pool.o: thread_pool.c thread_pool.h
	$(CC) $(CFLAGS) -c thread_pool.c -pthread


.PHONY: all clean

clean:
	-rm coder *.o 2>/dev/null