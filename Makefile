all: inject client server

inject: inject.c
	$(CC) -g -O2 -static -o inject inject.c

client: client.c
	$(CC) -g -O2 -o client client.c

server: server.c
	$(CC) -g -O2 -o server server.c

clean:
	rm -f inject server
