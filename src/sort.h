
/* Sort.H */

#ifndef SORT_H
#define SORT_H

#include "board.h"
#include "mboard.h"

/* Constants */

#define LEGAL_MOVE_NB 32

/* Types */

typedef struct {
   int Move;
   int Depth;
   int Min;
   int Max;
} ROOT_MOVE;

typedef struct {
   void *Move;
   int   Note;
} MOVE_NOTE;

/* Variables */

extern ROOT_MOVE  RootMove[LEGAL_MOVE_NB+1];
extern int        RootMoveNb;

extern MSQUARE   *KillerLists[2][SQUARE_NB][64]; /* EMPTY_NB+2 */

/* Prototypes */

extern void InitRootList    (const BOARD *Board, const int Forbidden[]);
extern void SortRootList    (void);
extern void MoveToFront     (int MoveNo);

extern void InitKillerLists (const BOARD *Board);

extern void SortMove        (MOVE_NOTE *First, MOVE_NOTE *Last);

#endif /* ! defined SORT_H */

/* End of Sort.H */

