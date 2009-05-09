CFLAGS += -Werror -Wall #-pedantic

test: runtime.o test.o sync.o
	$(CC) runtime.o test.o sync.o -lc -lobjc && ./a.out

runtime.o: runtime.c
	$(CC) $(CFLAGS) -std=c99 -c runtime.c

sync.o: sync.m runtime.c
	$(CC) $(CFLAGS) -std=c99 -c sync.m

