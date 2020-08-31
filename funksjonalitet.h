#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "pgmread.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#include "pgmread.h"
#include "send_packet.h"
#define MAPPE "big_set/"

//int placeholder = 0; //skal være en sammenligningside
struct package {

  int lengde; //Lengde på pakke
  unsigned char sequence;
  unsigned char ack;
  unsigned char flags;
  unsigned char unused;
  //Payload info
  //Lengde.
  int lengde_filnavn; //lengde på filnavnet
  char * filnavn;
  char * bilde_bytes;

  //Listepeker
  struct package * neste;
};

//Lenkeliste

int sequence_teller;
int legg_til(struct package * pakke);
int fjern_alle();
void free_en_pakke(struct package * pakke);

//Char packet som skal være all data i struct noden
char * packet_to_send;
char * liste;

//For å lese data fra mappe og bildefiler
int stoerrelsefil(char * navn);
int les_fra_fil(char * filnavn, int fil_stoerrelse);
int les_mappe();
int les_liste_av_filnavn(char * filnavn);
struct package * hent(int i);
void print(struct package * pakke);
void printalle();
int sammenlign_bilder(char * bilde1, char * bilde2);
//Funksjonalitet for å lage packets
char * serialize(struct package * akke);
struct package * deserialize(char * payload);
char * create_ack(struct package * pakke);
unsigned char check_ack(char * ack_bytes);
int totlengde();
int sammenlign_ack(char * buffer, int tall);
void send_terminerings_pakke(int sockfd, const struct sockaddr * addr, socklen_t addrlen);
