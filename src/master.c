
/* Master.C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "master.h"
#include "board.h"
#include "book.h"
#include "clock.h"
#include "database.h"
#include "endgame.h"
#include "eval.h"
#include "game.h"
#include "hash.h"
#include "loop.h"
#include "mboard.h"
#include "midgame.h"
#include "output.h"
#include "sort.h"

/* Constants */

#define SHORT_MIN_DEPTH   1
#define SHORT_MAX_DEPTH   8

#define TEST_TIME         3600.0

#define PROBCUT_FILE      "data/probcut/stats"
#define PROBCUT_TIME      60.0
#define PROBCUT_DEPTH     9

#define EVAL_FILE         "data/eval/stats"
#define EVAL_TIME         60.0

#define BOOK_FILE         "data/book/stats"

#define OPPONENT_TIME     0.1

#define MID_GAME_TIME     1.2
#define MID_GAME_EARLY    0.4

#define SEL_SOLVE_DEPTH   26
#define SEL_SOLVE_TIME    0.4
#define SEL_SOLVE_EARLY   0.7

#define WLD_SOLVE_DEPTH   22
#define WLD_SOLVE_TIME    0.6
#define WLD_SOLVE_EARLY   0.7

#define FULL_SOLVE_DEPTH  18
#define FULL_SOLVE_TIME   0.75
#define FULL_SOLVE_EARLY  0.85

#define WINDOW_SIZE       4

#define MIN_ENDGAME_DEPTH 8
#define MAX_ENDGAME_DEPTH 8

#define MIN_DEPTH         1
#define MAX_DEPTH         32

#define MIN_TIME          1.0
#define MAX_TIME          604800.0 /* 1 week :) */

#define BF                2.35
#define BW                2.15
#define S                 400000.0

#define STRING_SIZE       1024

/* Macros */

#define SGN(X) (((X) == 0) ? 0 : (((X) < 0) ? -1 : +1))

/* "Constants" */

static const char *EventString[] = {
   "[ ONLY ]", "[ BOOK ]", "[UPDATE]", "[ DONE ]", "[EARLY ]",
   "[ TIME ]", "[ STOP ]", "[ BEST ]", "[SOLVED]"
};

/* Variables */

int WldSolve, FullSolve;

SEARCH   Search[1];

int      PvNodeNb, NodeNb, EvalNb;

static int PosNb;

/* Prototypes */

static void        ThinkProbCut  (const BOARD *Board, int Result);
static void        ThinkEval     (const BOARD *Board, int Result);
static void        ThinkSolve    (const BOARD *Board, BEST_MOVE *Best);

static void        InitThinkPC   (const BOARD *Board);
static void        InitThinkE    (const BOARD *Board);

static void        AllocTime     (SEARCH *Search, double Time);

static void        SetBestValue  (BEST_MOVE *Best);

static const char *InfoString    (const BEST_MOVE *Best);

static void        WriteString   (const char *String, const char *FileName);

static double      Div0          (double X, double Y);
static int         Div2          (int X);

/* Functions */

/* ProbCutStat() */

void ProbCutStat(void) {

   FILE *File;
   char Buffer[256];

   PosNb = 0;

   File = fopen(PROBCUT_FILE,"r");
   if (File != NULL) {
      while (fgets(Buffer,256,File) != NULL) PosNb++;
      fclose(File);
   }

   ScanDatabase(ThinkProbCut);
}

/* EvalStat() */

void EvalStat(void) {

   FILE *File;
   char Buffer[256];

   PosNb = 0;

   File = fopen(EVAL_FILE,"r");
   if (File != NULL) {
      while (fgets(Buffer,256,File) != NULL) PosNb++;
      fclose(File);
   }

   ScanDatabase(ThinkEval);
}

/* Think() */

void Think(void) {

   int Depth;

   InitRootList(Search->Board,Search->Forbidden);

   Search->Best->Move       = RootMove[0].Move;
   Search->Best->Depth      = RootMove[0].Depth;
   Search->Best->Min        = RootMove[0].Min;
   Search->Best->Max        = RootMove[0].Max;
   Search->Best->PV[0]      = RootMove[0].Move;
   Search->Best->PV[1]      = NO_MOVE;
   Search->Best->Time       = Duration(Search->Start,CurrentTime());
   Search->Best->IosValue   = 0.0;
   Search->Best->BookValue  = 0;
   strcpy(Search->Best->StringValue,"?");
   Search->Best->CurrDepth  = 0;
   strcpy(Search->Best->CurrType,"??");
   Search->Best->CurrMove   = NO_MOVE;
   Search->Best->CurrMoveNo = 0;
   Search->Best->Event      = ALONE;
   Search->Best->Modified   = FALSE;

   if (RootMoveNb == 0) return;

   strcpy(Search->Best->CurrType,"LG");
   if (RootMoveNb == 1 && Search->ForceAlone) {
      Search->Best->Time  = Duration(Search->Start,CurrentTime());
      Search->Best->Event = ALONE;
      DisplayInfos(Search->Best);
      return;
   }

   strcpy(Search->Best->CurrType,"BK");
   if (Search->Book && BookMove(Search->Board,Search->Forbidden,Search->Best)) {
      Search->Best->Time  = Duration(Search->Start,CurrentTime());
      Search->Best->Event = BOOK;
      DisplayInfos(Search->Best);
      return;
   }

   InitKillerLists(Search->Board);

   PvNodeNb  = 0;
   NodeNb    = 0;
   EvalNb    = 0;
   HashNb    = 0;
   HashRead  = 0;
   HashWrite = 0;

   InitSearch(Search->Board);

   Search->Early = Search->MidEarly;

   Depth = Search->MaxDepth / 2;
   if (Depth > SHORT_MAX_DEPTH) Depth = SHORT_MAX_DEPTH;
   if (Depth < SHORT_MIN_DEPTH) Depth = SHORT_MIN_DEPTH;
   Search->Best->CurrDepth = Depth;
   sprintf(Search->Best->CurrType,"%2d",Depth);
   Search->Best->Event = ShortSearch(-INF,+INF,Depth,Search->Best);
   if (Search->Best->Event == DEPTH && Search->TimeLimit && Search->Best->Time >= Search->MidEarly) {
      Search->Best->Event = EARLY;
   }
   DisplayInfos(Search->Best);
   if (Search->Best->Event != DEPTH) return;
   SortRootList();

   for (Depth++; Depth <= Search->MaxDepth; Depth++) {
      Search->Best->CurrDepth = Depth;
      sprintf(Search->Best->CurrType,"%2d",Depth);
      Search->Best->Event = MGSearch(-INF,+INF,Depth,Search->Best);
      if (Search->Best->Event == DEPTH && Search->TimeLimit && Search->Best->Time >= Search->MidEarly) {
         Search->Best->Event = EARLY;
      }
      if (Search->Best->Event == DEPTH) { 
         UpdateInfos(Search->Best);
      } else {
         DisplayInfos(Search->Best);
         return;
      }
   }

   if (EmptyNb(Search->Board) - Search->MaxDepth > MIN_ENDGAME_DEPTH) return;

   ThinkSolve(Search->Board,Search->Best);
}

/* ThinkBook() */

void ThinkBook(void) {

   int Depth, Write;
   char String[STRING_SIZE+1], Note[STRING_SIZE+1];

   InitRootList(Search->Board,Search->Forbidden);

   Search->Best->Move       = RootMove[0].Move;
   Search->Best->Depth      = RootMove[0].Depth;
   Search->Best->Min        = RootMove[0].Min;
   Search->Best->Max        = RootMove[0].Max;
   Search->Best->PV[0]      = RootMove[0].Move;
   Search->Best->PV[1]      = NO_MOVE;
   Search->Best->Time       = Duration(Search->Start,CurrentTime());
   Search->Best->IosValue   = 0.0;
   Search->Best->BookValue  = 0;
   strcpy(Search->Best->StringValue,"?");
   Search->Best->CurrDepth  = 0;
   strcpy(Search->Best->CurrType,"??");
   Search->Best->CurrMove   = NO_MOVE;
   Search->Best->CurrMoveNo = 0;
   Search->Best->Event      = ALONE;
   Search->Best->Modified   = FALSE;

   if (RootMoveNb == 0) return;

   strcpy(Search->Best->CurrType,"LG");
   if (RootMoveNb == 1 && Search->ForceAlone) {
      Search->Best->Time  = Duration(Search->Start,CurrentTime());
      Search->Best->Event = ALONE;
      DisplayInfos(Search->Best);
      return;
   }

   strcpy(Search->Best->CurrType,"BK");
   if (Search->Book && BookMove(Search->Board,Search->Forbidden,Search->Best)) {
      Search->Best->Time  = Duration(Search->Start,CurrentTime());
      Search->Best->Event = BOOK;
      DisplayInfos(Search->Best);
      return;
   }

   InitKillerLists(Search->Board);

   PvNodeNb  = 0;
   NodeNb    = 0;
   EvalNb    = 0;
   HashNb    = 0;
   HashRead  = 0;
   HashWrite = 0;

   DispBoard2(Search->Board,Search->Forbidden);

   InitSearch(Search->Board);

   Search->Early = Search->MidEarly;

   ClearHashTable();
   sprintf(String,"%2d %+5d",60-EmptyNb(Search->Board),Eval());
   Write = TRUE;

   Depth = 1;
   Search->Best->CurrDepth = Depth;
   sprintf(Search->Best->CurrType,"%2d",Depth);
   Search->Best->Event = ShortSearch(-INF,+INF,Depth,Search->Best);
   if (Search->Best->Event == DEPTH) {
      if (Search->Best->Min != Search->Best->Max || Search->Best->Max < -MID_INF || Search->Best->Min > +MID_INF) {
	 Write = FALSE;
      } else {
	 sprintf(Note," %+5d",Search->Best->Min);
	 strcat(String,Note);
      }
      if (Search->TimeLimit && Search->Best->Time >= Search->MidEarly) {
	 Search->Best->Event = EARLY;
      }
   }
   DisplayInfos(Search->Best);
   if (Search->Best->Event != DEPTH) {
      if (Write) WriteString(String,BOOK_FILE);
      return;
   }
   SortRootList();

   for (Depth++; Depth <= Search->MaxDepth; Depth++) {
      Search->Best->CurrDepth = Depth;
      sprintf(Search->Best->CurrType,"%2d",Depth);
      Search->Best->Event = MGSearch(-INF,+INF,Depth,Search->Best);
      if (Search->Best->Event == DEPTH) {
         if (Search->Best->Min != Search->Best->Max || Search->Best->Max < -MID_INF || Search->Best->Min > +MID_INF) {
	    Write = FALSE;
	 } else {
	    sprintf(Note," %+5d",Search->Best->Min);
	    strcat(String,Note);
	 }
         if (Search->TimeLimit && Search->Best->Time >= Search->MidEarly) {
            Search->Best->Event = EARLY;
         }
      }
      if (Search->Best->Event == DEPTH) { 
         UpdateInfos(Search->Best);
      } else {
         DisplayInfos(Search->Best);
         if (Write) WriteString(String,BOOK_FILE);
         return;
      }
   }

   if (Write) WriteString(String,BOOK_FILE);

   if (EmptyNb(Search->Board) - Search->MaxDepth > MIN_ENDGAME_DEPTH) return;

   ThinkSolve(Search->Board,Search->Best);
}

/* ThinkProbCut() */

static void ThinkProbCut(const BOARD *Board, int Result) {

   int Depth, Event, *Forbidden;
   BEST_MOVE Best[1];
   FILE *File;

   if (60 - EmptyNb(Board) < 12 || 60 - EmptyNb(Board) > 52 - 3) return;

   if (PosNb > 0) {
      PosNb--;
      return;
   }

   InitThinkPC(Board);
   Forbidden = NULL;

   InitRootList(Board,Forbidden);

   Best->Move       = RootMove[0].Move;
   Best->Depth      = RootMove[0].Depth;
   Best->Min        = RootMove[0].Min;
   Best->Max        = RootMove[0].Max;
   Best->PV[0]      = RootMove[0].Move;
   Best->PV[1]      = NO_MOVE;
   Best->Time       = Duration(Search->Start,CurrentTime());
   Best->IosValue   = 0.0;
   Best->BookValue  = 0;
   strcpy(Best->StringValue,"?");
   Best->CurrDepth  = 0;
   strcpy(Best->CurrType,"??");
   Best->CurrMove   = NO_MOVE;
   Best->CurrMoveNo = 0;
   Best->Event      = ALONE;
   Best->Modified   = FALSE;

   if (RootMoveNb == 0) return;
   if (RootMoveNb == 1 && Search->ForceAlone) return;

   if (Search->Book && BookMove(Board,Forbidden,Best)) {
      Best->Time = Duration(Search->Start,CurrentTime());
      Output("BK %s\n",InfoString(Best));
      return;
   }

   InitKillerLists(Board);

   PvNodeNb  = 0;
   NodeNb    = 0;
   EvalNb    = 0;
   HashNb    = 0;
   HashRead  = 0;
   HashWrite = 0;

   InitSearch(Board);
   ClearHashTable();

   Search->Early = Search->MidEarly;

   File = fopen(PROBCUT_FILE,"a");
   if (File == NULL) FatalError("Couldn't open file \"%s\" for writing",PROBCUT_FILE);

   fprintf(File,"%+2d %+3d %2d %+5d",SGN(Result),Result,60-EmptyNb(Board),Eval());

   for (Depth = 1; Depth <= Search->MaxDepth; Depth++) {
      Event = MGSearch(-INF,+INF,Depth,Best);
      if (Depth == 1) SortRootList();
      if (Event != DEPTH) break;
      if (Best->Min != Best->Max || Best->Max < -MID_INF || Best->Min > +MID_INF) {
         break;
      }
      fprintf(File," %+5d",Best->Min);
   }

   fprintf(File,"\n");
   fclose(File);

   if (EmptyNb(Board) - Search->MaxDepth > MIN_ENDGAME_DEPTH) return;

   ThinkSolve(Board,Best);
}

/* ThinkEval() */

static void ThinkEval(const BOARD *Board, int Result) {

   int Depth, Event, *Forbidden;
   BEST_MOVE Best[1];
   FILE *File;

   if (60 - EmptyNb(Board) > 52) return;

   if (PosNb > 0) {
      PosNb--;
      return;
   }

   InitThinkE(Board);
   Forbidden = NULL;

   InitRootList(Board,Forbidden);

   Best->Move       = RootMove[0].Move;
   Best->Depth      = RootMove[0].Depth;
   Best->Min        = RootMove[0].Min;
   Best->Max        = RootMove[0].Max;
   Best->PV[0]      = RootMove[0].Move;
   Best->PV[1]      = NO_MOVE;
   Best->Time       = Duration(Search->Start,CurrentTime());
   Best->IosValue   = 0.0;
   Best->BookValue  = 0;
   strcpy(Best->StringValue,"?");
   Best->CurrDepth  = 0;
   strcpy(Best->CurrType,"??");
   Best->CurrMove   = NO_MOVE;
   Best->CurrMoveNo = 0;
   Best->Event      = ALONE;
   Best->Modified   = FALSE;

   if (RootMoveNb == 0) return;
   if (RootMoveNb == 1 && Search->ForceAlone) return;

   if (Search->Book && BookMove(Board,Forbidden,Best)) {
      Best->Time = Duration(Search->Start,CurrentTime());
      Output("BK %s\n",InfoString(Best));
      return;
   }

   InitKillerLists(Board);

   PvNodeNb  = 0;
   NodeNb    = 0;
   EvalNb    = 0;
   HashNb    = 0;
   HashRead  = 0;
   HashWrite = 0;

   InitSearch(Board);
   ClearHashTable();

   Search->Early = Search->MidEarly;

   File = fopen(EVAL_FILE,"a");
   if (File == NULL) FatalError("Couldn't open file \"%s\" for writing",EVAL_FILE);

   fprintf(File,"%+2d %+3d %2d %+5d",SGN(Result),Result,60-EmptyNb(Board),Eval());

   for (Depth = 1; Depth <= Search->MaxDepth; Depth++) {
      Event = MGSearch(-INF,+INF,Depth,Best);
      if (Depth == 1) SortRootList();
      if (Event != DEPTH) break;
      if (Best->Min != Best->Max || Best->Max < -MID_INF || Best->Min > +MID_INF) {
         break;
      }
      fprintf(File," %+5d",Best->Min);
   }

   fprintf(File,"\n");
   fclose(File);

   if (EmptyNb(Board) - Search->MaxDepth > MIN_ENDGAME_DEPTH) return;

   ThinkSolve(Board,Best);
}

/* ThinkSolve() */

#define SOLVE_STEP(StepType,Alpha,Beta)\
\
{\
   sprintf(Best->CurrType,(StepType));\
   Best->Event = Solve((Alpha),(Beta),Best);\
   SetBestValue(Best);\
   if (UNDIFFNOTE(Best->Min) > Min) Min = UNDIFFNOTE(Best->Min);\
   if (UNDIFFNOTE(Best->Max) < Max) Max = UNDIFFNOTE(Best->Max);\
   if (Best->Event == DEPTH) {\
      if (Search->TimeLimit && Best->Time >= Search->Early) Best->Event = EARLY;\
      if (Min == Max) Best->Event = SOLVED;\
   }\
   if (Best->Event == DEPTH) {\
      UpdateInfos(Best);\
   } else {\
      DisplayInfos(Best);\
      return;\
   }\
}

static void ThinkSolve(const BOARD *Board, BEST_MOVE *Best) {

   int Min, Max, NewMin, NewMax, Middle;
   double P;

   Min = -64;
   Max = +64;

   InitSolve(Board);

   if (Search->Sel) {

      Search->Early = Search->SelEarly;

      for (P = 0.05; Min < 0 && Max > 0 && P < 0.975; P += 0.05) {

         Best->CurrDepth = SELECTIVE_DEPTH + Round(100.0*P);
         sprintf(Best->CurrType,"%2.0f",100.0*P);
         Best->Event = SelSolve(-1,+1,P,Best);
         SetBestValue(Best);
         if (Best->Event == DEPTH && Search->TimeLimit && Best->Time >= Search->Early) {
            Best->Event = EARLY;
         }
         if (Best->Event == DEPTH) { 
            UpdateInfos(Best);
         } else {
            DisplayInfos(Best);
            return;
         }
         if (Best->Min > +MID_INF) Min = UNDIFFNOTE(Best->Min);
         if (Best->Max < -MID_INF) Max = UNDIFFNOTE(Best->Max);
      }
   }

   if (Search->Wld) {

      Best->CurrDepth = SOLVE_DEPTH;

      Search->Early = Search->WldEarly;

      switch (Search->WldAlgo) {

      case WLD :

         if (Min <= 0 && Max >= 0 && Min != Max) SOLVE_STEP("LW",SGN(Min),SGN(Max))
         break;

      case LD :

         if (Min <  0 && Max >= 0) SOLVE_STEP("LD",-1, 0)
         if (Min == 0 && Max >  0) SOLVE_STEP("DW", 0,+1)
         break;

      case DW :

         if (Min <= 0 && Max >  0) SOLVE_STEP("DW", 0,+1);
         if (Min <  0 && Max == 0) SOLVE_STEP("LD",-1, 0);
         break;
      }
   }

   if (Search->Full) {

      Best->CurrDepth = SOLVE_DEPTH;

      Search->Early = Search->FullEarly;

      switch (Search->FullAlgo) {

      case FULL :

         if (Min < Max) SOLVE_STEP("FL",DIFFNOTE(Min),DIFFNOTE(Max));
         break;

      case WINDOW :

	 while (Min < Max) {
	    if (Max <= 0) {
	       NewMin = Max - WINDOW_SIZE;
	       if (NewMin < Min) NewMin = Min;
	       NewMax = Max;
	    } else {
	       NewMin = Min;
	       NewMax = Min + WINDOW_SIZE;
	       if (NewMax > Max) NewMax = Max;
	    }
	    SOLVE_STEP("WN",DIFFNOTE(NewMin),DIFFNOTE(NewMax));
         }
         break;

      case NEGA_C :

         while (Min < Max) {
            Middle = 2 * Div2(Min/2+Max/2);
            SOLVE_STEP("C*",DIFFNOTE(Middle),DIFFNOTE(Middle)+1);
         }
         break;

      case PESSIMISM :

         while (Min < Max) SOLVE_STEP("PS",DIFFNOTE(Min),DIFFNOTE(Min)+1);
         break;
      }
   }
}

/* InitThinkMT() */

void InitThinkMT(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = TRUE;
   Search->Book       = UseBook;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = 0;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = 0.0;
   Search->MidEarly   = 0.0;
   Search->Sel        = EmptyNb(Board) >= 20;
   Search->SelDepth   = SEL_SOLVE_DEPTH;
   Search->SelEarly   = 0.0;
   Search->Wld        = TRUE;
   Search->WldDepth   = WLD_SOLVE_DEPTH;
   Search->WldAlgo    = (EmptyNb(Board) >= 20) ? WLD : WldSolve;
   Search->WldEarly   = 0.0;
   Search->Full       = EmptyNb(Board) <= 26;
   Search->FullDepth  = FULL_SOLVE_DEPTH;
   Search->FullAlgo   = FullSolve;
   Search->FullEarly  = 0.0;

   AllocTime(Search,RemainingTime(Search->Board->Colour));

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;

   Search->MidEarly  = Search->MaxTime * MID_GAME_EARLY;
   Search->SelEarly  = Search->MaxTime * SEL_SOLVE_EARLY;
   Search->WldEarly  = Search->MaxTime * WLD_SOLVE_EARLY;
   Search->FullEarly = Search->MaxTime * FULL_SOLVE_EARLY;
}

/* InitThinkOP() */

void InitThinkOP(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = TRUE;
   Search->Book       = UseBook;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = 0;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = 0.0;
   Search->MidEarly   = 0.0;
   Search->Sel        = EmptyNb(Board) >= 20;
   Search->SelDepth   = SEL_SOLVE_DEPTH;
   Search->SelEarly   = 0.0;
   Search->Wld        = TRUE;
   Search->WldDepth   = WLD_SOLVE_DEPTH;
   Search->WldAlgo    = (EmptyNb(Board) >= 20) ? WLD : WldSolve;
   Search->WldEarly   = 0.0;
   Search->Full       = EmptyNb(Board) <= 26;
   Search->FullDepth  = FULL_SOLVE_DEPTH;
   Search->FullAlgo   = FullSolve;
   Search->FullEarly  = 0.0;

   AllocTime(Search,RemainingTime(-Search->Board->Colour)*OPPONENT_TIME);

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;

   Search->MidEarly  = Search->MaxTime * MID_GAME_EARLY;
   Search->SelEarly  = Search->MaxTime * SEL_SOLVE_EARLY;
   Search->WldEarly  = Search->MaxTime * WLD_SOLVE_EARLY;
   Search->FullEarly = Search->MaxTime * FULL_SOLVE_EARLY;
}

/* InitThinkAS() */

void InitThinkAS(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = TRUE;
   Search->Book       = UseBook;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = 0;
   Search->TimeLimit  = FALSE;
   Search->MaxTime    = 0.0;
   Search->MidEarly   = 0.0;
   Search->Sel        = EmptyNb(Board) >= 20;
   Search->SelDepth   = SEL_SOLVE_DEPTH;
   Search->SelEarly   = 0.0;
   Search->Wld        = TRUE;
   Search->WldDepth   = WLD_SOLVE_DEPTH;
   Search->WldAlgo    = (EmptyNb(Board) >= 20) ? WLD : WldSolve;
   Search->WldEarly   = 0.0;
   Search->Full       = EmptyNb(Board) <= 26;
   Search->FullDepth  = FULL_SOLVE_DEPTH;
   Search->FullAlgo   = FullSolve;
   Search->FullEarly  = 0.0;

   AllocTime(Search,RemainingTime(Search->Board->Colour));

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;

   Search->MidEarly  = Search->MaxTime * MID_GAME_EARLY;
   Search->SelEarly  = Search->MaxTime * SEL_SOLVE_EARLY;
   Search->WldEarly  = Search->MaxTime * WLD_SOLVE_EARLY;
   Search->FullEarly = Search->MaxTime * FULL_SOLVE_EARLY;
}

/* InitThinkOB() */

void InitThinkOB(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = FALSE;
   Search->Book       = UseBook;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = 0;
   Search->TimeLimit  = FALSE;
   Search->MaxTime    = 0.0;
   Search->MidEarly   = 0.0;
   Search->Sel        = EmptyNb(Board) >= 20;
   Search->SelDepth   = SEL_SOLVE_DEPTH;
   Search->SelEarly   = 0.0;
   Search->Wld        = TRUE;
   Search->WldDepth   = WLD_SOLVE_DEPTH;
   Search->WldAlgo    = (EmptyNb(Board) >= 20) ? WLD : WldSolve;
   Search->WldEarly   = 0.0;
   Search->Full       = EmptyNb(Board) <= 26;
   Search->FullDepth  = FULL_SOLVE_DEPTH;
   Search->FullAlgo   = FullSolve;
   Search->FullEarly  = 0.0;

   AllocTime(Search,RemainingTime(Search->Board->Colour));

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;

   Search->MidEarly  = Search->MaxTime * MID_GAME_EARLY;
   Search->SelEarly  = Search->MaxTime * SEL_SOLVE_EARLY;
   Search->WldEarly  = Search->MaxTime * WLD_SOLVE_EARLY;
   Search->FullEarly = Search->MaxTime * FULL_SOLVE_EARLY;
}

/* InitThinkBK() */

void InitThinkBK(const BOARD *Board, const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = FALSE;
   Search->Book       = FALSE;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = EmptyNb(Board) - MIN_ENDGAME_DEPTH;
   Search->TimeLimit  = EmptyNb(Board) > 22;
   Search->MaxTime    = TEST_TIME;
   Search->MidEarly   = LearningTime * MID_GAME_EARLY;
   Search->Sel        = EmptyNb(Board) >= 22;
   Search->SelDepth   = 0;
   Search->SelEarly   = LearningTime * SEL_SOLVE_EARLY;
   Search->Wld        = TRUE;
   Search->WldDepth   = 0;
   Search->WldAlgo    = WLD;
   Search->WldEarly   = LearningTime * WLD_SOLVE_EARLY;
   Search->Full       = EmptyNb(Board) <= 18;
   Search->FullDepth  = 0;
   Search->FullAlgo   = FULL;
   Search->WldEarly   = LearningTime * FULL_SOLVE_EARLY;

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;
}

/* InitThinkTS() */

void InitThinkTS(const BOARD *Board,const int Forbidden[], BEST_MOVE *Best) {

   int I;

   Search->Start = CurrentTime();

   CopyBoard(Board,Search->Board);
   if (Forbidden != NULL) {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = Forbidden[I];
   } else {
      for (I = 0; I < SQUARE_NB; I++) Search->Forbidden[I] = FALSE;
   }
   Search->Best = Best;

   Search->ForceAlone = FALSE;
   Search->Book       = FALSE;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = EmptyNb(Board) - MAX_ENDGAME_DEPTH;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = TEST_TIME;
   Search->MidEarly   = TEST_TIME;
   Search->Sel        = TRUE;
   Search->SelDepth   = 0;
   Search->SelEarly   = TEST_TIME;
   Search->Wld        = TRUE;
   Search->WldDepth   = 0;
   Search->WldAlgo    = WLD;
   Search->WldEarly   = TEST_TIME;
   Search->Full       = FALSE; /* TRUE */
   Search->FullDepth  = 0;
   Search->FullAlgo   = WINDOW;
   Search->FullEarly  = TEST_TIME;

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;
}

/* InitThinkPC() */

static void InitThinkPC(const BOARD *Board) {

   Search->Start = CurrentTime();

   Search->ForceAlone = FALSE;
   Search->Book       = FALSE;
   Search->DepthLimit = TRUE;
   Search->MaxDepth   = EmptyNb(Board) - MAX_ENDGAME_DEPTH;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = PROBCUT_TIME;
   Search->MidEarly   = PROBCUT_TIME;
   Search->Sel        = FALSE;
   Search->Wld        = FALSE;
   Search->Full       = FALSE;

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;
}

/* InitThinkE() */

static void InitThinkE(const BOARD *Board) {

   Search->Start = CurrentTime();

   Search->ForceAlone = FALSE;
   Search->Book       = FALSE;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = EmptyNb(Board) - MAX_ENDGAME_DEPTH;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = EVAL_TIME;
   Search->MidEarly   = EVAL_TIME;
   Search->Sel        = FALSE;
   Search->Wld        = FALSE;
   Search->Full       = FALSE;

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;
}

/* UpdateThinkAS() */

void UpdateThinkAS(void) {

   Search->ForceAlone = TRUE;
   Search->Book       = UseBook;
   Search->DepthLimit = FALSE;
   Search->MaxDepth   = 0;
   Search->TimeLimit  = TRUE;
   Search->MaxTime    = 0.0;
   Search->MidEarly   = 0.0;
   Search->Sel        = EmptyNb(Search->Board) > WLD_SOLVE_DEPTH;
   Search->SelDepth   = SEL_SOLVE_DEPTH;
   Search->SelEarly   = 0.0;
   Search->Wld        = TRUE;
   Search->WldDepth   = WLD_SOLVE_DEPTH;
   Search->WldAlgo    = WldSolve;
   Search->WldEarly   = 0.0;
   Search->Full       = EmptyNb(Search->Board) <= WLD_SOLVE_DEPTH;
   Search->FullDepth  = FULL_SOLVE_DEPTH;
   Search->FullAlgo   = FullSolve;
   Search->FullEarly  = 0.0;

   AllocTime(Search,RemainingTime(Search->Board->Colour));
   Search->MaxTime += 0.5 * Duration(Search->Start,CurrentTime());

   if (Search->MaxDepth > MAX_DEPTH) Search->MaxDepth = MAX_DEPTH;
   if (Search->MaxDepth < MIN_DEPTH) Search->MaxDepth = MIN_DEPTH;

   if (Search->MaxTime  > MAX_TIME)  Search->MaxTime  = MAX_TIME;
   if (Search->MaxTime  < MIN_TIME)  Search->MaxTime  = MIN_TIME;

   Search->MidEarly  = Search->MaxTime * MID_GAME_EARLY;
   Search->SelEarly  = Search->MaxTime * SEL_SOLVE_EARLY;
   Search->WldEarly  = Search->MaxTime * WLD_SOLVE_EARLY;
   Search->FullEarly = Search->MaxTime * FULL_SOLVE_EARLY;
}

/* AllocTime() */

static void AllocTime(SEARCH *Search, double Time) {

   int Empties, MaxDepth;
   double MaxTime, TimeFull, TimeWld;

   Empties = EmptyNb(Search->Board);
   MaxTime = Time;

   TimeFull = pow(BF,(double)Empties) / S;
   TimeWld  = pow(BW,(double)Empties) / S;

   if (Empties <= Search->FullDepth || MaxTime >= TimeFull) {
      MaxDepth = Empties - MAX_ENDGAME_DEPTH;
      MaxTime *= FULL_SOLVE_TIME;
   } else if (Empties <= Search->WldDepth || MaxTime >= TimeWld) {
      MaxDepth = Empties - MAX_ENDGAME_DEPTH;
      MaxTime *= WLD_SOLVE_TIME;
   } else if (Empties <= Search->SelDepth) {
      MaxDepth = Empties - MAX_ENDGAME_DEPTH;
      MaxTime *= SEL_SOLVE_TIME;
   } else {
      MaxDepth = Empties - MIN_ENDGAME_DEPTH;
/*
      MaxDepth = 3;
*/
      MaxTime *= MID_GAME_TIME / (double) ((Empties - 19) / 2);
   }

   Search->MaxDepth = MaxDepth;
   Search->MaxTime  = MaxTime;
}

/* DisplayInfos() */

void DisplayInfos(BEST_MOVE *Best) {

   SetBestValue(Best);
   Output("");
   Output("%s\n",InfoString(Best));
}

/* UpdateInfos() */

void UpdateInfos(BEST_MOVE *Best) {

   SetBestValue(Best);
   Output("%s",InfoString(Best));
}

/* SetBestValue() */

static void SetBestValue(BEST_MOVE *Best) {

   int Depth;

   Best->Time = Duration(Search->Start,CurrentTime());

   if (! Best->Modified) return;

   if (Best->Depth == SOLVE_DEPTH
   || (Best->Min > -INF && Best->Min < -MID_INF) || Best->Min > +MID_INF
   || (Best->Max > +MID_INF && Best->Max < +INF) || Best->Max < -MID_INF) {
      if (Best->Min == Best->Max) {
         Best->BookValue = Best->Min;
         Best->IosValue  = (double) UNDIFFNOTE(Best->Min);
         sprintf(Best->StringValue,"= %+d",UNDIFFNOTE(Best->Min));
      } else if (Best->Min != -INF) {
         Best->BookValue = Best->Min;
         Best->IosValue  = (double) UNDIFFNOTE(Best->Min) - 0.01;
         sprintf(Best->StringValue,"> %+d",UNDIFFNOTE(Best->Min));
      } else if (Best->Max != +INF) {
         Best->BookValue = Best->Max;
         Best->IosValue  = (double) UNDIFFNOTE(Best->Max) + 0.01;
         sprintf(Best->StringValue,"< %+d",UNDIFFNOTE(Best->Max));
      } else {
         Best->BookValue = 0;
         Best->IosValue  = 0.00;
      }
   } else if (Best->Depth >= SELECTIVE_DEPTH) {
      Depth = Best->Depth - SELECTIVE_DEPTH;
      if (Best->Min > 0) {
         Best->BookValue = Best->Min;
         Best->IosValue  = +1.0 + (DoubleEval(Best->Min) - 0.5) * 2.0;
         sprintf(Best->StringValue,"= +1@%.0f%%",200.0*(0.5-DoubleEval(Best->Max)));
      } else if (Best->Max < 0) {
         Best->BookValue = Best->Max;
         Best->IosValue  = -1.0 - (0.5 - DoubleEval(Best->Max)) * 2.0;
         sprintf(Best->StringValue,"= -1@%.0f%%",200.0*(0.5-DoubleEval(Best->Max)));
      } else {
         Best->BookValue = 0;
         Best->IosValue  = (double) Depth / 100.0;
         sprintf(Best->StringValue,"= 0@%d%%",Depth);
      }
   } else {
      Depth = Best->Depth;
      if (Depth >= MIDGAME_DEPTH) Depth -= MIDGAME_DEPTH;
      if (Best->Min == Best->Max) {
         Best->BookValue = Best->Min;
         Best->IosValue  = (double) Best->Depth + BoundedEval(Best->Min);
         sprintf(Best->StringValue,"= %.0f%%@%d",100.0*DoubleEval(Best->Min),Best->Depth);
      } else if (Best->Min != -INF) {
         Best->BookValue = Best->Min;
         Best->IosValue  = (double) Best->Depth + BoundedEval(Best->Min);
         sprintf(Best->StringValue,"> %.0f%%@%d",100.0*DoubleEval(Best->Min),Best->Depth);
      } else if (Best->Max != +INF) {
         Best->BookValue = Best->Max;
         Best->IosValue  = (double) Best->Depth + BoundedEval(Best->Max);
         sprintf(Best->StringValue,"< %.0f%%@%d",100.0*DoubleEval(Best->Max),Best->Depth);
      } else {
         Best->BookValue = 0;
         Best->IosValue  = 0.00;
      }
   }

   Best->Modified = FALSE;
}

/* InfoString() */

static const char *InfoString(const BEST_MOVE *Best) {

   int I;
   static char String[256], String2[256];

   strcpy(String,"");

      sprintf(String2,"%s",Best->CurrType);
      strcat(String,String2);

   if (Best->Event == UPDATE) {

      sprintf(String2," %s",SquareString[Best->CurrMove]);
      strcat(String,String2);

      sprintf(String2," %2d/%2d",Best->CurrMoveNo+1,RootMoveNb);
      strcat(String,String2);

   } else {

      sprintf(String2," %s",EventString[Best->Event]);
      strcat(String,String2);
   }

   sprintf(String2," %+6.2f",Best->IosValue);
   strcat(String,String2);

   for (I = 0; I < 8 && Best->PV[I] != NO_MOVE && (Best->PV[I] != PASS || Best->PV[I+1] != PASS); I++) {
      strcat(String," ");
      strcat(String,SquareString[Best->PV[I]]);
   }
   for (; I < 8; I++) strcat(String," --");

   sprintf(String2," %5s",TimeString(Best->Time));
   strcat(String,String2);

   sprintf(String2," %5s",TimeString(Search->MaxTime));
   strcat(String,String2);

   sprintf(String2," %3.0f",100.0*Div0((double)HashRead,(double)HashNb));
   strcat(String,String2);

   sprintf(String2," %3.0f",100.0*Div0((double)(HashRead+HashWrite),(double)HashNb));
   strcat(String,String2);

   sprintf(String2," %9d",PvNodeNb+NodeNb+EvalNb);
   strcat(String,String2);

   sprintf(String2," %6.0f",Div0((double)(PvNodeNb+NodeNb+EvalNb),Best->Time));
   strcat(String,String2);

/*
   sprintf(String2," %4.2f",pow((double)EvalNb,1.0/(double)Best->Depth));
   strcat(String,String2);
*/

   return String;
}

/* WriteString() */

static void WriteString(const char *String, const char *FileName) {

   FILE *File;

   File = fopen(FileName,"a");
   if (File == NULL) {
      Error("Couldn't open file \"%s\" for writing",FileName);
   } else {
      fprintf(File,"%s\n",String);
      fclose(File);
   }
}

/* Div0() */

static double Div0(double X, double Y) {

   if (Y == 0.0) return 0.0;

   return X / Y;
}

/* Div2() */

static int Div2(int X) {

   if (X >= 0) {
      return X / 2;
   } else {
      return -((1-X) / 2);
   }
}

/* Round() */

int Round(double X) {

   return (int) floor(X+0.5);
}

/* End of Master.C */

