CC=gcc
CFLAGS=-I.
PROGRAMMOUT=programm.run

make: programm.c
	$(CC) -o $(PROGRAMMOUT) programm.c -L. -lhpdf -Wall -W -pedantic

clean:
	rm $(PROGRAMMOUT)
