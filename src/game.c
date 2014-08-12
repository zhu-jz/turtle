
/* Game.C */

#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "types.h"
#include "board.h"
#include "clock.h"
#include "output.h"

/* Variables */

double GameTime;
int    Trust;
int    RestoreClocks;

GAME   Game[1];

double ElapsedTime[2];
TIME   TurnBegin;

static double TurnTime;

/* Functions */

/* ClearGame() */

void ClearGame(GAME *Game, const BOARD *StartBoard) {

   if (StartBoard == NULL) {
      ClearBoard(Game->StartBoard);
   } else {
      CopyBoard(StartBoard,Game->StartBoard);
   }

   CopyBoard(Game->StartBoard,Game->Board);

   Game->MoveNo       = 0;
   Game->Move[0].Move = NO_MOVE;

   ElapsedTime[0] = 0.0;
   ElapsedTime[1] = 0.0;

   TurnBegin = CurrentTime();
}

/* DoGameMove() */

void DoGameMove(GAME *Game, int Move, double Value, double Time) {

   if (! IsLegalMove(Game->Board,Move)) FatalError("Illegal move in DoGameMove()");

   TurnTime = (Trust && Time >= 0.0) ? Time : Duration(TurnBegin,CurrentTime());
   ElapsedTime[Game->Board->Colour>0] += TurnTime;

   DoMove(Game->Board,Move);

   Game->Move[Game->MoveNo].Move  = Move;
   Game->Move[Game->MoveNo].Value = Value;
   Game->Move[Game->MoveNo].Time  = TurnTime;
   Game->MoveNo++;
   Game->Move[Game->MoveNo].Move  = NO_MOVE;

   TurnBegin = CurrentTime();
}

/* UndoGameMove() */

void UndoGameMove(GAME *Game) {

   if (Game->MoveNo == 0) {
      Error("Game->MoveNo == 0 in UndoGameMove()");
      return;
   }

   Game->MoveNo--;
   UndoMove(Game->Board,Game->Move[Game->MoveNo].Move);
   if (RestoreClocks) ElapsedTime[Game->Board->Colour>0] -= Game->Move[Game->MoveNo].Time;

   TurnBegin = CurrentTime();
}

/* RedoGameMove() */

void RedoGameMove(GAME *Game) {

   if (Game->Move[Game->MoveNo].Move == NO_MOVE) {
      Error("Game->Move[Game->MoveNo].Move == NO_MOVE in RedoGameMove()");
      return;
   }

   if (RestoreClocks) ElapsedTime[Game->Board->Colour>0] += Game->Move[Game->MoveNo].Time;
   DoMove(Game->Board,Game->Move[Game->MoveNo].Move);
   Game->MoveNo++;

   TurnBegin = CurrentTime();
}

/* GameMoveNb() */

int GameMoveNb(const GAME *Game) {

   int MoveNb;

   for (MoveNb = 0; Game->Move[MoveNb].Move != NO_MOVE; MoveNb++)
      ;

   return MoveNb;
}

/* GotoGameMove() */

void GotoGameMove(GAME *Game, int MoveNo) {

   CopyBoard(Game->StartBoard,Game->Board);
   if (RestoreClocks) ElapsedTime[0] = ElapsedTime[1] = 0.0;

   for (Game->MoveNo = 0; Game->MoveNo < MoveNo && Game->Move[Game->MoveNo].Move != NO_MOVE; Game->MoveNo++) {
      if (RestoreClocks) ElapsedTime[Game->Board->Colour>0] += Game->Move[Game->MoveNo].Time;
      DoMove(Game->Board,Game->Move[Game->MoveNo].Move);
   }

   TurnBegin = CurrentTime();
}

/* LoadGame() */

void LoadGame(GAME *Game, const char *FileName) {

}

/* SaveGame() */

void SaveGame(const GAME *Game, const char *FileName) {

   int I, J, Colour, Square, MoveNo[SQUARE_NB];
   FILE *File;

   printf("\n");
   File = fopen(FileName,"a");
   if (File == NULL) Warning("Couldn't open file \"%s\" for writing",FileName);

   for (I = 0; Game->Move[I].Move != NO_MOVE; I++) {
      printf("%s",SquareString[Game->Move[I].Move]);
      if (File != NULL) fprintf(File,"%s",SquareString[Game->Move[I].Move]);
   }

   printf("\n");
   if (File != NULL) {
      fprintf(File,"\n");
      fclose(File);
   }

   for (I = 0; I < SQUARE_NB; I++) MoveNo[I] = 0;

   J = 60 - EmptyNb(Game->StartBoard);
   Colour = Game->StartBoard->Colour;

   for (I = 0; Game->Move[I].Move != NO_MOVE; I++) {
      if (Game->Move[I].Move != PASS) MoveNo[Game->Move[I].Move] = ++J;
      Colour = -Colour;
   }

   printf("\n");

   if (Colour > 0) {
      printf("-> ()\n");
   } else {
      printf("-> ##\n");
   }

   printf("\n");

   printf("   A  B  C  D  E  F  G  H\n");
   for (I = 1, Square = A1; I <= 8; I++, Square++) {
      printf("%d ",I);
      for (J = 1; J <= 8; J++, Square++) {
	 switch (Game->StartBoard->Square[Square]) {
	 case WHITE :
            printf("|()");
            break;
         case BLACK :
            printf("|##");
            break;
         case EMPTY :
	    if (MoveNo[Square] != 0) {
               printf("|%2d",MoveNo[Square]);
            } else {
               printf("|  ");
            }
            break;
         }
      }
      printf("| %d\n",I);
   }
   printf("   A  B  C  D  E  F  G  H\n\n");
}

/* RemainingTime() */

double RemainingTime(int Colour) {

   double RemainingTime;

   RemainingTime = GameTime - ElapsedTime[Colour>0];

   if (! Trust && Game->Board->Colour == Colour) {
      RemainingTime -= Duration(TurnBegin,CurrentTime());
   }

   if (RemainingTime < 0.0) RemainingTime = 0.0;

   return RemainingTime;
}

/* End of Game.C */

