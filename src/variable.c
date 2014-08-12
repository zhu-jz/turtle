
/* Variable.C */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "variable.h"
#include "types.h"
#include "book.h"
#include "clock.h"
#include "endgame.h"
#include "filesys.h"
#include "game.h"
#include "hash.h"
#include "ios.h"
#include "loop.h"
#include "master.h"
#include "midgame.h"
#include "output.h"
#include "probcut.h"
#include "turtle.h"

/* Constants */

#define SETTINGS_DIR "settings/"

enum { BOOLEAN = 1, INTEGER, FLOAT, ENUM, STRING };

/* Types */

typedef union {
   void    *Void;
   int     *Integer;
   double  *Float;
   char   **String;
} VALUE;

typedef struct {
   const char  *Name;
   int          Type;
   VALUE        Value;
   void       (*Function)(void);
   const char **Enum;
} VARIABLE;

/* "Constants" */

static const char *ModeEnum[]        = { "Play", "BookLearn", "EndTest", "ProbCut", "Eval", NULL };
static const char *DefaultEnum[]     = { "Console", "GUI", "FileSystem", "IOS", "Turtle", NULL };
static const char *TimeModeEnum[]    = { "Real", "CPU", NULL };
static const char *ClockWiseEnum[]   = { "Forward", "Backward", NULL };
static const char *WldSolveEnum[]    = { "WLD", "LD", "DW", NULL };
static const char *FullSolveEnum[]   = { "Full", "Window", "NegaC*", "Pessimism", NULL };
static const char *StoreGameEnum[]   = { "None", "Loss", "All", NULL };

/* Variables */

static VARIABLE Variable[] = {

   { "Mode",          ENUM,    { &Mode          }, NULL, ModeEnum        },
   { "Default",       ENUM,    { &Default       }, NULL, DefaultEnum     },
   { "Blocking",      BOOLEAN, { &Blocking      }                        },
   { "SigIntQuit",    BOOLEAN, { &SigIntQuit    }                        },
   { "IosAddress",    STRING,  { &IosAddress    }                        },
   { "IosPort",       INTEGER, { &IosPort       }                        },
   { "IosName",       STRING,  { &IosName       }                        },
   { "IosPassword",   STRING,  { &IosPassword   }                        },
   { "IosEcho",       BOOLEAN, { &IosEcho       }                        },
   { "IosInput",      BOOLEAN, { &IosInput      }                        },
   { "IosMaster",     STRING,  { &IosMaster     }                        },
   { "IosSendEval",   BOOLEAN, { &IosSendEval   }                        },
   { "IosMinTime",    INTEGER, { &IosMinTime    }                        },
   { "IosMaxTime",    INTEGER, { &IosMaxTime    }                        },
   { "IosMinInc",     INTEGER, { &IosMinInc     }                        },
   { "IosMaxInc",     INTEGER, { &IosMaxInc     }                        },
   { "IosMinDef",     INTEGER, { &IosMinDef     }                        },
   { "IosMaxDef",     INTEGER, { &IosMaxDef     }                        },
   { "IosRated",      BOOLEAN, { &IosRated      }                        },
   { "IosRndColour",  BOOLEAN, { &IosRndColour  }                        },
   { "IosWinPoints",  BOOLEAN, { &IosWinPoints  }                        },
   { "IosStored",     BOOLEAN, { &IosStored     }                        },
   { "InPipe",        STRING,  { &InPipe        }                        },
   { "OutPipe",       STRING,  { &OutPipe       }                        },
   { "UseBook",       BOOLEAN, { &UseBook       }                        },
   { "BookFileName",  STRING,  { &BookFileName  }                        },
   { "DrawValue",     INTEGER, { &DrawValue     }                        },
   { "SwitchValue",   BOOLEAN, { &SwitchValue   }                        },
   { "LearnGame",     BOOLEAN, { &LearnGame     }                        },
   { "LearningTime",  FLOAT,   { &LearningTime  }                        },
   { "LearnLine",     STRING,  { &LearnLine     }                        },
   { "TimeMode",      ENUM,    { &TimeMode      }, NULL, TimeModeEnum    },
   { "GameTime",      FLOAT,   { &GameTime      }                        },
   { "Trust",         BOOLEAN, { &Trust         }                        },
   { "RestoreClocks", BOOLEAN, { &RestoreClocks }                        },
   { "ClockWise",     ENUM,    { &ClockWise     }, NULL, ClockWiseEnum   },
   { "HashBits",      INTEGER, { &HashBits      }                        },
   { "UseOppTime",    BOOLEAN, { &UseOppTime    }                        },
   { "UseProbCut",    BOOLEAN, { &UseProbCut    }                        },
   { "ProbCutLevel",  FLOAT,   { &ProbCutLevel  }                        },
   { "WldSolve",      ENUM,    { &WldSolve      }, NULL, WldSolveEnum    },
   { "FullSolve",     ENUM,    { &FullSolve     }, NULL, FullSolveEnum   },
   { "MgEventPeriod", INTEGER, { &MgEventPeriod }                        },
   { "EgEventPeriod", INTEGER, { &EgEventPeriod }                        },
   { "StoreGame",     ENUM,    { &StoreGame     }, NULL, StoreGameEnum   },
   { "AutoRestart",   BOOLEAN, { &AutoRestart   }                        },
   { "AutoSwap",      BOOLEAN, { &AutoSwap      }                        },
   { "AutoSaveGame",  BOOLEAN, { &AutoSaveGame  }                        },
   { NULL                                                                }
};

/* Prototypes */

static VARIABLE *FindVar   (const char *Var);
static char     *NewString (const char *String);

/* Functions */

/* InitVar() */

void InitVar(const char *SettingsFile) {

   char  String[256];
   char *Command, *Variable, *Value;
   FILE *File;

   Mode          = PLAY;
   Default       = CONSOLE;
   Blocking      = TRUE;
   SigIntQuit    = TRUE;
   IosAddress    = NewString("external.nj.nec.com");
   IosPort       = 5000;
   IosName       = NewString("xanntest");
   IosPassword   = NewString(NULL);
   IosEcho       = TRUE;
   IosInput      = TRUE;
   IosMaster     = NewString("xann");
   IosSendEval   = TRUE;
   IosMinTime    = 5;
   IosMaxTime    = 30;
   IosMinInc     = 0;
   IosMaxInc     = 0;
   IosMinDef     = 120;
   IosMaxDef     = 120;
   IosRated      = FALSE;
   IosRndColour  = FALSE;
   IosWinPoints  = FALSE;
   IosStored     = TRUE;
   InPipe        = NewString("inpipe");
   OutPipe       = NewString("outpipe");
   UseBook       = TRUE;
   BookFileName  = NewString("book");
   DrawValue     = 0;
   SwitchValue   = TRUE;
   LearnGame     = FALSE;
   LearningTime  = 60.0;
   LearnLine     = NewString(NULL);
   TimeMode      = CPU;
   GameTime      = 300.0;
   Trust         = TRUE;
   RestoreClocks = TRUE;
   ClockWise     = BACKWARD;
   HashBits      = 20;
   UseOppTime    = TRUE;
   UseProbCut    = TRUE;
   ProbCutLevel  = 1.5;
   WldSolve      = LD;
   FullSolve     = WINDOW;
   MgEventPeriod = 8192;
   EgEventPeriod = 32768;
   StoreGame     = NONE;
   AutoRestart   = FALSE;
   AutoSwap      = FALSE;
   AutoSaveGame  = FALSE;

   if (SettingsFile == NULL || SettingsFile[0] == '\0') SettingsFile = "default";

   sprintf(String,SETTINGS_DIR "%s",SettingsFile);
   File = fopen(String,"r");
   if (File != NULL) {
      printf("[ \"%s\" ] ",SettingsFile);
      fflush(stdout);
      while (fgets(String,256,File)) {
         if (String[strlen(String)-1] == '\n') String[strlen(String)-1] = '\0';
         Command = strtok(String," ");
         if (Command != NULL && SameString(Command,"Set")) {
            Variable = strtok(NULL," ");
            Value    = strtok(NULL," ");
            SetVar(Variable,Value);
         }
      }
      fclose(File);
   } else {
      printf("[ default settings ] ");
      fflush(stdout);
   }
}

/* FindVar() */

static VARIABLE *FindVar(const char *Var) {

   VARIABLE *V;

   if (Var != NULL) {
      for (V = Variable; V->Name != NULL; V++) {
         if (SameString(Var,V->Name)) return V;
      }
   }

   return NULL;
}

/* SetVar() */

int SetVar(const char *Var, const char *Value) {

   VARIABLE *V;
   int       I;

   if (Var   == NULL) Var   = "";
   if (Value == NULL) Value = "";

   V = FindVar(Var);

   if (V == NULL) {
      Warning("Unknown variable \"%s\"",Var);
      return FALSE;
   }

   switch (V->Type) {

      case BOOLEAN :  if (SameString(Value,"True") || SameString(Value,"On") || SameString(Value,"Yes") || SameString(Value,"1")) {
                         *V->Value.Integer = TRUE;
                      } else if (SameString(Value,"False") || SameString(Value,"Off") || SameString(Value,"No") || SameString(Value,"0")) {
                         *V->Value.Integer = FALSE;
                      } else {
                         Warning("Unknown value \"%s\" for boolean variable \"%s\"",Value,V->Name);
                         return FALSE;
                      }
                      break;
      case INTEGER :  sscanf(Value,"%d",V->Value.Integer);
                      break;
      case FLOAT :    sscanf(Value,"%lf",V->Value.Float);
                      break;
      case ENUM :     for (I = 0; V->Enum[I] != NULL; I++) {
                         if (SameString(Value,V->Enum[I])) break;
                      }
                      if (V->Enum[I] != NULL) {
                         *V->Value.Integer = I;
                      } else {
                         Warning("Unknown value \"%s\" for enumeration variable \"%s\"",Value,V->Name);
                         return FALSE;
                      }
                      break;
      case STRING :   if (*V->Value.String != NULL) free(*V->Value.String);
                      *V->Value.String = NewString(Value);
                      break;
      default :       Error("Unknown type variable \"%s\"",V->Name);
                      return FALSE;
   }

   if (V->Function != NULL) (*V->Function)();

   return TRUE;
}

/* SwitchVar() */

int SwitchVar(const char *Var) {

   VARIABLE *V;

   if (Var == NULL) Var = "";

   V = FindVar(Var);

   if (V == NULL) {
      Warning("Unknown variable \"%s\"",Var);
      return FALSE;
   }

   if (V->Type != BOOLEAN) {
      Warning("Cannot switch a non boolean variable : \"%s\"",V->Name);
      return FALSE;
   }

   *V->Value.Integer = ! *V->Value.Integer;
   printf("%s is now %s\n",V->Name,*V->Value.Integer?"On":"Off");
   if (V->Function) (*V->Function)();

   return TRUE;
}

/* ListVar() */

void ListVar(void) {

   VARIABLE *V;

   for (V = Variable; V->Name != NULL; V++) {
      printf("%-13s = ",V->Name);
      switch (V->Type) {
         case BOOLEAN :  printf(*V->Value.Integer?"On":"Off");
                         break;
         case INTEGER :  printf("%d",*V->Value.Integer);
                         break;
         case FLOAT :    printf("%g",*V->Value.Float);
                         break;
         case ENUM :     printf(V->Enum[*V->Value.Integer]);
                         break;
         case STRING :   printf("\"%s\"",(*V->Value.String) ? *V->Value.String : "* NULL POINTER *");
                         break;
         default :       Error("Unknown type variable \"%s\"",V->Name);
                         break;
      }
      printf("\n");
   }
}

/* NewString() */

static char *NewString(const char *String) {

   char *NewString;

   if (String == NULL) String = "";

   NewString = malloc(strlen(String)+1);

   if (NewString != NULL) {
      strcpy(NewString,String);
   } else {
      Error("Not enough memory for string storage");
   }

   return NewString;
}

/* SameString() */

int SameString(const char *String1, const char *String2) {

   while (toupper(*String1) == toupper(*String2)) {
      if (*String1 == '\0') return TRUE;
      String1++;
      String2++;
   }

   return FALSE;
}

/* StartWith() */

int StartWith(const char *String1, const char *String2) {

   while (toupper(*String1) == toupper(*String2)) {
      if (*String1 == '\0') return TRUE;
      String1++;
      String2++;
   }

   return *String2 == '\0';
}

/* End of Variable.C */

