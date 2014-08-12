
/* MBoard.C */

#include <stdio.h>
#include <stdlib.h>

#include "mboard.h"
#include "types.h"
#include "board.h"
#include "eval.h"
#include "hash.h"

/* Constants */

#define MAX_SQUARE_NB 10

/* Types */

typedef struct {
   int Direction; /*   0 -   1 |   2 /   3 \   */
   int SquareNb;
   int Square[MAX_SQUARE_NB];
} PATTERN;

typedef struct {
   int DiskDiff;
   int EmptyNb;
} COUNT;

/* "Constants" */

static const int LineIndexNo[8] = { /* Horizontal patterns */
   0, 1, 2, 3, 8, 9, 12, 13
};

static const PATTERN Pattern[INDEX_NB] = {

   /* Edge & Hor2 */

   { 0,  8, { A1, B1, C1, D1, E1, F1, G1, H1 }         },
   { 0,  8, { A2, B2, C2, D2, E2, F2, G2, H2 }         },
   { 0,  8, { A8, B8, C8, D8, E8, F8, G8, H8 }         },
   { 0,  8, { A7, B7, C7, D7, E7, F7, G7, H7 }         },
   { 1,  8, { A8, A7, A6, A5, A4, A3, A2, A1 }         },
   { 1,  8, { B8, B7, B6, B5, B4, B3, B2, B1 }         },
   { 1,  8, { H8, H7, H6, H5, H4, H3, H2, H1 }         },
   { 1,  8, { G8, G7, G6, G5, G4, G3, G2, G1 }         },

   /* Hor3 */

   { 0,  8, { A3, B3, C3, D3, E3, F3, G3, H3 }         },
   { 0,  8, { A6, B6, C6, D6, E6, F6, G6, H6 }         },
   { 1,  8, { C8, C7, C6, C5, C4, C3, C2, C1 }         },
   { 1,  8, { F8, F7, F6, F5, F4, F3, F2, F1 }         },

   /* Hor4 */

   { 0,  8, { A4, B4, C4, D4, E4, F4, G4, H4 }         },
   { 0,  8, { A5, B5, C5, D5, E5, F5, G5, H5 }         },
   { 1,  8, { D8, D7, D6, D5, D4, D3, D2, D1 }         },
   { 1,  8, { E8, E7, E6, E5, E4, E3, E2, E1 }         },

   /* Triangle */

   { 2, 10, { A1, A2, B1, A3, B2, C1, A4, B3, C2, D1 } },
   { 2, 10, { H8, G8, H7, F8, G7, H6, E8, F7, G6, H5 } },
   { 3, 10, { H1, G1, H2, F1, G2, H3, E1, F2, G3, H4 } },
   { 3, 10, { A8, A7, B8, A6, B7, C8, A5, B6, C7, D8 } },

   /* Diag5 */

   { 2,  5, { A5, B4, C3, D2, E1 }                     },
   { 2,  5, { D8, E7, F6, G5, H4 }                     },
   { 3,  5, { D1, E2, F3, G4, H5 }                     },
   { 3,  5, { A4, B5, C6, D7, E8 }                     },

   /* Diag6 */

   { 2,  6, { A6, B5, C4, D3, E2, F1 }                 },
   { 2,  6, { C8, D7, E6, F5, G4, H3 }                 },
   { 3,  6, { C1, D2, E3, F4, G5, H6 }                 },
   { 3,  6, { A3, B4, C5, D6, E7, F8 }                 },

   /* Diag7 */

   { 2,  7, { A7, B6, C5, D4, E3, F2, G1 }             },
   { 2,  7, { B8, C7, D6, E5, F4, G3, H2 }             },
   { 3,  7, { B1, C2, D3, E4, F5, G6, H7 }             },
   { 3,  7, { A2, B3, C4, D5, E6, F7, G8 }             },

   /* Diag8 */

   { 2,  8, { A8, B7, C6, D5, E4, F3, G2, H1 }         },
   { 3,  8, { A1, B2, C3, D4, E5, F6, G7, H8 }         }
};

/* Variables */

MBOARD MBoard[1];

static COUNT Count[6561];

/* Prototypes */

static void InitPatt    (void);
static void InitPattKey (void);
static void InitCount   (void);

/* Functions */

/* MInitBoard() */

void MInitBoard(void) {

   InitPatt();
   InitPattKey();
   InitCount();
}

/* InitPatt() */

static void InitPatt(void) {

   int I, J, Inc;
   int Power[MAX_SQUARE_NB];

   for (Power[0] = 1, I = 1; I < MAX_SQUARE_NB; I++) Power[I] = 3 * Power[I-1];

   for (I = 0; I < SQUARE_NB; I++) {
      MBoard->Square[I].Move = I;
      for (J = 0; J < 4; J++) {
         MBoard->Square[I].Index[J].Index = &MBoard->Index[INDEX_NB];
         MBoard->Square[I].Index[J].Inc1  = 0;
         MBoard->Square[I].Index[J].Inc2  = 0;
      }
   }

   for (I = 0; I < INDEX_NB; I++) {
      for (J = 0; J < Pattern[I].SquareNb; J++) {
         Inc = Power[Pattern[I].SquareNb-J-1];
         MBoard->Square[Pattern[I].Square[J]].Index[Pattern[I].Direction].Index = &MBoard->Index[I];
         MBoard->Square[Pattern[I].Square[J]].Index[Pattern[I].Direction].Inc1  = Inc;
         MBoard->Square[Pattern[I].Square[J]].Index[Pattern[I].Direction].Inc2  = 2 * Inc;
      }
   }
}

/* InitPattKey() */

static void InitPattKey(void) {

   int Square, Rank, File;

   for (Rank = 0, Square = A1; Rank < 8; Rank++, Square++) {
      for (File = 0; File < 8; File++, Square++) {
         MBoard->Square[Square].FlipKey[0]    = HashCode[2][Square][0];
         MBoard->Square[Square].FlipKey[1]    = HashCode[2][Square][1];
         MBoard->Square[Square].MoveKey[0][0] = HashCode[2][0][0] ^ HashCode[0][Square][0];
         MBoard->Square[Square].MoveKey[0][1] = HashCode[2][0][1] ^ HashCode[0][Square][1];
         MBoard->Square[Square].MoveKey[1][0] = HashCode[2][0][0] ^ HashCode[1][Square][0];
         MBoard->Square[Square].MoveKey[1][1] = HashCode[2][0][1] ^ HashCode[1][Square][1];
      }
   }
}

/* InitCount() */

static void InitCount(void) {

   int Index, I, J, Tab, DiskDiff, EmptyNb;

   for (Index = 0; Index < 6561; Index++) {
      DiskDiff = 0;
      EmptyNb  = 0;
      for (I = 0, J = Index; I < 8; I++, J /= 3) {
         Tab = J % 3 - 1;
         DiskDiff += Tab;
         if (Tab == 0) EmptyNb++;
      }
      Count[Index].DiskDiff = DiskDiff;
      Count[Index].EmptyNb  = EmptyNb;
   }
}

/* MCopyBoard() */

void MCopyBoard(const BOARD *Board) {

   int I, J;

   MBoard->Colour = Board->Colour;
   MBoard->FlipSP = MBoard->FlipStack;

   for (I = 0; I < SQUARE_NB; I++) MBoard->Square[I].Disk = Board->Square[I];

   for (I = 0; I < INDEX_NB; I++) {
      MBoard->Index[I] = 0;
      for (J = 0; J < Pattern[I].SquareNb; J++) {
         MBoard->Index[I] = 3 * MBoard->Index[I] + Board->Square[Pattern[I].Square[J]];
      }
   }
   MBoard->Index[INDEX_NB] = 0;

   InitIndices();

   CompHashKeys(Board,MBoard->Key);
}

/* MDoPass() */

void MDoPass(void) {

   MBoard->Colour  = -MBoard->Colour;
   MBoard->Key[0] ^= HashCode[2][0][0];
   MBoard->Key[1] ^= HashCode[2][0][1];
}

#define M_DO_FLIPS(Me,Opp,Dir)\
\
   if (Move[Dir].Disk Opp) {\
      Square = Move + Dir;\
      SP2    = SP;\
      do {\
         *++SP2  = Square;\
         Square += Dir;\
      } while (Square->Disk Opp);\
      if (Square->Disk Me) SP = SP2;\
   }

/* MDoHashFlips() */

int MDoHashFlips(MSQUARE *Move) {

   int       Colour, NewKey[2];
   MSQUARE **SP, **SP2;
   MSQUARE  *Square;

   Colour = MBoard->Colour;
   SP     = MBoard->FlipSP;

   if (Colour > 0) {

      M_DO_FLIPS(>0,<0,-10)
      M_DO_FLIPS(>0,<0, -9)
      M_DO_FLIPS(>0,<0, -8)
      M_DO_FLIPS(>0,<0, -1)
      M_DO_FLIPS(>0,<0, +1)
      M_DO_FLIPS(>0,<0, +8)
      M_DO_FLIPS(>0,<0, +9)
      M_DO_FLIPS(>0,<0,+10)

      if (SP == MBoard->FlipSP) return FALSE;
      MBoard->FlipSP = SP;

      NewKey[0] = MBoard->Key[0];
      NewKey[1] = MBoard->Key[1];

      Square = *SP;
      do {
         Square->Disk = Colour;
         NewKey[0] ^= Square->FlipKey[0];
         NewKey[1] ^= Square->FlipKey[1];
         *Square->Index[0].Index += Square->Index[0].Inc2;
         *Square->Index[1].Index += Square->Index[1].Inc2;
         *Square->Index[2].Index += Square->Index[2].Inc2;
         *Square->Index[3].Index += Square->Index[3].Inc2;
      } while ((Square = *--SP) != NULL);

      NewKey[0] ^= Move->MoveKey[1][0];
      NewKey[1] ^= Move->MoveKey[1][1];
      *Move->Index[0].Index += Move->Index[0].Inc1;
      *Move->Index[1].Index += Move->Index[1].Inc1;
      *Move->Index[2].Index += Move->Index[2].Inc1;
      *Move->Index[3].Index += Move->Index[3].Inc1;

   } else {

      M_DO_FLIPS(<0,>0,-10)
      M_DO_FLIPS(<0,>0, -9)
      M_DO_FLIPS(<0,>0, -8)
      M_DO_FLIPS(<0,>0, -1)
      M_DO_FLIPS(<0,>0, +1)
      M_DO_FLIPS(<0,>0, +8)
      M_DO_FLIPS(<0,>0, +9)
      M_DO_FLIPS(<0,>0,+10)

      if (SP == MBoard->FlipSP) return FALSE;
      MBoard->FlipSP = SP;

      NewKey[0] = MBoard->Key[0];
      NewKey[1] = MBoard->Key[1];

      Square = *SP;
      do {
         Square->Disk = Colour;
         NewKey[0] ^= Square->FlipKey[0];
         NewKey[1] ^= Square->FlipKey[1];
         *Square->Index[0].Index -= Square->Index[0].Inc2;
         *Square->Index[1].Index -= Square->Index[1].Inc2;
         *Square->Index[2].Index -= Square->Index[2].Inc2;
         *Square->Index[3].Index -= Square->Index[3].Inc2;
      } while ((Square = *--SP) != NULL);

      NewKey[0] ^= Move->MoveKey[0][0];
      NewKey[1] ^= Move->MoveKey[0][1];
      *Move->Index[0].Index -= Move->Index[0].Inc1;
      *Move->Index[1].Index -= Move->Index[1].Inc1;
      *Move->Index[2].Index -= Move->Index[2].Inc1;
      *Move->Index[3].Index -= Move->Index[3].Inc1;
   }

   Move->Disk     = Colour;
   MBoard->Colour = -Colour;
   MBoard->Key[0] = NewKey[0];
   MBoard->Key[1] = NewKey[1];

   return TRUE;
}

/* MDoFlips() */

int MDoFlips(MSQUARE *Move) {

   int       Colour;
   MSQUARE **SP, **SP2;
   MSQUARE  *Square;

   Colour = MBoard->Colour;
   SP     = MBoard->FlipSP;

   if (Colour > 0) {
      M_DO_FLIPS(>0,<0,-10)
      M_DO_FLIPS(>0,<0, -9)
      M_DO_FLIPS(>0,<0, -8)
      M_DO_FLIPS(>0,<0, -1)
      M_DO_FLIPS(>0,<0, +1)
      M_DO_FLIPS(>0,<0, +8)
      M_DO_FLIPS(>0,<0, +9)
      M_DO_FLIPS(>0,<0,+10)
   } else {
      M_DO_FLIPS(<0,>0,-10)
      M_DO_FLIPS(<0,>0, -9)
      M_DO_FLIPS(<0,>0, -8)
      M_DO_FLIPS(<0,>0, -1)
      M_DO_FLIPS(<0,>0, +1)
      M_DO_FLIPS(<0,>0, +8)
      M_DO_FLIPS(<0,>0, +9)
      M_DO_FLIPS(<0,>0,+10)
   }

   if (SP == MBoard->FlipSP) return FALSE;
   MBoard->FlipSP = SP;

   Square = *SP;
   do Square->Disk = Colour; while ((Square = *--SP) != NULL);

   Move->Disk     = Colour;
   MBoard->Colour = -Colour;

   return TRUE;
}

/* MUndoHashFlips() */

void MUndoHashFlips(MSQUARE *Move) {

   int       Colour;
   MSQUARE **SP;
   MSQUARE  *Square;

   Colour = MBoard->Colour;
   SP     = MBoard->FlipSP;
   Square = *SP;

   if (Colour > 0) {

      do {
         Square->Disk = Colour;
         *Square->Index[0].Index += Square->Index[0].Inc2;
         *Square->Index[1].Index += Square->Index[1].Inc2;
         *Square->Index[2].Index += Square->Index[2].Inc2;
         *Square->Index[3].Index += Square->Index[3].Inc2;
      } while ((Square = *--SP) != NULL);

      MBoard->FlipSP = SP;

      *Move->Index[0].Index += Move->Index[0].Inc1;
      *Move->Index[1].Index += Move->Index[1].Inc1;
      *Move->Index[2].Index += Move->Index[2].Inc1;
      *Move->Index[3].Index += Move->Index[3].Inc1;

   } else {

      do {
         Square->Disk = Colour;
         *Square->Index[0].Index -= Square->Index[0].Inc2;
         *Square->Index[1].Index -= Square->Index[1].Inc2;
         *Square->Index[2].Index -= Square->Index[2].Inc2;
         *Square->Index[3].Index -= Square->Index[3].Inc2;
      } while ((Square = *--SP) != NULL);

      MBoard->FlipSP = SP;

      *Move->Index[0].Index -= Move->Index[0].Inc1;
      *Move->Index[1].Index -= Move->Index[1].Inc1;
      *Move->Index[2].Index -= Move->Index[2].Inc1;
      *Move->Index[3].Index -= Move->Index[3].Inc1;
   }

   Move->Disk     = EMPTY;
   MBoard->Colour = -Colour;
}

/* MUndoFlips() */

void MUndoFlips(MSQUARE *Move) {

   int       Colour;
   MSQUARE **SP;
   MSQUARE  *Square;

   Colour = MBoard->Colour;
   SP     = MBoard->FlipSP;
   Square = *SP;

   do Square->Disk = Colour; while ((Square = *--SP) != NULL);

   MBoard->FlipSP = SP;

   Move->Disk     = EMPTY;
   MBoard->Colour = -Colour;
}

/* MIsLegal() */

#define M_IS_LEGAL(Me,Opp,Dir)\
\
   if (Move[Dir].Disk Opp) {\
      Square = Move + 2 * Dir;\
      while (Square->Disk Opp) Square += Dir;\
      if (Square->Disk Me) return TRUE;\
   }

int MIsLegal(const MSQUARE *Move, int Colour) {

   const MSQUARE *Square;

   if (Colour > 0) {
      M_IS_LEGAL(>0,<0,-10)
      M_IS_LEGAL(>0,<0, -9)
      M_IS_LEGAL(>0,<0, -8)
      M_IS_LEGAL(>0,<0, -1)
      M_IS_LEGAL(>0,<0, +1)
      M_IS_LEGAL(>0,<0, +8)
      M_IS_LEGAL(>0,<0, +9)
      M_IS_LEGAL(>0,<0,+10)
   } else {
      M_IS_LEGAL(<0,>0,-10)
      M_IS_LEGAL(<0,>0, -9)
      M_IS_LEGAL(<0,>0, -8)
      M_IS_LEGAL(<0,>0, -1)
      M_IS_LEGAL(<0,>0, +1)
      M_IS_LEGAL(<0,>0, +8)
      M_IS_LEGAL(<0,>0, +9)
      M_IS_LEGAL(<0,>0,+10)
   }

   return FALSE;
}

/* MDiskDiff() */

/*
int MDiskDiff(void) {

   int    DiskDiff, EmptyNb, I;
   COUNT *CountPtr;
   const int *IndexNoPtr;

   DiskDiff = 0;
   EmptyNb  = 0;

   IndexNoPtr = LineIndexNo;
   CountPtr   = &Count[3280];
   I = 8;
   do {
      DiskDiff += CountPtr[MBoard->Index[*IndexNoPtr]].DiskDiff;
      EmptyNb  += CountPtr[MBoard->Index[*IndexNoPtr]].EmptyNb;
      IndexNoPtr++;
   } while (--I != 0);

   if      (DiskDiff > 0) DiskDiff += EmptyNb;
   else if (DiskDiff < 0) DiskDiff -= EmptyNb;

   if (MBoard->Colour < 0) DiskDiff = -DiskDiff;

   return DiskDiff;
}
*/

int MDiskDiff(void) {

   int DiskDiff, EmptyNb, Square, Rank, File;

   EmptyNb  = 64;
   DiskDiff = 0;

   Square = A1;
   Rank = 8;
   do {
      File = 8;
      do {
         if (MBoard->Square[Square].Disk != EMPTY) {
            EmptyNb--;
            DiskDiff += MBoard->Square[Square].Disk;
         }
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   if      (DiskDiff > 0) DiskDiff += EmptyNb;
   else if (DiskDiff < 0) DiskDiff -= EmptyNb;

   if (MBoard->Colour < 0) DiskDiff = -DiskDiff;

   return DiskDiff;
}

/* End of MBoard.C */

