#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>


#include "funksjonalitet.h"
#include "pgmread.h"
#include "send_packet.h"
int antall_elementer = 0;
struct package * pakke_liste = NULL;
//Åppner fil for fil og lager
int les_fra_fil(char * filnavn, int fil_stoerrelse)
{

  //char * navn = filnavn;
  //navn = malloc(strlen(filnavn)+1);
  //memset(navn, 0, strlen(filnavn)+1);


  //Kode for å splitte filnavn ved "/"
  char p[strlen(filnavn)+1];
  strcpy(p, filnavn);
  char * plass;
  char * rest;
  plass = strtok(p, "/");
  rest = strtok(NULL, "");
  char * navn = malloc(strlen(rest)+1);
  memset(navn, 0, strlen(rest)+1);
  strncpy(navn, rest, strlen(rest)+1);

  int stoerrelse;
  stoerrelse = fil_stoerrelse;

  //Leser fra filen
  FILE * fptr;
  fptr = fopen(filnavn, "rb"); //Filene er binære bildefiler pgm
  char * buffer;

  struct package * pakke; //initialiserer en pakke
  if(fptr == NULL )
  {
    printf("%s\n", "Finner ikke filen");
    fclose(fptr);
    return -1;
  }
  //Alloker bytes for bildet
  buffer = malloc(stoerrelse+1);
  memset(buffer, 0, stoerrelse+1);
  fread(buffer, stoerrelse, 1, fptr);
  //Allokerer plass til structet
  pakke = malloc(sizeof(struct package));
  memset(pakke, 0 , sizeof(struct package));

  //Fyller opp structen med info.
  pakke->filnavn = navn;
  pakke->sequence = sequence_teller;
  sequence_teller++;
  pakke->ack = -1;
  pakke->flags = 0x1;
  pakke->lengde_filnavn = strlen(navn)+1;
  pakke->bilde_bytes = buffer;
  pakke->lengde = sizeof(pakke->filnavn) + sizeof(pakke->sequence)+ sizeof(pakke->flags) + (int)pakke->lengde_filnavn + strlen(pakke->bilde_bytes);
  //printf("%s\n", pakke->bilde_bytes);
  legg_til(pakke);

  //printf("%s", buffer);


  //printf("%s\n", filnavn);

  //free(buffer);
  //free(pakke);
  fclose(fptr);

  return 0;
}
//Leser alle filene i mappen


int stoerrelsefil(char * navn)
{
  //printf("%s\n", navn);
  struct stat buf;
  int rc;
  char * linje = navn;
  rc = stat(linje, &buf);
  if(rc == -1)
  {
    printf("feil\n");
    perror("stat");
    return -1;
  }
  //printf("Filnavn: %s med stoerrelse: %ld bytes\n", linje, buf.st_size);
  return buf.st_size;
}
//Leser en tekstfil fra terminal og sender linjene til ny funksjon for å lese fil for fil til heapen
int les_liste_av_filnavn(char * filnavn)
{
  char * navn = filnavn;
  FILE * fptr;
  char linje[50]; //Regner ikke med noen er større enn 25.
  fptr = fopen(navn, "r");
  int stoerrelse;
  char * string;

  if(fptr == NULL)
  {
    printf("Fant ikke filen\n");
    return -1;
  }
  //printf("%s\n", fgets(linje, sizeof(linje), fptr));

  while(fgets(linje, (int)sizeof(linje), fptr)!= NULL){
    string = strtok(linje, "\n");
    stoerrelse = stoerrelsefil(string);
    les_fra_fil(string, stoerrelse);
    //printf("%s, har stoerrelse: %d\n", linje, stoerrelse);
  }

  fclose(fptr);
}


char * serialize(struct package * pakke)
{
  char * pakke_array;
  pakke_array = malloc(pakke->lengde); //Må fries etter bruk
  int i = 0;
  //Kopierer bytes fra structet til pakken. Gjøres med hvert element for å unngå padding
  memcpy(&pakke_array[i], &pakke->lengde, sizeof(pakke->lengde));
  i += sizeof(pakke->lengde);//int
  memcpy(&pakke_array[i], &pakke->sequence, sizeof(pakke->sequence));//Unique number og request
  i += sizeof(pakke->sequence);
  memcpy(&pakke_array[i], &pakke->flags, sizeof(pakke->flags));
  i += sizeof(pakke->flags);
  memcpy(&pakke_array[i], &pakke->lengde_filnavn, sizeof(pakke->lengde_filnavn));//int
  i += sizeof(pakke->lengde_filnavn);
  strncpy(&pakke_array[i], pakke->filnavn, pakke->lengde_filnavn);//size of siden den har med terminerende /0
  i += pakke->lengde_filnavn;/*
  memcpy(&pakke_array[i], pakke->bilde_bytes, strlen(pakke->bilde_bytes));*/
  strncpy(&pakke_array[i], pakke->bilde_bytes, strlen(pakke->bilde_bytes));
  i += strlen(pakke->bilde_bytes);
  //printf("i er %d\n", i);
  return pakke_array;
}

int totlengde()
{
  return antall_elementer;
}

struct package * deserialize(char * buffer)
{
  struct package * pakke;
  pakke = malloc(sizeof(struct package));

  int bytes = 0;
  int i = 0;
  memcpy(&pakke->lengde, &buffer[i], sizeof(pakke->lengde));
  i += sizeof(pakke->lengde);
  memcpy(&pakke->sequence, &buffer[i], sizeof(pakke->sequence));
  i += sizeof(pakke->sequence);
  memcpy(&pakke->flags, &buffer[i], sizeof(pakke->flags));
  i += sizeof(pakke->flags);
  memcpy(&pakke->lengde_filnavn, &buffer[i], sizeof(pakke->lengde_filnavn));
  i += sizeof(pakke->lengde_filnavn);
  //printf("Lengde filnavn %d\n", pakke->lengde_filnavn);/*
  //memcpy(&pakke->filnavn, &buffer[i], pakke->lengde_filnavn);*/
  pakke->filnavn = malloc(pakke->lengde_filnavn);
  memcpy(pakke->filnavn, &buffer[i], pakke->lengde_filnavn);
  //printf("%s\n", pakke->filnavn);
  i += pakke->lengde_filnavn;

  bytes = pakke->lengde - i;
  pakke->bilde_bytes = malloc(bytes+1);
  memset(pakke->bilde_bytes, 0 , bytes+1);
  memcpy(pakke->bilde_bytes, &buffer[i], bytes);
  //printf("Fra deserialize: \n%s\n", pakke->bilde_bytes);

  return pakke;
}


//Legger pakke til slutten av listen
int legg_til(struct package * pakke)
{

  struct package * temp;
  temp = pakke_liste;
  if(pakke_liste == NULL)
  {
    pakke->neste = pakke_liste;
    pakke_liste = pakke;
  }
  else{
    while(temp->neste != NULL)
      {
        temp = temp->neste;
      }
      temp->neste = pakke;
      pakke->neste = NULL;
  }
  antall_elementer++;
  return 0;
}

//Frigjør minnet etter bruk
int fjern_alle()
{
  struct package *temp;

  while(pakke_liste)
  {
    temp = pakke_liste;
    pakke_liste = pakke_liste->neste;
    //printf("pakke %d blir frigjort : %s\n", temp->lengde, temp->payload);

    free(temp->filnavn);
    free(temp->bilde_bytes);
    free(temp);

    antall_elementer--;
  }
  //pakke_liste = NULL;


  return 0;
}

//Hente spesifikk pakke. Første node er i = 0
struct package * hent(int i)
{
  int teller = 0;
  if(teller < 0 || teller > antall_elementer)
  {
      printf("Må skrive inn verdi stoerre enn 0 eller minder enn %d\n", antall_elementer);
      return NULL;
  }

  struct package * temp = pakke_liste;
  while(teller < i)
  {
    //printf("i = %d og teller = %d\n", i, teller);
    temp = temp->neste;
    teller++;
  }
  return temp;
}

void print(struct package * pakke)
{

    printf("%s\n", pakke->filnavn);
    printf("Lengden: %d\n",pakke->lengde);
    printf("Sekvensnummer: %d\n", pakke->sequence);
  //  printf("Ack: %d\n", pakke->ack);
    printf("Flags: %x \n", pakke->flags );
    printf("Lengden på filnavn: %d (inkludert 0 byte)\n", pakke->lengde_filnavn); //lengde på filnavnet
    printf("%s\n", pakke->bilde_bytes);

}

//Hjelpefunksjon for å printe all info for alle noder i lenkelista.
void printalle()
{
  struct package * temp = pakke_liste;
  while(temp != NULL)
  {
    printf("%s\n", temp->filnavn);
    printf("Lengden: %d\n",temp->lengde);
    printf("Sekvensnummer: %d\n", temp->sequence);
//  printf("Ack: %d\n", temp->ack);
    printf("Flags: %x \n",temp->flags );
    printf("Lengden på filnavn: %d (inkludert 0 byte)\n", temp->lengde_filnavn); //lengde på filnavnet
    printf("%s\n", temp->bilde_bytes);
    temp = temp->neste;
  }
}


void free_en_pakke(struct package * pakke)
{
  free(pakke->bilde_bytes);
  free(pakke->filnavn);
  free(pakke);
}

//Gjør opperasjonen ved å lage en ack fra mottatt pakke og så sender acken tilbake
void send_ack(struct package * pakke, int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{

  struct package * ack = malloc(sizeof(struct package));
  ack->sequence = pakke->sequence;
  ack->ack = pakke->sequence;
  ack->flags = 0x2;
  ack->unused = 0x7f;
  ack->lengde = sizeof(ack->sequence) + sizeof(ack->ack) + sizeof(ack->flags)+sizeof(ack->unused);
  char buffer[ack->lengde];
  int i = 0;
  memcpy(&buffer[i], &ack->lengde, sizeof(ack->lengde));
  i += sizeof(ack->lengde); //int
  memcpy(&buffer[i], &ack->ack, sizeof(ack->ack));
  i += sizeof(ack->ack); //unsigned char
  memcpy(&buffer[i], &ack->flags, sizeof(ack->flags));
  i += sizeof(ack->flags); //unsigned char
  memcpy(&buffer[i], &ack->unused, sizeof(ack->unused));
  i += sizeof(ack->unused);//unsigned char

  sendto(sockfd, (const char*)buffer, i, 0, addr, addrlen);
  //printf("Sender ack for pakke : %d\n", ack->ack);
  //sammenlign_ack((char *)buffer, ack->ack);
  free(ack);
}

int sammenlign_ack(char * buffer, int forventet)
{
  struct package * pakke;
  pakke = malloc(sizeof(struct package));

  int bytes = 0;
  int i = 0;
  memcpy(&pakke->lengde, &buffer[i], sizeof(pakke->lengde));
  i += sizeof(pakke->lengde);
  memcpy(&pakke->ack, &buffer[i], sizeof(pakke->ack));
  i += sizeof(pakke->ack);
  memcpy(&pakke->flags, &buffer[i], sizeof(pakke->flags));
  i += sizeof(pakke->flags);
  memcpy(&pakke->unused, &buffer[i], sizeof(pakke->unused));
  i += sizeof(pakke->unused);

  if(pakke->ack == forventet)
  {
    //Hvis ack er det samme som forventet returnerer den 1
    free(pakke);
    return 1;
  }
  free(pakke);
  return 0;

  free(pakke);
}
void send_terminerings_pakke(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
{
  int storrelse = sizeof(int) + 4; //Alle termineringspakker er 8 bytes med de underliggende verdiene
  unsigned char sequence = 0;
  unsigned char ack = 0;
  unsigned char flag = 0x4;
  unsigned char unused = 0x7f;

  char * buffer = malloc(storrelse);
  printf("lengde er %d\n", storrelse);
  int i = 0;
  memcpy(&buffer[i], &storrelse, sizeof(storrelse));
  i += sizeof(storrelse); //int
  memcpy(&buffer[i], &sequence, sizeof(sequence));
  i += sizeof(sequence);
  memcpy(&buffer[i], &ack, sizeof(ack));
  i += sizeof(ack);
  memcpy(&buffer[i], &flag, sizeof(flag));
  i += sizeof(flag); //unsigned char
  memcpy(&buffer[i], &unused, sizeof(unused));
  i += sizeof(unused);//unsigned char


  int n = send_packet(sockfd, (char*)buffer, storrelse, 0, addr, addrlen);
  printf("Sender termineringspakke med size %d\n", n);
  //sammenlign_ack((char *)buffer, ack->ack);
  free(buffer);
}
//Returnerer 1 om de er like
int sammenlign_bilder(char * bilde1, char * bilde2)
{
  struct Image * i1 = Image_create(bilde1);
  struct Image * i2 = Image_create(bilde2);

  int svar = Image_compare(i1, i2);
  printf("%d\n", svar);
  Image_free(i1);
  Image_free(i2);
  return svar;
}

int les_mappe(char * filnavn)
{

  DIR * mappe = opendir(MAPPE); //Må endres til argv

  if(!mappe)
  {

    perror("opendir");
    exit(EXIT_FAILURE);
  }

  struct dirent *entry;
  entry = readdir(mappe);
  int stoerrelse;
  char filepath[50] = MAPPE;

  while(entry)
  {
    if(entry->d_type == DT_REG)
    {
      printf("%s\n", filepath);
      strncat(filepath, entry->d_name, strlen(filepath)+strlen(entry->d_name));
      printf("%s\n", filepath);
      stoerrelse = stoerrelsefil(filepath);
      printf("%d\n", stoerrelse);
      int ut;
      if(strcmp(filnavn, entry->d_name))
      {
        printf("%s\n%s\n", entry->d_name, filnavn);
      }
      FILE * fptr;
      fptr = fopen(filepath, "rb"); //Filene er binære bildefiler pgm
      char * buffer;

      struct package * pakke; //initialiserer en pakke
      if(fptr == NULL )
      {
        printf("%s\n", "Finner ikke filen");
        fclose(fptr);
        return -1;
      }


      //Alloker bytes for bildet
      buffer = malloc(stoerrelse+1);
      memset(buffer, 0, stoerrelse+1);
      fread(buffer, stoerrelse, 1, fptr);

      printf("UT er :%d\n", ut);

      fclose(fptr);
      free(buffer);
    }

    if(entry->d_type == DT_DIR)
    {
      printf("skip\n");
    }
    entry = readdir(mappe);
  }

  closedir(mappe);
  return 0;
}
/*
int main()
{
  les_liste_av_filnavn("list_of_filenames.txt");
  struct package * p = hent(1);
  struct package * p2 = hent(1);
  struct Image * i1 = Image_create(p->bilde_bytes);
  printf("%d\n", i1->width);

  //int n = strcmp(p->bilde_bytes, p2->bilde_bytes);
  //struct Image * i2 = Image_create(p->bilde_bytes);
  //int svar = Image_compare(i1, i2);
  //printf("%d\n", svar);
  //sammenlign_bilder(p2->bilde_bytes, p->bilde_bytes );
  //printf("%d\n", n);
  printf("%s\n", p->filnavn );
  //les_mappe(p->bilde_bytes);
  fjern_alle();
  return 0;
}*/
