
/* IOS.C */

#include "ios.h"
#include "types.h"

/* Variables */

char *IosAddress;
int   IosPort;
char *IosName, *IosPassword;
int   IosEcho, IosInput;
char *IosMaster;
int   IosSendEval;
int   IosMinTime, IosMaxTime, IosMinInc, IosMaxInc, IosMinDef, IosMaxDef;
int   IosRated, IosRndColour, IosWinPoints, IosStored;

/* Functions */

void IosLogin      (void) { }
void IosLogout     (void) { }

void IosSendString (const char *Message, ...)            { }
void IosSendMove   (int Move, double Value, double Time) { }

void IosSetState   (int NewState) { }
void IosEvent      (void)         { }

/* End of IOS.C */

