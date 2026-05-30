/* FILE = input.h */

/* Contains declarations for functions called from the file input.c.
 */

#ifdef MSDOS
extern  int     parse(DATA_FILE);
#else
extern  int     parse();
#endif

#ifdef MSDOS
extern  I_O *   get_data(DATA_FILE);
#else
extern  I_O *   get_data();
#endif

#ifdef MSDOS
extern  QUESTION *      input_parameters(void);
#else
extern  QUESTION *      input_parameters();
#endif

#ifdef MSDOS
extern  QUESTION *      finput_parameters(char *);
#else
extern  QUESTION *      finput_parameters();
#endif

#ifdef MSDOS
extern  LOAD_BOTH *     load_network_and_parameters(char *);
#else
extern  LOAD_BOTH *     load_network_and_parameters();
#endif

