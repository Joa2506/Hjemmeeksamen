#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include "funksjonalitet.h"
#include "pgmread.h"
#include "send_packet.h"

//#define PORT 8080
#define MAXLINE 1500
void send_ack(struct package * pakke, int sockfd, const struct sockaddr * addr, socklen_t addrlen);
//void sammenlign_bilder(char * bilde_1, char * filnavn, const char*)
int main(int argc, char *argv[]) {

  int PORT;
  char * mappe;

  if(argc != 4)
  {
    perror("Skriv inn på formen: /portnr mappe filnavn");
    exit(EXIT_FAILURE);
  }

  char * en_port = argv[1];
  PORT = atoi(en_port);
  FILE * utfil = fopen(argv[3], "wb");

  int socket_s;
  char * buffer = malloc(MAXLINE);
  char * hello = "0";
  struct sockaddr_in servaddr, cliaddr;


  if((socket_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);


  if(bind(socket_s, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("Bind failed");
    exit(EXIT_FAILURE);
  }

  int len, n;

  len = sizeof(cliaddr);
  int tall = 0;
  int sann = 0;
  char * ack;
  struct package * p;// = malloc(sizeof(struct package));
  int Rn = 0;//Forventet verdi
while(sann == 0){
  n = recvfrom(socket_s, (char*)buffer, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
    //buffer[n] = '\0';
  //Termineringspakken er på 8 bytes
  //printf("Lengden på pakken er %d\n", n);
  if(n <= 8)
  {
    printf("Termineringspakken er mottat\n");
    sann++;
  }
  else
  {

    p = deserialize(buffer);
    //print(p);

    //Sjekk at det er riktig pakke
    if(p->sequence == Rn){
      //les_mappe(p->bilde_bytes);
      //Sammenligning av bilder ville ikke oppføre seg. Så jeg skrev bare ut filnavnet til tekstfilen
      fwrite(p->filnavn, sizeof(char), p->lengde_filnavn-1, utfil);
      fwrite("\n", sizeof(char), sizeof(char), utfil);
      printf("Mottat pakke er: %s\n", p->filnavn);
      printf("Sekvensnummer for mottatt pakke: %d\n", p->sequence);
      //printf("Forventet Sekvensnummer: %d\n", Rn);
      send_ack(p, socket_s, (const struct sockaddr *)&cliaddr, len);
      Rn++;

      free_en_pakke(p);
    }
    //Ellers bare frigjør man og lar den ligge
    else
    {

      printf("Forventet Sekvensnummer %d og mottok %d\n", Rn, p->sequence);
      printf("Sender på nytt med foresporsel %d\n", Rn);
      send_ack(p, socket_s, (const struct sockaddr *)&cliaddr, len);
      free_en_pakke(p);
    }
  }

}
  //printf("Client : %s\n", buffer);
  free(buffer);
  fclose(utfil);
  printf("Message sent\n");
  return 0;
}
