
/* Master.H */

#ifndef MASTER_H
#define MASTER_H

#include "board.h"
#include "clock.h"
#include "mboard.h"

/* Constants */

#define MIDGAME_DEPTH   64
#define SELECTIVE_DEPTH 128
#define SOLVE_DEPTH     255

#define PV_MOVE_NB      96

enum { ALONE, BOOK, UPDATE, DEPTH, EARLY, TIME_OUT, STOP, BEST, SOLVED };
enum { WLD, LD, DW };
enum { FULL, WINDOW, NEGA_C, PESSIMISM };

/* Types */

typedef struct {
   int    Move;
   int    Depth;
   int    Min;
   int    Max;
   int    PV[PV_MOVE_NB+1];
   double Time;
   double IosValue;
   int    BookValue;
   char   StringValue[8+1];
   int    CurrDepth;
   char   CurrType[3];
   int    CurrMove;
   int    CurrMoveNo;
   int    Event;
   int    Modified;
} BEST_MOVE;

typedef struct {
   BOARD      Board[1];
   int        Forbidden[SQUARE_NB];
   BEST_MOVE *Best;
   int        ForceAlone;
   int        Book;
   int        DepthLimit;
   int        MaxDepth;
   int        TimeLimit;
   double     MaxTime;
   double     MidEarly;
   int        Sel;
   int        SelDepth;
   double     SelEarly;
   int        Wld;
   int        WldDepth;
   int        WldAlgo;
   double     WldEarly;
   int        Full;
   int        FullDepth;
   int        FullAlgo;
   double     FullEarly;
   TIME       Start;
   double     Early;
} SEARCH;

/* Variables */

extern int WldSolve;  /* Algorithm to use for WLD solving */
extern int FullSolve; /* Algorithm to use for full solving */

extern SEARCH Search[1];

extern int    PvNodeNb, NodeNb, EvalNb;

/* Prototypes */

extern void InitThinkAS   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void InitThinkBK   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void InitThinkMT   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void InitThinkOB   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void InitThinkOP   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void InitThinkTS   (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void UpdateThinkAS (void);

extern void Think         (void);
extern void ThinkBook     (void);

extern void ProbCutStat   (void);
extern void EvalStat      (void);

extern void DisplayInfos  (BEST_MOVE *Best);
extern void UpdateInfos   (BEST_MOVE *Best);

extern int  Round         (double X);

#endif /* ! defined MASTER_H */

/* End of Master.H */

