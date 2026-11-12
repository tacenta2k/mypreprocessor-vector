mypreprocessor: main.o comment_removal.o file_inclusion.o macro_handler.o
	gcc main.o comment_removal.o file_inclusion.o macro_handler.o -o mypreprocessor

main.o: main.c comment_removal.h file_inclusion.h macro_handler.h
	gcc -Wall -c main.c

comment_removal.o: comment_removal.c comment_removal.h
	gcc -Wall -c comment_removal.c

file_inclusion.o: file_inclusion.c file_inclusion.h
	gcc -Wall -c file_inclusion.c

macro_handler.o: macro_handler.c macro_handler.h
	gcc -Wall -c macro_handler.c

clean:
	rm -f *.o mypreprocessor
