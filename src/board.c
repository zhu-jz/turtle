
/* Board.C */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "board.h"
#include "types.h"
#include "output.h"

/* "Constants" */

const char SquareString[SQUARE_NB][3] = {

 "PS", "--", "??", "??", "??", "??", "??", "??", "??", "??",
       "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1", "??",
       "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2", "??",
       "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3", "??",
       "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4", "??",
       "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5", "??",
       "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6", "??",
       "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7", "??",
       "A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8", "??",
       "??", "??", "??", "??", "??", "??", "??", "??", "??"
};

const int BestToWorst[EMPTY_NB] = {

   A1, H1, A8, H8,                 /*  Corners  */
   C1, F1, A3, H3, A6, H6, C8, F8, /* A Squares */
   C3, F3, C6, F6,                 /* F Squares */
   D1, E1, A4, H4, A5, H5, D8, E8, /* B Squares */
   D3, E3, C4, F4, C5, F5, D6, E6, /* G Squares */
   D2, E2, B4, G4, B5, G5, D7, E7, /* E Squares */
   C2, F2, B3, G3, B6, G6, C7, F7, /* D Squares */
   B1, G1, A2, H2, A7, H7, B8, G8, /* C Squares */
   B2, G2, B7, G7,                 /* X Squares */
   D4, E4, D5, E5                  /*  Centre   */
};

/* Prototypes */

static int  IsLegal   (const int *Move, int Colour);
static int  DoFlips   (int *Move, int Colour, int ***FlipSP);
static void UndoFlips (int ***FlipSP, int Colour);

/* Functions */

/* StringSquare() */

int StringSquare(const char *String) {

   int C, L;

   C = toupper(String[0]);
   L = String[1];

   if (C < 'A' || C > 'H' || L < '1' || L > '8') {
      Error("Couldn't get square from string \"%s\"",String);
      return NO_MOVE;
   }

   return 9 * (L - '1' + 1) + C - 'A' + 1;
}

/* ClearBoard() */

void ClearBoard(BOARD *Board) {

   int *Square;

   Square = Board->Square;
   do *Square++ = EMPTY; while (Square < &Board->Square[SQUARE_NB]);
   Board->Square[D4] = WHITE;
   Board->Square[E4] = BLACK;
   Board->Square[D5] = BLACK;
   Board->Square[E5] = WHITE;
   Board->Colour = BLACK;
   Board->FlipSP = Board->FlipStack;
}

/* CopyBoard() */

void CopyBoard(const BOARD *Source, BOARD *Dest) {

   const int *Square1;
   int *Square2;

   Square1 = Source->Square;
   Square2 = Dest->Square;
   do *Square2++ = *Square1++; while (Square1 < &Source->Square[SQUARE_NB]);
   Dest->Colour = Source->Colour;
   Dest->FlipSP = Dest->FlipStack;
}

/* SameBoard() */

int SameBoard(const BOARD *Board1, const BOARD *Board2) {

   int Rank, File;
   const int *Square1, *Square2;

   if (Board1->Colour != Board2->Colour) return FALSE;

   Square1 = &Board1->Square[A1];
   Square2 = &Board2->Square[A1];
   Rank    = 8;
   do {
      File = 8;
      do if (*Square1++ != *Square2++) return FALSE; while (--File != 0);
      Square1++;
      Square2++;
   } while (--Rank != 0);

   return TRUE;
}

/* DoMove() */

void DoMove(BOARD *Board, int Move) {

   int *Square;

   if (Move != PASS) {
      Square = &Board->Square[Move];
      DoFlips(Square,Board->Colour,&Board->FlipSP);
      *Square = Board->Colour;
   }
   Board->Colour = -Board->Colour;
}

/* UndoMove() */

void UndoMove(BOARD *Board, int Move) {

   if (Move != PASS) {
      UndoFlips(&Board->FlipSP,Board->Colour);
      Board->Square[Move] = EMPTY;
   }
   Board->Colour = -Board->Colour;
}

/* IsLegalMove() */

int IsLegalMove(const BOARD *Board, int Move) {

   if (Move == PASS) {
      return ! CanPlay(Board,Board->Colour) && CanPlay(Board,-Board->Colour);
   } else {
      return Board->Square[Move] == EMPTY && IsLegal(&Board->Square[Move],Board->Colour);
   }
}

/* CanPlay() */

int CanPlay(const BOARD *Board, int Colour) {

   int Rank, File;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square == EMPTY && IsLegal(Square,Colour)) return TRUE;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   return FALSE;
}

/* IsFinished() */

int IsFinished(const BOARD *Board) {

   int Rank, File;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square == EMPTY && (IsLegal(Square,BLACK) || IsLegal(Square,WHITE))) {
            return FALSE;
         }
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   return TRUE;
}

/* DiskNb() */

int DiskNb(const BOARD *Board, int Colour) {

   int Rank, File, DiskNb = 0;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square == Colour) DiskNb++;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   return DiskNb;
}

/* Mobility() */

int Mobility(const BOARD *Board, int Colour) {

   int Rank, File, Mobility = 0;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square == EMPTY && IsLegal(Square,Colour)) Mobility++;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   return Mobility;
}

/* EmptyNb() */

int EmptyNb(const BOARD *Board) {

   int Rank, File, EmptiesNb = 0;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square == EMPTY) EmptiesNb++;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   return EmptiesNb;
}

/* DiskDiff() */

int DiskDiff(const BOARD *Board) {

   int Rank, File, Score = 0, Empties = 64;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square != EMPTY) {
            Score += *Square;
            Empties--;
         }
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   if      (Score > 0) Score += Empties;
   else if (Score < 0) Score -= Empties;

   if (Board->Colour < 0) Score = -Score;

   return Score;
}

/* DiskBalance() */

int DiskBalance(const BOARD *Board) {

   int Rank, File, Score = 0;
   const int *Square = &Board->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         Score += *Square;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   if (Board->Colour < 0) Score = -Score;

   return Score;
}

/* IsLegal() */

#define IS_LEGAL(Me,Opp,Dir) \
\
   if (Move[Dir] Opp) {\
      Square = Move + 2 * Dir;\
      while (*Square Opp) Square += Dir;\
      if (*Square Me) return TRUE;\
   }

static int IsLegal(const int *Move, int Colour) {

   const int *Square;

   if (Colour > 0) {
      IS_LEGAL(>0,<0,-10)
      IS_LEGAL(>0,<0, -9)
      IS_LEGAL(>0,<0, -8)
      IS_LEGAL(>0,<0, -1)
      IS_LEGAL(>0,<0, +1)
      IS_LEGAL(>0,<0, +8)
      IS_LEGAL(>0,<0, +9)
      IS_LEGAL(>0,<0,+10)
   } else {
      IS_LEGAL(<0,>0,-10)
      IS_LEGAL(<0,>0, -9)
      IS_LEGAL(<0,>0, -8)
      IS_LEGAL(<0,>0, -1)
      IS_LEGAL(<0,>0, +1)
      IS_LEGAL(<0,>0, +8)
      IS_LEGAL(<0,>0, +9)
      IS_LEGAL(<0,>0,+10)
   }

   return FALSE;
}

/* DoFlips() */

#define DO_FLIPS(Me,Opp,Dir)\
\
   if (Move[Dir] Opp) {\
      Square = Move + Dir;\
      SP2    = SP;\
      Count2 = 0;\
      do {\
         *SP2++  = Square;\
         Count2 += 2;\
         Square += Dir;\
      } while (*Square Opp);\
      if (*Square Me) {\
         SP     = SP2;\
         Count += Count2;\
      }\
   }

static int DoFlips(int *Move, int Colour, int ***FlipSP) {

   int Count, Count2, **SP, **SP2, *Square;

   Count = 0;
   SP    = *FlipSP;
   *SP++ = NULL;

   if (Colour > 0) {
      DO_FLIPS(>0,<0,-10)
      DO_FLIPS(>0,<0, -9)
      DO_FLIPS(>0,<0, -8)
      DO_FLIPS(>0,<0, -1)
      DO_FLIPS(>0,<0, +1)
      DO_FLIPS(>0,<0, +8)
      DO_FLIPS(>0,<0, +9)
      DO_FLIPS(>0,<0,+10)
   } else {
      DO_FLIPS(<0,>0,-10)
      DO_FLIPS(<0,>0, -9)
      DO_FLIPS(<0,>0, -8)
      DO_FLIPS(<0,>0, -1)
      DO_FLIPS(<0,>0, +1)
      DO_FLIPS(<0,>0, +8)
      DO_FLIPS(<0,>0, +9)
      DO_FLIPS(<0,>0,+10)
   }

   if (Count != 0) {
      *FlipSP = SP;
      Square = *--SP;
      do {
         *Square = Colour;
         Square  = *--SP;
      } while (Square != NULL);
   }

   return Count;
}

/* UndoFlips() */

static void UndoFlips(int ***FlipSP, int Colour) {

   int **SP, *Square;

   SP = *FlipSP;

   Square = *--SP;
   do {
      *Square = Colour;
      Square  = *--SP;
   } while (Square != NULL);

   *FlipSP = SP;
}

/* End of Board.C */

