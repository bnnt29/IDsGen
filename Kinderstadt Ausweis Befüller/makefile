CC=gcc
CFLAGS=-I.
PROGRAMMOUT=programm.run
PIPEINPUTEROUT=pipe_inputer.run
PIPEINPUTERDUALKEYOUT=pipe_inputer_dualkey.run

make: programm.c pipe_inputer.c
	$(CC) -o $(PROGRAMMOUT) programm.c -L. -lhpdf -lm -Wall -W -pedantic
	$(CC) -o $(PIPEINPUTEROUT) pipe_inputer.c -Wall -W -pedantic
	$(CC) -o $(PIPEINPUTERDUALKEYOUT) pipe_inputer_dualkey.c -Wall -W -pedantic

clean:
	rm $(PROGRAMMOUT) $(PIPEINPUTEROUT) $(PIPEINPUTERDUALKEYOUT)
