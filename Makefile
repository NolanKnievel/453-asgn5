CC = gcc
CFLAGS = -Wall -Wextra -Werror
HELPERS = printers.h reads.h searches.h util.h minls.h
FINAL_DEP = printers.o reads.o searches.o util.o minls.o


all: $(FINAL_DEP)
	$(CC) $(CFLAGS) -o minls $(FINAL_DEP)

minls.o: minls.c minls.h
	$(CC) $(CFLAGS) -c minls.c

printers.o: util.o reads.o printers.c printers.h 
	$(CC) $(CFLAGS) -c printers.c

searches.o: util.o reads.o searches.c searches.h
	$(CC) $(CFLAGS) -c searches.c

reads.o: util.o reads.c reads.h
	$(CC) $(CFLAGS) -c reads.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

clean: 
	rm -f minls *.o

.PHONY: clean