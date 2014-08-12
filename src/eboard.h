
/* EBoard.H */

#ifndef EBOARD_H
#define EBOARD_H

#include "board.h"

/* Types */

typedef struct ESquare {
   int Disk;
   int Move;
   int FlipKey[2];
   int MoveKey[2][2];
   struct ESquare *Succ;
   struct ESquare *Pred;
} ESQUARE;

typedef struct {
   int       Colour;
   ESQUARE **FlipSP;
   int       Key[2];
   ESQUARE   Square[SQUARE_NB];
   ESQUARE  *FlipStack[FLIP_NB];
} EBOARD;

/* Variables */

extern EBOARD EBoard[1];

/* Prototypes */

extern void EInitBoard   (void);
extern void ECopyBoard   (const BOARD *Board);
extern void EDoPass      (void);
extern int  EDoHashFlips (ESQUARE *Move);
extern int  EDoFlips     (ESQUARE *Move);
extern void EUndoFlips   (ESQUARE *Move);
extern int  ECountFlips  (const ESQUARE *Move, int Colour);
extern int  EIsLegal     (const ESQUARE *Move, int Colour);
extern int  EDiskDiff    (void);

#endif /* ! defined EBOARD_H */

/* End of EBoard.H */

