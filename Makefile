# Makefile for mypreprocessor
# make       -> builds mypreprocessor
# make clean -> removes the .o files and the program

CC = gcc
CFLAGS = -Wall

all: mypreprocessor

mypreprocessor: main.o comment_removal.o file_inclusion.o macro_handler.o
	$(CC) main.o comment_removal.o file_inclusion.o macro_handler.o -o mypreprocessor

main.o: main.c comment_removal.h file_inclusion.h macro_handler.h
	$(CC) $(CFLAGS) -c main.c

comment_removal.o: comment_removal.c comment_removal.h
	$(CC) $(CFLAGS) -c comment_removal.c

file_inclusion.o: file_inclusion.c file_inclusion.h
	$(CC) $(CFLAGS) -c file_inclusion.c

macro_handler.o: macro_handler.c macro_handler.h
	$(CC) $(CFLAGS) -c macro_handler.c

clean:
	rm -f *.o mypreprocessor *.i
