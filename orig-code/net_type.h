/* FILE = net_type.h */

/*
 * This file contains the definitions for the types used throughout
 * the program.
 */

/* This is the definition which contains data about the files. */

typedef struct {
    char    name[50];
    char    type;
} DATA_FILE;

/* Parameters which are used by the program and entered by the user are
 * in this structure.
 */

typedef struct {
    int         layers;
    int         per_layer[17];
    int         net_width;
    double      alpha;
    double      epsilon;
    double      repsilon;
    int         test_sweeps;
    int         sample_in_cases;
    int         data_in_cases;
    char        save_name[50];
    char        network_comment[50];
    DATA_FILE   sample_in;
    DATA_FILE   sample_out;
    DATA_FILE   data_in;
    DATA_FILE   data_out;
} QUESTION;

/* Definitions for the network follow here. */

/* WEIGHT contains the information used by the program while teaching and
 * operating the network.
 */

typedef struct {
    double  w;
    double  hold_delta_w;
    double  old_dw;
} WEIGHT;

/* A NEURON contains the output and a pointer to an array of weights,
 * each one of which is referring to the layer above.
 */

typedef struct {
    double  ty;
    WEIGHT  *weight;
} NEURON;

/* The type LAYER is used to point to arrays of neurons. Each pointer points
 * to a different layer of neurons.
 */

typedef struct {
    NEURON  *layer;
} LAYER;

/*
 * The following type is used to store input and output data for the network.
 */

typedef struct {
    double  x;
} I_S;

typedef struct {
    I_S     *items;
} I_O;

/* This type is for use by the function which returns both pointers to a
 * network and a set of QUESTION.
 */

typedef struct {
    LAYER       *network;
    QUESTION    *parameter;
    int          start_time;
} LINK_NODE;

/* These are values that can be taken by a flag that deals with whether a
 * loaded network is to be taught, executed, or don't know.
 */

#define NONE    0
#define TEACH   1
#define XX      3

/* Values for variables that save the network */

#define DO      1
#define DO_SET  2
#define PARTLY  3

/* Mode for operation of back-propagation of errors.
 * The two options available are ALL or EACH, meaning that the delta_w's are
 * either added after EACH I/O case or are summed over all I/O cases.
 */

#define ALL     1
#define EACH    2
