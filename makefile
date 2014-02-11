all: client server

client:client.c 
	gcc -W -Wall -o client client.c 

server:server.c 
	gcc -W -Wall -o server server.c
