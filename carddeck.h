#ifndef _CARDDECK_H
#define _CARDDECK_H

#include "solitaire.h"

int isRed(card_t *c);
int isBlack(card_t *c);
int isAce(card_t *c);
int isKing(card_t *c);
int isOnTop(card_t *c);
int isLain(card_t *c);
int isUp(card_t *c);
int isSuccessorOf(card_t *c1, card_t *c2);
int suitsDiffer(card_t *c1, card_t *c2);
int isOkOn(card_t *c1, card_t *c2);
void putCardOfCode(int face, int suit);
void putCard(card_t *c);
card_t *cardOf(char *c, deck_t *d);

deck_t *newDeck();
void shuffle(deck_t *deck);

#endif
