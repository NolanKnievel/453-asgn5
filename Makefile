CC = gcc
CFLAGS = -Wall -Wextra -Werror

LS_OBJS  = minls.o printers.o reads.o searches.o util.o
GET_OBJS = minget.o printers_get.o reads_get.o searches_get.o util_get.o

all: minls minget

minls: $(LS_OBJS)
	$(CC) $(CFLAGS) -o $@ $(LS_OBJS)

minget: $(GET_OBJS)
	$(CC) $(CFLAGS) -o $@ $(GET_OBJS)

minls.o: minls.c minls.h
	$(CC) $(CFLAGS) -c minls.c -o $@

minget.o: minget.c minget.h
	$(CC) $(CFLAGS) -c minget.c -o $@

printers.o: printers.c printers.h util.h reads.h
	$(CC) $(CFLAGS) -c printers.c -o $@

printers_get.o: printers_get.c printers_get.h util_get.h reads_get.h
	$(CC) $(CFLAGS) -c printers_get.c -o $@

searches.o: searches.c searches.h util.h reads.h
	$(CC) $(CFLAGS) -c searches.c -o $@

searches_get.o: searches_get.c searches_get.h util_get.h reads_get.h
	$(CC) $(CFLAGS) -c searches_get.c -o $@

reads.o: reads.c reads.h util.h
	$(CC) $(CFLAGS) -c reads.c -o $@

reads_get.o: reads_get.c reads_get.h util_get.h
	$(CC) $(CFLAGS) -c reads_get.c -o $@

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -o $@

util_get.o: util_get.c util_get.h
	$(CC) $(CFLAGS) -c util_get.c -o $@


clean:
	rm -f minls minget *.o

.PHONY: all clean