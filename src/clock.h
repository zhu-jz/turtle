
/* Clock.H */

#ifndef CLOCK_H
#define CLOCK_H

#include <time.h>

/* Constants */

enum { REAL, CPU };

/* Types */

typedef union {
   clock_t Clock;
   time_t  Time;
} TIME;

/* Variables */

extern int TimeMode; /* Is time measuring REAL or CPU ? */

/* Prototypes */

extern TIME        CurrentTime (void);
extern double      Duration    (TIME Time1, TIME Time2); /* in seconds, Time1 <= Time2 */
extern const char *TimeString  (double Duration);

#endif /* ! defined CLOCK_H */

/* End of Clock.H */

