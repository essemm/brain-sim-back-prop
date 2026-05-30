/* FILE = show.h */

/* Contains declarations for functions called from file show.c.
 */

#ifdef MSDOS
extern  void    show_network(LAYER *, QUESTION *);
#else
extern  void    show_network();
#endif

#ifdef MSDOS
extern  void    print_network(LAYER *, QUESTION *);
#else
extern  void    print_network();
#endif

#ifdef MSDOS
extern  void    show_data(I_O *, DATA_FILE);
#else
extern  void    show_data();
#endif

#ifdef MSDOS
extern  void    print_data(I_O *, DATA_FILE);
#else
extern  void    print_data();
#endif

#ifdef MSDOS
extern  void    show_case(OP_DATA *, int, int);
#else
extern  void    show_case();
#endif

#ifdef MSDOS
extern  void    print_case(OP_DATA *, int);
#else
extern  void    print_case();
#endif

#ifdef MSDOS
extern  void    show_top_layer(LAYER *, QUESTION *);
#else
extern  void    show_top_layer();
#endif

#ifdef MSDOS
extern  void    print_top_layer(LAYER *, QUESTION *);
#else
extern  void    print_top_layer();
#endif

#ifdef MSDOS
extern  void    save_top_layer(LAYER *, QUESTION *, FILE *);
#else
extern  void    save_top_layer();
#endif


#ifdef MSDOS
extern  void    store_network(LAYER *, QUESTION *);
#else
extern  void    store_network();
#endif

#ifdef MSDOS
extern  void    store_learnt_parameters(QUESTION *);
#else
extern  void    store_learnt_parameters();
#endif

#ifdef MSDOS
extern  void    show_help(void);
#else
extern  void    show_help();
#endif
