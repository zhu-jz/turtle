
/* Sort.C */

#include <stdio.h>

#include "sort.h"
#include "board.h"
#include "eval.h"
#include "mboard.h"

/* Variables */

int        RootMoveNb;
ROOT_MOVE  RootMove[LEGAL_MOVE_NB+1];

MSQUARE   *KillerLists[2][SQUARE_NB][64]; /* EMPTY_NB+2 */

/* Functions */

/* InitRootList() */

void InitRootList(const BOARD *Board, const int Forbidden[]) {

   const int *I;

   RootMoveNb = 0;

   for (I = BestToWorst; I < &BestToWorst[EMPTY_NB]; I++) {
      if (IsLegalMove(Board,*I) && (Forbidden == NULL || ! Forbidden[*I])) {
         RootMove[RootMoveNb].Move  = *I;
         RootMove[RootMoveNb].Depth = 0;
         RootMove[RootMoveNb].Min   = -INF;
         RootMove[RootMoveNb].Max   = +INF;
         RootMoveNb++;
      }
   }

   if (RootMoveNb == 0) {
      RootMove[RootMoveNb].Depth = 0;
      RootMove[RootMoveNb].Min   = -INF;
      RootMove[RootMoveNb].Max   = +INF;
      if (IsLegalMove(Board,PASS) && (Forbidden == NULL || ! Forbidden[PASS])) {
         RootMove[RootMoveNb].Move = PASS;
         RootMoveNb++;
      } else {
         RootMove[RootMoveNb].Move = NO_MOVE;
      }
   }
}

/* SortRootList() */

void SortRootList(void) {

   ROOT_MOVE *I, *J, K;

   I = &RootMove[RootMoveNb-2];
   if (I < RootMove) return;

   (I+2)->Depth = -1;

   do {
      K = *I;
      for (J = I; (J+1)->Depth > K.Depth || ((J+1)->Depth == K.Depth && (J+1)->Min + (J+1)->Max > K.Min + K.Max); J++) {
         *J = *(J+1);
      }
      *J = K;
   } while (--I >= RootMove);
}

/* MoveToFront() */

void MoveToFront(int MoveNo) {

   ROOT_MOVE Move;

   Move = RootMove[MoveNo];
   for (; MoveNo > 0; MoveNo--) RootMove[MoveNo] = RootMove[MoveNo-1];
   RootMove[0] = Move;
}

/* InitKillerLists() */

void InitKillerLists(const BOARD *Board) {

   int C, I, J;
   const int *K;

   for (C = 0; C < 2; C++) {
      for (I = 0; I < SQUARE_NB; I++) {
         J = 0;
         KillerLists[C][I][J++] = NULL;
         for (K = BestToWorst; K < &BestToWorst[EMPTY_NB]; K++) {
            if (*K != I && Board->Square[*K] == EMPTY) {
               KillerLists[C][I][J++] = &MBoard->Square[*K];
            }
         }
         KillerLists[C][I][J] = NULL;
      }
   }
}

/* SortMove() */

void SortMove(MOVE_NOTE *First, MOVE_NOTE *Last) {

   MOVE_NOTE *J, K;

   if (First >= Last) return;

   (Last+1)->Note = -9999;

   Last--;
   do {
      K = *Last;
      for (J = Last; (J+1)->Note > K.Note; J++) *J = *(J+1);
      *J = K;
   } while (--Last >= First);
}

/* End of Sort.C */

