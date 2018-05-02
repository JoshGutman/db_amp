CC = gcc
LCC = mpicc
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -lm

dbamp : dbamp.o getfastas.o extend.o search.o list.o
	$(LCC) $(LFLAGS) dbamp.o getfastas.o extend.o search.o list.o -o dbamp

dbamp.o : dbamp.c getfastas.c extend.c search.c list.c
	$(LCC) -c dbamp.c

getfastas.o : getfastas.c list.c
	$(LCC) -c getfastas.c

extend.o : extend.c list.c
	$(LCC) -c extend.c

search.o : search.c list.c
	$(LCC) -c search.c

list.o : list.c
	$(LCC) -c list.c

clean:
	\rm *.o dbamp
