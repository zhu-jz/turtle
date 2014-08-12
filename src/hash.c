
/* Hash.C */

#include <stdio.h>
#include <stdlib.h>

#include "hash.h"
#include "types.h"
#include "board.h"
#include "eval.h"
#include "output.h"

/* Macros */

#define BCH(N)          (BchTable[(N)&0xFF])
#define CODE(A,B,C,D,N) (((BCH((A)*(N))*256+BCH((B)*(N)))*256+BCH((C)*(N)))*256+BCH((D)*(N)))

/* "Constants" */

static const INFO NullInfo = { 0, 0, PASS, FALSE, FALSE, 0 };

/* Variables */

int    HashBits;

ENTRY *Hash;
int    HashCode[3][SQUARE_NB][2];
int    Date;

int    HashNb, HashRead, HashWrite;

static int BchTable[256];
static int Seed;

/* Prototypes */

static void AllocHashTable (void);
static void InitRandomKeys (void);
static void InitRandom     (void);
static void FillRandom     (int Key[]);

/* Functions */

/* InitHashTable() */

void InitHashTable(void) {

   int Size;

   AllocHashTable();

   Size = (1 << HashBits) * sizeof(ENTRY);
   printf("[ %d %s : %d * %d ] ",(Size>=(1<<20))?(Size>>20):(Size>=(1<<10))?(Size>>10):Size,(Size>=(1<<20))?"MB":(Size>=(1<<10))?"KB":"Bytes",1<<HashBits,(int)sizeof(ENTRY));
   fflush(stdout);

   InitRandomKeys();
   ClearHashTable();
}

/* ClearHashTable() */

void ClearHashTable(void) {

   ENTRY *Ptr, *End;

   Ptr = Hash;
   End = &Hash[1<<HashBits];
   do {
      Ptr->Lock = 0x00000000;
      Ptr->Info = NullInfo;
   } while (++Ptr < End);

   Date = 0;
}

/* CompHashKeys() */

void CompHashKeys(const BOARD *Board, int HashKey[]) {

   int HashKey0, HashKey1, Rank, File, (*Code)[SQUARE_NB][2];
   const int *Square;

   if (Board->Colour < 0) {           /* Black to move ? */
      HashKey0 = HashCode[0][0][0];
      HashKey1 = HashCode[0][0][1];
   } else {                           /* White to move ? */
      HashKey0 = HashCode[1][0][0];
      HashKey1 = HashCode[1][0][1];
   }

   Square = &Board->Square[A1];
   Code   = (int (*)[SQUARE_NB][2]) HashCode[0][A1];
   Rank = 8;
   do {
      File = 8;
      do {
         if (*Square < 0) {        /* Black disk ? */
            HashKey0 ^= Code[0][0][0];
            HashKey1 ^= Code[0][0][1];
         } else if (*Square > 0) { /* White disk ? */
            HashKey0 ^= Code[1][0][0];
            HashKey1 ^= Code[1][0][1];
         }
         Square++;
         Code = (int (*)[SQUARE_NB][2]) (*Code + 1);
      } while (--File != 0);
      Square++;
      Code = (int (*)[SQUARE_NB][2]) (*Code + 1);
   } while (--Rank != 0);

   HashKey[0] = HashKey0;
   HashKey[1] = HashKey1;
}

/* AllocHashTable() */

static void AllocHashTable(void) {

   Hash = malloc((size_t)((1<<HashBits)*sizeof(ENTRY)));
   if (Hash == NULL) FatalError("Not enough memory for Hash Table");
}

/* InitRandomKeys() */

static void InitRandomKeys(void) {

   int SquareNo, Square;

   for (Square = 0; Square < SQUARE_NB; Square++) {
      HashCode[0][Square][0] = 0x00000000;
      HashCode[0][Square][1] = 0x00000000;
      HashCode[1][Square][0] = 0x00000000;
      HashCode[1][Square][1] = 0x00000000;
   }

   InitRandom();

   FillRandom(HashCode[0][0]); /* Black to move ? */

   for (SquareNo = 0; SquareNo < LEGAL_NB; SquareNo++) {        /* Normal squares */
      FillRandom(HashCode[0][BestToWorst[SquareNo]]); /*  Black disk ?  */
      FillRandom(HashCode[1][BestToWorst[SquareNo]]); /*  White disk ?  */
   }
   for (SquareNo = LEGAL_NB; SquareNo < EMPTY_NB; SquareNo++) { /* Centre squares */
      FillRandom(HashCode[0][BestToWorst[SquareNo]]); /*  Black disk ?  */
   }

   for (Square = 0; Square < SQUARE_NB; Square++) {
      HashCode[2][Square][0] = HashCode[0][Square][0] ^ HashCode[1][Square][0];
      HashCode[2][Square][1] = HashCode[0][Square][1] ^ HashCode[1][Square][1];
   }
}

/* HashInfo() */

void HashInfo(void) {

   int I, Used, Exact = 0, Bound = 0, Bad = 0, Move = 0;
   int Date[DATE_NB], Depth[HASH_MAX_DEPTH+1];
   ENTRY *Ptr = Hash, *End = &Hash[1<<HashBits];

   for (I = 0; I < DATE_NB; I++) Date[I] = 0;
   for (I = 0; I <= HASH_MAX_DEPTH; I++) Depth[I] = 0;

   do {
      switch (Ptr->Info.Min + Ptr->Info.Max) {
      case FALSE+FALSE :
         if (Ptr->Info.Value != 0) {
            Bad++;
         } else {
            if (Ptr->Info.Killer != PASS) Move++;
            Date[Ptr->Info.Date]++;
            Depth[Ptr->Info.Depth]++;
         }
         break;
      case TRUE+FALSE :
         Bound++;
         if (Ptr->Info.Killer != PASS) Move++;
         Date[Ptr->Info.Date]++;
         Depth[Ptr->Info.Depth]++;
         break;
      case TRUE+TRUE :
         Exact++;
         if (Ptr->Info.Killer != PASS) Move++;
         Date[Ptr->Info.Date]++;
         Depth[Ptr->Info.Depth]++;
         break;
      }
   } while (++Ptr < End);

   Used = Exact + Bound + Bad;

   printf("\n");
   printf("Total    entries : %7d\n",1<<HashBits);
   printf("Move     entries : %7d (%6.2f%%)\n",Move,100.0*(double)Move/(double)(1<<HashBits));
   printf("Used     entries : %7d (%6.2f%%)\n",Used,100.0*(double)Used/(double)(1<<HashBits));
   if (Used) {
      printf("Exact    entries : %7d (%6.2f%%)\n",Exact,100.0*(double)Exact/(double)Used);
      printf("Bound    entries : %7d (%6.2f%%)\n",Bound,100.0*(double)Bound/(double)Used);
      printf("Bad      entries : %7d (%6.2f%%)\n",Bad,100.0*(double)Bad/(double)Used);
      for (I = 0; I < DATE_NB; I++) {
         if (Date[I] != 0) {
            printf("Date = %d entries : %7d (%6.2f%%)\n",I,Date[I],100.0*(double)Date[I]/(double)Used);
         }
      }
      for (I = 0; I <= HASH_MAX_DEPTH; I++) {
         if (Depth[I] != 0) {
            printf("Depth %2d entries : %7d (%6.2f%%)\n",I,Depth[I],100.0*(double)Depth[I]/(double)Used);
         }
      }
   }
   printf("\n");
}

/* InitRandom() */

static void InitRandom(void) {

   int I, N;

   for (I = 0, N = 0x01; I < 256; I++) {
      BchTable[I] = N;
      if ((N += N) >= 0x100) N ^= 0x171;
   }
   BchTable[255] = 0; /* So that every value is used only once */

   Seed = 0;
}

/* FillRandom() */

static void FillRandom(int Key[]) {

   Seed++; /* To avoid 0 value */

   Key[0] = CODE(7,5,3,1,Seed) & ((1<<HashBits)-1); /*   HashTable Index   */
   Key[1] = CODE(15,13,11,9,Seed);                  /* Entry Security Lock */
}

/* End of Hash.C */

