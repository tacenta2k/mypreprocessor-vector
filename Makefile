# Makefile for mypreprocessor
#
#   make          build the program
#   make run      build and run it on the example file test.c
#   make clean    delete the program and the .o files
#
# CC can be changed, e.g.:  make CC=clang

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2
TARGET  = mypreprocessor
OBJECTS = main.o util.o comment_removal.o file_inclusion.o macro_handler.o
HEADERS = util.h comment_removal.h file_inclusion.h macro_handler.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Every .o depends on every .h: this is a small project, so rebuilding a
# little more than needed is simpler than writing one line per file.
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) test.c

clean:
	rm -f $(OBJECTS) $(TARGET) *.i

.PHONY: all run clean
