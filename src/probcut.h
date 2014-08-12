
/* ProbCut.H */

#ifndef PROBCUT_H
#define PROBCUT_H

/* Variables */

extern int    UseProbCut;   /* Use ProbCut selective search ? */
extern double ProbCutLevel; /* ProbCut confidence level */

/* Prototypes */

extern void InitProbCut      (void);
extern void SetProbCutMoveNo (int MoveNo);
extern void ProbCutBounds    (int Depth, int *Alpha, int *Beta);

#endif /* ! defined PROBCUT_H */

/* End of ProbCut.H */

