
/* Loop.H */

#ifndef LOOP_H
#define LOOP_H

#include "master.h"

/* Constants */

enum { CONSOLE, GUI, FILE_SYSTEM, IOS, TURTLE };
enum { MY_TURN, OPPONENT, ASSUME, LEARN, OBSERVE, WAIT };
enum { NONE, LOSS, ALL };

/* Variables */

extern int Default;      /* Is default opponent CONSOLE, FILE_SYSTEM or IOS ? */
extern int Blocking;     /* Is default media blocking ? (eg: CONSOLE is) */

extern int UseOppTime;   /* Think on opponent time ? */

extern int StoreGame;    /* Which games to store in book ? */

extern int AutoRestart;  /* Automatically restart another game ? */
extern int AutoSwap;     /* Swap colours after each game ? */
extern int AutoSaveGame; /* Save all games in ASCII ? */

extern int Player[2];

/* Prototypes */

extern void InitState   (int NewState);
extern void InitPlayers (int Black, int White);

extern void Loop        (void);
extern int  Event       (void);

extern void NewGame     (void);
extern void GameEnd     (void);

extern void DoUserMove  (int Move, double Value, double Time);
extern void DoCompMove  (const BEST_MOVE *Best);

extern void Supervisor  (void);
extern void AutoPlay    (void);
extern void ChangeSides (void);
extern void Swap        (void);
extern void ForceMove   (void);
extern void TakeBack    (void);
extern void TakeBackAll (void);
extern void Replay      (void);
extern void ReplayAll   (void);

#endif /* ! defined LOOP_H */

/* End of Loop.H */

