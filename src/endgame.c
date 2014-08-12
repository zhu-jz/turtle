
/* EndGame.C */

#include <stdio.h>
#include <setjmp.h>

#include "endgame.h"
#include "types.h"
#include "board.h"
#include "clock.h"
#include "eboard.h"
#include "eval.h"
#include "hash.h"
#include "hash.i"
#include "loop.h"
#include "master.h"
#include "mboard.h"
#include "mboard.i"
#include "output.h"
#include "sort.h"

/* Constants */

#define SORT_DEPTH      32 /* 32 */
#define SEL_MIN_DEPTH    9 /*  9 */
#define SEL_MAX_DEPTH   17 /* 17 */
#define FAST_SORT_DEPTH  9 /*  9 */
#define HASH_DEPTH       2 /*  2 */

#define EXAMPLE_NB ((int)(sizeof(Example)/sizeof(Example[0])))

/* "Constants" */

static const char Example[][65] = {

   "bw..wwwwb.wwwwwwbwwbbwwwbwwbwwwbbwwwwwwbb...wwwwb....w..b........",
   "b.wwwww....wwwwb..wwwwww.bbbbbww..bbwwb..wwbwbb....wbbw...www..w.",
   "b..www.......bb.wwwwwwbww.wwwwbwwb.wwwbbw...wwbww...wwwbw..wwww..",
   "w..bbbbb...bbbb...wwwbb...wwbbbb..wwbbbw.wwwwbww....bwb....bbbbb.",
   "w..w.b.w...w.bw.w.wwbbbwwwwwwbbbwwwwwbb..bbwwbw....bbbb.....bbb..",
   "b...bbbb.b.bbbw..bbwbww..bbbwbw..bbwbbw...wbbbww.w.wwww......ww..",
   "b...bbb....wwwb....wwwbb..wwwwbbb..wwwwbb..wbwbbb..bbww...bbbb.w.",
   "w.wwwww....wwww...wwwwb..bbbbbb...wbwwb..wwwbwb....wwbb....bbbb..",
   "w.....b..b.bbb...bbbbww..bwbwwbb.bwwbbb..bwwbb.....wwwb...bbbbbb.",
   "b..wb.w....bbww..wwwwwbb.wwwwwb..wwwbwbb.wwwwbb.....wwb....b.w...",
   "b....b.....bbb....wwwbwww.wwwbwww.wbwbwbw.wwbbwww..wwbw....w..w..",
   "w....w.b......b.....bbbw.wbbbbbww.bbwwbwwbbwbbbww..wwww.w....ww..",
   "w...b.......wb..b..bwwbbbbbbwbbbbbbbwwbbbbbbwwwbb..bw...b........",
   "b....ww.....www...bbbbwww..bbwwbw.bbbbbww..wwwbww..b.wb.w.....b..",
   "b..www...bbww....bbbbwwwwbbbbwb..bbbwbb..bbwww......www.....w....",
   "w........b.b......bbbbwwwwwbwbb..wwwbbbb.wwbbbb..w.wwwb.....ww...",
   "w..bbbbb...bbbb...wwwbb...wwbwb...wbbbbb.wwwwwbw....wbb..........",
   "b...................bbwww..bbbwww..bbwbww.wwwbbbw..wbww.w.wwwww..",
   "b..bwww....www....wwwbww..wwwwbw..wbwbbb.wwbbbb....b.bb..........",
   "b.......................w..wwwww...wwwwwbwwwwbbbb..bbwwbb..bb.w.b"

/*
   "bbbbwbbbbwbbbbbbbwwbbbbbbwwwbbbbbwwwbbww.wwwww...wwwwwww.wwwwwww.",
   "wwwwwwwwwbwwbbb..bbwwbww.bwbwww..bwwwwb..bwwbww..bwwwww..bbbb....",
   "w..wwww..b.wwwww.bbwwbwbbbwbwbbbbbbbwbbbb.bbwbwbb..wbbb.b....b...",
   "b..w.......wwb...wwwbbbw.wwwwbwbbbbbwwbwbbbbbbwwbb.bbbbwb..bbbb..",
   "w..w..w.....wwwb..b.bwbww..bbbwwwbbbbwwwwbbbwbbwwbbbbbb..bwbb.w..",
   "w....b......bbbw..wwwbbbbbwwwwbbw.bbwwbbwwwbwbbbbwwwbb...b.bbbb..",
   "b.wwwww....wbbw...wwwwbbw.wwwbwbb.wwbwwbb.bwbbwbb..w.bbbb..w....w",
   "b..bw.w....wwww..wwbwbbw.wwwwbbwwwwwbbwb.wbwbbbbb..bbbb....b.w.b.",
   "b..w.......www..b.bwwwwbbbbbbwbwb.bbwbwwbbbwbwwbb.wwwww.b...www..",
   "b.wbbbb....wbbw..bbwwbwwwbbbwwbwwbbwwbwwwbbbbww.bb.bbw...........",
   "b.bbb....b.bww...bbwbww..bwbwbw..bwwbwbbbbwwbbwb...wwwww..bbbbb..",
   "b.wwwww....wwww..wbbwww...bbbww..bbbbbbw.bbbwww.wb.wwww...wwwww..",
   "b..bb....w.bbwb..wwbww...wbwbwww.wwbbwwwbwwbbbwwb..bbbbwb..b..b.b",
   "b.bbbbbbb..bwww....wbwwbb.wwbbwbb.wwwwwbb.b.bwwbb...w.b.b..wwww..",
   "b.............w.w.wwwwwwwwwwwwbwwwbbwwwbw.bbbwbww..bbbwbw..wbbbbw",
   "w..bbb.....bbbb.wwwbbwwwwwwwwwwbw.wwbbbbw.wwwbbbw...bwbb...b.....",
   "w...b.w....bbbw.bbbbbbbbbbwwbbwwbbwbwwwbbbbwwww.bb..wwww.........",
   "b..wwww..w.wwww..wbbbwww.wbbwbw..wwbbwbb.wwbbbb..w.bbb.....bb.w..",
   "b..wwww....wwww...bwbbwwbwwbwwwwb.wwwwwbbbwwbbbbb..b.b...........",
   "ww.wwww..bwbbwb..bwwwbbb.bwwwbb..bwwbwb..bwbbb...b.bb............"
*/
};

/* Variables */

int EgEventPeriod;

static INFO    NewInfo = { 0, 0, PASS, FALSE, FALSE, 0};

static int     WinProb;
static int     SelMinDepth, SelMaxDepth;

static jmp_buf Return;
static int     Empties;
static int     DiscDiff;
static int     Depth;

/* Prototypes */

static int  PVS       (int PV[], int Alpha, int Beta, int Depth, int PrevMove);
static int  NWS       (int Alpha, int Depth, int PrevMove);

static void SolveInit (const MBOARD *MBoard);
static int  SolvePVS  (int PV[], int Alpha, int Beta, int Depth, int DiskDiff, int PrevMove);
static int  SolveNWS  (int Alpha, int Depth, int DiskDiff, int PrevMove);

/* Functions */

/* EndTest() */

void EndTest(void) {

   int       Pos, Rank, File, Square, Char, Empties;
   BOARD     Board[1];
   BEST_MOVE Best[1];

   ClearBoard(Board);

   for (Pos = 0; Pos < EXAMPLE_NB; Pos++) {

      Empties = 0;

      for (Rank = 0, Square = A1, Char = 1; Rank < 8; Rank++, Square++) {
         for (File = 0; File < 8; File++, Square++, Char++) {
            switch (Example[Pos][Char]) {
               case '.' : Board->Square[Square] = EMPTY;
                          Empties++;
                          break;
               case 'b' : Board->Square[Square] = BLACK;
                          break;
               case 'w' : Board->Square[Square] = WHITE;
                          break;
               default  : Error("Unknown character '%c' (0x%02X) in endgame test positions",Example[Pos][Char],Example[Pos][Char]);
                          break;
            }
         }
      }

      Board->Colour = (Example[Pos][0] == 'b') ? BLACK : WHITE;
      DispBoard(Board);

      ClearHashTable();

      InitThinkTS(Board,NULL,Best);
      Think();
   }
}

/* InitSolve() */

void InitSolve(const BOARD *Board) {

   Depth = EmptyNb(Board);
   SetEvalMoveNo(60-Depth);
   MCopyBoard(Board);

   NewInfo.Date = Date;
}

/* SelSolve() */

int SelSolve(int Alpha, int Beta, double Confidence, BEST_MOVE *Best) {

   int Event, I, J, MoveNo;
   int Move, Value, Depth1;
   int NewKey[2], PV[PV_MOVE_NB+1];

   if (Confidence >= 1.0) return Solve(Alpha,Beta,Best);

   Event = setjmp(Return);
   if (Event != 0) {
      Best->Time = Duration(Search->Start,CurrentTime());
      return Event;
   }

   SelMinDepth = SEL_MIN_DEPTH;
   SelMaxDepth = SEL_MAX_DEPTH;

   for (WinProb = 0; DoubleEval(WinProb+1) < 0.5+Confidence/2.0; WinProb++)
      ;

   NewKey[0] = MBoard->Key[0];
   NewKey[1] = MBoard->Key[1];

   *++MBoard->FlipSP = NULL;

   Best->Event = UPDATE;

   for (I = 0; I < RootMoveNb; I++) {

      MoveNo = I;
      Move = RootMove[MoveNo].Move;

      Best->CurrMove   = Move;
      Best->CurrMoveNo = MoveNo;

      UpdateInfos(Best);

      if (Move != PASS) {
         MDoHashFlips(&MBoard->Square[Move]);
         Depth1 = Depth - 1;
      } else {
         MDoPass();
         Depth1 = Depth;
      }

      PV[0] = Move;
      PV[1] = NO_MOVE;

      if (I == 0) {

         Value = -PVS(PV+1,-Beta,-Alpha,Depth1,Move);

      } else {

	 if (Depth1 <= 1) {
            Value = -PVS(PV+1,-Alpha-1,-Alpha,Depth1,Move);
         } else {
            Value = -NWS(-Alpha-1,Depth1,Move);
         }

         if (Value > Alpha && Value < Beta && Depth1 > 2) {

	    RootMove[MoveNo].Depth = Best->CurrDepth;
	    RootMove[MoveNo].Min   = Value;
	    RootMove[MoveNo].Max   = +INF;

	    MoveToFront(MoveNo);
	    MoveNo = 0;

            Best->Move  = RootMove[MoveNo].Move;
            Best->Depth = RootMove[MoveNo].Depth;
            Best->Min   = RootMove[MoveNo].Min;
            Best->Max   = RootMove[MoveNo].Max;
            for (J = 0; PV[J] != NO_MOVE; J++) Best->PV[J] = PV[J];
            Best->PV[J] = NO_MOVE;
            Best->Modified = TRUE;

            UpdateInfos(Best);

            Value = -PVS(PV+1,-Beta,-Value,Depth1,Move);
         }
      }

      if (Move != PASS) {
         MUndoHashFlips(&MBoard->Square[Move]);
         MBoard->Key[0] = NewKey[0];
         MBoard->Key[1] = NewKey[1];
      } else {
         MDoPass();
      }

      RootMove[MoveNo].Depth = Best->CurrDepth;
      RootMove[MoveNo].Min   = (Value > Alpha) ? Value : -INF;
      RootMove[MoveNo].Max   = (Value < Beta)  ? Value : +INF;

      if (I == 0 || Value > Alpha) {

	 MoveToFront(MoveNo);
         MoveNo = 0;

	 Best->Move  = RootMove[MoveNo].Move;
	 Best->Depth = RootMove[MoveNo].Depth;
	 Best->Min   = RootMove[MoveNo].Min;
	 Best->Max   = RootMove[MoveNo].Max;
         for (J = 0; PV[J] != NO_MOVE; J++) Best->PV[J] = PV[J];
         Best->PV[J] = NO_MOVE;
	 Best->Modified = TRUE;

         if (I != 0) DisplayInfos(Best);

         if (Value > Alpha) {
   	    if (Value >= Beta) break;
	    Alpha = Value;
         }
      }
   }

   MBoard->FlipSP--;

   Best->Time = Duration(Search->Start,CurrentTime());

   return DEPTH;
}

/* Solve() */

int Solve(int Alpha, int Beta, BEST_MOVE *Best) {

   int Event, I, J, MoveNo;
   int Move, Value, Depth1;
   int NewKey[2], PV[PV_MOVE_NB+1];

   Event = setjmp(Return);
   if (Event != 0) {
      Best->Time = Duration(Search->Start,CurrentTime());
      return Event;
   }

   SelMinDepth = SORT_DEPTH;
   SelMaxDepth = SORT_DEPTH - 1;

   WinProb = MID_INF;

   NewKey[0] = MBoard->Key[0];
   NewKey[1] = MBoard->Key[1];

   *++MBoard->FlipSP = NULL;

   Best->Event = UPDATE;

   for (I = 0; I < RootMoveNb; I++) {

      MoveNo = I;
      Move   = RootMove[MoveNo].Move;

      Best->CurrMove   = Move;
      Best->CurrMoveNo = MoveNo;

      UpdateInfos(Best);

      if (Move != PASS) {
         MDoHashFlips(&MBoard->Square[Move]);
         Depth1 = Depth - 1;
      } else {
         MDoPass();
         Depth1 = Depth;
      }

      PV[0] = Move;
      PV[1] = NO_MOVE;

      if (I == 0) {

         Value = -PVS(PV+1,-Beta,-Alpha,Depth1,Move);

      } else {

	 if (Depth1 <= 1) {
            Value = -PVS(PV+1,-Alpha-1,-Alpha,Depth1,Move);
         } else {
            Value = -NWS(-Alpha-1,Depth1,Move);
         }

         if (Value > Alpha && Value < Beta && Depth1 > 2) {

	    if (RootMove[MoveNo].Depth == SOLVE_DEPTH) {
	       if (Value > RootMove[MoveNo].Min) RootMove[MoveNo].Min = Value;
	    } else {
	       RootMove[MoveNo].Depth = SOLVE_DEPTH;
	       RootMove[MoveNo].Min   = Value;
	       RootMove[MoveNo].Max   = +INF;
	    }

	    MoveToFront(MoveNo);
	    MoveNo = 0;

            Best->Move  = RootMove[MoveNo].Move;
            Best->Depth = RootMove[MoveNo].Depth;
            Best->Min   = RootMove[MoveNo].Min;
            Best->Max   = RootMove[MoveNo].Max;
            for (J = 0; PV[J] != NO_MOVE; J++) Best->PV[J] = PV[J];
            Best->PV[J] = NO_MOVE;
            Best->Modified = TRUE;

            UpdateInfos(Best);

            Value = -PVS(PV+1,-Beta,-Value,Depth1,Move);
         }
      }

      if (Move != PASS) {
         MUndoHashFlips(&MBoard->Square[Move]);
         MBoard->Key[0] = NewKey[0];
         MBoard->Key[1] = NewKey[1];
      } else {
         MDoPass();
      }

      if (RootMove[MoveNo].Depth == SOLVE_DEPTH) {
         if (Value > Alpha && Value > RootMove[MoveNo].Min) {
            RootMove[MoveNo].Min = Value;
         }
         if (Value < Beta && Value < RootMove[MoveNo].Max) {
            RootMove[MoveNo].Max = Value;
         }
      } else {
         RootMove[MoveNo].Depth = SOLVE_DEPTH;
         RootMove[MoveNo].Min   = (Value > Alpha) ? Value : -INF;
         RootMove[MoveNo].Max   = (Value < Beta)  ? Value : +INF;
      }

      if (I == 0 || Value > Alpha) {

	 MoveToFront(MoveNo);
         MoveNo = 0;

	 Best->Move  = RootMove[MoveNo].Move;
	 Best->Depth = RootMove[MoveNo].Depth;
	 Best->Min   = RootMove[MoveNo].Min;
	 Best->Max   = RootMove[MoveNo].Max;
         for (J = 0; PV[J] != NO_MOVE; J++) Best->PV[J] = PV[J];
         Best->PV[J] = NO_MOVE;
	 Best->Modified = TRUE;

         if (I != 0) DisplayInfos(Best);

         if (Value > Alpha) {
   	    if (Value >= Beta) break;
	    Alpha = Value;
         }

      } else {

	 if (RootMove[MoveNo].Min > Best->Min) Best->Min = RootMove[MoveNo].Min;
	 if (RootMove[MoveNo].Max > Best->Max) Best->Max = RootMove[MoveNo].Max;
      }
   }

   MBoard->FlipSP--;

   Best->Time = Duration(Search->Start,CurrentTime());

   return DEPTH;
}

/* PVS() */

static int PVS(int PV[], int Alpha, int Beta, int Depth, int PrevMove) {

   int        Value, BestValue;
   int        NewKey[2], NewHash;
   MSQUARE   *HashKiller, **KillerList, **I, **BestMove, *Square;
   ENTRY     *Entry;
   INFO       Info;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   if (Depth < SelMinDepth) {
      SolveInit(MBoard);
      return SolvePVS(PV,Alpha,Beta,Empties,DiscDiff,PrevMove);
   }

   PvNodeNb++;

   Entry = NULL;
   if (Depth >= HASH_DEPTH) Entry = ReadHash(MBoard->Key);

   if (Entry != NULL) {
      if (Entry->Info.Depth >= Depth) {
         Info = Entry->Info;
         if (Info.Min && Info.Value > Alpha) {
            Alpha = Info.Value;
            if (Alpha >= Beta) {
               PV[0] = Info.Killer;
               PV[1] = NO_MOVE;
               return Alpha;
            }
         }
         if (Info.Max && Info.Value < Beta) {
            Beta = Info.Value;
            if (Beta <= Alpha) {
               PV[0] = Info.Killer;
               PV[1] = NO_MOVE;
               return Beta;
            }
         }
         NewHash   = TRUE;
         Info.Date = Date;
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      if (Depth >= HASH_DEPTH) Entry = WriteHash(MBoard->Key,Depth);
      if (Entry != NULL) {
         NewHash    = TRUE;
         Info       = NewInfo;
         Info.Depth = Depth;
      } else {
         NewHash = FALSE;
         Info    = NewInfo;
      }
   }

   KillerList = (MBoard->Colour > 0) ? &KillerLists[1][PrevMove][1] : &KillerLists[0][PrevMove][1];
   NewKey[0]  = MBoard->Key[0];
   NewKey[1]  = MBoard->Key[1];
   BestValue  = -INF-1;

   *++MBoard->FlipSP = NULL;

   if (Info.Killer != PASS) {
      I          = &KillerList[-1];
      HashKiller = &MBoard->Square[Info.Killer];
      Square     = HashKiller;
   } else {
      I          = KillerList;
      HashKiller = NULL;
      Square     = *I;
   }

   Move = MoveList;

   SetEvalMoveNo(61-Depth);

   while (TRUE) {

      if (Square->Disk == EMPTY && MDoIndices(Square)) {

	 EvalNb++;
	 Value = -Eval();

	 MUndoIndices(Square);

	 Move->Move = I;
	 if (I < KillerList) {
	    Move->Note = +INF + 1;
	 } else {
	    Move->Note = Value;
	 }
	 Move++;
      }

      Square = *++I;
      if (Square == NULL) break;
      if (Square == HashKiller) {
 	 Square = *++I;
	 if (Square == NULL) break;
      }
   }

   Move->Move = NULL;
   SortMove(MoveList,Move-1);

   for (Move = MoveList; Move->Move != NULL; Move++) {

      I = Move->Move;

      if (I < KillerList) {
         Square = HashKiller;
      } else {
         Square = *I;
      }

      MDoHashFlips(Square);

      if (BestValue == -INF-1) {
         PV[0] = Square->Move;
         Value = -PVS(PV+1,-Beta,-Alpha,Depth-1,Square->Move);
      } else {
 	 if (Depth <= 2) {
            Value = -PVS(PV+1,-Alpha-1,-Alpha,Depth-1,Square->Move);
         } else {
            Value = -NWS(-Alpha-1,Depth-1,Square->Move);
         }
         if (Value > Alpha) {
            PV[0] = Square->Move;
            PV[1] = NO_MOVE;
            if (Value < Beta && Depth > 3) {
               Value = -PVS(PV+1,-Beta,-Value,Depth-1,Square->Move);
            }
         }
      }

      MUndoHashFlips(Square);
      MBoard->Key[0] = NewKey[0];
      MBoard->Key[1] = NewKey[1];

      if (Value > BestValue) {
         if (Value > Alpha) {
            if (Value >= Beta) {
               if (PrevMove != PASS) {
                  if (I < KillerList) {
                     do I++; while (*I != Square);
                  }
                  if (I > KillerList) {
                     *I    = I[-1];
                     I[-1] = Square;
                  }
               }
               if (NewHash) {
                  Info.Killer = Square->Move;
                  if (WinProb >= MID_INF || Value > +MID_INF) {
                     Info.Min   = TRUE;
                     Info.Max   = FALSE;
                     Info.Value = Value;
                  }
                  Entry->Lock = MBoard->Key[1];
                  Entry->Info = Info;
               }
               MBoard->FlipSP--;
               return Value;
            }
            Alpha = Value;
            BestMove = I;
            if (NewHash && (WinProb >= MID_INF || Value > +MID_INF)) {
               Info.Min   = TRUE;
               Info.Max   = FALSE;
               Info.Value = Value;
            }
         }
         if (BestValue == -INF-1) BestMove = I;
         BestValue = Value;
      }
   }

   MBoard->FlipSP--;

   if (BestValue == -INF-1) {
      PV[0] = PASS;
      if (PrevMove == PASS) {
         PV[1] = NO_MOVE;
         BestValue = DIFFNOTE(MDiskDiff());
         if (NewHash) {
            Info.Min    = TRUE;
            Info.Max    = TRUE;
            Info.Value  = BestValue;
            Entry->Lock = MBoard->Key[1];
            Entry->Info = Info;
         }
      } else {
         MDoPass();
         BestValue = -PVS(PV+1,-Beta,-Alpha,Depth,PASS);
         MDoPass();
         if (NewHash) {
            Info.Min    = BestValue > Alpha && (WinProb >= MID_INF || BestValue > +MID_INF);
            Info.Max    = BestValue < Beta  && (WinProb >= MID_INF || BestValue < -MID_INF);
            if (Info.Min || Info.Max) {
               Info.Value  = BestValue;
               Entry->Lock = MBoard->Key[1];
               Entry->Info = Info;
            }
         }
      }
      return BestValue;
   }

   if (BestMove < KillerList) {
      Square = HashKiller;
   } else {
      Square = *BestMove;
   }

   if (PrevMove != PASS) {
      if (BestMove < KillerList) {
         do BestMove++; while (*BestMove != HashKiller);
      }
      if (BestMove > KillerList) {
         *BestMove    = BestMove[-1];
         BestMove[-1] = Square;
      }
   }

   if (NewHash) {
      Info.Killer = Square->Move;
      if (WinProb >= MID_INF || BestValue < -MID_INF) {
	 if (Info.Value == BestValue) {
            Info.Max = TRUE;
	 } else {
	    Info.Min   = Depth <= 1;
	    Info.Max   = TRUE;
	    Info.Value = BestValue;
         }
      }
      Entry->Lock = MBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* NWS() */

static int NWS(int Alpha, int Depth, int PrevMove) {

   int        Value, BestValue, BestValue1;
   int        NewKey[2], NewHash, NewHash1;
   MSQUARE   *HashKiller, **KillerList, **I, **BestMove, *BestMove1, *Square;
   ENTRY     *Entry, *Entry1;
   INFO       Info, Info1;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   if (Depth < SelMinDepth) {
      SolveInit(MBoard);
      return SolveNWS(Alpha,Empties,DiscDiff,PrevMove);
   }

   NodeNb++;

   if ((NodeNb & (EgEventPeriod - 1)) == 0) {
      if (Event()) longjmp(Return,STOP);
      if (Search->TimeLimit && Duration(Search->Start,CurrentTime()) >= Search->MaxTime) {
         longjmp(Return,TIME_OUT);
      }
   }

   Entry = NULL;
   if (Depth >= HASH_DEPTH) Entry = ReadHash(MBoard->Key);

   if (Entry != NULL) {
      if (Entry->Info.Depth >= Depth) {
         if (Entry->Info.Min && Entry->Info.Value >  Alpha) {
            return Entry->Info.Value;
         }
         if (Entry->Info.Max && Entry->Info.Value <= Alpha) {
            return Entry->Info.Value;
         }
         NewHash   = TRUE;
         Info      = Entry->Info;
         Info.Date = Date;
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      if (Depth >= HASH_DEPTH) Entry = WriteHash(MBoard->Key,Depth);
      if (Entry != NULL) {
         NewHash    = TRUE;
         Info       = NewInfo;
         Info.Depth = Depth;
      } else {
         NewHash = FALSE;
         Info    = NewInfo;
      }
   }

   KillerList = (MBoard->Colour > 0) ? &KillerLists[1][PrevMove][1] : &KillerLists[0][PrevMove][1];
   NewKey[0]  = MBoard->Key[0];
   NewKey[1]  = MBoard->Key[1];
   BestValue  = -INF-1;

   *++MBoard->FlipSP = NULL;

   if (Info.Killer != PASS) {
      I          = &KillerList[-1];
      HashKiller = &MBoard->Square[Info.Killer];
      Square     = HashKiller;
   } else {
      I          = KillerList;
      HashKiller = NULL;
      Square     = *I;
   }

   Move = MoveList;

   if (Depth <= SelMaxDepth) { /* SelMinDepth <= Depth <= SelMaxDepth */

      Entry1 = ReadHash(MBoard->Key);

      if (Entry1 != NULL) {
	 if (Entry1->Info.Depth >= 1) {
	    Info1 = Entry1->Info;
	    if (Entry1->Info.Depth == 1) {
	       if (Info1.Min && Info1.Value > +WinProb) {
		  MBoard->FlipSP--;
		  return Info1.Value;
	       }
	       if (Info1.Max && Info1.Value < -WinProb) {
		  MBoard->FlipSP--;
		  return Info1.Value;
	       }
	       NewHash1   = TRUE;
	       Info1.Date = Date;
	    } else {
	       NewHash1          = FALSE;
	       Entry1->Info.Date = Date;
	    }
	 } else {
	    NewHash1     = TRUE;
	    Info1        = NewInfo;
	    Info1.Depth  = 1;
	    Info1.Killer = Entry1->Info.Killer;
	 }
      } else {
	 Entry1 = WriteHash(MBoard->Key,1);
	 if (Entry1 != NULL) {
	    NewHash1    = TRUE;
	    Info1       = NewInfo;
	    Info1.Depth = 1;
	 } else {
	    NewHash1 = FALSE;
	    Info1    = NewInfo;
	 }
      }

      BestValue1 = -INF-1;

      SetEvalMoveNo(61-Depth);

      while (TRUE) {

         if (Square->Disk == EMPTY && MDoIndices(Square)) {

            EvalNb++;
            Value = -Eval();

            MUndoIndices(Square);

            if (Value > BestValue1) {
               if (Value > +WinProb) {
                  if (NewHash1) {
                     Info1.Killer = Square->Move;
                     Info1.Min    = TRUE;
                     Info1.Max    = FALSE;
                     Info1.Value  = Value;
                     Entry1->Lock = MBoard->Key[1];
                     Entry1->Info = Info1;
                  }
                  MBoard->FlipSP--;
                  return Value;
               }
               BestValue1 = Value;
               BestMove1  = Square;
            }

            Move->Move = I;
            if (I < KillerList) {
               Move->Note = +INF + 1;
            } else {
               Move->Note = Value;
            }
            Move++;
         }

         Square = *++I;
         if (Square == NULL) break;
         if (Square == HashKiller) {
            Square = *++I;
            if (Square == NULL) break;
         }
      }

      if (BestValue1 != -INF-1) {
         if (NewHash1) {
            Info1.Killer = BestMove1->Move;
            Info1.Min    = TRUE;
            Info1.Max    = TRUE;
            Info1.Value  = BestValue1; /* Depth = 1 => exact value */
            Entry1->Lock = MBoard->Key[1];
            Entry1->Info = Info1;
         }
         if (BestValue1 < -WinProb) {
            MBoard->FlipSP--;
            return BestValue1;
         }
      }

   } else { /* SelMaxDepth < Depth */

      if (HashKiller != NULL) {

         if (Square->Disk == EMPTY && MDoHashFlips(Square)) {

            BestValue = -NWS(-Alpha-1,Depth-1,Square->Move);

            MUndoHashFlips(Square);
            MBoard->Key[0] = NewKey[0];
            MBoard->Key[1] = NewKey[1];

            if (BestValue > Alpha) {
               if (PrevMove != PASS) {
                  do I++; while (*I != Square);
                  if (I > KillerList) {
                     *I    = I[-1];
                     I[-1] = Square;
                  }
               }
               if (NewHash) {
                  Info.Killer = Square->Move;
                  if (WinProb >= MID_INF || BestValue > +MID_INF) {
                     Info.Min   = TRUE;
                     Info.Max   = FALSE;
                     Info.Value = BestValue;
                  }
                  Entry->Lock = MBoard->Key[1];
                  Entry->Info = Info;
               }
               MBoard->FlipSP--;
               return BestValue;
            }

            BestMove = I;
         }

         Square = *++I;
         if (Square == HashKiller) Square = *++I;
      }

      SetEvalMoveNo(61-Depth);

      while (TRUE) {

         if (Square->Disk == EMPTY && MDoIndices(Square)) {

            EvalNb++;
            Value = -Eval();

            MUndoIndices(Square);

            Move->Move = I;
            Move->Note = Value;
            Move++;
         }

         Square = *++I;
         if (Square == NULL) break;
         if (Square == HashKiller) {
            Square = *++I;
            if (Square == NULL) break;
         }
      }
   }

   Move->Move = NULL;
   SortMove(MoveList,Move-1);

   for (Move = MoveList; Move->Move != NULL; Move++) {

      I = Move->Move;

      if (I < KillerList) {
         Square = HashKiller;
      } else {
         Square = *I;
      }

      MDoHashFlips(Square);

      Value = -NWS(-Alpha-1,Depth-1,Square->Move);

      MUndoHashFlips(Square);
      MBoard->Key[0] = NewKey[0];
      MBoard->Key[1] = NewKey[1];

      if (Value > BestValue) {
         if (Value > Alpha) {
            if (PrevMove != PASS) {
               if (I < KillerList) {
                  do I++; while (*I != Square);
               }
               if (I > KillerList) {
                  *I    = I[-1];
                  I[-1] = Square;
               }
            }
            if (NewHash) {
               Info.Killer = Square->Move;
               if (WinProb >= MID_INF || Value > +MID_INF) {
                  Info.Min   = TRUE;
                  Info.Max   = FALSE;
                  Info.Value = Value;
               }
               Entry->Lock = MBoard->Key[1];
               Entry->Info = Info;
            }
            MBoard->FlipSP--;
            return Value;
         }
         if (BestValue == -INF-1) BestMove = I;
         BestValue = Value;
      }
   }

   MBoard->FlipSP--;

   if (BestValue == -INF-1) {
      if (PrevMove == PASS) {
         BestValue = DIFFNOTE(MDiskDiff());
         if (NewHash) {
            Info.Min    = TRUE;
            Info.Max    = TRUE;
            Info.Value  = BestValue;
            Entry->Lock = MBoard->Key[1];
            Entry->Info = Info;
         }
      } else {
         MDoPass();
         BestValue = -NWS(-Alpha-1,Depth,PASS);
         MDoPass();
         if (NewHash) {
            Info.Min    = BestValue >  Alpha && (WinProb >= MID_INF || BestValue > +MID_INF);
            Info.Max    = BestValue <= Alpha && (WinProb >= MID_INF || BestValue < -MID_INF);
            if (Info.Min || Info.Max) {
               Info.Value  = BestValue;
               Entry->Lock = MBoard->Key[1];
               Entry->Info = Info;
            }
         }
      }
      return BestValue;
   }

   if (BestMove < KillerList) {
      Square = HashKiller;
   } else {
      Square = *BestMove;
   }

   if (PrevMove != PASS) {
      if (BestMove < KillerList) {
         do BestMove++; while (*BestMove != HashKiller);
      }
      if (BestMove > KillerList) {
         *BestMove    = BestMove[-1];
         BestMove[-1] = Square;
      }
   }

   if (NewHash) {
      Info.Killer = Square->Move;
      if (WinProb >= MID_INF || BestValue < -MID_INF) {
         Info.Min   = Depth <= 1;
         Info.Max   = TRUE;
         Info.Value = BestValue;
      }
      Entry->Lock = MBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* SolveInit() */

static void SolveInit(const MBOARD *MBoard) {

   int            D, E, Rank, File, Disk;
   const int     *EmptyNo;
   const MSQUARE *MSquare;
   ESQUARE       *ESquare, *PrevNode, *CurrNode;

   EBoard->Colour = MBoard->Colour;
   EBoard->FlipSP = EBoard->FlipStack;
   EBoard->Key[0] = MBoard->Key[0];
   EBoard->Key[1] = MBoard->Key[1];

   D = 0;
   E = 0;

   MSquare = &MBoard->Square[A1];
   ESquare = &EBoard->Square[A1];

   Rank = 8;
   do {
      File = 8;
      do {
         Disk = MSquare++->Disk;
         ESquare++->Disk = Disk;
         D += Disk;
         if (Disk == EMPTY) E++;
      } while (--File != 0);
      MSquare++;
      ESquare++;
   } while (--Rank != 0);

   if (MBoard->Colour < 0) D = -D;

   DiscDiff = D;
   Empties  = E;

   PrevNode = EBoard->Square;
   CurrNode = EBoard->Square;
   CurrNode->Pred = NULL;

   EmptyNo = BestToWorst;
   do {
      if (EBoard->Square[*EmptyNo].Disk == EMPTY) {
         CurrNode       = &EBoard->Square[*EmptyNo];
         CurrNode->Pred = PrevNode;
         PrevNode->Succ = CurrNode;
         PrevNode       = CurrNode;
      }
   } while (++EmptyNo < &BestToWorst[EMPTY_NB]);

   CurrNode->Succ = NULL;
}

/* SolvePVS() */

static int SolvePVS(int PV[], int Alpha, int Beta, int Depth, int DiskDiff, int PrevMove) {

   int        NewHash, Flipped, Value, BestMove, BestValue, Colour;
   int        NewKey[2], HashKiller;
   ENTRY     *Entry;
   INFO       Info;
   ESQUARE   *E, *E2;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   PvNodeNb++;

   Entry = NULL;
   if (Depth >= HASH_DEPTH) Entry = ReadHash(EBoard->Key);

   if (Entry != NULL) {
      if (Entry->Info.Depth >= Depth) {
         Info = Entry->Info;
         if (Info.Min && Info.Value > Alpha) {
            Alpha = Info.Value;
            if (Alpha >= Beta) {
               PV[0] = Info.Killer;
               PV[1] = NO_MOVE;
               return Alpha;
            }
         }
         if (Info.Max && Info.Value < Beta) {
            Beta = Info.Value;
            if (Beta <= Alpha) {
               PV[0] = Info.Killer;
               PV[1] = NO_MOVE;
               return Beta;
            }
         }
         NewHash   = TRUE;
         Info.Date = Date;
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      if (Depth >= HASH_DEPTH) Entry = WriteHash(EBoard->Key,Depth);
      if (Entry != NULL) {
         NewHash    = TRUE;
         Info       = NewInfo;
         Info.Depth = Depth;
      } else {
         NewHash = FALSE;
         Info    = NewInfo;
      }
   }

   if (Depth == 0) {

      PV[0] = NO_MOVE;
      PvNodeNb--;
      EvalNb++;

      return DIFFNOTE(DiskDiff);

   } else if (Depth == 1) {

      E = EBoard->Square->Succ;

      if ((Flipped = ECountFlips(E,EBoard->Colour)) != 0) {
         PV[0] = E->Move;
         PV[1] = NO_MOVE;
         EvalNb++;
         DiskDiff += Flipped + 1;
      } else if ((Flipped = ECountFlips(E,-EBoard->Colour)) != 0) {
         PV[0] = PASS;
         PV[1] = E->Move;
         PV[2] = NO_MOVE;
         PvNodeNb++;
         EvalNb++;
         DiskDiff -= Flipped + 1;
      } else {
         PV[0] = NO_MOVE;
         PvNodeNb++;
         if (DiskDiff > 0) DiskDiff++; else DiskDiff--;
      }

      return DIFFNOTE(DiskDiff);
   }

   HashKiller = Info.Killer;
   NewKey[0]  = EBoard->Key[0];
   NewKey[1]  = EBoard->Key[1];
   BestValue  = -INF-1;

   *++EBoard->FlipSP = NULL;

   Move = MoveList;

   if (Depth < FAST_SORT_DEPTH) {

      Colour = EBoard->Colour;

      for (E = EBoard->Square->Succ; E != NULL; E = E->Succ) {

         if (EIsLegal(E,Colour)) {

            Move->Move = E;
            if (E->Move == HashKiller) {
               Move->Note = +1;
            } else {
               Move->Note = 0;
            }
            Move++;
         }
      }

   } else { /* FAST_SORT_DEPTH <= Depth */

      Colour = -EBoard->Colour;

      for (E = EBoard->Square->Succ; E != NULL; E = E->Succ) {

         if (EDoFlips(E) != 0) {

            Move->Move = E;
            if (E->Move == HashKiller) {
               Move->Note = +1;
            } else {
               E->Pred->Succ = E->Succ;
               Move->Note = 0;
               for (E2 = EBoard->Square->Succ; E2 != NULL; E2 = E2->Succ) {
                  if (EIsLegal(E2,Colour)) Move->Note--;
               }
               E->Pred->Succ = E;
            }
            Move++;

            EUndoFlips(E);
         }
      }
   }

   Move->Move = NULL;
   SortMove(MoveList,Move-1);

   for (Move = MoveList; Move->Move != NULL; Move++) {

      E = Move->Move;

      Flipped = EDoHashFlips(E);

      E->Pred->Succ = E->Succ;
      if (E->Succ != NULL) E->Succ->Pred = E->Pred;

      if (BestValue == -INF-1) {
         PV[0] = E->Move;
         Value = -SolvePVS(PV+1,-Beta,-Alpha,Depth-1,-DiskDiff-Flipped,NO_MOVE);
      } else {
	 if (Depth <= 2) {
            Value = -SolvePVS(PV+1,-Alpha-1,-Alpha,Depth-1,-DiskDiff-Flipped,NO_MOVE);
	 } else {
            Value = -SolveNWS(-Alpha-1,Depth-1,-DiskDiff-Flipped,NO_MOVE);
         }
         if (Value > Alpha) {
            PV[0] = E->Move;
            PV[1] = NO_MOVE;
            if (Value < Beta && Depth > 3) {
               Value = -SolvePVS(PV+1,-Beta,-Value,Depth-1,-DiskDiff-Flipped,NO_MOVE);
            }
         }
      }

      E->Pred->Succ = E;
      if (E->Succ != NULL) E->Succ->Pred = E;

      EUndoFlips(E);
      EBoard->Key[0] = NewKey[0];
      EBoard->Key[1] = NewKey[1];

      if (Value > BestValue) {
         if (Value > Alpha) {
            if (Value >= Beta) {
               if (NewHash) {
                  Info.Killer = E->Move;
                  Info.Min    = TRUE;
                  Info.Max    = FALSE;
                  Info.Value  = Value;
                  Entry->Lock = EBoard->Key[1];
                  Entry->Info = Info;
               }
               EBoard->FlipSP--;
               return Value;
            }
            Alpha = Value;
            BestMove = E->Move;
            if (NewHash) {
               Info.Min   = TRUE;
               Info.Max   = FALSE;
               Info.Value = Value;
            }
         }
         if (BestValue == -INF-1) BestMove = E->Move;
         BestValue = Value;
      }
   }

   EBoard->FlipSP--;

   if (BestValue == -INF-1) {
      PV[0] = PASS;
      if (PrevMove == PASS) {
         PV[1] = NO_MOVE;
         if (DiskDiff > 0)      DiskDiff += Depth;
         else if (DiskDiff < 0) DiskDiff -= Depth;
         BestValue = DIFFNOTE(DiskDiff);
         if (NewHash) {
            Info.Min    = TRUE;
            Info.Max    = TRUE;
            Info.Value  = BestValue;
            Entry->Lock = EBoard->Key[1];
            Entry->Info = Info;
         }
      } else {
         EDoPass();
         BestValue = -SolvePVS(PV+1,-Beta,-Alpha,Depth,-DiskDiff,PASS);
         EDoPass();
         if (NewHash) {
            Info.Min    = BestValue > Alpha;
            Info.Max    = BestValue < Beta;
            Info.Value  = BestValue;
            Entry->Lock = EBoard->Key[1];
            Entry->Info = Info;
         }
      }
      return BestValue;
   }

   if (NewHash) {
      Info.Killer = BestMove;
      if (Info.Value != BestValue) {
         Info.Min   = Depth <= 2;
         Info.Value = BestValue;
      }
      Info.Max    = TRUE;
      Info.Value  = BestValue;
      Entry->Lock = EBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* SolveNWS() */

static int SolveNWS(int Alpha, int Depth, int DiskDiff, int PrevMove) {

   int        NewHash, Colour, DiskDiff2, Flipped, Flipped2, Value, BestMove, BestValue;
   int        NewKey[2], HashKiller;
   ENTRY     *Entry;
   INFO       Info;
   ESQUARE   *E, *E2;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   NodeNb++;

   if ((NodeNb & (EgEventPeriod - 1)) == 0) {
      if (Event()) longjmp(Return,STOP);
      if (Search->TimeLimit && Duration(Search->Start,CurrentTime()) >= Search->MaxTime) {
         longjmp(Return,TIME_OUT);
      }
   }

   Entry = NULL;
   if (Depth >= HASH_DEPTH) Entry = ReadHash(EBoard->Key);

   if (Entry != NULL) {
      if (Entry->Info.Depth >= Depth) {
         if (Entry->Info.Min && Entry->Info.Value >  Alpha) {
            return Entry->Info.Value;
         }
         if (Entry->Info.Max && Entry->Info.Value <= Alpha) {
            return Entry->Info.Value;
         }
         NewHash   = TRUE;
         Info      = Entry->Info;
         Info.Date = Date;
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      if (Depth >= HASH_DEPTH) Entry = WriteHash(EBoard->Key,Depth);
      if (Entry != NULL) {
         NewHash    = TRUE;
         Info       = NewInfo;
         Info.Depth = Depth;
      } else {
         NewHash = FALSE;
         Info    = NewInfo;
      }
   }

   HashKiller = Info.Killer;
   BestValue  = -INF-1;

   *++EBoard->FlipSP = NULL;

   if (Depth == 2) {

      Colour = EBoard->Colour;

      E  = EBoard->Square->Succ;
      E2 = E->Succ;

      if (E2->Move == HashKiller) {
         E  = E2;
         E2 = EBoard->Square->Succ;
      }

      Flipped = EDoFlips(E);

      if (Flipped != 0) {

         NodeNb++;

         DiskDiff2 = DiskDiff + Flipped;

         if ((Flipped2 = ECountFlips(E2,-Colour)) != 0) {
            EvalNb++;
            DiskDiff2 -= Flipped2 + 1;
         } else if ((Flipped2 = ECountFlips(E2,Colour)) != 0) {
            NodeNb++;
            EvalNb++;
            DiskDiff2 += Flipped2 + 1;
         } else {
            NodeNb++;
            if (DiskDiff2 > 0)      DiskDiff2++;
            else if (DiskDiff2 < 0) DiskDiff2--;
         }

         BestValue = DIFFNOTE(DiskDiff2);

         EUndoFlips(E);

         if (BestValue > Alpha) {
            if (NewHash) {
               Info.Killer = E->Move;
               Info.Min    = TRUE;
               Info.Max    = FALSE;
               Info.Value  = BestValue;
               Entry->Lock = EBoard->Key[1];
               Entry->Info = Info;
            }
            EBoard->FlipSP--;
            return BestValue;
         }

         BestMove = E->Move;
      }

      Flipped = EDoFlips(E2);

      if (Flipped != 0) {

         NodeNb++;

         DiskDiff2 = DiskDiff + Flipped;

         if ((Flipped2 = ECountFlips(E,-Colour)) != 0) {
            EvalNb++;
            DiskDiff2 -= Flipped2 + 1;
         } else if ((Flipped2 = ECountFlips(E,Colour)) != 0) {
            NodeNb++;
            EvalNb++;
            DiskDiff2 += Flipped2 + 1;
         } else {
            NodeNb++;
            if (DiskDiff2 > 0)      DiskDiff2++;
            else if (DiskDiff2 < 0) DiskDiff2--;
         }

         Value = DIFFNOTE(DiskDiff2);

         EUndoFlips(E2);

         if (Value > BestValue) {
            BestValue = Value;
            BestMove = E2->Move;
         }

         if (NewHash) {
            Info.Killer = BestMove;
	    Info.Min    = TRUE;
	    Info.Max    = TRUE;
	    Info.Value  = BestValue;
	    Entry->Lock = EBoard->Key[1];
	    Entry->Info = Info;
	 }

	 EBoard->FlipSP--;
	 return BestValue;
      }

   } else if (Depth <= HASH_DEPTH) { /* 2 < Depth <= HASH_DEPTH < */

      E2 = EBoard->Square;
      if (HashKiller != PASS) {
         E = &EBoard->Square[HashKiller];
         if (E->Disk != EMPTY) {
            E2 = E2->Succ;
            E  = E2;
         }
      } else {
         E2 = E2->Succ;
         E  = E2;
      }

      while (TRUE) {

         Flipped = EDoFlips(E);

         if (Flipped != 0) {

            E->Pred->Succ = E->Succ;
            if (E->Succ != NULL) E->Succ->Pred = E->Pred;

            Value = -SolveNWS(-Alpha-1,Depth-1,-DiskDiff-Flipped,NO_MOVE);

            E->Pred->Succ = E;
            if (E->Succ != NULL) E->Succ->Pred = E;

            EUndoFlips(E);

            if (Value > BestValue) {
               if (Value > Alpha) {
                  if (NewHash) {
                     Info.Killer = E->Move;
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = Value;
                     Entry->Lock = EBoard->Key[1];
                     Entry->Info = Info;
                  }
                  EBoard->FlipSP--;
                  return Value;
               }
               if (BestValue == -INF-1) BestMove = E->Move;
               BestValue = Value;
            }
         }

         E2 = E2->Succ;
         if (E2 == NULL) break;
         if (E2->Move == HashKiller) {
            E2 = E2->Succ;
            if (E2 == NULL) break;
         }
         E = E2;
      }

   } else if (Depth < FAST_SORT_DEPTH) { /* HASH_DEPTH < Depth < FAST_SORT_DEPTH */

      NewKey[0] = EBoard->Key[0];
      NewKey[1] = EBoard->Key[1];

      E2 = EBoard->Square;
      if (HashKiller != PASS) {
         E = &EBoard->Square[HashKiller];
         if (E->Disk != EMPTY) {
            E2 = E2->Succ;
            E  = E2;
         }
      } else {
         E2 = E2->Succ;
         E  = E2;
      }

      while (TRUE) {

         Flipped = EDoHashFlips(E);

         if (Flipped != 0) {

            E->Pred->Succ = E->Succ;
            if (E->Succ != NULL) E->Succ->Pred = E->Pred;

            Value = -SolveNWS(-Alpha-1,Depth-1,-DiskDiff-Flipped,NO_MOVE);

            E->Pred->Succ = E;
            if (E->Succ != NULL) E->Succ->Pred = E;

            EUndoFlips(E);
            EBoard->Key[0] = NewKey[0];
            EBoard->Key[1] = NewKey[1];

            if (Value > BestValue) {
               if (Value > Alpha) {
                  if (NewHash) {
                     Info.Killer = E->Move;
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = Value;
                     Entry->Lock = EBoard->Key[1];
                     Entry->Info = Info;
                  }
                  EBoard->FlipSP--;
                  return Value;
               }
               if (BestValue == -INF-1) BestMove = E->Move;
               BestValue = Value;
            }
         }

         E2 = E2->Succ;
         if (E2 == NULL) break;
         if (E2->Move == HashKiller) {
            E2 = E2->Succ;
            if (E2 == NULL) break;
         }
         E = E2;
      }

   } else { /* FAST_SORT_DEPTH <= Depth */

      NewKey[0] = EBoard->Key[0];
      NewKey[1] = EBoard->Key[1];

      if (HashKiller != PASS) {

         E = &EBoard->Square[HashKiller];

         if (E->Disk == EMPTY) {

            Flipped = EDoHashFlips(E);

            if (Flipped != 0) {

               E->Pred->Succ = E->Succ;
               if (E->Succ != NULL) E->Succ->Pred = E->Pred;

               BestValue = -SolveNWS(-Alpha-1,Depth-1,-DiskDiff-Flipped,NO_MOVE);

               E->Pred->Succ = E;
               if (E->Succ != NULL) E->Succ->Pred = E;

               EUndoFlips(E);
               EBoard->Key[0] = NewKey[0];
               EBoard->Key[1] = NewKey[1];

               if (BestValue > Alpha) {
                  if (NewHash) {
                     Info.Killer = E->Move;
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = BestValue;
                     Entry->Lock = EBoard->Key[1];
                     Entry->Info = Info;
                  }
                  EBoard->FlipSP--;
                  return BestValue;
               }

               BestMove = E->Move;
            }
         }
      }

      Move   = MoveList;
      Colour = -EBoard->Colour;

      for (E = EBoard->Square->Succ; E != NULL; E = E->Succ) {

         if (E->Move != HashKiller && EDoFlips(E) != 0) {

            E->Pred->Succ = E->Succ;

            Move->Move = E;
            Move->Note = 0;
            for (E2 = EBoard->Square->Succ; E2 != NULL; E2 = E2->Succ) {
               if (EIsLegal(E2,Colour)) Move->Note--;
            }
            Move++;

            E->Pred->Succ = E;

            EUndoFlips(E);
         }
      }

      Move->Move = NULL;
      SortMove(MoveList,Move-1);

      for (Move = MoveList; Move->Move != NULL; Move++) {

         E = Move->Move;

         Flipped = EDoHashFlips(E);

         E->Pred->Succ = E->Succ;
         if (E->Succ != NULL) E->Succ->Pred = E->Pred;

         Value = -SolveNWS(-Alpha-1,Depth-1,-DiskDiff-Flipped,NO_MOVE);

         E->Pred->Succ = E;
         if (E->Succ != NULL) E->Succ->Pred = E;

         EUndoFlips(E);
         EBoard->Key[0] = NewKey[0];
         EBoard->Key[1] = NewKey[1];

         if (Value > BestValue) {
            if (Value > Alpha) {
               if (NewHash) {
                  Info.Killer = E->Move;
                  Info.Min    = TRUE;
                  Info.Max    = FALSE;
                  Info.Value  = Value;
                  Entry->Lock = EBoard->Key[1];
                  Entry->Info = Info;
               }
               EBoard->FlipSP--;
               return Value;
            }
            if (BestValue == -INF-1) BestMove = E->Move;
            BestValue = Value;
         }
      }
   }

   EBoard->FlipSP--;

   if (BestValue == -INF-1) {
      if (PrevMove == PASS) {
         if (DiskDiff > 0)      DiskDiff += Depth;
         else if (DiskDiff < 0) DiskDiff -= Depth;
         BestValue = DIFFNOTE(DiskDiff);
         if (NewHash) {
            Info.Min    = TRUE;
            Info.Max    = TRUE;
            Info.Value  = BestValue;
            Entry->Lock = EBoard->Key[1];
            Entry->Info = Info;
         }
      } else {
         EDoPass();
         BestValue = -SolveNWS(-Alpha-1,Depth,-DiskDiff,PASS);
         EDoPass();
         if (NewHash) {
            Info.Min    = BestValue >  Alpha;
            Info.Max    = BestValue <= Alpha;
            Info.Value  = BestValue;
            Entry->Lock = EBoard->Key[1];
            Entry->Info = Info;
         }
      }
      return BestValue;
   }

   if (NewHash) {
      Info.Killer = BestMove;
      Info.Min    = Depth <= 2;
      Info.Max    = TRUE;
      Info.Value  = BestValue;
      Entry->Lock = EBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* End of EndGame.C */
