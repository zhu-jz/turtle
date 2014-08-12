
/* Hash.I */

#include "hash.h"

/* Functions */

/* ReadHash() */

inline static ENTRY *ReadHash(const int HashKey[2]) {

   ENTRY *Entry;

   HashNb++;

   Entry = &Hash[HashKey[0]&-2];

   if (Entry[0].Lock == HashKey[1]) {
      HashRead++;
      return &Entry[0];
   } else if (Entry[1].Lock == HashKey[1]) {
      HashRead++;
      return &Entry[1];
   }

   return NULL;
}

/* WriteHash() */

inline static ENTRY *WriteHash(const int HashKey[2], int Depth) {

   ENTRY *Entry;

   Entry = &Hash[HashKey[0]&-2];

   if ((AGE(Entry[1].Info.Date) >  AGE(Entry[0].Info.Date)
    || (AGE(Entry[1].Info.Date) == AGE(Entry[0].Info.Date)
    && (Entry[1].Info.Depth < Entry[0].Info.Depth)))) {
      Entry++;
   }

   if (Entry->Info.Date != Date || Entry->Info.Depth <= Depth) HashWrite++;

   return Entry;
}

/* End of Hash.I */

