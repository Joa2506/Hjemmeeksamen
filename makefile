all: client server

client: client.c
	gcc -g -std=gnu11 -o 	client funksjonalitet.c funksjonalitet.h pgmread.h pgmread.c send_packet.h send_packet.c client.c
server: server.c
	gcc -g -std=gnu11 -o server funksjonalitet.c funksjonalitet.h pgmread.h pgmread.c send_packet.h send_packet.c server.c

clean:
	-rm -f client
	-rm -f server
