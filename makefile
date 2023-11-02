server:
	gcc server.c -o bin/server
	./bin/server

client:
	gcc client.c -o bin/client
	./bin/client