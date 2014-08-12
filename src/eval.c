
/* Eval.C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "eval.h"
#include "types.h"
#include "board.h"
#include "mboard.h"
#include "output.h"

/* Constants */

#define EVAL_DIR     "data/eval/"

#define MAX_SQUARE_NB 10

#define FEATURE_NB    ((int)(sizeof(Feature)/sizeof(FEATURE)))

#define FIRST_MOVE    12
#define LAST_MOVE     52

#define QUANTUM       0.01296796293528 /* Max value / 127.5 */

/* Types */

typedef struct {
   const char *Name;
   int         SquareNb;
} FEATURE;

typedef struct {
   int Size;
   int Middle;
} TAB_INFO;

typedef struct {
   int Corner1;
   int Corner2;
} EDGE_INFO;

typedef struct {
   int   XSquares;
   short Corner1;
   short Corner2;
} HOR2_INFO;

/* "Constants" */

static const FEATURE Feature[] = {

   { "corner5x2", 10 },

   { "edge2x",    10 },
   { "hor2",       8 },
   { "hor3",       8 },
   { "hor4",       8 },

   { "triangle",  10 },
   { "diag5",      5 },
   { "diag6",      6 },
   { "diag7",      7 },
   { "diag8",      8 }
};

static const TAB_INFO TabInfo[MAX_SQUARE_NB+1] = {

   {     1,     0 },
   {     3,     1 },
   {     9,     4 },
   {    27,    13 },
   {    81,    40 },
   {   243,   121 },
   {   729,   364 },
   {  2187,  1093 },
   {  6561,  3280 },
   { 19683,  9841 },
   { 59049, 29524 }
};

/* Variables */

int   DiffNote[2*64+1];
schar UnDiffNote[2*INF+1];

static int        FeaturePos[FEATURE_NB];

static schar     *ValList[61][FEATURE_NB];
static int        Turn[61], Parity[61];

static EDGE_INFO  EdgeInfo[6561];
static HOR2_INFO  Hor2Info[6561];

static int        EvalMoveNo; /* No of move in leaves (60 - Empties) */
static schar     *EvalPtr;
static int        TurnVal;

/* Prototypes */

static void InitDiffNote   (void);
static void InitUnDiffNote (void);
static void InitFeatures   (void);
static void InitInfos      (void);

/* Functions */

/* InitEval() */

void InitEval(void) {

   InitDiffNote();
   InitUnDiffNote();
   InitFeatures();
   InitInfos();
}

/* InitDiffNote() */

static void InitDiffNote(void) {

   int DiscDiff;

   DIFFNOTE(0) = 0;
   for (DiscDiff = 1; DiscDiff <= 64; DiscDiff++) {
      DIFFNOTE(+DiscDiff) = +MID_INF + (DiscDiff + 1) / 2;
      DIFFNOTE(-DiscDiff) = -DIFFNOTE(+DiscDiff);
   }
}

/* InitUnDiffNote() */

static void InitUnDiffNote(void) {

   int Value;

   UNDIFFNOTE(0) = (schar) 0;
   for (Value = 1; Value <= MID_INF; Value++) {
      UNDIFFNOTE(+Value) = (schar) 0;
      UNDIFFNOTE(-Value) = -UNDIFFNOTE(+Value);
   }
   for (Value = MID_INF+1; Value <= INF; Value++) {
      UNDIFFNOTE(+Value) = (schar) (+2 * (Value - MID_INF));
      UNDIFFNOTE(-Value) = -UNDIFFNOTE(+Value);
   }
}

/* InitFeatures() */

static void InitFeatures(void) {

   int    I, J, Middle, MoveNo, FeatureSize;
   char   FileName[256];
   FILE  *File;
   schar  SChar, *Value;

   FeatureSize = 0;
   for (I = 0; I < FEATURE_NB; I++) {
      FeaturePos[I] = FeatureSize + TabInfo[Feature[I].SquareNb].Middle;
      FeatureSize += TabInfo[Feature[I].SquareNb].Size;
   }

   Value = malloc((size_t)((LAST_MOVE-FIRST_MOVE+1)*FeatureSize));
   if (Value == NULL) FatalError("Not enough memory for evaluation function");

   printf("[ %d Bytes : %d * %d ] ",(int)(LAST_MOVE-FIRST_MOVE+1)*FeatureSize,(int)(LAST_MOVE-FIRST_MOVE+1),FeatureSize);
   fflush(stdout);

   for (I = 0; I < FEATURE_NB; I++) {

      Middle = TabInfo[Feature[I].SquareNb].Middle;
      sprintf(FileName,EVAL_DIR "%s",Feature[I].Name);

      if ((File = fopen(FileName,"rb")) != NULL) {
         for (MoveNo = FIRST_MOVE; MoveNo <= LAST_MOVE; MoveNo++) {
            ValList[MoveNo][I] = &Value[(MoveNo-FIRST_MOVE)*FeatureSize+FeaturePos[I]];
            fread(ValList[MoveNo][I],(size_t)1,(size_t)(Middle+1),File);
            for (J = 1; J <= Middle; J++) {
               ValList[MoveNo][I][-J] = -ValList[MoveNo][I][+J];
            }
         }
         for (MoveNo = 0; MoveNo < FIRST_MOVE; MoveNo++) {
            ValList[MoveNo][I] = ValList[FIRST_MOVE][I];
         }
         for (MoveNo = LAST_MOVE+1; MoveNo <= 60; MoveNo++) {
            ValList[MoveNo][I] = ValList[LAST_MOVE][I];
         }
         fclose(File);
      } else {
         Error("Couldn't open \"%s\" pattern file",FileName);
      }
   }

   if ((File = fopen(EVAL_DIR "turn","rb")) != NULL) {
      for (MoveNo = FIRST_MOVE; MoveNo <= LAST_MOVE; MoveNo++) {
         fread(&SChar,(size_t)1,(size_t)1,File);
         Turn[MoveNo] = SChar;
      }
      fclose(File);
      for (MoveNo = 0; MoveNo < FIRST_MOVE; MoveNo++) {
         Turn[MoveNo] = Turn[FIRST_MOVE];
      }
      for (MoveNo = LAST_MOVE+1; MoveNo <= 60; MoveNo++) {
         Turn[MoveNo] = Turn[LAST_MOVE];
      }
   } else {
      Error("Couldn't open \"%s\" turn file",EVAL_DIR "turn");
   }

   if ((File = fopen(EVAL_DIR "parity","rb")) != NULL) {
      for (MoveNo = FIRST_MOVE; MoveNo <= LAST_MOVE; MoveNo++) {
         fread(&SChar,(size_t)1,(size_t)1,File);
         Parity[MoveNo] = SChar;
      }
      fclose(File);
      for (MoveNo = 0; MoveNo < FIRST_MOVE; MoveNo++) {
         Parity[MoveNo] = Parity[FIRST_MOVE];
      }
      for (MoveNo = LAST_MOVE+1; MoveNo <= 60; MoveNo++) {
         Parity[MoveNo] = Parity[LAST_MOVE];
      }
   } else {
/*
      Error("Couldn't open \"%s\" parity file",EVAL_DIR "parity");
*/
      for (MoveNo = 0; MoveNo <= 60; MoveNo++) {
         Parity[MoveNo] = 0;
      }
   }
}

/* InitInfos() */

static void InitInfos(void) {

   int Index, I, J, Tab[8];

   for (Index = 0; Index < 6561; Index++) {

      for (I = 7, J = Index; I >= 0; I--, J /= 3) Tab[I] = J % 3 - 1;

      EdgeInfo[Index].Corner1  = ((((Tab[0] * 3 + Tab[1]) * 3 + Tab[2]) * 3 + Tab[3]) * 243 + Tab[4]) * 3;
      EdgeInfo[Index].Corner2  = ((((Tab[7] * 3 + Tab[6]) * 3 + Tab[5]) * 3 + Tab[4]) * 243 + Tab[3]) * 3;
      Hor2Info[Index].Corner1  =  (((Tab[0] * 3 + Tab[1]) * 3 + Tab[2]) * 3 + Tab[3]) *   9 + Tab[4];
      Hor2Info[Index].Corner2  =  (((Tab[7] * 3 + Tab[6]) * 3 + Tab[5]) * 3 + Tab[4]) *   9 + Tab[3];
      Hor2Info[Index].XSquares =    (Tab[1] * 3 + Tab[6]) * 6561;

      EdgeInfo[Index].Corner1  += FeaturePos[0] - FeaturePos[2];
      EdgeInfo[Index].Corner2  += FeaturePos[0] - FeaturePos[2];
      Hor2Info[Index].XSquares += FeaturePos[1] - FeaturePos[2];
   }
}

/* InitIndices() */

void InitIndices(void) {

   MBoard->Index[ 8] += FeaturePos[3] - FeaturePos[2];
   MBoard->Index[ 9] += FeaturePos[3] - FeaturePos[2];
   MBoard->Index[10] += FeaturePos[3] - FeaturePos[2];
   MBoard->Index[11] += FeaturePos[3] - FeaturePos[2];

   MBoard->Index[12] += FeaturePos[4] - FeaturePos[2];
   MBoard->Index[13] += FeaturePos[4] - FeaturePos[2];
   MBoard->Index[14] += FeaturePos[4] - FeaturePos[2];
   MBoard->Index[15] += FeaturePos[4] - FeaturePos[2];

   MBoard->Index[16] += FeaturePos[5] - FeaturePos[2];
   MBoard->Index[17] += FeaturePos[5] - FeaturePos[2];
   MBoard->Index[18] += FeaturePos[5] - FeaturePos[2];
   MBoard->Index[19] += FeaturePos[5] - FeaturePos[2];

   MBoard->Index[20] += FeaturePos[6] - FeaturePos[2];
   MBoard->Index[21] += FeaturePos[6] - FeaturePos[2];
   MBoard->Index[22] += FeaturePos[6] - FeaturePos[2];
   MBoard->Index[23] += FeaturePos[6] - FeaturePos[2];

   MBoard->Index[24] += FeaturePos[7] - FeaturePos[2];
   MBoard->Index[25] += FeaturePos[7] - FeaturePos[2];
   MBoard->Index[26] += FeaturePos[7] - FeaturePos[2];
   MBoard->Index[27] += FeaturePos[7] - FeaturePos[2];

   MBoard->Index[28] += FeaturePos[8] - FeaturePos[2];
   MBoard->Index[29] += FeaturePos[8] - FeaturePos[2];
   MBoard->Index[30] += FeaturePos[8] - FeaturePos[2];
   MBoard->Index[31] += FeaturePos[8] - FeaturePos[2];

   MBoard->Index[32] += FeaturePos[9] - FeaturePos[2];
   MBoard->Index[33] += FeaturePos[9] - FeaturePos[2];
}

/* SetEvalMoveNo() */

void SetEvalMoveNo(int MoveNo) {

   EvalMoveNo = MoveNo;

   EvalPtr = ValList[EvalMoveNo][0] - FeaturePos[0] + FeaturePos[2];

   if (EvalMoveNo % 2 != 0) {
      TurnVal = Turn[EvalMoveNo] + Parity[EvalMoveNo];
   } else {
      TurnVal = Turn[EvalMoveNo] - Parity[EvalMoveNo];
   }
}

/* ChangeEvalMoveNo() */

void ChangeEvalMoveNo(int Delta) {

   EvalMoveNo += Delta;

   EvalPtr = ValList[EvalMoveNo][0] - FeaturePos[0] + FeaturePos[2];

   if (EvalMoveNo % 2 != 0) {
      TurnVal = Turn[EvalMoveNo] + Parity[EvalMoveNo];
   } else {
      TurnVal = Turn[EvalMoveNo] - Parity[EvalMoveNo];
   }
}

/* Eval() */

int Eval(void) {

   int        Eval, Index1, Index2;
   schar     *ValPtr;
   EDGE_INFO *EdgePtr1, *EdgePtr2;
   HOR2_INFO *Hor2Ptr1, *Hor2Ptr2;

   ValPtr = EvalPtr;

   /* Corner5x2, Edge+2X & Hor2 */

   Hor2Ptr1 = &Hor2Info[3280];
   EdgePtr1 = &EdgeInfo[3280];

   Index1 = MBoard->Index[1];
   Eval = ValPtr[Index1];                               /*   Hor2    */
   Hor2Ptr2 = &Hor2Ptr1[Index1];
   Index2 = MBoard->Index[0];
   EdgePtr2 = &EdgePtr1[Index2];
   Index2 += Hor2Ptr2->XSquares;
   Eval += ValPtr[Index2];                              /*  Edge+2X  */
   Eval += ValPtr[Hor2Ptr2->Corner1+EdgePtr2->Corner1]; /* Corner5x2 */
   Eval += ValPtr[Hor2Ptr2->Corner2+EdgePtr2->Corner2]; /* Corner5x2 */

   Index1 = MBoard->Index[3];
   Eval += ValPtr[Index1];                              /*   Hor2    */
   Hor2Ptr2 = &Hor2Ptr1[Index1];
   Index2 = MBoard->Index[2];
   EdgePtr2 = &EdgePtr1[Index2];
   Index2 += Hor2Ptr2->XSquares;
   Eval += ValPtr[Index2];                              /*  Edge+2X  */
   Eval += ValPtr[Hor2Ptr2->Corner1+EdgePtr2->Corner1]; /* Corner5x2 */
   Eval += ValPtr[Hor2Ptr2->Corner2+EdgePtr2->Corner2]; /* Corner5x2 */

   Index1 = MBoard->Index[5];
   Eval += ValPtr[Index1];                              /*   Hor2    */
   Hor2Ptr2 = &Hor2Ptr1[Index1];
   Index2 = MBoard->Index[4];
   EdgePtr2 = &EdgePtr1[Index2];
   Index2 += Hor2Ptr2->XSquares;
   Eval += ValPtr[Index2];                              /*  Edge+2X  */
   Eval += ValPtr[Hor2Ptr2->Corner1+EdgePtr2->Corner1]; /* Corner5x2 */
   Eval += ValPtr[Hor2Ptr2->Corner2+EdgePtr2->Corner2]; /* Corner5x2 */

   Index1 = MBoard->Index[7];
   Eval += ValPtr[Index1];                              /*   Hor2    */
   Hor2Ptr2 = &Hor2Ptr1[Index1];
   Index2 = MBoard->Index[6];
   EdgePtr2 = &EdgePtr1[Index2];
   Index2 += Hor2Ptr2->XSquares;
   Eval += ValPtr[Index2];                              /*  Edge+2X  */
   Eval += ValPtr[Hor2Ptr2->Corner1+EdgePtr2->Corner1]; /* Corner5x2 */
   Eval += ValPtr[Hor2Ptr2->Corner2+EdgePtr2->Corner2]; /* Corner5x2 */

   /* Hor3 */

   Eval += ValPtr[MBoard->Index[ 8]];
   Eval += ValPtr[MBoard->Index[ 9]];
   Eval += ValPtr[MBoard->Index[10]];
   Eval += ValPtr[MBoard->Index[11]];

   /* Hor4 */

   Eval += ValPtr[MBoard->Index[12]];
   Eval += ValPtr[MBoard->Index[13]];
   Eval += ValPtr[MBoard->Index[14]];
   Eval += ValPtr[MBoard->Index[15]];

   /* Triangle */

   Eval += ValPtr[MBoard->Index[16]];
   Eval += ValPtr[MBoard->Index[17]];
   Eval += ValPtr[MBoard->Index[18]];
   Eval += ValPtr[MBoard->Index[19]];

   /* Diag5 */

   Eval += ValPtr[MBoard->Index[20]];
   Eval += ValPtr[MBoard->Index[21]];
   Eval += ValPtr[MBoard->Index[22]];
   Eval += ValPtr[MBoard->Index[23]];

   /* Diag6 */

   Eval += ValPtr[MBoard->Index[24]];
   Eval += ValPtr[MBoard->Index[25]];
   Eval += ValPtr[MBoard->Index[26]];
   Eval += ValPtr[MBoard->Index[27]];

   /* Diag7 */

   Eval += ValPtr[MBoard->Index[28]];
   Eval += ValPtr[MBoard->Index[29]];
   Eval += ValPtr[MBoard->Index[30]];
   Eval += ValPtr[MBoard->Index[31]];

   /* Diag8 */

   Eval += ValPtr[MBoard->Index[32]];
   Eval += ValPtr[MBoard->Index[33]];

   if (MBoard->Colour < 0) Eval = -Eval;

   Eval += TurnVal;

   return Eval;
}

/* DoubleEval() */

double DoubleEval(int Value) {

   return 1.0 / (1.0 + exp(-Value*QUANTUM));
}

/* BoundedEval() */

double BoundedEval(int Value) {

   double Eval;

   Eval = DoubleEval(Value);
   if (Eval < 0.02) Eval = 0.02;
   if (Eval > 0.98) Eval = 0.98;

   return Eval;
}

/* End of Eval.C */

