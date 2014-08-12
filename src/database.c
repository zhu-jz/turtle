
/* DataBase.C */

#include <stdio.h>
#include <string.h>

#include "database.h"
#include "types.h"
#include "board.h"
#include "output.h"

/* Constants */

#define VERBOSE FALSE
#define DEBUG   FALSE

#define DATABASE_DIR "data/database/"

#define PORTION 1

/* Functions */

/* ScanDatabase() */

void ScanDatabase(void (*Function)(const BOARD *Board, int Result)) {

   int    I, J, P, GameNb, CommonNb, MoveNb, Result;
   FILE  *DataBase;
   char   Data[4];
   BOARD  DbBoard[1];
   int    Move[128];

   DataBase = fopen(DATABASE_DIR "snail_db","rb");
   if (DataBase == NULL) FatalError("Couldn't open file \"%s\" for reading",DATABASE_DIR "snail_db");
   fread(Data,1,4,DataBase);
   if (strncmp(Data,"SNDB",4) != 0) FatalError("Unknown database format");
   fread(&GameNb,sizeof(int),1,DataBase);
   printf("[ %d games ] 00%%",GameNb);
   fflush(stdout);

   P = 0;
   for (I = 0; I < GameNb / PORTION; I++) {
      if (P < 100 * I / (GameNb / PORTION)) {
         P = 100 * I / (GameNb / PORTION);
         printf("\b\b\b%02d%%",P);
         fflush(stdout);
      }
      CommonNb = fgetc(DataBase);
      MoveNb   = fgetc(DataBase);
      Result   = (int) (schar) fgetc(DataBase);
      ClearBoard(DbBoard);
      for (J = 0; J < CommonNb; J++) {
#if DEBUG
         if (! IsLegalMove(DbBoard,Move[J])) FatalError("Illegal move in database");
#endif
         DoMove(DbBoard,Move[J]);
      }
      if (J % 2 != 0) Result = -Result;
      for (; J < CommonNb+MoveNb; J++, Result = -Result) {
         Move[J] = fgetc(DataBase);
#if DEBUG
         if (! IsLegalMove(DbBoard,Move[J])) FatalError("Illegal move in database");
#endif
         DoMove(DbBoard,Move[J]);
#if VERBOSE
         DispBoard(DbBoard);
         printf("Result: %+3d\n",Result);
#endif
         (*Function)(DbBoard,Result);
      }
   }
}

/* End of DataBase.C */

