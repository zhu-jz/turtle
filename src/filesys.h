
/* FileSystem.H */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/* Variables */

extern char *InPipe;
extern char *OutPipe;

/* Prototypes */

extern void Send    (const char *String);
extern int  Receive (char *String, int Size);

#endif /* ! defined FILESYSTEM_H */

/* End of FileSystem.H */

