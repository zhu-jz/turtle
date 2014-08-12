
/* MidGame.C */

#include <stdio.h>
#include <setjmp.h>

#include "midgame.h"
#include "board.h"
#include "clock.h"
#include "eval.h"
#include "hash.h"
#include "hash.i"
#include "loop.h"
#include "master.h"
#include "mboard.h"
#include "mboard.i"
#include "probcut.h"
#include "sort.h"

/* Constants */

#define SORT_DEPTH    3 /* 3 */

#define SEL_MIN_DEPTH 3 /* 3 */
#define SEL_MAX_DEPTH 9 /* 9 */

/* Variables */

int MgEventPeriod;

static INFO    NewInfo = { 0, 0, PASS, FALSE, FALSE, 0 };

static int     SelMinDepth, SelMaxDepth;

static jmp_buf Return;
static int     GlobalMoveNo;
static int     GlobalDepth;

/* Prototypes */

static int PVS (int PV[], int Alpha, int Beta, int Depth, int PrevMove);
static int NWS (int Alpha, int Depth, int PrevMove);

/* Functions */

/* InitSearch() */

void InitSearch(const BOARD *Board) {

   MCopyBoard(Board);
   GlobalMoveNo = 60 - EmptyNb(Board);
   SetEvalMoveNo(GlobalMoveNo);

   Date = (Date + 1) % DATE_NB;
   NewInfo.Date = Date;
}

/* ShortSearch() */

int ShortSearch(int Alpha, int Beta, int Depth, BEST_MOVE *Best) {

   int Event, I, J, MoveNo;
   int Move, BestValue, Value, Depth1;
   int NewKey[2], PV[PV_MOVE_NB+1];

   Event = setjmp(Return);
   if (Event != 0) {
      Best->Time = Duration(Search->Start,CurrentTime());
      return Event;
   }

   if (UseProbCut) {
      SelMinDepth = SEL_MIN_DEPTH;
      SelMaxDepth = SEL_MAX_DEPTH;
   } else {
      SelMinDepth = SORT_DEPTH;
      SelMaxDepth = SORT_DEPTH - 1;
   }

   SetEvalMoveNo(GlobalMoveNo+Depth);
   SetProbCutMoveNo(GlobalMoveNo+Depth);
   GlobalDepth = Depth;

   NewKey[0] = MBoard->Key[0];
   NewKey[1] = MBoard->Key[1];
   BestValue = -INF-1;

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
      Value = -PVS(PV+1,-Beta,-Alpha,Depth1,Move);

      if (Move != PASS) {
         MUndoHashFlips(&MBoard->Square[Move]);
         MBoard->Key[0] = NewKey[0];
         MBoard->Key[1] = NewKey[1];
      } else {
         MDoPass();
      }

      if (RootMove[MoveNo].Depth == Depth) {
         if (Value > Alpha && Value > RootMove[MoveNo].Min) {
            RootMove[MoveNo].Min = Value;
         }
         if (Value < Beta && Value < RootMove[MoveNo].Max) {
            RootMove[MoveNo].Max = Value;
         }
      } else {
         RootMove[MoveNo].Depth = Depth;
         RootMove[MoveNo].Min   = (Value > Alpha) ? Value : -INF;
         RootMove[MoveNo].Max   = (Value < Beta)  ? Value : +INF;
      }

      if (Value > BestValue) {

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

         if (Value >= Beta) break;

         BestValue = Value;
      }
   }

   MBoard->FlipSP--;

   Best->Time = Duration(Search->Start,CurrentTime());

   return DEPTH;
}

/* MGSearch() */

int MGSearch(int Alpha, int Beta, int Depth, BEST_MOVE *Best) {

   int Event, I, J, MoveNo;
   int Move, Value, Depth1;
   int NewKey[2], PV[PV_MOVE_NB+1];

   Event = setjmp(Return);
   if (Event != 0) {
      Best->Time = Duration(Search->Start,CurrentTime());
      return Event;
   }

   if (UseProbCut) {
      SelMinDepth = SEL_MIN_DEPTH;
      SelMaxDepth = SEL_MAX_DEPTH;
   } else {
      SelMinDepth = SORT_DEPTH;
      SelMaxDepth = SORT_DEPTH - 1;
   }

   SetEvalMoveNo(GlobalMoveNo+Depth);
   SetProbCutMoveNo(GlobalMoveNo+Depth);
   GlobalDepth = Depth;

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

 	 if (Depth1 == 0) {
            Value = -PVS(PV+1,-Alpha-1,-Alpha,Depth1,Move);
         } else {
            Value = -NWS(-Alpha-1,Depth1,Move);
         }

         if (Value > Alpha && Value < Beta && Depth1 > 1) {

	    if (RootMove[MoveNo].Depth == Depth) {
	       if (Value > RootMove[MoveNo].Min) RootMove[MoveNo].Min = Value;
	    } else {
	       RootMove[MoveNo].Depth = Depth;
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

      if (RootMove[MoveNo].Depth == Depth) {
         if (Value > Alpha && Value > RootMove[MoveNo].Min) {
            RootMove[MoveNo].Min = Value;
         }
         if (Value < Beta && Value < RootMove[MoveNo].Max) {
            RootMove[MoveNo].Max = Value;
         }
      } else {
         RootMove[MoveNo].Depth = Depth;
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
      }
   }

   MBoard->FlipSP--;

   Best->Time = Duration(Search->Start,CurrentTime());

   return DEPTH;
}

/* PVS() */

static int PVS(int PV[], int Alpha, int Beta, int Depth, int PrevMove) {

   int        Value, BestValue, Colour;
   int        NewKey[2], NewHash;
   MSQUARE   *HashKiller, **KillerList, **I, **BestMove, *Square;
   ENTRY     *Entry;
   INFO       Info;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   PvNodeNb++;

   Entry = ReadHash(MBoard->Key);

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
         if (Info.Depth == Depth) {
            NewHash   = TRUE;
            Info.Date = Date;
         } else {
            NewHash          = FALSE;
            Entry->Info.Date = Date;
         }
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      Entry = WriteHash(MBoard->Key,Depth);
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
      return Eval();
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

   if (Depth < 2) { /* Depth < SelMinDepth */

      Colour = MBoard->Colour;

      while (TRUE) {

         if (Square->Disk == EMPTY && MIsLegal(Square,Colour)) {
            Move->Move = I;
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

   } else { /* SelMinDepth <= Depth */

      ChangeEvalMoveNo(1-Depth);

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

      ChangeEvalMoveNo(Depth-1);

      Move->Move = NULL;
      SortMove(MoveList,Move-1);
   }

   for (Move = MoveList; Move->Move != NULL; Move++) {

      I = Move->Move;

      if (I < KillerList) {
         Square = HashKiller;
      } else {
         Square = *I;
      }

      if (Square->Disk == EMPTY && MDoHashFlips(Square)) {

         if (BestValue == -INF-1) {
            PV[0] = Square->Move;
            Value = -PVS(PV+1,-Beta,-Alpha,Depth-1,Square->Move);
         } else {
 	    if (Depth == 1) {
               Value = -PVS(PV+1,-Alpha-1,-Alpha,Depth-1,Square->Move);
            } else {
               Value = -NWS(-Alpha-1,Depth-1,Square->Move);
            }
            if (Value > Alpha) {
	       PV[0] = Square->Move;
	       PV[1] = NO_MOVE;
  	       if (Value < Beta && Depth > 2) {
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
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = Value;
                     Entry->Lock = MBoard->Key[1];
                     Entry->Info = Info;
                  }
                  MBoard->FlipSP--;
                  return Value;
               }
               Alpha = Value;
               BestMove = I;
               if (NewHash) {
                  Info.Min   = TRUE;
                  Info.Max   = FALSE;
                  Info.Value = Value;
               }
            }
            if (BestValue == -INF-1) BestMove = I;
            BestValue = Value;
         }
      }
   }

   MBoard->FlipSP--;

   if (BestValue == -INF-1) {
      PV[0] = PASS;
      if (PrevMove == PASS) {
         PV[1] = NO_MOVE;
         BestValue = DIFFNOTE(MDiskDiff());
         if (NewHash) {
            Info.Depth  = HASH_MAX_DEPTH;
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
            Info.Min    = BestValue > Alpha;
            Info.Max    = BestValue < Beta;
            Info.Value  = BestValue;
            Entry->Lock = MBoard->Key[1];
            Entry->Info = Info;
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
      if (Info.Value != BestValue) {
         Info.Min   = Depth <= 1;
         Info.Value = BestValue;
      }
      Info.Max    = TRUE;
      Entry->Lock = MBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* NWS() */

static int NWS(int Alpha, int Depth, int PrevMove) {

   int        Value, BestValue, Alpha1, Beta1, BestValue1;
   int        NewKey[2], NewHash, NewHash1;
   MSQUARE   *HashKiller, **KillerList, **I, **BestMove, *BestMove1, *Square;
   ENTRY     *Entry, *Entry1;
   INFO       Info, Info1;
   MOVE_NOTE  MoveList[LEGAL_MOVE_NB+1], *Move;

   NodeNb++;

   if ((NodeNb & (MgEventPeriod - 1)) == 0) {
      if (Event()) longjmp(Return,STOP);
      if (Search->TimeLimit && Duration(Search->Start,CurrentTime()) >= Search->MaxTime) {
         longjmp(Return,TIME_OUT);
      }
   }

   Entry = ReadHash(MBoard->Key);

   if (Entry != NULL) {
      if (Entry->Info.Depth >= Depth) {
         Info = Entry->Info;
         if (Info.Min && Info.Value >  Alpha) return Info.Value;
         if (Info.Max && Info.Value <= Alpha) return Info.Value;
         if (Entry->Info.Depth == Depth) {
            NewHash   = TRUE;
            Info.Date = Date;
         } else {
            NewHash          = FALSE;
            Entry->Info.Date = Date;
         }
      } else {
         NewHash     = TRUE;
         Info        = NewInfo;
         Info.Depth  = Depth;
         Info.Killer = Entry->Info.Killer;
      }
   } else {
      Entry = WriteHash(MBoard->Key,Depth);
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

   if (Depth == 1) {

      while (TRUE) {

         if (Square->Disk == EMPTY && MDoIndices(Square)) {

            EvalNb++;
            Value = -Eval();

            MUndoIndices(Square);

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
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = Value;
                     Entry->Lock = MBoard->Key[1];
                     Entry->Info = Info;
                  }
                  MBoard->FlipSP--;
                  return Value;
               }
               BestValue = Value;
               BestMove  = I;
            }
         }

         Square = *++I;
         if (Square == NULL) break;
         if (Square == HashKiller) {
            Square = *++I;
            if (Square == NULL) break;
         }
      }

   } else if (Depth < SelMinDepth) { /* 1 < Depth < SelMinDepth */

      NewKey[0] = MBoard->Key[0];
      NewKey[1] = MBoard->Key[1];

      while (TRUE) {

         if (Square->Disk == EMPTY && MDoHashFlips(Square)) {

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
                     Info.Min    = TRUE;
                     Info.Max    = FALSE;
                     Info.Value  = Value;
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

         Square = *++I;
         if (Square == NULL) break;
         if (Square == HashKiller) {
            Square = *++I;
            if (Square == NULL) break;
         }
      }

   } else if (Depth <= SelMaxDepth) { /* SelMinDepth <= Depth <= SelMaxDepth */

      Alpha1 = Alpha;
      Beta1  = Alpha + 1;
      ProbCutBounds(Depth,&Alpha1,&Beta1);

      Entry1 = ReadHash(MBoard->Key);

      if (Entry1 != NULL) {
	 if (Entry1->Info.Depth >= 1) {
	    Info1 = Entry1->Info;
	    if (Entry1->Info.Depth == 1) {
	       if (Info1.Min && Info1.Value >= Beta1) {
		  MBoard->FlipSP--;
		  return Alpha + 1;
	       }
	       if (Info1.Max && Info1.Value <= Alpha1) {
		  MBoard->FlipSP--;
		  return Alpha;
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

      Move = MoveList;

      ChangeEvalMoveNo(1-Depth);

      while (TRUE) {

         if (Square->Disk == EMPTY && MDoIndices(Square)) {

            EvalNb++;
            Value = -Eval();

            MUndoIndices(Square);

            if (Value > BestValue1) {
               if (Value >= Beta1) {
                  if (NewHash1) {
                     Info1.Killer = Square->Move;
                     Info1.Min    = TRUE;
                     Info1.Max    = FALSE;
                     Info1.Value  = Value;
                     Entry1->Lock = MBoard->Key[1];
                     Entry1->Info = Info1;
                  }
                  ChangeEvalMoveNo(Depth-1);
                  MBoard->FlipSP--;
                  return Alpha + 1;
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

      ChangeEvalMoveNo(Depth-1);

      if (BestValue1 != -INF-1) {
         if (NewHash1) {
            Info1.Killer = BestMove1->Move;
            Info1.Min    = TRUE;
            Info1.Max    = TRUE;
            Info1.Value  = BestValue1; /* Depth = 1 => exact value */
            Entry1->Lock = MBoard->Key[1];
            Entry1->Info = Info1;
         }
         if (BestValue1 <= Alpha1) {
            MBoard->FlipSP--;
            return Alpha;
         }
      }

      Move->Move = NULL;
      SortMove(MoveList,Move-1);

      NewKey[0] = MBoard->Key[0];
      NewKey[1] = MBoard->Key[1];

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
                  Info.Min    = TRUE;
                  Info.Max    = FALSE;
                  Info.Value  = Value;
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

   } else { /* Depth > SelMaxDepth */

      NewKey[0] = MBoard->Key[0];
      NewKey[1] = MBoard->Key[1];

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
                  Info.Min    = TRUE;
                  Info.Max    = FALSE;
                  Info.Value  = BestValue;
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

      Move = MoveList;

      ChangeEvalMoveNo(1-Depth);

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

      ChangeEvalMoveNo(Depth-1);

      Move->Move = NULL;
      SortMove(MoveList,Move-1);

      for (Move = MoveList; Move->Move != NULL; Move++) {

         I = Move->Move;
         Square = *I;

         MDoHashFlips(Square);

         Value = -NWS(-Alpha-1,Depth-1,Square->Move);

         MUndoHashFlips(Square);
         MBoard->Key[0] = NewKey[0];
         MBoard->Key[1] = NewKey[1];

         if (Value > BestValue) {
            if (Value > Alpha) {
               if (PrevMove != PASS) {
                  if (I > KillerList) {
                     *I    = I[-1];
                     I[-1] = Square;
                  }
               }
               if (NewHash) {
                  Info.Killer = Square->Move;
                  Info.Min    = TRUE;
                  Info.Max    = FALSE;
                  Info.Value  = Value;
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
   }

   MBoard->FlipSP--;

   if (BestValue == -INF-1) {
      if (PrevMove == PASS) {
         BestValue = DIFFNOTE(MDiskDiff());
         if (NewHash) {
            Info.Depth  = HASH_MAX_DEPTH;
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
            Info.Min    = BestValue >  Alpha;
            Info.Max    = BestValue <= Alpha;
            Info.Value  = BestValue;
            Entry->Lock = MBoard->Key[1];
            Entry->Info = Info;
         }
      }
      return BestValue;
   }

   if (BestMove < KillerList) {
      Square = HashKiller;
   } else {
      Square = *BestMove;
   }

   if ((Depth == 1 || Depth >= SelMinDepth) && PrevMove != PASS) {
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
      Info.Min    = Depth <= 1;
      Info.Max    = TRUE;
      Info.Value  = BestValue;
      Entry->Lock = MBoard->Key[1];
      Entry->Info = Info;
   }

   return BestValue;
}

/* End of MidGame.C */

