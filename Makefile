#Execution command- "./all"
CC=gcc

option=-c

all:	filesystem.o myalloc_0_1.o
	$(CC) filesystem.o myalloc_0_1.o -lm -o all

myalloc_0_1.o: myalloc_0_1.c myalloc.h
	$(CC) $(option) myalloc_0_1.c

filesystem.o: filesystem.c myalloc.h
	$(CC) $(option) filesystem.c -lm 

clean:
	rm -rf *o all
