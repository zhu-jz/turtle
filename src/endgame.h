
/* EndGame.H */

#ifndef ENDGAME_H
#define ENDGAME_H

#include "board.h"
#include "master.h"

/* Variables */

extern int EgEventPeriod; /* Number of nodes between two event checks */

/* Prototypes */

extern void EndTest   (void);

extern void InitSolve (const BOARD *Board);
extern int  SelSolve  (int Alpha, int Beta, double Confidence, BEST_MOVE *Best);
extern int  Solve     (int Alpha, int Beta, BEST_MOVE *Best);

#endif /* ! defined ENDGAME_H */

/* End of EndGame.H */

