
CFLAGS += -Werror -Wall -pedantic

test: runtime.o test.o
	$(CC) runtime.o test.o -lc -lobjc && ./a.out

runtime.o: runtime.c
	$(CC) $(CFLAGS) -std=c99 -c runtime.c

test.o: test.m
	$(CC) $(CFLAGS) -std=c99 -c test.m
