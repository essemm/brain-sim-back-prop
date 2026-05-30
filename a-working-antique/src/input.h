/* FILE = input.h */

/* Contains declarations for functions called from the file input.c.
 */

extern  int         parse(DATA_FILE);
extern  I_O *       get_data(DATA_FILE);
extern  QUESTION *  input_parameters(void);
extern  QUESTION *  finput_parameters(char *);
extern  LOAD_BOTH * load_network_and_parameters(char *);
