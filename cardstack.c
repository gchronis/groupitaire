#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "solitaire.h"
#include "cardstack.h"
#include "carddeck.h"

cardstack_t *new_cardstack(int type) {
  cardstack_t *stack = malloc(sizeof(cardstack_t));
  stack->top = NULL;
  stack->type = type;
  return stack;
}

cardstack_t *new_cardstack_fed_by(int type, cardstack_t *feed) {
  cardstack_t *stack = new_cardstack(type);
  stack->feed = feed;
  return stack;
}

void put_helper(card_t *c) {
  if (c != NULL) {
    if (c->below != NULL) {
      put_helper(c->below);
      printf(" ");
    }
    putCard(c);
  }
}
void put_cardstack(cardstack_t *stack, int reveal) {
  card_t *c = stack->top;
  if (reveal) {
    put_helper(c);
  } else {
    while (c != NULL) {
      printf("*");
      c = c->below;
    }
  }
}

int is_empty(cardstack_t *stack) {
  return stack->top == NULL;
}

void push(card_t *card, cardstack_t *stack) {
  card->below = stack->top;
  stack->top = card;
  card->stack = stack;
}

card_t *pop(cardstack_t *stack) {
  card_t *card = stack->top;
  stack->top = card->below;
  card->below = NULL;
  card->stack = NULL;
  return card;
}

card_t *top(cardstack_t *stack) {
  return stack->top;
}

void feed(cardstack_t *stack) {
  push(pop(stack->feed),stack);
}
  
void move_onto(card_t *card, cardstack_t *dest_stack) {

  // Keep track of the source stack, the stack that holds 
  // the card to be moved.
  //
  cardstack_t *srce_stack = card->stack;

  // Reassign the stack of the cards at and above the card 
  // being moved; reassign them to the destination stack.
  // 
  card_t *top_card = top(srce_stack);
  for (card_t *c = top_card; c != card; c = c->below) {
    c->stack = dest_stack;
  }
  card->stack = dest_stack;

  // Move them onto the destination stack.
  //
  srce_stack->top = card->below;
  card->below = dest_stack->top;
  dest_stack->top = top_card;

  // Perhaps flip the top card of the source stack's 
  // feeder pile onto the source stack.
  if (is_empty(srce_stack) && srce_stack->feed != NULL) {
    feed(srce_stack);
  }
}
