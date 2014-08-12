
/* ProbCut.C */

#include <stdio.h>
#include <math.h>

#include "probcut.h"
#include "types.h"
#include "eval.h"
#include "output.h"

/* Constants */

#define PROBCUT_DIR "data/probcut/"

#define MIN_DEPTH   3
#define MAX_DEPTH   9

/* Types */

typedef struct {
   double A;
   double BLow;
   double BHigh;
   double Dummy;
} PC_COEF;

/* Variables */

int    UseProbCut;
double ProbCutLevel;

static PC_COEF ProbCutCoef[61][MAX_DEPTH-MIN_DEPTH+1], *StaticCoef;

/* Prototypes */

static void LoadProbCut (const char *ProbCutFileName);

/* Functions */

/* InitProbCut() */

void InitProbCut(void) {

   if (UseProbCut) {

      LoadProbCut("midgame");

      printf("[ \"%s\" ] ","midgame");
      fflush(stdout);

   } else {

      printf("[ No probcut ] ");
      fflush(stdout);
   }
}

/* LoadProbCut() */

static void LoadProbCut(const char *ProbCutFileName) {

   char FileName[256];
   FILE *File;
   int MoveNo, Depth;
   PC_COEF *Coef;
   double A, B, S;

   for (MoveNo = 0; MoveNo <= 60; MoveNo++) {
      for (Depth = MIN_DEPTH; Depth <= MAX_DEPTH; Depth++) {
         Coef = &ProbCutCoef[MoveNo][Depth-MIN_DEPTH];
         Coef->A     = 0.0;
         Coef->BLow  = (double) -INF;
         Coef->BHigh = (double) +INF;
      }
   }

   sprintf(FileName,PROBCUT_DIR "%s",ProbCutFileName);
   File = fopen(FileName,"r");
   if (File == NULL) {
      Error("Couldn't open probcut file \"%s\" for reading",FileName);
      UseProbCut = FALSE;
      return;
   }

   while (fscanf(File,"%*d %d %d %lf %lf %lf %*f",&Depth,&MoveNo,&A,&B,&S) != EOF) {
      Coef = &ProbCutCoef[MoveNo+Depth][Depth-MIN_DEPTH];
      Coef->A     = 1.0 / A;
      Coef->BLow  = (-ProbCutLevel * S - B) / A + 0.5;
      Coef->BHigh = (+ProbCutLevel * S - B) / A + 0.5;
   }

   fclose(File);

   for (MoveNo = 0; MoveNo < 12; MoveNo++) {
      for (Depth = MIN_DEPTH; Depth <= MAX_DEPTH; Depth++) {
         ProbCutCoef[MoveNo+Depth][Depth-MIN_DEPTH] = ProbCutCoef[12+Depth][Depth-MIN_DEPTH];
      }
   }
}

/* SetProbCutMoveNo() */

void SetProbCutMoveNo(int MoveNo) {

   StaticCoef = &ProbCutCoef[MoveNo][-MIN_DEPTH];
}

/* ProbCutBounds() */

void ProbCutBounds(int Depth, int *Alpha, int *Beta) {

   PC_COEF *Coef;

   Coef = &StaticCoef[Depth];

   *Alpha = (int) floor(Coef->A*(double)*Alpha+Coef->BLow);
   *Beta  = (int) floor(Coef->A*(double)*Beta+Coef->BHigh);
}

/* End of ProbCut.C */

