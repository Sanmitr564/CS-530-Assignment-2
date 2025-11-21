EXEC = asx
FILES = main.c assembler.c instructions.c linkedlist.c

CC = gcc

LFLAGS = -g
CFLAGS = -g -c

OBJECTS = $(FILES:.c=.o)

$(EXEC):$(OBJECTS)
	$(CC) $(LFLAGS) -o $(EXEC) $(OBJECTS)
	rm -f *.o

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -f *.o $(EXEC)
