
/* Game.H */

#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "clock.h"

/* Constants */

#define GAME_MAX_LENGTH 96

/* Types */

typedef struct {
   int    Move;
   double Value;
   double Time;
} GAME_MOVE;

typedef struct {
   int       MoveNo;
   GAME_MOVE Move[GAME_MAX_LENGTH];
   BOARD     StartBoard[1];
   BOARD     Board[1];
} GAME;

/* Variables */

extern double GameTime;      /* in seconds */
extern int    Trust;         /* trust opponent time ? */
extern int    RestoreClocks; /* restore clocks when takeback ? */

extern GAME   Game[1];

extern double ElapsedTime[2];
extern TIME   TurnBegin;

/* Prototypes */

extern void   ClearGame     (GAME *Game, const BOARD *StartBoard);
extern void   DoGameMove    (GAME *Game, int Move, double Value, double Time);
extern void   UndoGameMove  (GAME *Game);
extern void   RedoGameMove  (GAME *Game);
extern int    GameMoveNb    (const GAME *Game);
extern void   GotoGameMove  (GAME *Game, int MoveNo);

extern void   LoadGame      (GAME *Game, const char *FileName);
extern void   SaveGame      (const GAME *Game, const char *FileName);

extern double RemainingTime (int Colour);

#endif /* ! defined GAME_H */

/* End of Game.H */

