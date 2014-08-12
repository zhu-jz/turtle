
/* Board.H */

#ifndef BOARD_H
#define BOARD_H

/* Constants */

#define SQUARE_NB 91
#define LEGAL_NB  60
#define EMPTY_NB  64
#define FLIP_NB   (60*(18+1))

enum { BLACK = -1, EMPTY, WHITE };

enum {
   PASS = 0, NO_MOVE = 1,
   A1=10, B1, C1, D1, E1, F1, G1, H1,
   A2=19, B2, C2, D2, E2, F2, G2, H2,
   A3=28, B3, C3, D3, E3, F3, G3, H3,
   A4=37, B4, C4, D4, E4, F4, G4, H4,
   A5=46, B5, C5, D5, E5, F5, G5, H5,
   A6=55, B6, C6, D6, E6, F6, G6, H6,
   A7=64, B7, C7, D7, E7, F7, G7, H7,
   A8=73, B8, C8, D8, E8, F8, G8, H8
};

/* Types */

typedef struct {
   int   Colour;
   int   Square[SQUARE_NB];
   int **FlipSP;
   int  *FlipStack[FLIP_NB];
} BOARD;

/* "Constants" */

extern const char SquareString[SQUARE_NB][3];
extern const int  BestToWorst[EMPTY_NB];

/* Prototypes */

extern int  StringSquare (const char *String);

extern void ClearBoard   (BOARD *Board);
extern void CopyBoard    (const BOARD *Source, BOARD *Dest);
extern int  SameBoard    (const BOARD *Board1, const BOARD *Board2);

extern void DoMove       (BOARD *Board, int Move); /* PASS allowed */
extern void UndoMove     (BOARD *Board, int Move); /* PASS allowed */

extern int  IsLegalMove  (const BOARD *Board, int Move); /* PASS allowed */
extern int  CanPlay      (const BOARD *Board, int Colour);
extern int  IsFinished   (const BOARD *Board);

extern int  DiskNb       (const BOARD *Board, int Colour);
extern int  Mobility     (const BOARD *Board, int Colour);
extern int  EmptyNb      (const BOARD *Board);
extern int  DiskDiff     (const BOARD *Board); /* Winner gets empties */
extern int  DiskBalance  (const BOARD *Board); /* Winner doesn't get empties */

#endif /* ! defined BOARD_H */

/* End of Board.H */

