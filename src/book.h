
/* Book.H */

#ifndef BOOK_H
#define BOOK_H

#include "board.h"
#include "game.h"
#include "master.h"

/* Variables */

extern int     UseBook;      /* Use book to play opening moves ? */
extern char   *BookFileName; /* Book file name to use */
extern int     DrawValue;    /* Midgame value for white draw */
extern int     SwitchValue;  /* Toggle DrawValue sign during learning ? */
extern int     LearnGame;    /* Learn game when adding it ? */
extern double  LearningTime; /* How much time to spend during opening learning */
extern char   *LearnLine;    /* ASCII line to learn */

/* Prototypes */

extern void InitBook     (void);
extern void SetDrawValue (int DrawValue);
extern void LearnBook    (void);

extern int  BookMove     (const BOARD *Board, const int Forbidden[], BEST_MOVE *Best);
extern void AddGame      (const GAME *Game);
extern void Normalize    (int *Binary);

#endif /* ! defined BOOK_H */

/* End of Book.H */

