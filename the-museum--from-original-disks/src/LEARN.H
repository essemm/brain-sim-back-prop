/* FILE = learn.h */

/* External declarations of variables used by interrupt handler.
 */

extern  LAYER           *ext_network;
extern  QUESTION        *ext_parameter;
extern  int             *ext_t_ptr;

/* Contains declarations for functions called from the file learn.c
 */

#ifdef MSDOS
extern  void    operate(LAYER *, OP_DATA *, QUESTION *);
#else
extern  void    operate();
#endif

#ifdef MSDOS
extern  LAYER * allocate(QUESTION *);
#else
extern  LAYER * allocate();
#endif

#ifdef MSDOS
extern  int     learn(LAYER *, QUESTION *, int);
#else
extern  int     learn();
#endif
