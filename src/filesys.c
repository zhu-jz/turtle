
/* FileSystem.C */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "filesys.h"
#include "types.h"

/* Variables */

char *InPipe;
char *OutPipe;

static char Received[256] = "";

/* Functions */

/* Send() */

void Send(const char *String) {

   char *Char;
   FILE *File;

   for (String = String, Char = Received; *String != '\0'; String++, Char++) {
      *Char = tolower(*String);
   }

   while (TRUE) {
      File = fopen(OutPipe,"w");
      if (File != NULL) {
         if (fwrite(Received,1,strlen(Received),File) > 0) {
            fclose(File);
            break;
         }
      }
   }
}

/* Receive() */

int Receive(char *String, int Size) {

   FILE *File;

   File = fopen(InPipe,"r");
   if (File != NULL) {
      if (fgets(String,Size,File)) {
         fclose(File);
         remove(InPipe);
         return TRUE;
      }
   }

   return FALSE;
}

/* End of FileSystem.C */

