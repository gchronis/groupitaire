#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "solitaire.h"
#include "cardstack.h"
#include "carddeck.h"

#define MAXLINE 80
#define MAXCONNECTIONS 1024

typedef struct _client_t {
	int id;
	int connections;
	srtruct sockaddr_in address;
	arena_t arena;
	deck_t deck;
} client_t;

arena_t *newArena() {
  arena_t *arena = (arena_t *)malloc(sizeof(arena_t));
  for (int s = 0; s < 4; s++) {
    arena->highest[s] = 0;
  }
  return arena;
}

deal_t *newDeal(deck_t *deck) {

  deal_t *deal = (deal_t *)malloc(sizeof(deal_t));

  // Make all the stacks.
  deal->draw = new_cardstack(DRAW);
  deal->discard = new_cardstack_fed_by(DISCARD,deal->draw);
  for (int p = 0; p < 7; p++) {
    deal->hidden[p] = new_cardstack(HIDDEN);
    deal->lain[p] = new_cardstack_fed_by(LAIN,deal->hidden[p]);
  }

  // Build the draw pile.
  for (int i = 0; i < 52; i++) {
    push(&deck->cards[deck->order[i]],deal->draw);
  }

  // Fill each of the hidden piles.
  for (int i = 0; i < 7; i++) {
    // Place first cards, then second cards, then third...
    for (int j = i; j < 7; j++) {
      push(pop(deal->draw),deal->hidden[j]);
    }
  }
  
  // Flip the top card of each pile.
  for (int i = 0; i < 7; i++) {
    feed(deal->lain[i]);
  }

  // Flip up the first card.
  feed(deal->discard);

  return deal;
}

void putArena(arena_t *arena) {
  for (int s=0; s<4; s++) {
    printf("[");
    putCardOfCode(arena->highest[s],s);
    printf("] ");
  }
  printf("\n");
}

void putDeal(deal_t *deal) {
  for (int p = 6; p >= 0; p--) {
    printf("[");
    put_cardstack(deal->hidden[p],FALSE);
    printf("]");
    put_cardstack(deal->lain[p],TRUE);
    printf("\n");
  }
  put_cardstack(deal->discard,TRUE);
  printf("\n");
  printf("[");
  put_cardstack(deal->draw,FALSE);
  printf("]\n");
}

int arenaTake(card_t *card, arena_t *arena) {
  if (arena->highest[card->suit] == card->face-1) {
    arena->highest[card->suit]++;
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

int makeArenaPlay(card_t *card, arena_t *arena) {

  cardstack_t *stack = card->stack;

  if (isOnTop(card) && (arenaTake(card,arena) == SUCCESS)) {
    pop(stack);
    if (stack->type == LAIN && is_empty(stack) && !is_empty(stack->feed)) {
      feed(stack);
    }
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

cardstack_t *findEmptyLainStack(deal_t *deal) {
  for (int p = 0; p < 7; p++) {
    if (is_empty(deal->lain[p])) {
      return deal->lain[p];
    }
  }
  return NULL;
}

int moveKingOntoFree(card_t *king, deal_t *deal) {
  cardstack_t *dest = findEmptyLainStack(deal);
  if (isUp(king) && dest != NULL) {
    move_onto(king,dest);
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

int moveCardOntoAnother(card_t *card, card_t *onto) {
  cardstack_t *dest = onto->stack;
  if ((dest->type == LAIN) && isUp(card) && isOnTop(onto) && isOkOn(card,onto)) {
    move_onto(card,dest);
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

int pullFromDrawPile(deal_t *deal) {
  if (is_empty(deal->draw)) {
    return FAILURE;
  } else {
    feed(deal->discard);
    return SUCCESS;
  }
}

void getNextPlay(int stream, play_t *play, deck_t *deck) {
  static char buffer[MAXLINE];
  static char cmd[MAXLINE];
  static char from[MAXLINE];
  static char onto[MAXLINE];
  read(stream,buffer,MAXLINE);
  sscanf(buffer,"%s",cmd);
  if (cmd[0] == 'm') {
    // it's a move onto a lain stack
    sscanf(buffer,"%s %s",cmd,from);
    play->from = cardOf(from,deck);
    if (isKing(play->from)) {
      play->type = KING_PLAY;
      play->onto = NULL;
    } else {
      play->type = MOVE_PLAY;
      sscanf(buffer,"%s %s %s",cmd,from,onto);
      play->onto = cardOf(onto,deck);
    }
  } else if (cmd[0] == 'p') {
    // it's a play onto the arena
    play->type = ARENA_PLAY;
    sscanf(buffer,"%s %s",cmd,from);
    play->from = cardOf(from,deck);
    play->onto = NULL;
  } else if (cmd[0] == 'd') {
    // it's a draw
    play->type = DRAW_PLAY;
    play->from = NULL;
    play->onto = NULL;
  }
}
  
void sendAck(int client, int ack) {
  if (ack == SUCCESS) {
    write(client,"SUCCESS",8);
  } else {
    write(client,"FAILURE",8);
  }
}

void playSolitaire(void *client) {

  // initialize
  deal_t *deal = newDeal(deck);

  play_t play;
  int status = SUCCESS;
  while (1) {
    putArena(arena);
    printf("\n");
    putDeal(deal);
    printf("\nEnter your play: ");
    getNextPlay(client,&play,deck);
    switch (play.type) {
    case ARENA_PLAY:
      // Play a card into the arena.
      status = makeArenaPlay(play.from,arena);
      break;
    case KING_PLAY:
      // Move a king card onto an empty lain stack.
      status = moveKingOntoFree(play.from,deal);
      break;
    case MOVE_PLAY:
      // Move a card (possibly with cards above it) onto some lain stack. 
      status = moveCardOntoAnother(play.from,play.onto);
      break;
    case DRAW_PLAY:
      // Draws the next card and puts it on top of the discard pile.
      status = pullFromDrawPile(deal);
      break;
    }
    
    sendAck(client,status);
  }
}

int acceptClientOn(int socket) {

    socklen_t acceptlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    int connection;

    if ((connection = accept(socket, (struct sockaddr *)&clientaddr, &acceptlen)) < 0) {
      fprintf(stderr,"ACCEPT failed.\n");
      exit(-1);
    }

    //
    // Report the client that connected.
    //
    struct hostent *hostp;
    if ((hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			       sizeof(struct in_addr), AF_INET)) == NULL) {
      fprintf(stderr, "GETHOSTBYADDR failed.");
    }
    
    printf("Accepted connection from client %s (%s)\n", 
	   hostp->h_name, inet_ntoa(clientaddr.sin_addr));
    
    return connection;
}

int initConnection(int port) {
  //
  // Open a socket to listen for client connections.
  //
  int listenfd;
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr,"SOCKET creation failed.\n");
    exit(-1);
  }
  
  //
  // Build the service's info into a (struct sockaddr_in).
  //
  struct sockaddr_in serveraddr;
  bzero((char *) &serveraddr, sizeof(struct sockaddr_in));
  serveraddr.sin_family = AF_INET; 
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
  serveraddr.sin_port = htons((unsigned short)port); 

  //
  // Bind that socket to a port.
  //
  if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) < 0) {
    fprintf(stderr,"BIND failed.\n");
    exit(-1);
  }

  //
  // Listen for client connections on that socket.
  //
  if (listen(listenfd, MAXCONNECTIONS) < 0) {
    fprintf(stderr,"LISTEN failed.\n");
    exit(-1);
  }
  
  fprintf(stderr,"Solitaire server listening on port %d...\n",port);

  return listenfd;
}
  
void srand48(long);
double drand48(void);

int main(int argc, char **argv) {

  // 
  // Make sure we've been given a port to listen on.
  //
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(0);
  }
  
  arena_t *arena = newArena();
  int listener = initConnection(atoi(argv[1]));
  
  int clients = 0
  while (1) {
  	clients++;
  	
  //
  // Build a client's profile to send to a handler thread
  //
  client_t *client = (client_t *)malloc(sizeof(client_t));
  
  //
  // Accept a connection from a client, get a file descriptor for communicating
  // with the client
  //
  client->id = clients;
  int client = acceptClientOn(listener);
  }
  
  //
  // create a thread to handle the client
  //
  pthread_t tid;
  pthread_create(&tid,NULL,playSolitaire, (void *)client);

  unsigned long seed = 0;
  if (argc == 1) {
    struct timeval tp; 
    gettimeofday(&tp,NULL); 
    seed = tp.tv_sec;
  } else {
    seed = atol(argv[1]);
  }
  printf("Shuffling with seed %ld.\n",seed);
  srand48(seed);


  deck_t *deck = newDeck();
  shuffle(deck);
  playSolitaire(client);
}
}
