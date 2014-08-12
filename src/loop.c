
/* Loop.C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "loop.h"
#include "types.h"
#include "book.h"
#include "filesys.h"
#include "game.h"
#include "hash.h"
#include "ios.h"
#include "master.h"
#include "output.h"
#include "turtle.h"
#include "variable.h"
#include "xboard.h"

/* Types */

typedef struct Answer {
   int Move;
   BEST_MOVE Answer[1];
   struct Answer *Succ;
} ANSWER;

/* Variables */

int Default;
int Blocking;

int UseOppTime;

int StoreGame;
int AutoRestart;
int AutoSwap;
int AutoSaveGame;

int Player[2];

static int     State, OpMove, Assumed, StateChanged, AbortSearch;
static ANSWER *Answer;

/* Prototypes */

static void    SetState     (int NewState);
static void    InitLoop     (void);

static void    ClearAnswer  (void);
static ANSWER *NewAnswer    (int Move);
static ANSWER *SearchAnswer (int Move);

static void    BookHelp     (void);

/* Functions */

/* InitState() */

void InitState(int NewState) {

   OpMove       = NO_MOVE;
   Assumed      = NO_MOVE;
   StateChanged = FALSE;
   AbortSearch  = FALSE;

   ClearAnswer();

   SetState(NewState);
}

/* SetState() */

static void SetState(int NewState) {

   if (State != NewState) {

      State = NewState;

      switch (State) {
      case MY_TURN :
         Output("MD [ MY TURN  ]\n");
         break;
      case OPPONENT :
         Output("MD [ OPPONENT ]\n");
         break;
      case ASSUME : 
         Output("MD [ %s GUESS ]\n",SquareString[Assumed]);
         break;
      case LEARN :
         Output("MD [  LEARN   ]\n");
         break;
      case OBSERVE :
         Output("MD [ OBSERVE  ]\n");
         break;
      case WAIT :
         Output("MD [   WAIT   ]\n");
         break;
      default :
         Error("Unknown state %d",State);
         InitLoop();
         break;
      }
   }
}

/* InitPlayers() */

void InitPlayers(int Black, int White) {

   Player[0] = Black;
   Player[1] = White;

   if (Player[0] == TURTLE) {
      if (Player[1] == TURTLE) {
         SetDrawValue(DrawValue);
      } else {
         SetDrawValue(-DrawValue);
      }
   } else {
      if (Player[1] == TURTLE) {
         SetDrawValue(+DrawValue);
      } else {
         SetDrawValue(0);
      }
   }

   InitLoop();
}

/* InitLoop() */

static void InitLoop(void) {

   OpMove       = NO_MOVE;
   Assumed      = NO_MOVE;
   StateChanged = TRUE;
   AbortSearch  = TRUE;

   ClearAnswer();

   if (Player[Game->Board->Colour>0] == TURTLE) {
      SetState(MY_TURN);
   } else if (Player[Game->Board->Colour<0] == TURTLE) {
      SetState(OPPONENT);
   } else {
      if (Blocking) {
         SetState(WAIT);
      } else {
         SetState(OBSERVE);
      }
   }
}

/* Loop() */

void Loop(void) {

   int        Square;
   int        Forbidden[SQUARE_NB];
   BEST_MOVE  Best[1];
   BOARD      Board[1];
   ANSWER    *Exp;

   if (Default == IOS) IosLogin();

   Player[0] = Default;
   Player[1] = TURTLE;

   NewGame();

   while (TRUE) { /* Game loop */

      while (! IsFinished(Game->Board)) { /* Move loop */

         CopyBoard(Game->Board,Board);

         StateChanged = FALSE; /* Stop search with state change ? */
         AbortSearch  = FALSE; /* Stop search without state change (force move) ? */

         if (State != MY_TURN && State != OBSERVE && (Blocking || ! UseOppTime)) {
            SetState(WAIT);
         }

         switch (State) {

	 case MY_TURN :

            Exp = SearchAnswer(OpMove); /* maybe NO_MOVE */
            if (Exp != NULL) {
               Assumed = Exp->Answer->PV[1];
               DoCompMove(Exp->Answer);
            } else {
               InitThinkMT(Board,NULL,Best);
               Think();
               if (! StateChanged && State == MY_TURN) {
                  Assumed = Best->PV[1];
                  DoCompMove(Best);
               }
            }
            ClearAnswer();
            break;

         case OPPONENT : /* Here Assumed == NO_MOVE */

            for (Square = 0; Square < SQUARE_NB; Square++) {
               Forbidden[Square] = FALSE;
            }
            for (Exp = Answer; Exp != NULL; Exp = Exp->Succ) {
               Forbidden[Exp->Move] = TRUE;
            }
            InitThinkOP(Board,Forbidden,Best);
            Think();
            if (! StateChanged && State == OPPONENT) {
               Assumed = Best->Move;
               if (Assumed != NO_MOVE) {
                  SetState(ASSUME);
               } else {
                  SetState(WAIT);
               }
            }
            break;

         case ASSUME : /* Here Assumed != NO_MOVE */

            DoMove(Board,Assumed);
            InitThinkAS(Board,NULL,Best);
            Think();
            if (! StateChanged) {
               Exp = NewAnswer(Assumed);
               *Exp->Answer = *Best;
               if (State == ASSUME) {
                  Assumed = NO_MOVE;
                  SetState(OPPONENT);
               }
            }
            break;

         case OBSERVE :

            InitThinkOB(Board,NULL,Best);
            Think();
            if (! StateChanged && State == OBSERVE) SetState(WAIT);
            break;

         case WAIT :

            do Event(); while (State == WAIT);
            break;

	 default :

            Error("Unknown state %d",State);
            InitLoop();
            break;
         }
      }

      if (AutoSaveGame) SaveGame(Game,"log/games");

      if (UseBook && StoreGame == ALL) {
 	 if (Default == IOS) IosSetState(IOS_LEARNING);
         AddGame(Game);
      }

      if (Default == IOS) IosSetState(IOS_WAITING);

      if (AutoRestart) {
         if (AutoSwap) Swap();
         NewGame();
      } else {
         SetState(WAIT);
         do Event(); while (State == WAIT);
      }
   }

   if (Default == IOS) IosLogout();
}

/* ClearAnswer() */

static void ClearAnswer(void) {

   ANSWER *Answer1, *Answer2;

   for (Answer1 = Answer; Answer1 != NULL; Answer1 = Answer2) {
      Answer2 = Answer1->Succ;
      free(Answer1);
   }
   Answer = NULL;
}

/* NewAnswer() */

static ANSWER *NewAnswer(int Move) {

   ANSWER *New;

   New = SearchAnswer(Move);
   if (New != NULL) {
      Error("Move already in answer list");
      return New;
   }

   New = malloc(sizeof(ANSWER));
   if (New == NULL) FatalError("Not enough memory for answer");
   New->Move = Move;
   New->Succ = Answer;
   Answer    = New;

   return New;
}

/* SearchAnswer() */

static ANSWER *SearchAnswer(int Move) {

   ANSWER *Found;

   for (Found = Answer; Found != NULL; Found = Found->Succ) {
      if (Found->Move == Move) return Found;
   }

   return NULL;
}

/* NewGame() */

void NewGame(void) {

   Output("EV [ NG RECVD ]\n");

   ClearGame(Game,NULL);
   DispBoard(Game->Board);
   ClearHashTable();

   InitLoop();
}

/* DoUserMove() */

void DoUserMove(int Move, double Value, double Time) {

   Output("EV [ %s RECVD ]\n",SquareString[Move]);

   if (Player[Game->Board->Colour>0] == TURTLE) {
      Warning("DoUserMove() called during Turtle's turn");
      return;
   }
   if (State == MY_TURN) {
      Warning("DoUserMove() called when State == MY_TURN");
      return;
   }
   if (! IsLegalMove(Game->Board,Move)) {
      Warning("Illegal move in DoUserMove()");
      return;
   }

   DoGameMove(Game,Move,Value,Time);
   DispBoard(Game->Board);

   OpMove = Move;

   if (Player[Game->Board->Colour>0] == TURTLE) {
      if (State == OPPONENT || (State == ASSUME && OpMove != Assumed)) {
         StateChanged = TRUE;
         AbortSearch  = TRUE;
      }
      if (State == ASSUME && OpMove == Assumed) UpdateThinkAS();
      SetState(MY_TURN);
   } else {
      InitLoop();
   }
}

/* DoCompMove() */

void DoCompMove(const BEST_MOVE *Best) {

   Output("EV [ %s SENT  ]\n",SquareString[Best->Move]);

   if (Player[Game->Board->Colour>0] != TURTLE) {
      Error("DoCompMove() called outside Turtle's turn");
      return;
   }
   if (State != MY_TURN) {
      Error("DoCompMove() called when State != MY_TURN");
      return;
   }
   if (! IsLegalMove(Game->Board,Best->Move)) {
      FatalError("Illegal move in DoCompMove()");
   }

   if (Default == FILE_SYSTEM) Send(SquareString[Best->Move]);
   if (Default == IOS)         IosSendMove(Best->Move,Best->IosValue,Duration(TurnBegin,CurrentTime()));

   DoGameMove(Game,Best->Move,Best->IosValue,Best->Time);
   DispBoard(Game->Board);
   if (Best->PV[1] != NO_MOVE) {
      Output("I play %d.%s (%+.2f) in %s, expecting %s\n\n",Best->Depth,SquareString[Best->Move],Best->IosValue,TimeString(Best->Time),SquareString[Best->PV[1]]);
   } else {
      Output("I play %d.%s (%+.2f) in %s\n\n",Best->Depth,SquareString[Best->Move],Best->IosValue,TimeString(Best->Time));
   }

   OpMove = NO_MOVE;

   if (Player[Game->Board->Colour>0] != TURTLE) {
      if (Assumed != NO_MOVE) {
         SetState(ASSUME);
      } else {
         SetState(OPPONENT);
      }
   }
}

/* Supervisor() */

void Supervisor(void) {

   Output("EV [ SV RECVD ]\n");

   InitPlayers(Default,Default);
}

/* AutoPlay() */

void AutoPlay(void) {

   Output("EV [ AP RECVD ]\n");

   InitPlayers(TURTLE,TURTLE);
}

/* ChangeSides() */

void ChangeSides(void) {

   Output("EV [ CS RECVD ]\n");

   if (Game->Board->Colour == BLACK) {
      InitPlayers(TURTLE,Player[0]);
   } else {
      InitPlayers(Player[1],TURTLE);
   }
}

/* Swap() */

void Swap(void) {

   Output("EV [ SW RECVD ]\n");

   InitPlayers(Player[1],Player[0]);
}

/* ForceMove() */

void ForceMove(void) {

   Output("EV [ FM RECVD ]\n");

   if (Player[Game->Board->Colour>0] != TURTLE) {
      Warning("ForceMove() called outside Turtle's turn");
      return;
   }
   if (State != MY_TURN) {
      Warning("ForceMove() called when State != MY_TURN");
      return;
   }

   AbortSearch = TRUE;
}

/* TakeBack() */

void TakeBack(void) {

   Output("EV [ TB RECVD ]\n");

   if (Game->MoveNo != 0) {
      UndoGameMove(Game);
      DispBoard(Game->Board);
      if (Player[Game->Board->Colour>0] == TURTLE && Player[Game->Board->Colour<0] != TURTLE) {
         InitPlayers(Player[1],Player[0]);
      } else {
         InitLoop();
      }
   }
}

/* TakeBackAll() */

void TakeBackAll(void) {

   Output("EV [ TA RECVD ]\n");

   if (Game->MoveNo != 0) {
      GotoGameMove(Game,0);
      DispBoard(Game->Board);
      if (Player[Game->Board->Colour>0] == TURTLE && Player[Game->Board->Colour<0] != TURTLE) {
         InitPlayers(Player[1],Player[0]);
      } else {
         InitLoop();
      }
   }
}

/* Replay() */

void Replay(void) {

   Output("EV [ RP RECVD ]\n");

   if (Game->Move[Game->MoveNo].Move != NO_MOVE) {
      RedoGameMove(Game);
      DispBoard(Game->Board);
      if (Player[Game->Board->Colour>0] == TURTLE && Player[Game->Board->Colour<0] != TURTLE) {
         InitPlayers(Player[1],Player[0]);
      } else {
         InitLoop();
      }
   }
}

/* ReplayAll() */

void ReplayAll(void) {

   Output("EV [ RA RECVD ]\n");

   if (Game->Move[Game->MoveNo].Move != NO_MOVE) {
      GotoGameMove(Game,GameMoveNb(Game));
      DispBoard(Game->Board);
      if (Player[Game->Board->Colour>0] == TURTLE && Player[Game->Board->Colour<0] != TURTLE) {
         InitPlayers(Player[1],Player[0]);
      } else {
         InitLoop();
      }
   }
}

/* GameEnd() */

void GameEnd(void) {

   Output("EV [ GE RECVD ]\n");

   InitLoop();
}

/* BookHelp() */

static void BookHelp(void) {

   int       I, MoveNb;
   BEST_MOVE BestMove[1];
   int       Forbidden[SQUARE_NB];

   for (I = 0; I < SQUARE_NB; I++) Forbidden[I] = FALSE;

   for (MoveNb = 0; BookMove(Game->Board,Forbidden,BestMove); MoveNb++) {
      Forbidden[BestMove->Move] = TRUE;
   }

   if (MoveNb != 0) {

/*
      Output("\n");
      for (I = 0; I < Game->MoveNo; I++) {
         Output("%2s ",SquareString[Game->Move[I].Move]);
      }
      Output("\n");
*/

      DispBoard(Game->Board);

      for (I = 0; I < SQUARE_NB; I++) Forbidden[I] = FALSE;

      while (BookMove(Game->Board,Forbidden,BestMove)) {
         Output("I suggest %2u.%2s (%+6.2f)\n",BestMove->Depth,SquareString[BestMove->Move],BestMove->IosValue);
         Forbidden[BestMove->Move] = TRUE;
      }

/*
      for (I = 0; I < SQUARE_NB; I++) Forbidden[I] = FALSE;

      while (BookMove(Game->Board,Forbidden,BestMove)) {
         DoGameMove(Game,BestMove->Move,0.0,0.0);
         BookHelp();
         UndoGameMove(Game);
         Forbidden[BestMove->Move] = TRUE;
      }
*/
   }
}

/* Event() */

int Event(void) {

   int  Move;
   char Line[1024], *Variable, *Value;

   if (Default != GUI) HandleXEvent();

   if (Blocking && (State == MY_TURN || State == LEARN)) return AbortSearch;

   switch (Default) {

   case CONSOLE :

      printf("Turtle> ");
      fflush(stdout);
      if (fgets(Line,1024,stdin) == NULL) {
         Output("\"stdin\" closed !\n");
         exit(EXIT_SUCCESS);
      }
      break;

   case GUI :

      switch (HandleXEvent()) {
      case XSQUARE :
         if (! CanPlay(Game->Board,Game->Board->Colour)) {
            DoUserMove(PASS,0.0,0.0);
         } else {
            Move = 9 * SquareY + SquareX + 10;
            if (IsLegalMove(Game->Board,Move)) DoUserMove(Move,0.0,0.0);
         }
         return AbortSearch;
         break;
      case XKEY :
	 Line[0] = Key;
         Line[1] = '\0';
         break;
      default :
         return AbortSearch;
         break;
      }
      break;

   case FILE_SYSTEM :

      if (! Receive(Line,1024)) return AbortSearch;
      break;

   case IOS :

      IosEvent();
      return AbortSearch;
      break;
   }

   if (Line[strlen(Line)-1] == '\n') Line[strlen(Line)-1] = '\0';

   if (SameString(Line,"A")) {
      AutoPlay();
   } else if (SameString(Line,"B")) {
      DispBoard(Game->Board);
   } else if (SameString(Line,"BH")) {
      BookHelp();
   } else if (SameString(Line,"C")) {
      ChangeSides();
   } else if (SameString(Line,"F")) {
      ForceMove();
   } else if (SameString(Line,"H")) {
      Output("\n");
      Output("Turtle commands :\n");
      Output("\n");
      Output("(A)utoplay\n");
      Output("(B)oard\n");
      Output("(B)ook (H)elp\n");
      Output("(C)hange sides\n");
      Output("(F)orce move\n");
      Output("(H)elp\n");
      Output("(H)ash (I)nfo\n");
      Output("(L)ist variables\n");
      Output("(L)oad (G)ame\n");
      Output("(N)ew game\n");
      Output("(P)ass\n");
      Output("(Q)uit\n");
      Output("(R)eplay\n");
      Output("(R)eplay (A)ll\n");
      Output("(S)upervisor\n");
      Output("(S)ave (G)ame\n");
      Output("(Set <variable> <value>)\n");
      Output("(Switch <variable>)\n");
      Output("(T)ake back\n");
      Output("(T)ake back (A)ll\n");
      Output("(V)ersion\n");
      Output("\n");
   } else if (SameString(Line,"HI")) {
      HashInfo();
   } else if (SameString(Line,"L")) {
      ListVar();
   } else if (SameString(Line,"LG")) {
      LoadGame(Game,"log/games");
   } else if (SameString(Line,"N")) {
      if (AutoSwap) Swap();
      NewGame();
   } else if (SameString(Line,"P") || SameString(Line,"PA") || SameString(Line,"PS")) {
      if (IsLegalMove(Game->Board,PASS)) {
         DoUserMove(PASS,0.0,0.0);
         return AbortSearch;
      }
   } else if (SameString(Line,"Q")) {
      Quit(NULL);
   } else if (SameString(Line,"R")) {
      Replay();
   } else if (SameString(Line,"RA")) {
      ReplayAll();
   } else if (SameString(Line,"S")) {
      Supervisor();
   } else if (StartWith(Line,"Set")) {
      strtok(Line," \t");
      Variable = strtok(NULL," \t"); /*  because of  */
      Value    = strtok(NULL," \t"); /* side effects */
      SetVar(Variable,Value);
   } else if (SameString(Line,"SG")) {
      SaveGame(Game,"log/games");
   } else if (StartWith(Line,"Switch")) {
      strtok(Line," \t");
      SwitchVar(strtok(NULL," \t"));
   } else if (SameString(Line,"T")) {
      TakeBack();
   } else if (SameString(Line,"TA")) {
      TakeBackAll();
   } else if (SameString(Line,"V")) {
      Output("\n" TURTLE_NAME " ! (Compiled on " __DATE__ " at " __TIME__ ")\n\n");
   } else if (((Line[0] >= 'A' && Line[0] <= 'H') || (Line[0] >= 'a' && Line[0] <= 'h'))
             && Line[1] >= '1' && Line[1] <= '8' && Line[2] == '\0') {
      Move = StringSquare(Line);
      if (IsLegalMove(Game->Board,Move)) {
         DoUserMove(Move,0.0,0.0);
      }
   }

   return AbortSearch;
}

/* End of Loop.C */

