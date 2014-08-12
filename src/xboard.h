
/* XBoard.H */

#ifndef XBOARD_H
#define XBOARD_H

/* Constants */

#define SQUARE_STRING_SIZE 11

enum { XNONE, XSQUARE, XKEY }; /* Events */

/* Types */

typedef struct {
   int Colour;
   struct {
     int  Colour;
     char String[SQUARE_STRING_SIZE+1];
   } Square[8][8];
} xboard;

/* Variables */

extern int    XBoard;
extern xboard board;
extern int    SquareX, SquareY;
extern int    Key;

/* Prototypes */

extern void OpenXBoard   (void);
extern void CloseXBoard  (void);
extern void DispXBoard   (void);

extern int  HandleXEvent (void);

#endif /* ! defined XBOARD_H */

/* End of XBoard.H */

