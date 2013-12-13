#ifndef _SOLITAIRE_H
#define _SOLITAIRE_H

#define FALSE 0
#define TRUE (!FALSE)

//
// CONSTANTS for the colors of suits
//
#define BLACK 0
#define RED   1

//
// CONSTANTS for marking suits of cards, and stacks of cards
//
#define SPADES    0
#define HEARTS    1
#define CLUBS     2
#define DIAMONDS  3

// 
// CONSTANTS for marking stacks of cards
//
#define DRAW           4
#define DISCARD        5
#define LAIN           6  
#define HIDDEN         7
#define TEMP           (-1)

// 
// ACKNOWLEDGEMENT of a card play action
//
#define FAILURE 0
#define SUCCESS (!FAILURE)

//
// PLAY action type
//
#define ARENA_PLAY 0
#define KING_PLAY  1
#define MOVE_PLAY  2
#define DRAW_PLAY  3

//
// CARD linked list struct (for stacking cards)
//
typedef struct _card_t {
  int player;
  int face;
  int suit;
  char get[3];
  char put[12];
  struct _card_t *below;
  struct _cardstack_t *stack;
} card_t;

//
// DECK struct for representing the full variety of 52 cards
// 
typedef struct _deck_t {
  int player;
  card_t cards[52];
  int order[52];
} deck_t;

//
// CARDSTACK struct for representing a stacked pile of cards, or a layout of played cards
//
typedef struct _cardstack_t {
  card_t *top;
  struct _cardstack_t *feed;
  int type;  
  // type is one of
  //   a SPADES arena pile
  //   a HEARTS arena pile
  //   a CLUBS arena pile
  //   a DIAMONDS arena pile
  //   the DRAW pile
  //   the DISCARD pile
  //   the 7 hidden piles HIDDEN
  //   the 7 lain piles LAIN
} cardstack_t;

//
// SOLITAIRE struct for the play in front of the player
//
typedef struct _deal_t {
  int player;
  deck_t *deck;
  cardstack_t *hidden[7];
  cardstack_t *lain[7];
  cardstack_t *discard;
  cardstack_t *draw;
} deal_t;

//
// ARENA struct for the playing the "ace through king" stacks
// 
typedef struct _arena_t {
  int highest[4];
} arena_t;

//
// PLAY struct for handling solitaire plays
//
typedef struct _play_t {
  int type;
  card_t *from;
  card_t *onto;
} play_t;

#endif
