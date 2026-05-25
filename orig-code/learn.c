/* FILE = learn.c */

/*
 * This file contains all the routines used to allocate memory,
 * operates with a network and operates the back-propagation
 * algorithm.
 */

/* The following definitions are for making the random number between
 * -3 and .3 for the initial weights.
 */

#define RANDINT  64516.0
#define RANDDEN  0.5
#define RANDMAX  5

/* The default maximum number of sweeps (a count) is set to... */

#define MAXSWEEPS  1000

#ifdef DEBUG
#include <math.h>
#endif
#include <stdlib.h>

/* Type definitions follow. */

#include "net_type.h"

/* declarations of external functions. */

#include "show.h"
#include "input.h"
#include "error.h"
#include "fishNET.h"

/* Below defines the transfer function of a neuron. */

#define TRANSFER(a)  (1.0 / (1.0 + exp(-(a))))

/* Allocate the correct amount of space for the network. */

LAYER *
allocate(parameter)
QUESTION *parameter;
{
    register int    i;
    register int    j;
    register int    k;
    long            seed;

    LAYER   *FIRST;
    LAYER   *network;
    NEURON  *neuron;
    WEIGHT  *weight;

    time (&seed);
    srand ((unsigned) seed);

    if ( (FIRST = (LAYER *) malloc(parameter->layers * sizeof(LAYER)))
        == NULL )
        error_m("LAYER", "allocate");

    for (
        i = 0, network = FIRST;
        i < parameter->layers;
        i++, network++
    ){
        if ( (network->layer =
            (NEURON *) malloc(parameter->per_layer[i] * sizeof(NEURON)))
            == NULL )
            error_m("NEURON", "allocate");

        for (
            j = 0, neuron = network->layer;
            j < parameter->per_layer[i];
            j++, neuron++
        ){
            neuron->ty = 0.0;
            if ( i < parameter->layers - 1 )
            {
                if ( (neuron->weight =
                    (WEIGHT *) malloc(parameter->per_layer[i+1] * sizeof(WEIGHT)))
                    == NULL )
                    error_m("WEIGHT", "allocate");

                for (
                    k = 0, weight = neuron->weight;
                    k < parameter->per_layer[i+1];
                    k++, weight++
                ){
                    weight->w = ((double) rand() / RANDINT) - RANDDEN;
                    weight->hold_delta_w = 0.0;
                    weight->old_dw = 0.0;
                }
            }
            else
                neuron->weight = NULL;
        }
    }

    return(FIRST);
}

/* Forward propagation: compute the output of the network for one input case. */

void
operate(network, input, parameter)
LAYER    *network;
I_S      *input;
QUESTION *parameter;
{
    I_S     *optu;
    NEURON  *neuron;
    NEURON  *prev_neuron;
    LAYER   *present_net;

    register int    i;
    register int    j;
    register int    k;

    double  sum;

    /* Handle the bottom layer first. */

    for (
        i = 0, optu = input, neuron = network->layer;
        i < parameter->per_layer[0];
        i++, optu++, neuron++
    ){
        neuron->ty = TRANSFER(optu->x);
    }

    /* Deal with the rest of the network. */

    for (
        i = 1, present_net = network + 1;
        i < parameter->layers;
        i++, present_net++
    ){
        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[i];
            j++, neuron++
        ){
            sum = 0.0;

            for (
                k = 0, prev_neuron = (present_net - 1)->layer;
                k < parameter->per_layer[i-1];
                k++, prev_neuron++
            ){
                sum += prev_neuron->ty * (prev_neuron->weight + j)->w;
            }

            neuron->ty = TRANSFER(sum);
        }
    }
}

/* The routine below executes the back-propagation algorithm as described in
 * the thesis. It calculates the weights. It is an extrapolation of the
 * two functions "back_propagate" and "apply_delta_w" described in Chapter 2.
 */

int
back_propagate_apply_delta_w(network, expected, parameter, t)
LAYER    *network;
I_S      *expected;
QUESTION *parameter;
int       t;
{
    register int    i;
    register int    j;
    register int    k;
    register int    l;

    LAYER   *present_net;
    LAYER   *prev_layer_ptr;
    NEURON  *layer;
    NEURON  *prev_layer;
    NEURON  *neuron;
    NEURON  *prev_source;
    WEIGHT  *weight;

    double  *dv_dc;
    double  *dd_dptr;
    double  alpha;
    double  epsilon;
    double  delta_w;
    double  sum;

    int     error = 0;

    /* Set up 'constants' to use later in the routine. */

#ifdef DEBUG
    /* This is a really ugly way of getting around an error in
     * GE. There is no way to read the underflow
     * exception, the _control87 doesn't work, so I will look
     * out for it myself, by letting nil
     * any values for a of less than .700 go to 0.
     */
    if ( t < 700 )
        alpha = parameter->alpha + exp(-fabs((double)t) * 4);
    else
        alpha = 0.0;
#else
    /* a normal, nothing (and probably more expensive) compiler */
    alpha = parameter->alpha + exp(-fabs((double)t) * 4);
#endif

    epsilon = parameter->epsilon;

    /* Handle top layer first. */

    if ( (dv_dc = (double *) malloc(sizeof(double) *
        parameter->per_layer[parameter->layers - 1]))
        == NULL )
        error_m("double", "back_propagate_apply_delta_w");

    layer      = (network + parameter->layers - 1)->layer;
    prev_layer = (network + parameter->layers - 2)->layer;

    for (
        i = 0, neuron = layer;
        i < parameter->per_layer[parameter->layers - 1];
        i++, neuron++
    ){
        dv_dc[i] = (expected[i].x - neuron->ty);

        if ( fabs(dv_dc[i]) > 0.2 )
            error++;

        for (
            j = 0, prev_source = prev_layer;
            j < parameter->per_layer[parameter->layers - 2];
            j++, prev_source++
        ){
            weight = prev_source->weight + i;
            delta_w = epsilon * dv_dc[i] *
                prev_source->ty *
                (neuron->ty * (1.0 - neuron->ty));

            weight->hold_delta_w += delta_w;

            /* Decay a little. */
            weight->w += alpha * weight->old_dw;
            weight->w -= 0.000;

            weight->hold_delta_w += delta_w;
            weight->old_dw = delta_w;
        }
    }

    /* Deal with the rest of the network. */

    for ( l = parameter->layers - 3; l >= 0; l-- )
    {
        if ( (dd_dptr = (double *) malloc(sizeof(double) *
            parameter->per_layer[l + 1]))
            == NULL )
            error_m("double", "back_propagate_apply_delta_w");

        present_net    = network + l + 1;
        prev_layer_ptr = network + l;

        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[l + 1];
            j++, neuron++
        ){
            sum = 0.0;
            for ( k = 0; k < parameter->per_layer[l + 2]; k++ )
                sum += (neuron->weight + k)->w * dv_dc[k];
            dd_dptr[j] = sum * neuron->ty * (1.0 - neuron->ty);
        }

        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[l + 1];
            j++, neuron++
        ){
            for (
                k = 0, prev_source = prev_layer_ptr->layer;
                k < parameter->per_layer[l];
                k++, prev_source++
            ){
                weight = prev_source->weight + j;
                delta_w = epsilon * dd_dptr[j] * prev_source->ty;
                weight->hold_delta_w += delta_w;
                weight->w += alpha * weight->old_dw;
                weight->old_dw = delta_w;
            }
        }

        free(dv_dc);
        dv_dc = dd_dptr;
    }

    free(dv_dc);

    return(error);
}

/* The routine below executes the back-propagation algorithm as described in
 * This the first algorithm in Chapter 2, and is the default.
 */

int
back_propagate(network, expected, parameter)
LAYER    *network;
I_S      *expected;
QUESTION *parameter;
{
    register int    i;
    register int    j;
    register int    k;
    register int    l;

    LAYER   *present_net;
    LAYER   *prev_layer_ptr;
    NEURON  *layer;
    NEURON  *prev_layer;
    NEURON  *neuron;
    NEURON  *prev_source;
    WEIGHT  *weight;

    double  *dv_dc;
    double  *dd_dptr;
    double  delta_w;
    double  sum;

    int     error = 0;

    /* Handle top layer first. */

    if ( (dv_dc = (double *) malloc(sizeof(double) *
        parameter->per_layer[parameter->layers - 1]))
        == NULL )
        error_m("double", "back_propagate");

    layer      = (network + parameter->layers - 1)->layer;
    prev_layer = (network + parameter->layers - 2)->layer;

    for (
        i = 0, neuron = layer;
        i < parameter->per_layer[parameter->layers - 1];
        i++, neuron++
    ){
        dv_dc[i] = (expected[i].x - neuron->ty);

        if ( fabs(dv_dc[i]) > 0.2 )
            error++;

        for (
            j = 0, prev_source = prev_layer;
            j < parameter->per_layer[parameter->layers - 2];
            j++, prev_source++
        ){
            weight = prev_source->weight + i;
            weight->hold_delta_w +=
                parameter->epsilon * dv_dc[i] *
                prev_source->ty *
                (neuron->ty * (1.0 - neuron->ty));
        }
    }

    /* Deal with the rest of the network. */

    for ( l = parameter->layers - 3; l >= 0; l-- )
    {
        if ( (dd_dptr = (double *) malloc(sizeof(double) *
            parameter->per_layer[l + 1]))
            == NULL )
            error_m("double", "back_propagate");

        present_net    = network + l + 1;
        prev_layer_ptr = network + l;

        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[l + 1];
            j++, neuron++
        ){
            sum = 0.0;
            for ( k = 0; k < parameter->per_layer[l + 2]; k++ )
                sum += (neuron->weight + k)->w * dv_dc[k];
            dd_dptr[j] = sum * neuron->ty * (1.0 - neuron->ty);
        }

        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[l + 1];
            j++, neuron++
        ){
            for (
                k = 0, prev_source = prev_layer_ptr->layer;
                k < parameter->per_layer[l];
                k++, prev_source++
            ){
                weight = prev_source->weight + j;
                weight->hold_delta_w +=
                    parameter->epsilon * dd_dptr[j] * prev_source->ty;
            }
        }

        free(dv_dc);
        dv_dc = dd_dptr;
    }

    free(dv_dc);

    return(error);
}

/* The function below modifies the weights according to the algorithm given
 * in the thesis, records the error for each weight, and returns a
 * cumulative total of the errors.
 */

int
apply_delta_w(network, parameter, t)
LAYER    *network;
QUESTION *parameter;
int       t;
{
    register int    i;
    register int    j;
    register int    k;

    LAYER   *present_net;
    NEURON  *neuron;
    WEIGHT  *weight;

    double  alpha;

    /* Perform exponential decay. */

#ifdef DEBUG
    if ( t < 700 )
        alpha = parameter->alpha + exp(-fabs((double)t) * 4);
    else
        alpha = 0.0;
#else
    alpha = parameter->alpha + exp(-fabs((double)t) * 4);
#endif

    for (
        i = 0, present_net = network;
        i < parameter->layers - 1;
        i++, present_net++
    ){
        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[i];
            j++, neuron++
        ){
            for (
                k = 0, weight = neuron->weight;
                k < parameter->per_layer[i+1];
                k++, weight++
            ){
                weight->w += weight->hold_delta_w +
                    (alpha * weight->old_dw);
                weight->old_dw = weight->hold_delta_w;
                weight->hold_delta_w = 0.0;
            }
        }
    }

    return(0);
}

/* The general function that teaches the network. */

/* External variable definitions for use by interrupt_handler during ctrl-c. */

LAYER    *ext_network;
QUESTION *ext_parameter;
int       ext_b_ptr = 0;

int
teach(network, parameter, start_time)
LAYER    *network;
QUESTION *parameter;
int       start_time;
{
    int     i;
    int     t = start_time;

    int     error;
    int     threshold;

    I_S     *input;
    I_S     *expected;
    I_S     *input_case;
    I_S     *expected_case;

    /* Define external variables to be the same as the internal ones. */

    ext_network   = network;
    ext_parameter = parameter;
    ext_b_ptr     = 0;

    input    = get_data(parameter->sample_in);
    expected = get_data(parameter->sample_out);

    if ( !VERBOSE )
    {
        show_data(input, parameter->sample_in);
        show_data(expected, parameter->sample_out);
    }

    if ( parameter->test_sweeps > 0 )
        threshold = parameter->test_sweeps;
    else
        threshold = MAXSWEEPS;

    if ( !QUIET )
        printf("selecting max number of learning iterations to %d.\n",
            threshold);

    /* For efficiency's sake (there would need to be thousands of
     * iterations on the side of speed instead of size --
     * efficiency and expect huge chunks of code in DELTA_MODE if necessary).
     * The global variable DELTA_MODE is necessary.
     */

    if ( DELTA_MODE == ALL ) /* do back_propagate over all cases then apply_delta_w */
    {
        do
        {
            error = 0;

            for (
                i = 0,
                input_case    = input,
                expected_case = expected;
                i < parameter->sample_in_cases;
                i++,
                input_case    += parameter->per_layer[0],
                expected_case += parameter->per_layer[parameter->layers - 1]
            ){
                operate(network, input_case, parameter);
                error += back_propagate(network,
                    expected_case,
                    parameter);

                if ( !VERBOSE )
                {
                    show_case(
                        expected_case,
                        parameter->sample_in_cases,
                        parameter->net_width
                    );
                    show_top_layer(network, parameter);
                    printf("\n");
                }
            }

            apply_delta_w(network, parameter, t);

            t++;
            if ( !QUIET )
                printf("+ = %d. error = %d.\n", t, error);

            parameter->test_sweeps = t;

        } while ( error > 0 && t < threshold );
    }
    else /* apply delta_w to calculate delta_w's every I/O case */
    {
        do
        {
            error = 0;

            for (
                i = 0,
                input_case    = input,
                expected_case = expected;
                i < parameter->sample_in_cases;
                i++,
                input_case    += parameter->per_layer[0],
                expected_case += parameter->per_layer[parameter->layers - 1]
            ){
                operate(network, input_case, parameter);
                error += back_propagate_apply_delta_w(
                    network,
                    expected_case,
                    parameter,
                    t);

                if ( !VERBOSE )
                {
                    show_case(
                        expected_case,
                        parameter->sample_in_cases,
                        parameter->net_width
                    );
                    show_top_layer(network, parameter);
                    printf("\n");
                }

                t++;
                if ( !QUIET )
                    printf("+ = %d. error = %d.\n", t, error);
            }

            parameter->test_sweeps = t;

        } while ( error > 0 && t < threshold );
    }

    if ( !QUIET )
        print_network(network, parameter);

    return(1);
}
