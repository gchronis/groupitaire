#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "solitaire.h"
#include "cardstack.h"
#include "carddeck.h"

//
// STRING CONSTANTS for changing the color of terminal output (ANSI)
//
char red[]       = {0x1b,0x5b,0x33,0x31,0x6d,0}; // set to red 
char black[]     = {0x1b,0x5b,0x33,0x30,0x6d,0}; // set to black
char neutral[]   = {0x1b,0x5b,0x33,0x39,0x6d,0}; // return to the default color

//
// STRING CONSTANTS for output of suit characters (UNICODE)
//
char spades[]   = {0xE2, 0x99, 0xA0, 0};
char hearts[]   = {0xE2, 0x99, 0xA5, 0};
char clubs[]    = {0xE2, 0x99, 0xA3, 0};
char diamonds[] = {0xE2, 0x99, 0xA6, 0};

//
// TABLES of outputs for card display, inputs for card play
//    
char *out_suit[] = {spades,hearts,clubs,diamonds};
char *in_suit_upper[] = {"S","H","C","D"};
char *in_suit_lower[] = {"s","h","c","d"};
char *face_upper[] = {" ","A","2","3","4","5","6","7","8","9","T","J","Q","K"};
char *face_lower[] = {" ","a","2","3","4","5","6","7","8","9","t","j","q","k"};

int isRed(card_t *c) {
  return c->suit % 2 == RED;
}

int isBlack(card_t *c) {
  return c->suit % 2 == BLACK;
}

int isAce(card_t *c) {
  return c->face == 1;
}

int isKing(card_t *c) {
  return c->face == 13;
}

int isOnTop(card_t *c) {
  return top(c->stack) == c;
}

int isUp(card_t *c) {
  return (c->stack->type == LAIN) || (isOnTop(c) && (c->stack->type == DISCARD));
}

int isSuccessorOf(card_t *c1, card_t *c2) {
  return (c1->face == c2->face+1);
}

int suitsDiffer(card_t *c1, card_t *c2) {
  return (isRed(c1) && isBlack(c2)) || (isRed(c2) && isBlack(c1));
}

int isOkOn(card_t *c1, card_t *c2) {
  return suitsDiffer(c1,c2) && isSuccessorOf(c2,c1);
}

void putCardOfCode(int face, int suit) {
  printf("%s", suit % 2 == RED ? red : black);
  printf("%s", out_suit[suit]);
  printf("%s", face_upper[face]);
  printf("%s", neutral);
}

void putCard(card_t *c) {
  printf("%s",c->put);
}

card_t *cardOf(char *c, deck_t *d) {
  int i;
  if (c[0] >= '2' && c[0] <= '9') {
    i = c[0] - '0';
  } else if (c[0] == 'T' || c[0] == 't') {
    i = 10;
  } else if (c[0] == 'J' || c[0] == 'j') {
    i = 11;
  } else if (c[0] == 'Q' || c[0] == 'q') {
    i = 12;
  } else if (c[0] == 'K' || c[0] == 'k') {
    i = 13;
  } else if (c[0] == 'A' || c[0] == 'a') {
    i = 1;
  } else {
    return NULL;
  }
  int j;
  if (c[1] == 'S' || c[1] == 's') {
    j = SPADES;
  } else if (c[1] == 'H' || c[1] == 'h') {
    j = HEARTS;
  } else if (c[1] == 'C' || c[1] == 'c') {
    j = CLUBS;
  } else if (c[1] == 'D' || c[1] == 'd') {
    j = DIAMONDS;
  } else {
    return NULL;
  }
  return &d->cards[j*13+i-1];
}

deck_t *newDeck() {

  // Make a fresh deck of cards.
  deck_t *deck = malloc(sizeof(deck_t));

  int i,s;
  for (i=0, s=0; s <= 3; s++) {
    for (int f=1; f <= 13; f++, i++) {
      deck->cards[i].suit = s;
      deck->cards[i].face = f;
      sprintf(deck->cards[i].put,"%s%s%s%s",s%2 == RED ? red : black, face_upper[f], out_suit[s], neutral);
      sprintf(deck->cards[i].get,"%s%s", face_lower[f], in_suit_lower[s]);
      deck->cards[i].below = NULL;
      deck->cards[i].stack = NULL;
    }
  }
  
  // Keep it sorted.
  for (i = 0; i < 52; i++) {
    deck->order[i] = i;
  }

  return deck;
}

void srand48(long);
double drand48(void);

void shuffle(deck_t *deck) {

  struct timeval tp; 
  gettimeofday(&tp,NULL); 
  srand48(tp.tv_sec);

  // Perform a Knuth shuffle.
  for (int i = 0; i < 52; i++) {

    // Select the next card.
    int r = (int)((52-i) * drand48());
    if (r != 0) {
      int tmp = deck->order[i];
      deck->order[i] = deck->order[i+r];
      deck->order[i+r] = tmp;
    }

  }
}
