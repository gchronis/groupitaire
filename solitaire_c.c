#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <strings.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "solitaire.h"
#include "cardstack.h"
#include "carddeck.h"

#define MAXLINE 80

int connectToServer(char *host, int port) {

  //
  // Look up the host's name to get its IP address.
  //
  struct hostent *hp;
  if ((hp = gethostbyname(host)) == NULL) {
    fprintf(stderr,"GETHOSTBYNAME failed.\n");
    exit(-1);
  }

  //
  // Request a socket and get its file descriptor.
  //
  int clientfd;
  if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"SOCKET creation failed.\n");
    exit(-1);
  }

  //
  // Fill in the host/port info into a (struct sockaddr_in).
  //
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET;
  bcopy((char *) hp->h_addr_list[0], (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
  serveraddr.sin_port = htons(port);

  //
  // Connect to the given host at the given port number.
  //
  if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr,"CONNECT failed.\n");
    exit(-1);
  }

  unsigned char *ip;
  ip = (unsigned char *)&serveraddr.sin_addr.s_addr;
  printf("Connected to solitaire service at %d.%d.%d.%d.\n",ip[0],ip[1],ip[2],ip[3]);

  return clientfd;
}

int receiveAck(int socket) {
  char buffer[MAXLINE];
  read(socket,buffer,MAXLINE);
  if (strcmp(buffer,"SUCCESS") == 0) {
    return SUCCESS;
  } else if (strcmp(buffer,"FAILURE") == 0) {
    return FAILURE;
  }
  fprintf(stderr,"Recieved unexpected acknowledgement from the server.\n");
  fprintf(stderr,"Message was '%s'.\n",buffer);
  exit(-1);
}


int main(int argc, char **argv) {

  char buffer[MAXLINE];
  
  //
  // Check for host name and port number.
  //
  if (argc != 3) {
    fprintf(stderr,"usage: %s <host> <port>\n", argv[0]);
    exit(0);
  }

  //
  // Connect to the server.
  //
  int server = connectToServer(argv[1],atoi(argv[2]));

  printf("Welcome to the remote SOLITAIRE game for MATH 442.\n\n");
  printf("Here is a key of possible card plays you can make:\n");
  printf("\tplay <face><suit> - throw a card onto an arena pile in the center.\n");
  printf("\tmove k<suit> - move a king onto an empty pile.\n");
  printf("\tmove <face><suit> <face><suit>- move a card, putting it on top of another card.\n");
  printf("\tdraw - draw and overturn the next card, placing it on the discard pile.\n");
  printf("where <face> is one of a,2,3,...,9,t,j,q,k and <suit> is one of d,c,s,h.\n");

  //
  // Send a series of solitaire play commands.
  //
  while (1) {
    printf("\nEnter a card play: ");
    
    fgets(buffer,MAXLINE,stdin);
    write(server,buffer,strlen(buffer)+1);
    if (receiveAck(server)) {
      printf("Done!\n");
    } else {
      printf("That play wasn't made.\n");
    } 
  }

}
