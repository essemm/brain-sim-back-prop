/* FILE = learn.c */

/* This is the file containing the functions to allocate memory,
 * operate with a network, and learn by the back-propagation method.
 */

/* The following definitions are for making the random number between
 * -.3 and .3 for the initial weights.
 * RANDDIV was originally 54612 for the DOS rand() whose RAND_MAX was 32767.
 * We now compute it from RAND_MAX so the range is portable. */

#define RANDDIV ((double)RAND_MAX / 0.6)
#define RANDSUB 0.3

/* The default maximum number of sweeps is set to... */
#define MAXSWEEPS       5000

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "net_type.h"

#include "show.h"
#include "input.h"
#include "error.h"
#include "fishnet.h"

/* below defines the transfer function of a neuron. */

#define TRANSFER(x)     (1.0 / (1.0 + exp(-(x))));

/* Allocate the correct amount of space for the network. */

LAYER *
allocate(QUESTION *parameter)
{
	time_t          seed;

	int             i;
	int             j;
	int             k;

	LAYER           *START;
	LAYER           *network;
	NEURON          *neuron;
	WEIGHT          *weight;

	time(&seed);
	srand((unsigned) seed);

	if ( (START = (LAYER *) malloc(parameter->layers * sizeof(LAYER)))
			== NULL )
		error_m("LAYER", "allocate");

	for
	(
		i = 0, network = START;
		i < parameter->layers;
		i++, network++
	){
		if ( (network->layer = (NEURON *) malloc(parameter->per_layer[i] *
						sizeof(NEURON)))
				== NULL )
			error_m("NEURON", "allocate");

		for
		(
			j = 0, neuron = network->layer;
			j < parameter->per_layer[i];
			j++, neuron++
		){
			neuron->y 	= 0.0;
			if ( (neuron->weight =
				(WEIGHT *) malloc(parameter->per_layer[i+1] *
						sizeof(WEIGHT)))
					== NULL )
				error_m("WEIGHT", "allocate");

			for
			(
				k = 0, weight = neuron->weight;
				k < parameter->per_layer[i+1];
				k++, weight++
			){
				weight->w		= (((double) rand()) /
								RANDDIV) -
									RANDSUB;
				weight->old_delta_w 	= 0.0;
				weight->dE_dw 		= 0.0;
			}
		}
	}

	return(START);
}

void
operate(LAYER *network, OP_DATA *input, QUESTION *parameter)
{
	OP_DATA         *xptr;

	LAYER           *present_net;
	NEURON          *layer;
	NEURON          *prev_layer;
	NEURON          *neuron;
	NEURON          *prev_neuron;
	WEIGHT          *weight;

	int             i;
	int             j;
	int             k;

	double          sum;

	/* Handle the bottom layer first. */

	for
	(
		i = 0, xptr = input, neuron = network->layer;
		i < parameter->per_layer[0];
		i++, xptr++, neuron++
	)
		neuron->y = TRANSFER(xptr->x);

	/* Now deal with the rest of the network */

	for
	(
		i = 1, present_net = network;
		i < parameter->layers;
		i++, present_net++
	){
		prev_layer	= present_net->layer;
		layer 		= (present_net + 1)->layer;

		for
		(
			j = 0, neuron = layer;
			j < parameter->per_layer[i];
			j++, neuron++
		){
			sum 	= 0.0;

			for
			(
				k = 0, prev_neuron = prev_layer;
				k < parameter->per_layer[i-1];
				k++, prev_neuron++
			){
				weight 	= (prev_neuron->weight) + j;
				sum 	+= prev_neuron->y * weight->w;
			}

			neuron->y = TRANSFER(sum);
		}
	}
}

/* The routine below executes the back-propagation algorithm as described in
 * the thesis. It also modifies the weights, and is an amalgamation of the
 * two later functions "back_propagate" and "apply_delta_w".
 * It is the second algorithm listed in Chapter 2.
 */

int
back_propagate_apply_delta_w(LAYER *network, OP_DATA *expected,
                             QUESTION *parameter, int t)
{
	double          *xptr;
	double          *dE_dx;
	double          *yptr;
	double          *dE_dy;

	double          alpha;
	double          epsilon;
	double          delta_w;

	int             i;
	int             j;
	int             k;

	OP_DATA         *dptr;

	NEURON          *layer;
	NEURON          *prev_layer;
	NEURON          *neuron;
	NEURON          *prev_neuron;
	WEIGHT          *weight;

	int             error           = 0;

	alpha = parameter->alpha * exp(-(double) t);

	epsilon = parameter->epsilon;

	/* Handle top layer first, as special case
	 */

	if ( (xptr = (double *) malloc(sizeof(double) *
				  parameter->per_layer[parameter->layers - 1]))
			== NULL )
		error_m("double", "back_propagate_apply_delta_w");

	layer 		= (network + parameter->layers - 1)->layer;
	prev_layer 	= ( network + parameter->layers - 2 )->layer;

	for
	(
		i = 0, dptr = expected, neuron = layer, dE_dx = xptr;
		i < parameter->per_layer[ parameter->layers - 1 ];
		i++, dptr++, neuron++, dE_dx++
	){
		*dE_dx = (neuron->y - dptr->x) * neuron->y *
					(1 - neuron->y);

		if ( fabs(neuron->y - dptr->x) > 0.2 )
			error++;

		for
		(
			j = 0, prev_neuron = prev_layer;
			j < parameter->per_layer[parameter->layers - 2];
			j++, prev_neuron++
		){
			weight		= prev_neuron->weight + i;
			delta_w 	= (-epsilon * (*dE_dx) *
							prev_neuron->y) +
						(alpha * weight->old_delta_w);

			weight->w	+= delta_w;

			/* Decay a bit...*/
			weight->w 	*= 0.9999;

			weight->old_delta_w 	= delta_w;
		}
	}

	/* Now deal with the rest of the network. */
	for ( i=0; i < parameter->layers - 2; i++ )
	{
		layer 		= (network + parameter->layers - i - 2)->layer;
		prev_layer 	= (network + parameter->layers - i - 3)->layer;

		if ( (yptr = (double *) malloc(sizeof(double) *
				parameter->per_layer[parameter->layers -
						i - 2]))
				== NULL )
			error_m("double", "back_propagate_apply_delta_w");

		for
		(
			j = 0, dE_dy = yptr;
			j < parameter->per_layer[ parameter->layers - (i+2) ];
			j++, dE_dy++
		){
			*dE_dy = 0.0;

			for
			(
				k = 0, weight = (layer + j)->weight,
					dE_dx = xptr;

				k < parameter->per_layer[parameter->layers -
					(i + 1)];

				k++, weight++, dE_dx++
			){
				*dE_dy += (*dE_dx) * weight->w;

			}
		}

		free(xptr);

		if ( (xptr = (double *) malloc(sizeof(double) *
				parameter->per_layer[parameter->layers -
							i - 2]))
				== NULL )
			error_m("double", "back_propagate_apply_delta_w");

		for
		(
			j = 0, dE_dx = xptr, dE_dy = yptr, neuron = layer;
			j < parameter->per_layer[parameter->layers - (i + 2)];
			j++, dE_dx++, dE_dy++, neuron++
		){
			*dE_dx = (*dE_dy) * neuron->y * (1 - neuron->y);

			for
			(
				k = 0, prev_neuron = prev_layer;
				k < parameter->per_layer[parameter->layers -
								(i + 3)];
				k++, prev_neuron++
			){
				weight	= prev_neuron->weight + j;

				delta_w = (-epsilon * (*dE_dx) *
							prev_neuron->y) +
					  (alpha * weight->old_delta_w);

				weight->w	+= delta_w;

				/* Decay a bit...*/
				weight->w 	*= 0.9999;

				weight->old_delta_w 	= delta_w;
			}
		}

		free(yptr);
	}

	free(xptr);
	return(error);
}

/* The routine below executes the back-propagation algorithm as described in
 * the thesis.
 * This the first algorithm in Chapter 2, and is the default.
 */

int
back_propagate(LAYER *network, OP_DATA *expected, QUESTION *parameter)
{
	double          *xptr;
	double          *dE_dx;
	double          *yptr;
	double          *dE_dy;

	int             i;
	int             j;
	int             k;

	OP_DATA         *dptr;

	NEURON          *layer;
	NEURON          *prev_layer;
	NEURON          *neuron;
	NEURON          *prev_neuron;
	WEIGHT          *weight;

	int             error   = 0;

	/* Handle top layer first, as special case
	 */

	if ( (xptr = (double *) malloc(sizeof(double) *
				  parameter->per_layer[parameter->layers - 1]))
			== NULL )
		error_m("double", "back_propagate");

	layer 		= (network + parameter->layers - 1)->layer;
	prev_layer	= (network + parameter->layers - 2)->layer;

	for
	(
		i = 0, dptr = expected, neuron = layer, dE_dx = xptr;
		i < parameter->per_layer[parameter->layers - 1];
		i++, dptr++, neuron++, dE_dx++
	){
		*dE_dx = (neuron->y - dptr->x) * neuron->y *
					(1 - neuron->y);

		if ( fabs(neuron->y - dptr->x) > 0.2 )
			error++;

		for
		(
			j = 0, prev_neuron = prev_layer;
			j < parameter->per_layer[parameter->layers - 2];
			j++, prev_neuron++
		){
			weight 		= prev_neuron->weight + i;
			weight->dE_dw 	+= (*dE_dx) * prev_neuron->y;
		}
	}

	/* Now deal with the rest of the network.*/
	for (i=0; i < parameter->layers - 2; i++)
	{
		layer 		= (network + parameter->layers - i - 2)->layer;
		prev_layer 	= (network + parameter->layers - i - 3)->layer;

		if ( (yptr = (double *) malloc(sizeof(double) *
				parameter->per_layer[parameter->layers -
							i - 2]))
				== NULL )
			error_m("double", "back_propagate");

		for
		(
			j = 0, dE_dy = yptr;
			j < parameter->per_layer[parameter->layers - i - 2];
			j++, dE_dy++
		){
			*dE_dy = 0.0;

			for
			(
				k = 0, weight = (layer + j)->weight,
					dE_dx = xptr;

				k < parameter->per_layer[parameter->layers -
					i - 1];

				k++, weight++, dE_dx++
			){

				*dE_dy += (*dE_dx) * weight->w;

			}
		}

		free(xptr);

		if ( (xptr = (double *) malloc(sizeof(double) *
				parameter->per_layer[parameter->layers -
							i - 2]))
				== NULL )
			error_m("double", "back_propagate");

		for
		(
			j = 0, dE_dx = xptr, dE_dy = yptr, neuron = layer;
			j < parameter->per_layer[parameter->layers - i - 2];
			j++, dE_dx++, dE_dy++, neuron++
		){

			*dE_dx = (*dE_dy) * neuron->y * (1 - neuron->y);

			for
			(
				k = 0, prev_neuron = prev_layer;
				k < parameter->per_layer[parameter->layers -
								i - 3];
				k++, prev_neuron++
			){

				weight		= prev_neuron->weight + j;
				weight->dE_dw 	+= (*dE_dx) * prev_neuron->y;

			}
		}

		free(yptr);
	}

	free(xptr);

	return(error);
}

/* The function below modifies the weights according to the algorithm given
 * in the thesis, resets dE_dw to zero for each weight, and returns a
 * cumulative total of the errors.
 */

void
apply_delta_w(LAYER *network, QUESTION *parameter, int t)
{
	LAYER   *present_net;
	NEURON  *neuron;
	WEIGHT  *weight;

	int     i;
	int     j;
	int     k;

	double  alpha;
	double	delta_w;

	alpha = parameter->alpha * exp(-(double) t);

	for
	(
		i = 0, present_net = network;
		i < parameter->layers;
		i++, present_net++
	){

		for
		(
			j = 0, neuron = present_net->layer;
			j < parameter->per_layer[i];
			j++, neuron++
		){

			for
			(
				k = 0, weight = neuron->weight;
				k < parameter->per_layer[i+1];
				k++, weight++
			){

				delta_w	= (-(parameter->epsilon) *
						weight->dE_dw) +
					  (alpha * weight->old_delta_w);

				weight->w += delta_w;

				/* Decay a little, as suggested in [20]. */

				weight->w *= 0.9999;

				weight->old_delta_w = delta_w;

				weight->dE_dw = 0.0;
			}
		}
	}
}

/* The general function that teaches the network.*/

/* External variable definitions for use by interrupt_handler during ctrl+c.
 */

LAYER           *ext_network;
QUESTION        *ext_parameter;
int             *ext_t_ptr;

int
learn(LAYER *network, QUESTION *parameter, int start_time)
{
	int     i;
	int     t 	= start_time;

	int     error;
	int     threshold;

	I_O     *input;
	I_O	*expected;
	I_O	*input_case;
	I_O	*expected_case;

	/* Define external variables to be the same as the internal ones: */

	ext_network 	= network;
	ext_parameter 	= parameter;
	ext_t_ptr 	= &t;

	input 		= get_data(parameter->sample_in);

	expected	= get_data(parameter->sample_out);

	if ( VERBOSE )
	{
		show_data(input, parameter->sample_in);
		show_data(expected, parameter->sample_out);
	}

	if ( parameter->max_sweeps > 0 )
		threshold = parameter->max_sweeps;
	else
		threshold = MAXSWEEPS;

	if ( ! QUIET )
		printf
		(
			"\nSetting max number of learning iterations to %d.\n",
			threshold
		);

	/* For efficiency's sake (there would need to be thousands of
	 * needless comparisons), I err on the side of speed instead of size
	 * efficiency and repeat huge chunks of code so that only one test of
	 * the global variable DELTA_MODE is necessary.
	 */

	if ( DELTA_MODE == ALL ) /* add together the dE/dw's over all cases. */
	{
		t--; /* need to do this because apply_delta_w is at the top */
		     /* of the loop, and otherwise time is one out of whack.*/

		do
		{
			error = 0;

			apply_delta_w(network, parameter, t++);

			for
			(
				i = 0, input_case = input,
				expected_case = expected;

				i < parameter->sample_in.i_o_cases;

				i++, input_case++, expected_case++
			){

				operate(network, input_case->item, parameter);

				error += back_propagate
					(
						network,
						expected_case->item,
						parameter
					);

				if ( VERBOSE )
				{
					show_case
					(
						expected_case->item,
						parameter->sample_out.neurons,
						parameter->out_width
					);

					show_top_layer(network, parameter);

					printf("\n");
				}
			}

			if ( ! QUIET )
				printf("t = %d. errors = %d.\n\n", t, error);

		} while ( error > 0 && t < threshold );
	}

	else /* apply dE/dw's to calculate delta_w's every I/O case. */
		do
		{
			error = 0;

			for
			(
				i = 0, input_case = input,
				expected_case = expected;

				i < parameter->sample_in.i_o_cases;

				i++, input_case++, expected_case++
			){

				operate(network, input_case->item, parameter);

				error += back_propagate_apply_delta_w
					(
						network,
						expected_case->item,
						parameter,
						t
					);

				if ( VERBOSE )
				{
					show_case
					(
						expected_case->item,
						parameter->sample_out.neurons,
						parameter->out_width
					);

					show_top_layer(network, parameter);

					printf("\n");
				}
			}

			t++;
			if ( ! QUIET )
				printf("t = %d. errors = %d.\n\n", t, error);

		} while ( error > 0 && t < threshold );

	parameter->max_sweeps = t;

#ifdef DEBUG
	if ( ! QUIET )
		print_network(network, parameter);
#endif

	return(1);
}
