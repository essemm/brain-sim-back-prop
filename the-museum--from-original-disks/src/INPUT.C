/* FILE = input.c */

/* This file contains all the routines used in inputing the parameters
 * used by the network. It includes the routine read_names, which parses
 * the input and output files and checks that they are the correct format.
 */

/* It also contains the routine used to read in the input and ouput
 * data into the data-structures of type I_O.
 */

#ifdef MSDOS
#include <stdlib.h>
#endif

#include <stdio.h>
#include <string.h>

/* Type declarations.
 */

#include "net_type.h"

/* External declarations.
 */

#include "error.h"
#include "learn.h"
#include "show.h"
#include "fishNET.h"

/* This routine checks the data files for validity. See the spec for
 * the correct format.
 */

int
parse(file_data)
DATA_FILE       file_data;
{
	int	cases 		= 0;
	int	neuron		= 0;
	int	total_neurons 	= 0;
	int	x;

	char    dummy[80];
	char    dummy_char;
	double  dummy_num;

	FILE    *file;

	if ( (file = fopen (file_data.name, "r")) == NULL )
		error_f("Can't open file for input", file_data.name);

	if ( fscanf(file, "%[^'[']", dummy) == EOF )
		error_f("Can't find first start token", file_data.name);

	if ( ! QUIET )
		printf("\nData for file %s:\n", file_data.name);

	for ( ; ; )
	{
		if ( (x = fscanf(file, "[start]%c", &dummy_char)) == EOF )
		{
			if (cases == 0)
				error_f
				(
					"Malformed start token or no space after token",
					file_data.name
				);
			else break;
		}

		if ( x == 0 )
		{
			fprintf(stderr, "I/O case = %d.\n", cases + 1);
			error_f("Bad character in file", file_data.name);
		}

		neuron = 0;

		while ( fscanf(file, " %lf", &dummy_num) == 1 )
			neuron++;

		if ( neuron > file_data.neurons )
		{
			if ( ! QUIET )
				fprintf
				(
					stderr,
					"Warning, %d data and %d neurons in case %d.\n",
					neuron, file_data.neurons, cases+1
				);
		}
		else if ( neuron < file_data.neurons )
		{
			fprintf(stderr, "I/O case = %d.\n", cases+1);
			error_f("Not enough data for neurons", file_data.name);
		}

		if ( dummy_num > 1.0 || dummy_num < -1.0 )
		{
			fprintf(stderr, "I/O case = %d.\n", cases+1);
			error_f("Input out of range", file_data.name);
		}

		cases++;
		total_neurons += neuron;
	}

	if ( ! QUIET )
		printf
		(
			"\nTotal data = %d, total I/O cases = %d.\n",
			total_neurons, cases
		);

	fclose(file);

	return(cases);
}

/* Checks for a filename extension. Returns 1 if there is one and 0 if there
 * is not.
 *
 * Uses the Microsoft library routine "_splitpath".
 */

#ifdef MSDOS

int
extension(name)
char    *name;
{
	char    drive[4];
	char	dir[21];
	char	fname[8];
	char	ext[5];

	_splitpath(name, drive, dir, fname, ext);

	if ( strcmp(ext, '\0') == 0 )
		return 0;
	else
		return 1;
}

#endif

/* Adds the extension following the input string.
 *
 * Uses the Microsoft library routine _makepath.
 * Returns a pointer to an array of char.
 */

#ifdef MSDOS

void
add_extension(name, ext)
char    name[35], ext[4];
{
	char    drive[4];
	char	dir[21];
	char	fname[8];
	char	dd[5];

	_splitpath(name, drive, dir, fname, dd);

	_makepath(name, drive, dir, fname, ext);
}

#endif

/* This basic routine calls the parsing routine with the correct file name
 * and type data.
 */

int
read_names(parameter)
QUESTION	*parameter;
{

	char    type;
	char	name[35];

	int     input_cases;

	/* At some stage in the future, a format query will lie here.
	 * However, at the moment, there is only ASCII used.
	 */

	parameter->sample_in.type = 'a';

	printf("\nEnter the file name of the sample input file: ");
	scanf(" %s", name);

#ifdef MSDOS
	if ( ! extension ( name ) )
	{
		if ( ! QUIET )
			printf
			("\nNo filename extension given, so .dat will be assumed.\n");
		add_extension(name, ".dat");
	}
#endif

	strcpy(parameter->sample_in.name ,name);

	input_cases = parameter->sample_in.i_o_cases = 
				parse(parameter->sample_in);

	/* At present, the only type implemented is ASCII, or 'a'. 
	 */

	parameter->sample_out.type = 'a';

	printf("\nEnter the file name of the sample output file: ");
	scanf(" %s", name);

#ifdef MSDOS
	if ( ! extension(name) )
	{
		if ( ! QUIET )
			printf
			( "\nNo filename extension given, so .dat will be assumed.\n");
		add_extension(name, ".dat");
	}
#endif

	strcpy(parameter->sample_out.name, name);

	if ( input_cases !=
		 ( parameter->sample_out.i_o_cases = 
			parse(parameter->sample_out) ) )

		error_f
		(
			"Number of input cases doesn't match number of output cases",
			parameter->sample_out.name
		);

	/* Once again, only 'a' implemented. */

	parameter->data_in.type = 'a';

	printf("\nEnter the file name of the running data input file: ");
	scanf(" %s", name);

#ifdef MSDOS
	if ( ! extension(name) )
	{
		if ( ! QUIET )
			printf
			( "\nNo filename extension given, so .dat will be assumed.\n");
		add_extension(name, ".dat");
	}
#endif

	strcpy(parameter->data_in.name, name);

	input_cases = parameter->data_in.i_o_cases = parse(parameter->data_in);

	/* Only ASCII or type 'a' is implemented. */

	parameter->data_out.type = 'a';

	printf("\nEnter the file name of the running data output file: ");
	scanf(" %s", name);

#ifdef MSDOS
	if ( ! extension(name) )
	{
		if ( ! QUIET )
			printf
			( "\nNo filename extension given, so .dat will be assumed.\n");
		add_extension(name, ".dat");
	}
#endif

	strcpy(parameter->data_out.name, name);

	printf("\nEnter name of file for saved network: ");
	scanf(" %s", name );

#ifdef MSDOS
	if ( ! extension(name) )
	{
		if ( ! QUIET )
			printf
			("\nNo filename extension given, so .net will be assumed.\n");

		add_extension(name, ".net");
	}
#endif

	strcpy(parameter->save_name, name);

	return(1);
}


/* Same as the function above, except that the input comes from stdin instead
 * of a file.
 */

QUESTION *
input_parameters()
{
	QUESTION        *x;

	int     	i;

	char            answer;
	char		comment[79];

	if ( (x = (QUESTION *) malloc(sizeof(QUESTION))) == NULL )
		error_m("QUESTION", "input_parameters");

	printf("\nEnter size of network in layers: ");
	scanf(" %d", &(x->layers));

	if ( x->layers < 2 )
		error("Network must have atleast 2 layers");

	printf("\n\n");

	for (i = 0; i < x->layers; i++)
	{
		printf("\nEnter number of neurons for layer %d :", i);
		scanf(" %d", &(x->per_layer[i]));
	}

	/* Set the number of weights coming from the top-layer to 0 */

	x->per_layer[x->layers] = 0;

	/* Set up sub-structure that says how many neurons there are in input
	 * and output layers.
	 */

	x->sample_in.neurons 	= x->per_layer[0];
	x->data_in.neurons 	= x->per_layer[0];

	x->sample_out.neurons 	= x->per_layer[x->layers - 1];
	x->data_out.neurons 	= x->per_layer[x->layers - 1];

	printf("\n\n");

	printf("Total output layer neurons = %d.", x->per_layer[x->layers - 1]);
	printf("\nEnter the output width in neurons: ");

	scanf(" %d", &(x->out_width));

	printf("\n\n");

	printf("Enter learning parameter alpha for network: ");
	scanf(" %lf", &(x->alpha));

	printf("\nEnter learning parameter epsilon for network: ");
	scanf(" %lf", &(x->epsilon));

	printf("\nDo you wish to set a maximum number of I/O sweeps : [y,n] ");
	scanf(" %c", &answer);

	if ( answer == 'y' || answer == 'Y' )
	{
		printf("\nEnter maximum number of sweeps: ");
		scanf(" %d", &(x->max_sweeps));
	}
	else
		x->max_sweeps = 0;

	read_names(x);

	/* A description of the network */

	printf("\nEnter a comment for the network:\n");
	scanf(" %s", comment);
	strcpy(x->save_comment, comment);

	return(x);
}


/* File input routine, reads all information necessary from a "configuration
 * file". See thesis for format.
 * Returns a pointer to the final structure which contains all the answers.
 */

QUESTION *
finput_parameters(file_name)
char    *file_name;
{
	QUESTION        *x;

	int     	i;

	char    	dummy[160];
	char		dummy_char;

	char		name[35];

	FILE    	*config_file;

	int     	input_cases;

	if ( (x = (QUESTION *) malloc(sizeof(QUESTION))) == NULL )
		error_m("QUESTION", "finput_parameters");

	if ( (config_file = fopen(file_name, "r")) == NULL )
		error_f("Can't open file as configuration file", file_name);

	/* Save comment at start of file */

	if ( fscanf(config_file, "%[^'[']", x->save_comment) == EOF )
		error_f("Can't find start token", file_name);

	if ( fscanf(config_file, "[start]%c", &dummy_char) == EOF )
		error_f("Mutant or deformed start token", file_name);

	if ( fscanf(config_file, " layers %d", &(x->layers)) != 1 )
		error_f("Can't find number of layers", file_name);

	if ( VERBOSE )
		printf("Number of layers = %d.\n", x->layers);

	if ( x->layers < 2 )
		error_f("Network must have atleast 2 layers!", file_name);

	if ( fscanf(config_file, " neurons") == EOF )
		error_f("Can't find keyword 'neurons'", file_name);

	for (i = 0; i < x->layers; i++)
	{
		if ( fscanf(config_file, " %d", &(x->per_layer[i])) != 1)
		{
			fprintf(stderr, "Layer %d.\n", i);
			error_f("Can't find number of neurons", file_name);
		}

		if ( VERBOSE )
			printf("\tLayer %d neurons = %d.\n",i,x->per_layer[i]);
	}

	/* Set the number of weights coming from the top-layer to 0 */

	x->per_layer[x->layers] = 0;

	/* Set up sub-structure that says how many neurons there are in input
	 * and output layers.
	 */

	x->sample_in.neurons 	= x->per_layer[0];
	x->data_in.neurons 	= x->per_layer[0];

	x->sample_out.neurons 	= x->per_layer[x->layers - 1];
	x->data_out.neurons 	= x->per_layer[x->layers - 1];

	if ( fscanf(config_file, " output width %d", &(x->out_width)) != 1 )
		error_f("Can't find output width", file_name);

	if ( VERBOSE )
		printf("\nOutput width %d.\n", x->out_width);

	if ( fscanf(config_file, " alpha %lf", &(x->alpha)) != 1 )
		error_f("Can't find alpha", file_name);

	if ( fscanf(config_file, " epsilon %lf", &(x->epsilon)) != 1 )
		error_f("Can't find epsilon", file_name);

	if ( VERBOSE )
		printf("\nAlpha = %lf. Epsilon = %lf.\n", x->alpha, x->epsilon);

	if ( fscanf(config_file, " sample in %s", name) != 1 )
		error_f
		(
			"Can't find name of sample input file for teaching", 
			file_name
		);

	strcpy(x->sample_in.name, name);

	input_cases = x->sample_in.i_o_cases = parse(x->sample_in);

	if ( fscanf(config_file, " sample out %s", name) != 1 )
		error_f
		(
			"Can't find name of sample output file for teaching",
			file_name
		);

	strcpy(x->sample_out.name, name);

	if ( input_cases != (x->sample_out.i_o_cases = parse(x->sample_out)) )
		error_f
		(
			"ERROR: Number of input cases doesn't match number of output cases",
			x->sample_out.name
		);

	if ( fscanf(config_file, " execute in %s", name) != 1 )
		error_f("Can't find name of execute input file", file_name);

	strcpy(x->data_in.name, name);

	x->data_in.i_o_cases = parse(x->data_in);

	if ( fscanf(config_file, " execute out %s", name) != 1 )
		error_f("Can't find name of execute output file", file_name);

	strcpy(x->data_out.name, name);

	if ( fscanf(config_file, " network save %s", name) != 1 )
		error_f("Can't find name of network save file", file_name);

	strcpy(x->save_name, name);

	if ( VERBOSE )
	{
		printf("\nTeaching input file = %s.\n", x->sample_in.name);
		printf("Teaching expected file = %s.\n", x->sample_out.name);
		printf("Execution input file = %s.\n", x->data_in.name);
		printf("Output file = %s.\n", x->data_out.name);
		printf("Network will be saved to file = %s.\n", x->save_name);
	}

	if ( fscanf(config_file, " max sweeps %d", &(x->max_sweeps)) != 1)
		x->max_sweeps = 0;

	if ( VERBOSE )
	{
		if ( x->max_sweeps == 0 )
			printf("Max sweeps not set.\n");
		else
			printf("Max sweeps = %d.\n", x->max_sweeps);
	}
	return(x);
}

/* Retrieves a network saved to disk, and puts it in memory.
 */

LOAD_BOTH *
load_network_and_parameters(net_file)
char	*net_file;
{
	LAYER           *network;
	QUESTION        *parameter;

	int     	i;
	int		j;
	int		k;

	char    	dummy[80];
	char		dummy_char;

	LAYER   	*START;
	LAYER		*work;
	NEURON  	*neuron;
	WEIGHT  	*weight;

	FILE    	*saved;

	int     	input_cases;
	char    	name[35];

	LOAD_BOTH       *WORK;

	if ( (WORK = (LOAD_BOTH *) malloc(sizeof(LOAD_BOTH))) == NULL )
		error_m("LOAD_BOTH", "load_network_and_parameters");

	if ( (parameter = (QUESTION *) malloc(sizeof(QUESTION))) == NULL )
		error_m("QUESTION", "load_network_and_parameters");

	strcpy(parameter->save_name, net_file);

	if ( (saved = fopen(parameter->save_name, "r")) == NULL )
		error_f("Can't load network", parameter->save_name);

	if ( fscanf(saved, "%[^'[']", parameter->save_comment) == EOF )
		error_f("Can't find first start token", parameter->save_name);

	if ( fscanf(saved, "[start]%c", &dummy_char) == EOF )
		error_f("Malformed start token", parameter->save_name);

	if ( fscanf(saved, " layers %d", &(parameter->layers)) == EOF )
		error_f("Can't find number of layers", parameter->save_name);

	if ( VERBOSE )
		printf("Number of layers = %d.\n", parameter->layers);

	if ( fscanf(saved, " neurons") == EOF )
		error_f("Can't find keyword 'neurons'", parameter->save_name);

	for (i = 0; i < parameter->layers; i++)
	{
		if ( fscanf(saved, " %d", &(parameter->per_layer[i])) != 1 )
		{
			fprintf(stderr, "Layer %d.\n", i);
			error_f("Can't find layer size", parameter->save_name);
		}

		if ( VERBOSE )
			printf
			(
				"\tLayer %d neurons = %d.\n", 
				i, parameter->per_layer[i] 
			);
	}

	parameter->per_layer[parameter->layers]	= 0;

	parameter->sample_in.neurons 	= parameter->per_layer[0];
	parameter->data_in.neurons 	= parameter->per_layer[0];

	parameter->sample_out.neurons 	= 
				parameter->per_layer[parameter->layers - 1];
	parameter->data_out.neurons 	= 
				parameter->per_layer[parameter->layers - 1];

	if ( fscanf(saved, " weights") == EOF )
		error_f("Can't find keyword 'weights'", parameter->save_name);

	if ( (START = (LAYER *) malloc(parameter->layers * sizeof(LAYER))) == NULL )
		error_m("LAYER", "load_network_and_parameters");

	for 
	(
		i = 0, work = START; 
		i < parameter->layers; 
		i++, work++
	){
		if ( (work->layer = (NEURON *) malloc(parameter->per_layer[i] *
								sizeof(NEURON)))
						== NULL )
			error_m("NEURON", "load_network_and_parameters");
		for
		(
			j = 0, neuron = work->layer;
			j < parameter->per_layer[i];
			j++, neuron++
		){
			neuron->y 	= 0.0;
			if ( (neuron->weight = (WEIGHT *) malloc
						(parameter->per_layer[i+1] * 
								sizeof(WEIGHT)))
						== NULL )
				error_m("WEIGHT", "load_network_and_parameters");

			for
			(
				k = 0, weight = neuron->weight;
				k < parameter->per_layer[i+1];
				k++, weight++
			){
				if ( fscanf(saved," %lf",&(weight->w)) == EOF )
				{
					fprintf
					(
						stderr,
						"Layer %d, weight %d.\n",
						i, j
					);
					error_f
					(
						"Can't find weight",
						parameter->save_name
					);
				}

				weight->old_delta_w 	= 0.0;
				weight->dE_dw 		= 0.0;

				if ( VERBOSE )
					printf(" %lf", weight->w);

			}
			if ( VERBOSE )
				printf("\n");
		}
	}
	if ( fscanf(saved, " output width %d", &(parameter->out_width)) == EOF )
		error_f("Can't find output width", parameter->save_name);

	if ( VERBOSE )
		printf("Output width %d\n", parameter->out_width);

	if ( fscanf(saved, " alpha %lf", &(parameter->alpha)) != 1 )
		error_f("Can't find alpha", parameter->save_name);

	if ( fscanf(saved, " epsilon %lf", &(parameter->epsilon)) != 1 )
		error_f("Can't find epsilon", parameter->save_name);

	if ( VERBOSE )
		printf
		(
			"Alpha = %lf, Epsilon = %lf.\n", 
			parameter->alpha, parameter->epsilon
		);

	if ( fscanf(saved, " sample in %s", name) != 1 )
		error_f
		(
			"Can't find name of sample input file for teaching",
			parameter->save_name
		);

	strcpy(parameter->sample_in.name, name);

	input_cases = parameter->sample_in.i_o_cases =
						parse(parameter->sample_in);

	if ( fscanf(saved, " sample out %s", name) != 1 )
		error_f
		(
			"Can't find name of sample output file for teaching",
			parameter->save_name
		);

	strcpy(parameter->sample_out.name, name);

	if
	(
		input_cases != (parameter->sample_out.i_o_cases =
						parse(parameter->sample_out))
	)
		error_f
		(
			"Number of input cases doesn't match number of output cases",
			parameter->sample_out.name
		);

	if ( fscanf(saved, " execute in %s", name) != 1 )
		error_f
		(
			"Can't find name of execute input file",
			parameter->save_name
		);

	strcpy(parameter->data_in.name, name);

	parameter->data_in.i_o_cases = parse(parameter->data_in);

	if ( fscanf(saved, " execute out %s", name) != 1 )
		error_f
		(
			"Can't find name of execute output file",
			parameter->save_name
		);

	strcpy(parameter->data_out.name, name);

	if ( VERBOSE )
	{
		printf
		(
			"\nTeaching input file = %s.\nTeaching expected file = %s.\n",
			parameter->sample_in.name, parameter->sample_out.name
		);
		printf
		(
			"Execution input file = %s.\nOutput file = %s.\n",
			parameter->data_in.name, parameter->data_out.name
		);
	}

	if ( fscanf(saved, " start time %d", &(WORK->start_time)) != 1 )
		error_f("Can't find start time", parameter->save_name);

	if ( fscanf(saved, " learn time %d", &(parameter->max_sweeps)) != 1 )
		error_f("Can't find learn time", parameter->save_name);

	/* Assign the parts of the data structure to the retrieved parameters
	 * and network.
	 */

	WORK->network 	= START;
	WORK->parameter = parameter;

	return(WORK);
}


/* The following routine is used to read in the input and output data for use
 * in the learning routines.
 */

I_O *
get_data(file_data)
DATA_FILE       file_data;
{
	int     i;
	int	j;

	double  dummy_num;
	char    dummy_char;
	char    dummy[80];

	int     discarded;

	I_O     *i_o;
	I_O	*work;
	OP_DATA *xptr;

	FILE    *file;

	if ( (file = fopen(file_data.name, "r")) == NULL )
		error_f("Can't open file for input", file_data.name);

	if ( fscanf(file, "%[^'[']", dummy) == EOF )
		error_f("Can't find first start token",file_data.name);

	if ( (i_o = (I_O *) malloc(file_data.i_o_cases * sizeof(I_O))) == NULL)
		error_m("I_O", "get_data");

	for
	(
		i = 0, work = i_o;
		i < file_data.i_o_cases;
		i++, work++
	){
		if ( (fscanf(file, "[start]%c", &dummy_char)) == EOF )
			error_f
			(
				"Malformed start token or no space after token",
				file_data.name
			);

		if ( (work->item = (OP_DATA *) malloc(file_data.neurons *
							sizeof(OP_DATA)))
						== NULL )
			error_m("OP_DATA", "get_data");

		for
		(
			j = 0, xptr = work->item;
			j < file_data.neurons;
			j++, xptr++
		){
			if ( (fscanf(file, " %lf", &(xptr->x))) != 1 )
				error_f("Can't get data item", file_data.name);
		}

		discarded = 0;

		while ( fscanf(file, " %lf", &dummy_num) == 1 )
			discarded++;

		if ( discarded != 0 )
			if ( VERBOSE )
				printf
				(
					"File = %s. Case = %d: discarded = %d.\n",
					file_data.name, i, discarded
				);
	}

	return(i_o);
}
