CC = gcc
CFLAGS = -Wall -Wextra

minls: minls.c, util.o
	$(CC) $(CFLAGS) -o minls minls.c util.o

util.o: util.c, util.h
	$(CC) $(CFLAGS) -c util.c

