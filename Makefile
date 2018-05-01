CC = gcc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

dbamp : dbamp.o getfastas.o extend.o search.o list.o
	$(CC) $(LFLAGS) dbamp.o getfastas.o extend.o search.o list.o -o dbamp

dbamp.o : getfastas.c extend.c search.c list.c
	$(CC) $(CFLAGS) dbamp.c

getfastas.o : getfastas.c list.c
	$(CC) $(CFLAGS) getfastas.c

extend.o : extend.c list.c
	$(CC) $(CFLAGS) extend.c

search.o : search.c list.c
	$(CC) $(CFLAGS) search.c

list.o : list.c
	$(CC) $(CFLAGS) list.c

clean:
	\rm *.o dbamp
