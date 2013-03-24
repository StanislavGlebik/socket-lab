all: clean server client

debug: clean server_debug client_debug

server:
	gcc server.c -o server

client:
	gcc client.c -o client

server_debug:
	gcc -g server.c -o server

client_debug:
	gcc -g client.c -o client

clean:
	rm -f server client
	rm -rf accepted_files
	mkdir accepted_files
