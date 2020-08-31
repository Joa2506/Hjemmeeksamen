#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#include "funksjonalitet.h"
#include "pgmread.h"
#include "send_packet.h"

//#define PORT 8080
#define MAXLINE 1500

int main(int argc, char *argv[])
{
  int PORT;
  float tapsprosent;
  if(argc != 5)
  {
    perror("Skriv inn på formen: /client_ip portnr filnavn tapsprosent");
    exit(EXIT_FAILURE);
  }

  char * port = argv[2];
  PORT = atoi(port);
  char * tap = argv[4];
  tapsprosent = atof(tap)/100;
  printf("%f\n", tapsprosent);


  int socket_c;
  char buffer[MAXLINE];
  set_loss_probability(tapsprosent);
  char * ut;// = "Hello from client";
  les_liste_av_filnavn(argv[3]);
  struct package * p;// = hent(2);
  //hello = serialize(p);
  struct sockaddr_in servaddr;
  if((socket_c = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = inet_addr(argv[1]);

  int n, len = sizeof(servaddr);
  unsigned char ack;
  int teller = 0;
  int sann = 0;
  int forventet = 0;
  int windowsize = 7;
  fd_set rset;
  int ready;
  struct timeval timeout;
while(sann == 0){


  p = hent(teller);

  if(p == NULL)
  {
    send_terminerings_pakke(socket_c, (const struct sockaddr *)&servaddr, len);
    sann = 1;
  }
  while(teller<windowsize && p != NULL)
  {

      ut = serialize(p);

      send_packet(socket_c, (const char *)ut, p->lengde, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
      printf("Sendt pakke: %s \nID: %d\n", p->filnavn, p->sequence);
      teller++;
      free(ut);

      p = hent(teller);
  }

  timeout.tv_sec = 5;
  timeout.tv_usec = 0;

  FD_ZERO(&rset);
  FD_SET(socket_c, &rset);

  ready = select(FD_SETSIZE, &rset, NULL, NULL, &timeout);
  //printf("%d\n", ready);
  if(ready == 0){
      teller = forventet;
  }
  else{

    n = recvfrom(socket_c, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&servaddr, &len);

    if(sammenlign_ack(buffer, forventet)==1)
    {
      windowsize++;
      forventet++;
      //printf("Forventet er %d\n", forventet);
    }
    else
    {
      //windowsize = (windowsize - teller);
      teller = forventet;
    }

  }

}

  fjern_alle();

  close(socket_c);
  return 0;

}
/*
void send_ack(const struct sockaddr* addr, socklen_t addrlen, int sock, struct package * pakke)
{
  struct package * ack = malloc(sizeof(struct package));
  char ackpack[8];
  ack->sequence = pakke->
}*/
