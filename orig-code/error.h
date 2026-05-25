/* FILE = error.h */

/* Definitions of externally used functions */

#ifndef DEBUG
  void   error(char *);
#else
  void   error();
#endif

#ifndef DEBUG
  void   error_f(char *, char *);
#else
  void   error_f();
#endif

#ifndef DEBUG
  void   error_m(char *, char *);
#else
  void   error_m();
#endif

#ifndef DEBUG
  void   interrupt_handler(int);
#else
  void   interrupt_handler();
#endif

#ifndef DEBUG
  void   sfpe_handler(int);
#else
  void   sfpe_handler();
#endif
