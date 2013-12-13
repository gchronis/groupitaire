#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "solitaire.h"
#include "cardstack.h"
#include "carddeck.h"

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
  printf("onto LAIN %d %d\n", dest->type == LAIN, dest->type);
  printf("card LAIN %d %d\n", card->stack->type == LAIN, card->stack->type);
  printf("isUp %d\n",isUp(card));
  printf("isOnTop %d\n",isOnTop(onto));
  printf("isOkOn %d\n",isOkOn(card,onto));
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

void getNextPlay(play_t *play, deck_t *deck) {
  static char buffer[80];
  static char cmd[80];
  static char from[80];
  static char onto[80];
  fgets(buffer,80,stdin);
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
  
void playSolitaire(deck_t *deck) {

  // initialize
  arena_t *arena = newArena();
  deal_t *deal = newDeal(deck);

  play_t play;
  int status = SUCCESS;
  while (1) {
    putArena(arena);
    printf("\n");
    putDeal(deal);
    printf("\nEnter your play: ");
    getNextPlay(&play,deck);
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
    
    if (status == SUCCESS) {
      printf("Done!\n");
    } else {
      printf("That play wasn't made.\n");
    } 

  }
}
  
void srand48(long);
double drand48(void);

int main(int argc, char **argv) {

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
  playSolitaire(deck);
}
