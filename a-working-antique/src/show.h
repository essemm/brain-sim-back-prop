/* FILE = show.h */

/* Contains declarations for functions called from file show.c.
 */

extern  void    show_network(LAYER *, QUESTION *);
extern  void    print_network(LAYER *, QUESTION *);
extern  void    show_data(I_O *, DATA_FILE);
extern  void    print_data(I_O *, DATA_FILE);
extern  void    show_case(OP_DATA *, int, int);
extern  void    print_case(OP_DATA *, int);
extern  void    show_top_layer(LAYER *, QUESTION *);
extern  void    print_top_layer(LAYER *, QUESTION *);
extern  void    save_top_layer(LAYER *, QUESTION *, FILE *);
extern  void    store_network(LAYER *, QUESTION *);
extern  void    store_learnt_parameters(QUESTION *);
extern  void    show_help(void);
