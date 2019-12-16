all : server client
client : ex2_client.o
	gcc -o client ex2_client.c
server : ex2_server.o
	gcc -o server ex2_server.c
ex2_client.o : ex2_client.c
	gcc -c ex2_client.c
ex2_server.o : ex2_server.c
	gcc -c ex2_server.c