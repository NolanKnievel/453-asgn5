CC = gcc
CFLAGS = -Wall -Wextra

minls: minls.c
	$(CC) $(CFLAGS) -o minls minls.c
