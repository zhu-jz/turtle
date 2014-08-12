
/* MBoard.H */

#ifndef MBOARD_H
#define MBOARD_H

#include "types.h"
#include "board.h"

/* Constants */

#define INDEX_NB 34

/* Types */

typedef struct {
   int *Index;
   int  Inc1;  /* ushort */
   int  Inc2;  /* ushort */
} INDEX;

typedef struct {
   int   Disk;
   int   Move;
   int   FlipKey[2];
   int   MoveKey[2][2];
   INDEX Index[4];
} MSQUARE;

typedef struct {
   int       Colour;
   MSQUARE **FlipSP;
   int       Key[2];
   MSQUARE   Square[SQUARE_NB];
   int       Index[INDEX_NB+1]; /* Index[INDEX_NB] = dummy index */
   MSQUARE  *FlipStack[FLIP_NB];
} MBOARD;

/* Variables */

extern MBOARD MBoard[1];

/* Prototypes */

extern void MInitBoard     (void);
extern void MCopyBoard     (const BOARD *Board);
extern void MDoPass        (void);
extern int  MDoHashFlips   (MSQUARE *Move);
extern int  MDoFlips       (MSQUARE *Move);
extern void MUndoHashFlips (MSQUARE *Move);
extern void MUndoFlips     (MSQUARE *Move);
extern int  MIsLegal       (const MSQUARE *Move, int Colour);
extern int  MDiskDiff      (void);

#endif /* ! defined MBOARD_H */

/* End of MBoard.H */

