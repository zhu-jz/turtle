
/* Variable.H */

#ifndef VARIABLE_H
#define VARIABLE_H

/* Prototypes */

extern void InitVar    (const char *SettingsFile);
extern int  SetVar     (const char *Var, const char *Value);
extern int  SwitchVar  (const char *Var);
extern void ListVar    (void);

extern int  SameString (const char *String1, const char *String2);
extern int  StartWith  (const char *String1, const char *String2);

#endif /* ! defined VARIABLE_H */

/* End of Variable.H */

