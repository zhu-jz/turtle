
/* EBoard.C */

#include <stdio.h>
#include <stdlib.h>

#include "eboard.h"
#include "types.h"
#include "board.h"
#include "hash.h"
#include "output.h"

/* Variables */

EBOARD EBoard[1];

/* Functions */

/* EInitBoard() */

void EInitBoard(void) {

   int Rank, File, Square;

   for (Square = 0; Square < SQUARE_NB; Square++) {
      EBoard->Square[Square].Move = Square;
   }

   for (Rank = 0, Square = A1; Rank < 8; Rank++, Square++) {
      for (File = 0; File < 8; File++, Square++) {
         EBoard->Square[Square].FlipKey[0]    = HashCode[2][Square][0];
         EBoard->Square[Square].FlipKey[1]    = HashCode[2][Square][1];
         EBoard->Square[Square].MoveKey[0][0] = HashCode[2][0][0] ^ HashCode[0][Square][0];
         EBoard->Square[Square].MoveKey[0][1] = HashCode[2][0][1] ^ HashCode[0][Square][1];
         EBoard->Square[Square].MoveKey[1][0] = HashCode[2][0][0] ^ HashCode[1][Square][0];
         EBoard->Square[Square].MoveKey[1][1] = HashCode[2][0][1] ^ HashCode[1][Square][1];
      }
   }
}

/* ECopyBoard() */

void ECopyBoard(const BOARD *Board) {

   int I;

   EBoard->Colour = Board->Colour;
   EBoard->FlipSP = EBoard->FlipStack;

   for (I = 0; I < SQUARE_NB; I++) EBoard->Square[I].Disk = Board->Square[I];

   CompHashKeys(Board,EBoard->Key);
}

/* EDoPass() */

void EDoPass(void) {

   EBoard->Colour  = -EBoard->Colour;
   EBoard->Key[0] ^= HashCode[2][0][0];
   EBoard->Key[1] ^= HashCode[2][0][1];
}

#define E_DO_FLIPS(Me,Opp,Dir)\
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

/* EDoHashFlips() */

int EDoHashFlips(ESQUARE *Move) {

   int       Colour;
   int       Flips;
   ESQUARE **SP, **SP2;
   ESQUARE  *Square;

   Colour = EBoard->Colour;
   SP     = EBoard->FlipSP;

   if (Colour > 0) {

      E_DO_FLIPS(>0,<0,-10)
      E_DO_FLIPS(>0,<0, -9)
      E_DO_FLIPS(>0,<0, -8)
      E_DO_FLIPS(>0,<0, -1)
      E_DO_FLIPS(>0,<0, +1)
      E_DO_FLIPS(>0,<0, +8)
      E_DO_FLIPS(>0,<0, +9)
      E_DO_FLIPS(>0,<0,+10)

      if (SP == EBoard->FlipSP) return 0;

      EBoard->Key[0] ^= Move->MoveKey[1][0];
      EBoard->Key[1] ^= Move->MoveKey[1][1];

   } else {

      E_DO_FLIPS(<0,>0,-10)
      E_DO_FLIPS(<0,>0, -9)
      E_DO_FLIPS(<0,>0, -8)
      E_DO_FLIPS(<0,>0, -1)
      E_DO_FLIPS(<0,>0, +1)
      E_DO_FLIPS(<0,>0, +8)
      E_DO_FLIPS(<0,>0, +9)
      E_DO_FLIPS(<0,>0,+10)

      if (SP == EBoard->FlipSP) return 0;

      EBoard->Key[0] ^= Move->MoveKey[0][0];
      EBoard->Key[1] ^= Move->MoveKey[0][1];
   }

   EBoard->FlipSP = SP;
   Square         = *SP;
   Flips          = 1;
   do {
      Square->Disk = Colour;
      EBoard->Key[0] ^= Square->FlipKey[0];
      EBoard->Key[1] ^= Square->FlipKey[1];
      Flips += 2;
   } while ((Square = *--SP) != NULL);

   Move->Disk     = Colour;
   EBoard->Colour = -Colour;

   return Flips;
}

/* EDoFlips() */

int EDoFlips(ESQUARE *Move) {

   int       Colour;
   int       Flips;
   ESQUARE **SP, **SP2;
   ESQUARE  *Square;

   Colour = EBoard->Colour;
   SP     = EBoard->FlipSP;

   if (Colour > 0) {
      E_DO_FLIPS(>0,<0,-10)
      E_DO_FLIPS(>0,<0, -9)
      E_DO_FLIPS(>0,<0, -8)
      E_DO_FLIPS(>0,<0, -1)
      E_DO_FLIPS(>0,<0, +1)
      E_DO_FLIPS(>0,<0, +8)
      E_DO_FLIPS(>0,<0, +9)
      E_DO_FLIPS(>0,<0,+10)
   } else {
      E_DO_FLIPS(<0,>0,-10)
      E_DO_FLIPS(<0,>0, -9)
      E_DO_FLIPS(<0,>0, -8)
      E_DO_FLIPS(<0,>0, -1)
      E_DO_FLIPS(<0,>0, +1)
      E_DO_FLIPS(<0,>0, +8)
      E_DO_FLIPS(<0,>0, +9)
      E_DO_FLIPS(<0,>0,+10)
   }

   if (SP == EBoard->FlipSP) return 0;

   EBoard->FlipSP = SP;
   Square         = *SP;
   Flips          = 1;
   do {
      Square->Disk = Colour;
      Flips += 2;
   } while ((Square = *--SP) != NULL);

   Move->Disk     = Colour;
   EBoard->Colour = -Colour;

   return Flips;
}

/* EUndoFlips() */

void EUndoFlips(ESQUARE *Move) {

   int       Colour;
   ESQUARE **SP;
   ESQUARE  *Square;

   Colour = EBoard->Colour;
   SP     = EBoard->FlipSP;
   Square = *SP;

   do Square->Disk = Colour; while ((Square = *--SP) != NULL);

   EBoard->FlipSP = SP;

   Move->Disk     = EMPTY;
   EBoard->Colour = -Colour;
}

/* ECountFlips() */

#define E_COUNT_FLIPS(Me,Opp,Dir)\
\
   if (Move[Dir].Disk Opp) {\
      Square = Move + 2 * Dir;\
      Flips2 = 2;\
      while (Square->Disk Opp) {\
         Square += Dir;\
         Flips2 += 2;\
      }\
      if (Square->Disk Me) Flips += Flips2;\
   }

int ECountFlips(const ESQUARE *Move, int Colour) {

   int Flips, Flips2;
   const ESQUARE *Square;

   Flips = 0;

   if (Colour > 0) {
      E_COUNT_FLIPS(>0,<0,-10)
      E_COUNT_FLIPS(>0,<0, -9)
      E_COUNT_FLIPS(>0,<0, -8)
      E_COUNT_FLIPS(>0,<0, -1)
      E_COUNT_FLIPS(>0,<0, +1)
      E_COUNT_FLIPS(>0,<0, +8)
      E_COUNT_FLIPS(>0,<0, +9)
      E_COUNT_FLIPS(>0,<0,+10)
   } else {
      E_COUNT_FLIPS(<0,>0,-10)
      E_COUNT_FLIPS(<0,>0, -9)
      E_COUNT_FLIPS(<0,>0, -8)
      E_COUNT_FLIPS(<0,>0, -1)
      E_COUNT_FLIPS(<0,>0, +1)
      E_COUNT_FLIPS(<0,>0, +8)
      E_COUNT_FLIPS(<0,>0, +9)
      E_COUNT_FLIPS(<0,>0,+10)
   }

   return Flips;
}

/* EIsLegal() */

#define E_IS_LEGAL(Me,Opp,Dir)\
\
   if (Move[Dir].Disk Opp) {\
      Square = Move + 2 * Dir;\
      while (Square->Disk Opp) Square += Dir;\
      if (Square->Disk Me) return TRUE;\
   }

int EIsLegal(const ESQUARE *Move, int Colour) {

   const ESQUARE *Square;

   if (Colour > 0) {
      E_IS_LEGAL(>0,<0, -9)
      E_IS_LEGAL(>0,<0, +9)
      E_IS_LEGAL(>0,<0, -1)
      E_IS_LEGAL(>0,<0, +1)
      E_IS_LEGAL(>0,<0,-10)
      E_IS_LEGAL(>0,<0,+10)
      E_IS_LEGAL(>0,<0, -8)
      E_IS_LEGAL(>0,<0, +8)
   } else {
      E_IS_LEGAL(<0,>0, -9)
      E_IS_LEGAL(<0,>0, +9)
      E_IS_LEGAL(<0,>0, -1)
      E_IS_LEGAL(<0,>0, +1)
      E_IS_LEGAL(<0,>0,-10)
      E_IS_LEGAL(<0,>0,+10)
      E_IS_LEGAL(<0,>0, -8)
      E_IS_LEGAL(<0,>0, +8)
   }

   return FALSE;
}

/* EDiskDiff() */

int EDiskDiff(void) {

   int DiskDiff, Rank, File;
   const ESQUARE *Square;

   DiskDiff = 0;

   Square = &EBoard->Square[A1];
   Rank = 8;
   do {
      File = 8;
      do {
         DiskDiff += Square->Disk;
         Square++;
      } while (--File != 0);
      Square++;
   } while (--Rank != 0);

   if (EBoard->Colour < 0) DiskDiff = -DiskDiff;

   return DiskDiff;
}

/* End of EBoard.C */

