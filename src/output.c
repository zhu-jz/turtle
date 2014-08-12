
/* Output.C */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "output.h"
#include "types.h"
#include "board.h"
#include "game.h"
#include "loop.h"
#include "xboard.h"

/* Constants */

#define LOG_PATH    "log/"
#define STRING_SIZE 256

#define QUIT_LOG    "quit"
#define ERROR_LOG   "error"

/* "Constants" */

static const int Signal[] = {
   SIGABRT, SIGFPE, SIGILL, SIGINT, SIGSEGV, SIGTERM
};

/* Variables */

int SigIntQuit;
int ClockWise;

static const char **SignalName;

static char Line[STRING_SIZE];
static int  LineSize = 0;

/* Prototypes */ 

static void SigHandler(int Sig);

/* Functions */

/* InitSignal() */

void InitSignal(void) {

   int I, SignalNb;

   SignalNb = 0;
   for (I = 0; I < (int)(sizeof(Signal)/sizeof(Signal[0])); I++) {
      if (Signal[I] >= SignalNb) SignalNb = Signal[I] + 1;
   }

   SignalName = malloc((size_t)(SignalNb*sizeof(char *)));
   if (SignalName == NULL) FatalError("Not enough memory for signal names");
   for (I = 0; I < SignalNb; I++) SignalName[I] = NULL;

   for (I = 0; I < (int)(sizeof(Signal)/sizeof(Signal[0])); I++) {
      switch (Signal[I]) {
         case SIGABRT : SignalName[SIGABRT] = "SIGABRT"; break;
         case SIGFPE  : SignalName[SIGFPE]  = "SIGFPE";  break;
         case SIGILL  : SignalName[SIGILL]  = "SIGILL";  break;
         case SIGINT  : SignalName[SIGINT]  = "SIGINT";  break;
         case SIGSEGV : SignalName[SIGSEGV] = "SIGSEGV"; break;
         case SIGTERM : SignalName[SIGTERM] = "SIGTERM"; break;
      }
      signal(Signal[I],SigHandler);
   }
}

/* SigHandler() */

static void SigHandler(int Sig) {

   signal(Sig,SigHandler);

   if (Sig == SIGINT) {
      if (SigIntQuit) {
         Quit("%s received",SignalName[Sig]);
      } else {
         ForceMove();
      }
      return;
   }

   if (SignalName[Sig] != NULL) {
      Warning("%s received",SignalName[Sig]);
   } else {
      Warning("Signal #%d received",Sig);
   }

   switch (Sig) {
   case SIGABRT :
   case SIGILL  :
   case SIGSEGV :
   case SIGTERM :
      Quit("%s received",SignalName[Sig]);
      break;
   }
}

/* Output() */

void Output(const char *Message, ...) {

   va_list Args;
   char String[STRING_SIZE];
   int StringSize;

   if (Message == NULL) Message = "";

   va_start(Args,Message);
   vsprintf(String,Message,Args);
   va_end(Args);

   StringSize = strlen(String);

   if (StringSize == 0 || String[StringSize-1] != '\n') {
      for (; LineSize > 0; LineSize--) printf("\b \b");
      if (StringSize != 0) printf("%s",String);
      fflush(stdout);
      strcpy(Line,String);
      LineSize = StringSize;
   } else {
      if (LineSize != 0) {
         printf("\n");
         LineSize = 0;
      }
      printf("%s",String);
   }
}

/* Quit() */

void Quit(const char *Message, ...) {

   va_list Args;
   char    String[STRING_SIZE];

   if (Message == NULL) Message = "";

   va_start(Args,Message);
   vsprintf(String,Message,Args);
   va_end(Args);

   fprintf(stderr,"%s\n",String);
   /* Trace(QUIT_LOG,"%s",String); */

   exit(EXIT_SUCCESS);
}

/* Warning() */

void Warning(const char *Message, ...) {

   va_list Args;
   char    String[STRING_SIZE];

   va_start(Args,Message);
   vsprintf(String,Message,Args);
   va_end(Args);

   fprintf(stderr,"WARNING : %s...\n",String);
   Trace(ERROR_LOG,"WARNING %s",String);
}

/* Error() */

void Error(const char *Message, ...) {

   va_list Args;
   char    String[STRING_SIZE];

   va_start(Args,Message);
   vsprintf(String,Message,Args);
   va_end(Args);

   fprintf(stderr,"ERROR : %s !\n",String);
   Trace(ERROR_LOG,"ERROR   %s",String);
}

/* FatalError() */

void FatalError(const char *Message, ...) {

   va_list Args;
   char    String[STRING_SIZE];

   va_start(Args,Message);
   vsprintf(String,Message,Args);
   va_end(Args);

   fprintf(stderr,"FATAL ERROR : %s !!!\n",String);
   Trace(ERROR_LOG,"FATAL   %s",String);

   exit(EXIT_FAILURE);
}

/* Trace() */

void Trace(const char *LogFile, const char *Message, ...) {

   char     FileName[256], TimeString[32];
   time_t   Time;
   FILE    *Log;
   va_list  Args;

   sprintf(FileName,LOG_PATH "%s",LogFile);
   if ((Log = fopen(FileName,"a")) == NULL) FatalError("Couldn't open file \"%s\"",FileName);

   Time = time(NULL);
   strftime(TimeString,(size_t)31,"%x %X ",localtime(&Time));

   fprintf(Log,"%s",TimeString);

   va_start(Args,Message);
   vfprintf(Log,Message,Args);
   va_end(Args);

   fprintf(Log,"\n");

   fclose(Log);
}

/* DispBoard() */

void DispBoard(const BOARD *Board) {

   int Time, Rank, File, Square;
   static const char *Colour[] = { "Black", "?!?!?", "White" };

   Time = 0;

   if (Board == Game->Board) {
      Time = (int) (ClockWise == FORWARD) ? ElapsedTime[0] : RemainingTime(0);
      if (Time < 0) Time = 0;
   }
   printf("\n  A B C D E F G H         %02d:%02d:%02d\n",Time/3600,(Time%3600)/60,Time%60);
   for (Rank = 0, Square = A1; Rank < 8; Rank++, Square++) {
      printf("%d ",Rank+1);
      for (File = 0; File < 8; File++, Square++) {
         board.Square[Rank][File].Colour = Board->Square[Square];
         if (IsLegalMove(Board,Square)) {
            printf(". ");
            strcpy(board.Square[Rank][File].String,"*");
         } else {
            printf("%c ","*-O"[Board->Square[Square]+1]);
            strcpy(board.Square[Rank][File].String,"");
         }
      }
      printf("%d",Rank+1);
      switch (Rank) {
         case 1 :
            printf("  %2d Disks * %2d Moves",DiskNb(Board,BLACK),Mobility(Board,BLACK));
            break;
         case 3 :
            printf("      %2d Empties",EmptyNb(Board));
            break;
         case 4 :
            if (IsFinished(Board)) {
               printf("      Game ended");
            } else {
               printf("     %s's Turn",Colour[Board->Colour+1]);
            }
            break;
         case 6 :
            printf("  %2d Disks O %2d Moves",DiskNb(Board,WHITE),Mobility(Board,WHITE));
            break;
      }
      printf("\n");
   }
   if (Board == Game->Board) {
      Time = (int) (ClockWise == FORWARD) ? ElapsedTime[1] : RemainingTime(1);
      if (Time < 0) Time = 0;
   }
   printf("  A B C D E F G H         %02d:%02d:%02d\n\n",Time/3600,(Time%3600)/60,Time%60);

   board.Colour = Board->Colour;
   DispXBoard();
}

/* DispBoard2() */

void DispBoard2(const BOARD *Board, const int Forbidden[]) {

   int Time, Rank, File, Square;
   static const char *Colour[] = { "Black", "?!?!?", "White" };

   Time = 0;

   if (Board == Game->Board) {
      Time = (int) ElapsedTime[0];
      if (Time < 0) Time = 0;
   }
   printf("\n  A B C D E F G H         %02d:%02d:%02d\n",Time/3600,(Time%3600)/60,Time%60);
   for (Rank = 0, Square = A1; Rank < 8; Rank++, Square++) {
      printf("%d ",Rank+1);
      for (File = 0; File < 8; File++, Square++) {
         board.Square[Rank][File].Colour = Board->Square[Square];
         if (Forbidden != NULL && Forbidden[Square]) {
            printf("= ");
            strcpy(board.Square[Rank][File].String,"#");
         } else if (IsLegalMove(Board,Square)) {
            printf(". ");
            strcpy(board.Square[Rank][File].String,"*");
         } else {
            printf("%c ","*-O"[Board->Square[Square]+1]);
            strcpy(board.Square[Rank][File].String,"");
         }
      }
      printf("%d",Rank+1);
      switch (Rank) {
         case 1 :
            printf("  %2d Disks * %2d Moves",DiskNb(Board,BLACK),Mobility(Board,BLACK));
            break;
         case 3 :
            printf("      %2d Empties",EmptyNb(Board));
            break;
         case 4 :
            if (IsFinished(Board)) {
               printf("      Game ended");
            } else {
               printf("     %s's Turn",Colour[Board->Colour+1]);
            }
            break;
         case 6 :
            printf("  %2d Disks O %2d Moves",DiskNb(Board,WHITE),Mobility(Board,WHITE));
            break;
      }
      printf("\n");
   }
   if (Board == Game->Board) {
      Time = (int) ElapsedTime[1];
      if (Time < 0) Time = 0;
   }
   printf("  A B C D E F G H         %02d:%02d:%02d\n\n",Time/3600,(Time%3600)/60,Time%60);

   board.Colour = Board->Colour;
   DispXBoard();
}

/* End of Output.C */

