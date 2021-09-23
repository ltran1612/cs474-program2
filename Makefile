all: program2

program2: program2.o utilities.o
	gcc program2.o utilities.o -o program2

program2.o: program2.c CONSTANTS.h utilities.h
	gcc -c program2.c

utilities.o: utilities.c utilities.h
	gcc -c utilities.c

clean: 
	rm *.o
