/* FILE = show.c */

/*
 * This file contains display procedures to be used during debugging and
 * verification to display various pieces of data.
 */

#ifndef DEBUG
#include <math.h>
#endif
#include <stdlib.h>

/* The declarations follow. */

#include "net_type.h"

/* Use the external routines and variables. */

#include "error.h"
#include "fishNET.h"

/* Display the whole network on stdout. */

void
show_network(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    LAYER   *present_net;
    NEURON  *neuron;
    WEIGHT  *weight;

    int     i;
    int     j;
    int     k;

    FILE    *PORT;

    PORT = stdout;

    for (
        i = 0, present_net = network;
        i < parameter->layers;
        i++, present_net++
    ){
        fprintf(PORT, "Layer = %d.\n", i);

        for (
            j = 0, neuron = present_net->layer;
            j < parameter->per_layer[i];
            j++, neuron++
        ){
            fprintf(PORT, "  n = %d, out = %f, ", j, neuron->ty);

            if ( i < parameter->layers - 1 )
            {
                for (
                    k = 0, weight = neuron->weight;
                    k < parameter->per_layer[i+1];
                    k++, weight++
                ){
                    fprintf(PORT,
                        "%f(%f,%f,%f) ",
                        k, weight->w,
                        weight->hold_delta_w,
                        weight->old_dw);
                }
            }

            fprintf(PORT, "\n");
        }
    }

    fprintf(PORT, "\n");
}

/* Display the whole network on edges. */

void
show_network_edges(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    LAYER   *present_net;
    NEURON  *neuron;
    WEIGHT  *weight;

    int     i;
    int     j;
    int     k;

    FILE    *PORT;

    PORT = stdout;

    for (
        i = 0, present_net = network;
        i < parameter->layers - 1;
        i++, present_net++
    ){
        fprintf(PORT, "Layer = %d.\n", i);

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
                fprintf(PORT,
                    "%f(%f,%f,%f) ",
                    k, weight->w,
                    weight->hold_delta_w,
                    weight->old_dw);

                if ( (k+1) % parameter->net_width == 0 )
                    fprintf(PORT, "\n");
            }
        }
    }

    fprintf(PORT, "\n");
}

/* This routine displays all the data from the file relating to file_data. */

void
show_data(data, file_data)
I_S      *data;
DATA_FILE file_data;
{
    int     i;
    int     j;

    I_S     *work;

    FILE    *PORT;

    PORT = stdout;

    for (
        i = 0, work = data;
        i < file_data.name[0];   /* uses name as placeholder */
        i++
    ){
        fprintf(PORT, "Case = %d.\n", i);

        for (
            j = 0;
            j < file_data.name[0];
            j++, work++
        ){
            fprintf(PORT, "%f", work->x);
        }

        fprintf(PORT, "\n");
    }
}

/* This routine prints all the data from the file relating to file_data. */

void
print_data(data, file_data)
I_S      *data;
DATA_FILE file_data;
{
    int     i;
    int     j;

    I_S     *work;

    FILE    *PORT;

    PORT = stdout;

    for (
        i = 0, work = data;
        i < file_data.name[0];
        i++
    ){
        fprintf(PORT, "Case = %d.\n", i);

        for (
            j = 0;
            j < file_data.name[0];
            j++, work++
        ){
            fprintf(PORT, "%f", work->x);
        }

        fprintf(PORT, "\n");
    }
}

/* Displays the required I/O case on stdout. */

void
show_case(data, neurons, width)
I_S     *data;
int     neurons;
int     width;
{
    int     i;

    I_S     *work;

    FILE    *PORT;

    PORT = stdout;

    fprintf(PORT, "Data =\n");

    for ( i = 0, work = data; i < neurons; i++, work++ )
    {
        fprintf(PORT, "%f ", work->x);
        if ( (i+1) % width == 0 )
            fprintf(PORT, "\n");
    }

    fprintf(PORT, "\n");
}

/* Displays the required I/O case on edges. */

void
show_case_edges(data, neurons, width)
I_S     *data;
int     neurons;
int     width;
{
    int     i;

    I_S     *work;

    FILE    *PORT;

    PORT = stdout;

    fprintf(PORT, "Data =\n");

    for ( i = 0, work = data; i < neurons; i++, work++ )
    {
        fprintf(PORT, "%f ", work->x);
        if ( (i+1) % width == 0 )
            fprintf(PORT, "\n");
    }

    fprintf(PORT, "\n");
}

/* Prints the top (output) layer of neurons' output. */

void
show_top_layer(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    NEURON  *neuron;

    int     i;
    int     j;

    FILE    *PORT;

    PORT = stdout;

    i = 0, neuron = (network + parameter->layers - 1)->layer;

    for (
        j = 0, neuron = (network + parameter->layers - 1)->layer;
        j < parameter->per_layer[parameter->layers - 1];
        j++, neuron++
    ){
        fprintf(PORT, "%f ", neuron->ty);
        if ( (j+1) % parameter->net_width == 0 )
            fprintf(PORT, "\n");
    }

    fprintf(PORT, "\n");
}

/* Prints the top (control) layer of neurons' output. */

void
print_top_layer(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    NEURON  *neuron;

    int     j;

    FILE    *PORT;

    PORT = stdout;

    for (
        j = 0, neuron = (network + parameter->layers - 1)->layer;
        j < parameter->per_layer[parameter->layers - 1];
        j++, neuron++
    ){
        fprintf(PORT, "%f ", neuron->ty);
        if ( (j+1) % parameter->net_width == 0 )
            fprintf(PORT, "\n");
    }

    fprintf(PORT, "\n");
}

/* Prints the top (output) layer of neurons' output. */

void
save_top_layer(network, parameter, port)
LAYER    *network;
QUESTION *parameter;
FILE     *port;
{
    NEURON  *neuron;

    int     j;

    for (
        j = 0, neuron = (network + parameter->layers - 1)->layer;
        j < parameter->per_layer[parameter->layers - 1];
        j++, neuron++
    ){
        fprintf(port, "%f ", neuron->ty);
        if ( (j+1) % parameter->net_width == 0 )
            fprintf(port, "\n");
    }

    fprintf(port, "\n");
}

/* Save a network to disk. Format see the thesis. */

void
store_network(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    LAYER   *present_net;
    NEURON  *neuron;
    WEIGHT  *weight;

    int     i;
    int     j;
    int     k;

    FILE    *saved;

    if ( (saved = fopen(parameter->save_name, "w")) == NULL )
    {
        error_f("Can't open file for saving network",
            parameter->save_name);
    }

    fprintf(saved, "%s\n", parameter->network_comment);
    fprintf(saved, "[layers]\n");
    fprintf(saved, "%d\n", parameter->layers);

    for ( i = 0; i < parameter->layers; i++ )
        fprintf(saved, "%d\n", parameter->per_layer[i]);

    fprintf(saved, "\n%s\n", parameter->save_name);

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
                fprintf(saved, "%f\n", weight->w);
            }
        }
    }

    fprintf(saved, "output width%d\n", parameter->net_width);

    fclose(saved);
}

/* Show learning parameters and length of teaching time to disk. */

void
store_learnt_parameters(parameter)
QUESTION *parameter;
{
    int     i;

    FILE    *saved;

    if ( (saved = fopen(parameter->save_name, "w")) == NULL )
    {
        error_f("Can't open file for saving network",
            parameter->save_name);
    }

    fprintf(saved, "Name: %s\n", parameter->save_name);
    fprintf(saved, "Inputs=%d\n", parameter->per_layer[0]);
    fprintf(saved, "\n");

    for ( i = 0; i < parameter->layers; i++ )
        fprintf(saved, "%d\n", parameter->per_layer[i]);

    fprintf(saved, "\n");
    fprintf(saved, "alpha=%f\n", parameter->alpha);
    fprintf(saved, "epsilon=%f\n", parameter->epsilon);
    fprintf(saved, "repsilon=%f\n", parameter->repsilon);
    fprintf(saved, "sample in/file=%s\n", parameter->sample_in.name);
    fprintf(saved, "sample out/file=%s\n", parameter->sample_out.name);
    fprintf(saved, "max sweeps=%d\n", parameter->test_sweeps);
    fprintf(saved, "\n");

    fclose(saved);
}

/* Show learning parameters to show. */

void
print_network(network, parameter)
LAYER    *network;
QUESTION *parameter;
{
    int     i;
    FILE    *saved;

    saved = stdout;

    fprintf(saved, "Title: %s\n", parameter->network_comment);
    fprintf(saved, "Inputs=%d\n", parameter->per_layer[0]);

    for ( i = 0; i < parameter->layers; i++ )
        fprintf(saved, "%d ", parameter->per_layer[i]);

    fprintf(saved, "\n");
    fprintf(saved, "alpha=%f\n", parameter->alpha);
    fprintf(saved, "epsilon=%f\n", parameter->epsilon);
    fprintf(saved, "repsilon=%f\n", parameter->repsilon);
    fprintf(saved, "sample in/file=%s\n", parameter->sample_in.name);
    fprintf(saved, "sample out/file=%s\n", parameter->sample_out.name);
    fprintf(saved, "max sweeps=%d\n", parameter->test_sweeps);
    fprintf(saved, "\n");
}

/* The routine below displays a publication of the main network block from
 * the file fishNET.c.
 */

void
show_help()
{
    FILE    *help;

    char    line[80];

    if ( (help = fopen("help.hlp", "r")) == NULL )
        error_f("Can't open help file", "help.hlp");

    while ( fgets(line, 80, help) != NULL )
        printf("%s", line);

    fclose(help);
}
