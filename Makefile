CFLAGS=-Wall -Werror -pedantic
LDFLAGS=

mumble-ping: main.o mumble-ping.o
	$(CC) -o mumble-ping main.o mumble-ping.o $(LDFLAGS)

main.o: main.c
	$(CC) $(CFLAGS) -o main.o -c main.c

mumble-ping.o: mumble-ping.c
	$(CC) $(CFLAGS) -o mumble-ping.o -c mumble-ping.c

clean:
	$(RM) *.o mumble-ping
