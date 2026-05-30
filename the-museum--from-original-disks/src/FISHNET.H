/* FILE = fishNET.h */

/* This is the header file for the main program. It contains external
 * references to global variables.
 */

extern  int     VERBOSE;
extern  int     QUIET;

extern  int     CONFIG_FILE;
extern  int     NET_FILE;

extern  int     SAVE_NET;

extern  int     DELTA_MODE;

#ifndef MSDOS
extern  FILE    *stdprn;
#endif
