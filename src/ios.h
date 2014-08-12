
/* IOS.H */

#ifndef IOS_H
#define IOS_H

/* Constants */

enum { IOS_PLAYING, IOS_LEARNING, IOS_OBSERVING, IOS_WAITING};

/* Variables */

extern char *IosAddress;   /* <ios> IP address  */
extern int   IosPort;      /* <ios> port number */

extern char *IosName;      /* <ios> login */
extern char *IosPassword;  /* <ios> password */
extern int   IosEcho;      /* Output <ios> stream to stdout ? */
extern int   IosInput;     /* Send stdin to <ios> ? */

extern char *IosMaster;    /* operator <ios> login */

extern int   IosSendEval;  /* Send evaluation function to <ios> ? */

extern int   IosMinTime;   /* Game minimum time in minutes */
extern int   IosMaxTime;   /* Game maximum time in minutes */
extern int   IosMinInc;    /* Game minimum increment time in seconds */
extern int   IosMaxInc;    /* Game maximum increment time in seconds */
extern int   IosMinDef;    /* Game minimum default time in seconds */
extern int   IosMaxDef;    /* Game maximum default time in seconds */

extern int   IosRated;     /* Accept only rated games ? */
extern int   IosRndColour; /* Accept only randomly choosen colours game ? */
extern int   IosWinPoints; /* Accept game only if win points on 64-0 ? */
extern int   IosStored;    /* Accept stored games ? */

/* Prototypes */

extern void IosLogin      (void);
extern void IosLogout     (void);
extern void IosSendString (const char *Message, ...);
extern void IosSendMove   (int Move, double Value, double Time);

extern void IosSetState   (int NewState);
extern void IosEvent      (void);

#endif /* ! defined IOS_H */

/* End of IOS.H */

