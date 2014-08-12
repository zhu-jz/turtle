
/* Turtle.H */

#ifndef TURTLE_H
#define TURTLE_H

/* Constants */

#define TURTLE_NAME "Turtle v0.6a, the slow pets defender"

enum { PLAY, BOOK_LEARN, END_TEST, PROB_CUT, EVAL };

/* Variables */

extern int Mode; /* Playing, self-learning or endgame testing ? */

extern int ArgC;
extern char **ArgV;

#endif /* ! defined TURTLE_H */

/* End of Turtle.H */

