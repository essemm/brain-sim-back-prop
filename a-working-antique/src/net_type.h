/* FILE = net_type.h */

/* This file contains the definitions for the types used throughout the
 * program.
 */

/* This is the definition which contains data about the files.
 */

typedef struct {

	char    name[35];
	char    type;
	int     i_o_cases;
	int     neurons;

	} DATA_FILE;

/* Parameters which are used by the program and entered by the user are
 * stored in this structure.
 */
typedef struct {

	int             layers;
	int             per_layer[7];

	int             out_width;

	int             max_sweeps;

	double          alpha;
	double          epsilon;

	char            save_name[35];
	char            save_comment[79];

	DATA_FILE       sample_in;
	DATA_FILE       sample_out;
	DATA_FILE       data_in;
	DATA_FILE       data_out;

	} QUESTION;


/*
 * Definitions for the network follow below.
 */

/* WEIGHT contains the information used by the program while teaching and
 * operating the network.
 */

typedef struct {

	double  w;
	double	old_delta_w;
	double	dE_dw;

	} WEIGHT;

/* A neuron contains its output and a pointer to an array of weights,
 * each one of which is referring to the layer above.
 */

typedef struct {

	double  y;
	WEIGHT  *weight;

	} NEURON;

/* The type LAYER is used to point to arrays of neurons. Each pointer points
 * to a different layer of neurons.
 */

typedef struct {

	NEURON   *layer;

	} LAYER;


/*
 * The following type is used to store input and output data for the network.
 */

typedef struct {

	double  x;

	} OP_DATA;

/* This is a pointer to above, an array of which is used for input and output.
 */

typedef struct {

	OP_DATA *item;

	} I_O;

/* This type is for use by the function which returns both pointers to a
 * network and a block of QUESTION.
 */

typedef struct {

	LAYER           *network;
	QUESTION        *parameter;
	int             start_time;

	} LOAD_BOTH;

/* These are values that can be taken by a flag that deals with whether a
 * loaded network is to be taught, executed, or don't know.
 */

#define NONE    1
#define TEACH   2
#define EX      3

/* Values for variables that save the network.
 */

#define DO      1
#define DO_NOT  2
#define	PARTLY	3

/* Mode for operation of back-propagation of errors.
 * The two options available are ALL or EACH, meaning that the dE/dw's are
 * either added after EACH I/O case or are summed over all I/O cases.
 */

#define ALL     1
#define EACH    2
