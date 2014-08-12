
/* Output.H */

#ifndef OUTPUT_H
#define OUTPUT_H

#include "board.h"

/* Constants */

#define IOS_LOG "ios" /* Name of the <ios> log file */

enum { FORWARD, BACKWARD };

/* Variables */

extern int SigIntQuit; /* ^C => Quit or ForceMove ? */
extern int ClockWise;  /* Display elapsed time or remaining time ? */

/* Prototypes */

extern void InitSignal (void);

extern void Output     (const char *Message, ...);
extern void Quit       (const char *Message, ...);
extern void Warning    (const char *Message, ...);
extern void Error      (const char *Message, ...);
extern void FatalError (const char *Message, ...);
extern void Trace      (const char *LogFile, const char *Message, ...);

extern void DispBoard  (const BOARD *Board);
extern void DispBoard2 (const BOARD *Board, const int Forbidden[]);

#endif /* ! defined OUTPUT_H */

/* End of Output.H */

