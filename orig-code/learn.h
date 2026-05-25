/* FILE = learn.h */

/* Formal declarations of variables used by interrupt handler */

extern LAYER    *ext_network;
extern QUESTION *ext_parameter;
extern int       ext_b_ptr;

/* Function declarations for functions called from this file learn. */

#ifndef DEBUG
  extern void   operate(LAYER *, I_S *, QUESTION *);
#else
  void          operate();
#endif

#ifndef DEBUG
  LAYER *   allocate(QUESTION *);
#else
  LAYER *   allocate();
#endif

#ifndef DEBUG
  int   teach(LAYER *, QUESTION *, int);
#else
  int   teach();
#endif

#ifndef DEBUG
  int   back_propagate(LAYER *, I_S *, QUESTION *);
#else
  int   back_propagate();
#endif

#ifndef DEBUG
  int   back_propagate_apply_delta_w(LAYER *, I_S *, QUESTION *, int);
#else
  int   back_propagate_apply_delta_w();
#endif

#ifndef DEBUG
  int   apply_delta_w(LAYER *, QUESTION *, int);
#else
  int   apply_delta_w();
#endif
