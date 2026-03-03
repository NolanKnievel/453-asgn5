CC = gcc
CFLAGS = -Wall -Wextra

minls: minls.o util.o
	$(CC) $(CFLAGS) -o minls minls.o util.o

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

minls.o: minls.c minls.h
	$(CC) $(CFLAGS) -c minls.c

clean: 
	rm -f minls *.o
