/* FILE = input.h */

/* Function declarations for functions called from the file input.c */

#ifndef DEBUG
  QUESTION *   input_parameters(char *);
#else
  QUESTION *   input_parameters();
#endif

#ifndef DEBUG
  LINK_NODE *  load_network_and_parameters(char *);
#else
  LINK_NODE *  load_network_and_parameters();
#endif

#ifndef DEBUG
  I_S *        get_data(DATA_FILE);
#else
  I_S *        get_data();
#endif
