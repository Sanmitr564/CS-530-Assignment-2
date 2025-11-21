#Aaron Alegre,       cssc2101, Red ID: 828222103
#Patrick Stewart,    cssc2138, Red ID: 130969511
#Edwin Vega,         cssc2141, Red ID: 827746186

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
