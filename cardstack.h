#ifndef _CARDSTACK_H
#define _CARDSTACK_H

#include "solitaire.h"

cardstack_t *new_cardstack(int type);
cardstack_t *new_cardstack_fed_by(int type, cardstack_t *feed);
void put_cardstack(cardstack_t *stack, int reveal);
int is_empty(cardstack_t *stack);
void push(card_t *card, cardstack_t *stack);
card_t *pop(cardstack_t *stack);
card_t *top(cardstack_t *stack);
void feed(cardstack_t *stack);
void move_onto(card_t *card, cardstack_t *dest_stack);

#endif
