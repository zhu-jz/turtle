
/* Eval.H */

#ifndef EVAL_H
#define EVAL_H

#include "types.h"

/* Constants */

#define MID_INF 8158         /* Midgame eval is in [-MID_INF,+MID_INF] */
#define INF     (MID_INF+32) /* Endgame eval is is     [-INF,+INF]     */

/* Macros */

#define DIFFNOTE(X)   (DiffNote[64+(X)])
#define UNDIFFNOTE(X) (UnDiffNote[INF+(X)])

/* Variables */

extern int   DiffNote[2*64+1];    /*  [-64,+64]  -> [-INF,+INF] */
extern schar UnDiffNote[2*INF+1]; /* [-INF,+INF] ->  [-64,+64]  */

/* Prototypes */

extern void   InitEval         (void);
extern void   InitIndices      (void);
extern void   SetEvalMoveNo    (int MoveNo);
extern void   ChangeEvalMoveNo (int Delta);
extern int    Eval             (void);
extern double DoubleEval       (int Value);
extern double BoundedEval      (int Value);

#endif /* ! defined EVAL_H */

/* End of Eval.H */

