
/* Turtle.C */

#include <stdio.h>
#include <stdlib.h>

#include "turtle.h"
#include "types.h"
#include "book.h"
#include "eboard.h"
#include "endgame.h"
#include "eval.h"
#include "hash.h"
#include "loop.h"
#include "mboard.h"
#include "output.h"
#include "probcut.h"
#include "variable.h"
#include "xboard.h"

/* Variables */

int Mode; /* Playing, self-learning or endgame testing ? */

int ArgC;
char **ArgV;

/* Functions */

/* Main() */

int main(int argc, char *argv[]) {

   char *SettingsFile;

   printf("\n");
   printf(TURTLE_NAME " ! (Compiled on " __DATE__ " at " __TIME__ ")\n");
   printf("\n");

   SettingsFile = NULL;

   if (argc == 3 && SameString(argv[1],"-S")) {
      SettingsFile = argv[2];
   } else if (argc != 1) {
      Quit("Usage : %s [-s <settings_file>]\n",argv[0]);
   }

   ArgC = argc;
   ArgV = argv;

   printf("Init Signals  ... ");
   fflush(stdout);
   InitSignal();
   printf("Done.\n");

   printf("Load Settings ... ");
   fflush(stdout);
   InitVar(SettingsFile);
   printf("Done.\n");

   printf("Init Hash     ... ");
   fflush(stdout);
   InitHashTable();
   printf("Done.\n");

   printf("Load Eval     ... ");
   fflush(stdout);
   InitEval();
   printf("Done.\n");

   printf("Load ProbCut  ... ");
   fflush(stdout);
   InitProbCut();
   printf("Done.\n");

   OpenXBoard();

   MInitBoard();
   EInitBoard();

   printf("Init Book     ... ");
   fflush(stdout);
   InitBook();
   printf("Done.\n");

   switch (Mode) {
      case PLAY :
         Loop();
         break;
      case BOOK_LEARN :
         LearnBook();
         break;
      case END_TEST :
         EndTest();
         break;
      case PROB_CUT :
         ProbCutStat();
         break;
      case EVAL :
         EvalStat();
         break;
   }

   CloseXBoard();

   return EXIT_SUCCESS;
}

/* End of Turtle.C */

