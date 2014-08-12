
/* Clock.C */

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#include "clock.h"

/* Constants */

#ifdef CLOCKS_PER_SEC
#undef  CLK_TCK
#define CLK_TCK CLOCKS_PER_SEC /* For non ANSI-compliant compilers :( */
#endif

/* Variables */

int TimeMode;

/* Functions */

/* CurrentTime() */

TIME CurrentTime(void) {

   TIME Time;

   if (TimeMode == REAL) {
      Time.Time = time(NULL);
   } else { /* TimeMode == CPU */
      Time.Clock = clock();
   }

   return Time;
}

/* Duration() */

double Duration(TIME Time1, TIME Time2) {

   double Duration;

   if (TimeMode == REAL) {
      Duration = difftime(Time2.Time,Time1.Time);
   } else { /* TimeMode == CPU */
      Duration = ((double) Time2.Clock - (double) Time1.Clock) / (double) CLK_TCK;
   }

   return Duration;
}

/* TimeString() */

const char *TimeString(double Duration) {

   int D;
   static char String[6];

   D = (int) floor(100.0*Duration+0.5);
   if (D < 0) D = 0;

   if (D < 60 * 100) {
      sprintf(String,"%d\"%02d",D/100,D%100);
   } else {
      D /= 100;
      if (D < 60 * 60) {
         sprintf(String,"%d'%02d",D/60,D%60);
      } else {
         D /= 60;
         if (D < 24 * 60) {
            sprintf(String,"%d:%02d",D/60,D%60);
         } else {
            D /= 60;
            if (D < 100 * 24) {
               sprintf(String,"%dd%02d",D/24,D%24);
            } else {
               D /= 24;
               sprintf(String,"%dd",D);
            }
         }
      }
   }

   return String;
}

/* End of Clock.C */

