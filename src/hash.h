
/* Hash.H */

#ifndef HASH_H
#define HASH_H

#include "board.h"

/* Constants */

#define DATE_NB        16 /* 4 bit date stamping */
#define HASH_MAX_DEPTH 31 /* 5 bit depth */

/* Macros */

#define AGE(D) ((Date - (D)) & (DATE_NB - 1))

/* Types */

typedef struct {
   unsigned int Date   :  4;
   unsigned int Depth  :  5;
   unsigned int Killer :  7;
   unsigned int Min    :  1;
   unsigned int Max    :  1;
   signed   int Value  : 14;
} INFO;

typedef struct {
   int  Lock;
   INFO Info;
} ENTRY;

/* Variables */

extern int    HashBits; /* Bit number in the hash key */

extern ENTRY *Hash;
extern int    HashCode[3][SQUARE_NB][2];
extern int    Date;

extern int    HashNb, HashRead, HashWrite;

/* Prototypes */

extern void   InitHashTable  (void);
extern void   ClearHashTable (void);
extern void   CompHashKeys   (const BOARD *Board, int HashKey[]);

extern void   HashInfo       (void);

#endif /* ! defined HASH_H */

/* End of Hash.H */

