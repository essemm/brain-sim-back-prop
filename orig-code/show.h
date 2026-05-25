/* FILE = show.h */

#include <stdio.h>

/* Function declarations for functions called from the file show.c */

#ifndef DEBUG
  void   show_network(LAYER *, QUESTION *);
#else
  void   show_network();
#endif

#ifndef DEBUG
  void   print_network(LAYER *, QUESTION *);
#else
  void   print_network();
#endif

#ifndef DEBUG
  void   show_data(I_O *, DATA_FILE);
#else
  void   show_data();
#endif

#ifndef DEBUG
  void   print_data(I_O *, DATA_FILE);
#else
  void   print_data();
#endif

#ifndef DEBUG
  void   show_case(OP_DATA *, int, int);
#else
  void   show_case();
#endif

#ifndef DEBUG
  void   print_case(OP_DATA *, int);
#else
  void   print_case();
#endif

#ifndef DEBUG
  void   show_top_layer(LAYER *, QUESTION *);
#else
  void   show_top_layer();
#endif

#ifndef DEBUG
  void   print_top_layer(LAYER *, QUESTION *);
#else
  void   print_top_layer();
#endif

#ifndef DEBUG
  void   save_top_layer(LAYER *, QUESTION *, FILE *);
#else
  void   save_top_layer();
#endif

#ifndef DEBUG
  void   store_network(LAYER *, QUESTION *);
#else
  void   store_network();
#endif

#ifndef DEBUG
  void   store_learnt_parameters(QUESTION *);
#else
  void   store_learnt_parameters();
#endif

#ifndef DEBUG
  void   show_help(void);
#else
  void   show_help();
#endif
