/* FILE = learn.h */

/* External declarations of variables used by interrupt handler.
 */

extern  LAYER           *ext_network;
extern  QUESTION        *ext_parameter;
extern  int             *ext_t_ptr;

/* Contains declarations for functions called from the file learn.c
 */

extern  void    operate(LAYER *, OP_DATA *, QUESTION *);
extern  LAYER * allocate(QUESTION *);
extern  int     learn(LAYER *, QUESTION *, int);
