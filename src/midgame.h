
/* MidGame.H */

#ifndef MIDGAME_H
#define MIDGAME_H

#include "board.h"
#include "master.h"

/* Variables */

extern int MgEventPeriod; /* Number of nodes between two event checks */

/* Prototypes */

extern void InitSearch   (const BOARD *Board);
extern int  ShortSearch  (int Alpha, int Beta, int Depth, BEST_MOVE *Best);
extern int  MGSearch     (int Alpha, int Beta, int Depth, BEST_MOVE *Best);

#endif /* ! defined MIDGAME_H */

/* End of MidGame.H */

