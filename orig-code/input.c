/* FILE = input.c */

/*
 * This file contains all the routines used to inputting the parameters
 * to the network. It includes the routines for reading files, print_cases
 * the input and output files and checks that they are the correct format.
 */

/* It also contains the routine used to read in the input and output
 * data into the data structures of type I_S.
 */

#ifndef DEBUG
#include <math.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Type declarations. */

#include "net_type.h"

/* External declarations. */

#include "error.h"
#include "learn.h"
#include "show.h"
#include "fishNET.h"

/* This routine checks the data files for validity. See the spec for
 * the correct format.
 */

int
parse(file_data)
DATA_FILE file_data;
{
    int     count;
    int     cases;
    int     i;
    int     total_neurons;

    char    dummy[80];
    char    dummy_char;
    double  dummy_num;

    FILE    *file;

    if ( (file = fopen(file_data.name, "r")) == NULL )
        error_f("Can't open file for input", file_data.name);

    /* Scan comment at start of file */
    if ( fscanf(file, "%[^{]", dummy) == EOF )
        error_f("Can't find first start token", file_data.name);

    if ( !QUIET )
        printf("Values for file %s\n", file_data.name);

    cases = 0;
    total_neurons = 0;

    while ( fscanf(file, " %c", &dummy_char) == 1 )
    {
        if ( dummy_char != '{' )
            break;

        count = 0;
        while ( fscanf(file, "%lf", &dummy_num) == 1 )
        {
            if ( dummy_num > 1.0 || dummy_num < 0.0 )
            {
                fprintf(stderr, "I/O case = %d.\n", cases);
                error_f("Input out of range", file_data.name);
            }
            count++;
        }

        if ( count == 0 )
        {
            fprintf(stderr, "%d case = %d.\n", i, cases + 1);
            error_f("Not enough characters in file", file_data.name);
        }

        if ( count > file_data.neurons )
        {
            if ( !QUIET )
                fprintf(stderr,
                    "Warning: %d data and %d sources in case %d.\n",
                    count, file_data.neurons, cases);
        }
        else if ( count < file_data.neurons )
        {
            fprintf(stderr, "I/O case = %d.\n", cases);
            error_f("Not enough data in file", file_data.name);
        }

        cases++;
        total_neurons += count;
    }

    if ( !QUIET )
        printf("\nTotal data = %d, total I/O cases = %d.\n",
            total_neurons, cases);

    fclose(file);

    return(cases);
}

/* Returns 0 if there is none and 1 if there is a file extension. */

#ifdef MSDOS
int
extension(name)
char *name;
{
    char    drive[4];
    char    dir[21];
    char    fname[8];
    char    ext[4];

    _splitpath(name, drive, dir, fname, ext);

    if ( strncmp(ext, "\0", 1) == 0 )
        return 0;
    else
        return 1;
}
#else
int
extension(name)
char *name;
{
    char *dot   = strrchr(name, '.');
    char *slash = strrchr(name, '/');

    if ( dot && (slash == NULL || dot > slash) )
        return 1;
    return 0;
}
#endif

/* Adds the extension to the filename string. */

#ifdef MSDOS
void
add_extension(name, ext)
char *name;
char *ext;
{
    char    drive[4];
    char    dir[21];
    char    fname[8];
    char    fext[4];

    _splitpath(name, drive, dir, fname, fext);
    _makepath(name, drive, dir, fname, ext);
}
#else
void
add_extension(name, ext)
char *name;
char *ext;
{
    strcat(name, ext);
}
#endif

/* This basic routine calls the parsing routine with the correct file name
 * and type data.
 */

int
net_count(parameter)
QUESTION *parameter;
{
    char    type;
    char    name[50];

    int     input_cases;

    /* At some stage in the future, a format query will lie here.
     * However, at the moment, the input is only ASCII.
     */

    parameter->sample_in.type = 'a';

    printf("Enter the file name of the sample input file: ");
    scanf("%s", name);

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->sample_in.name, name);

    input_cases = parameter->sample_in.i_o_cases = parse(parameter->sample_in);

    if ( input_cases <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);

    /* At present, the only type implemented is ASCII, or 'a'. */

    parameter->sample_in.type = 'a';

    printf("Enter the file name of the sample output file: ");
    scanf("%s", name);

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->sample_out.name, name);

    if ( input_cases != parse(parameter->sample_out) )
        error_f("Number of input cases doesn't match number of output",
            parameter->sample_out.name);

    parameter->sample_in_cases = input_cases;
    parameter->sample_out.type = 'a';

    printf("Enter the file name of the running data input file: ");
    scanf("%s", name);

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->data_in.name, name);

    input_cases = parameter->data_in.i_o_cases = parse(parameter->data_in);

    parameter->data_in_cases = input_cases;
    parameter->data_in.type = 'a';

    printf("Enter the file name of the running data output file: ");
    scanf("%s", name);

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->data_out.name, name);

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->save_name, name);

    return(1);
}

/* input_parameters reads all information necessary from a *configuration
 * file*. See the thesis for format.
 * Returns a pointer to the final structure which contains all the answers.
 */

QUESTION *
input_parameters(file_name)
char *file_name;
{
    FILE    *config_file;

    int     i;
    int     input_cases;

    char    dummy_char;
    char    dummy_name[50];

    QUESTION    *parameter;

    parameter = (QUESTION *) malloc(sizeof(QUESTION));
    if ( parameter == NULL )
        error_m("QUESTION", "input_parameters");

    if ( (config_file = fopen(file_name, "r")) == NULL )
        error_f("Can't open configuration file", file_name);

    /* Scan comment at start of file */
    if ( fscanf(config_file, "%[^{]", dummy_name) == EOF )
        error_f("Can't find start token", file_name);

    if ( fscanf(config_file, "{%c", &dummy_char) == EOF )
        error_f("Can't find 'dummies'", file_name);

    if ( fscanf(config_file, " layers %d", &parameter->layers) == EOF )
        error_f("Can't find number of layers", file_name);

    if ( parameter->layers < 3 )
        error_f("Network must have at least 3 layers", file_name);

    if ( VERBOSE )
        printf("Number of layers = %d.\n", parameter->layers);

    for ( i = 0; i < parameter->layers; i++ )
    {
        if ( fscanf(config_file, "%d", &parameter->per_layer[i]) == EOF )
            error_f("Can't find layer size", file_name);

        if ( VERBOSE )
            printf("Number of neurons in layer %d is %d.\n",
                i, parameter->per_layer[i]);
    }

    /* Set the number of neurons coming from the top-layer to 0 */

    parameter->per_layer[parameter->layers] = 0;

    /* Set up sub-structure that says how many neurons there are in input
     * and output layers.
     */

    parameter->sample_in.neurons  = parameter->per_layer[0];
    parameter->data_in.neurons    = parameter->per_layer[0];
    parameter->sample_out.neurons = parameter->per_layer[parameter->layers - 1];
    parameter->data_out.neurons   = parameter->per_layer[parameter->layers - 1];

    if ( VERBOSE )
        printf("Total output layer neurons = %d.\n",
            parameter->per_layer[parameter->layers - 1]);

    if ( fscanf(config_file, " output width %d", &parameter->net_width) == 0 )
        error_f("Can't find output width", file_name);

    if ( VERBOSE )
        printf("network output width: %d\n", parameter->net_width);

    if ( fscanf(config_file, " alpha %lf", &parameter->alpha) == 0 )
        error_f("Can't find alpha", file_name);

    if ( fscanf(config_file, " epsilon %lf", &parameter->epsilon) == 0 )
        error_f("Can't find epsilon", file_name);

    if ( fscanf(config_file, " repsilon %lf", &parameter->repsilon) == 0 )
        error_f("Can't find repsilon", file_name);

    printf("Do you wish to specify a maximum number of sweeps? (y/n) ");
    scanf("%c", &dummy_char);

    if ( dummy_char == 'y' || dummy_char == 'Y' )
        fscanf(config_file, "%d", &parameter->test_sweeps);
    else
        parameter->test_sweeps = 0;

    if ( fscanf(config_file, " sample in %s", parameter->sample_in.name) == 0 )
        error_f("Can't find sample input file", file_name);
    parameter->sample_in.type = 'a';

    if ( fscanf(config_file, " sample out %s", parameter->sample_out.name) == 0 )
        error_f("Can't find sample output file", file_name);
    parameter->sample_out.type = 'a';

    if ( !extension(parameter->sample_in.name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(parameter->sample_in.name, ".dat");
    }

    if ( !extension(parameter->sample_out.name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(parameter->sample_out.name, ".dat");
    }

    parameter->sample_in_cases = parse(parameter->sample_in);

    if ( parameter->sample_in_cases <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);

    strcpy(parameter->data_in.name, parameter->sample_in.name);
    parameter->data_in.type = 'a';

    if ( parameter->sample_in_cases != parse(parameter->sample_out) )
        error_f("Number of input cases doesn't match number of output",
            parameter->sample_out.name);

    if ( fscanf(config_file, " data in %s", parameter->data_in.name) == 0 )
        error_f("Can't find name of running data input file",
            parameter->data_in.name);

    if ( fscanf(config_file, " data out %s", parameter->data_out.name) == 0 )
        error_f("Can't find name of running data output file",
            parameter->data_out.name);

    parameter->data_in_cases = parse(parameter->data_in);

    if ( parameter->data_in_cases <= 0 )
        error_f("Can't find name of running data output file",
            parameter->data_out.name);

    strncpy(parameter->data_out.name, parameter->data_in.name, 50);

    if ( !QUIET )
    {
        printf("(Entering input file = %s. (Entering expected file = %s.\n",
            parameter->sample_in.name, parameter->sample_out.name);
        printf("(Execution input file = %s.\n", parameter->data_in.name);
        printf("(Network will be saved to file '%s'.\n", parameter->save_name);
    }

    if ( fscanf(config_file, " start time %d", &input_cases) == 0 )
        error_f("Can't find start time", file_name);

    if ( fscanf(config_file, " learn time %d", &input_cases) == 0 )
        error_f("Can't find learn time", file_name);

    fclose(config_file);

    /* Assign the parts of the data structure to the retrieved parameters
     * and network.
     */

    return(parameter);
}

/* It initialises a network saved to disk, and puts it in memory. */

LINK_NODE *
load_network_and_parameters(save_file)
char    *save_file;
{
    int     i;
    int     k;

    char    dummy[50];
    char    dummy_char;

    LAYER   *current;
    NEURON  *neuron;
    WEIGHT  *weight;

    FILE    *saved;

    int     input_cases;

    LINK_NODE   *node;
    QUESTION    *parameter;

    if ( (node = (LINK_NODE *) malloc(sizeof(LINK_NODE))) == NULL )
        error_m("LOAD_NODE", "load_network_and_parameters");

    if ( (parameter = (QUESTION *) malloc(sizeof(QUESTION))) == NULL )
        error_m("QUESTION", "load_network_and_parameters");

    strcpy(parameter->save_name, save_file);

    if ( (saved = fopen(parameter->save_name, "r")) == NULL )
        error_f("Can't load network", parameter->save_name);

    /* Scan comment at start of file */
    if ( fscanf(saved, "%[^{]", dummy) == EOF )
        error_f("Malformed start token", parameter->save_name);

    if ( fscanf(saved, "{ layers %d", &parameter->layers) == EOF )
        error_f("Can't find number of layers", parameter->save_name);

    if ( parameter->layers < 3 )
        error_f("Network must have at least 3 layers", parameter->save_name);

    if ( VERBOSE )
        printf("Number of layers = %d.\n", parameter->layers);

    for ( i = 0; i < parameter->layers; i++ )
    {
        if ( fscanf(saved, "%d", &parameter->per_layer[i]) != 1 )
        {
            fprintf(stderr, "layer %d neurons = %d.\n", i, parameter->per_layer[i]);
            error_f("Can't find layer size", parameter->save_name);
        }

        if ( VERBOSE )
            printf("Player %d neurons = %d.\n", i + 1, parameter->per_layer[i]);
    }

    /* Set the number of neurons coming from the top-layer to 0 */

    parameter->per_layer[parameter->layers] = 0;

    /* Set up sub-structure that says how many neurons there are in input
     * and output layers.
     */

    parameter->sample_in.neurons  = parameter->per_layer[0];
    parameter->data_in.neurons    = parameter->per_layer[0];
    parameter->sample_out.neurons = parameter->per_layer[parameter->layers - 1];
    parameter->data_out.neurons   = parameter->per_layer[parameter->layers - 1];

    if ( VERBOSE )
        printf("Total output layer neurons = %d.\n",
            parameter->per_layer[parameter->layers - 1]);

    if ( fscanf(saved, " output width %d", &parameter->net_width) != 1 )
        error_f("Can't find output width", parameter->save_name);

    if ( VERBOSE )
        printf("network output width: %d\n", parameter->net_width);

    if ( fscanf(saved, " alpha %lf", &parameter->alpha) != 1 )
        error_f("Can't find alpha", parameter->save_name);

    if ( fscanf(saved, " epsilon %lf", &parameter->epsilon) != 1 )
        error_f("Can't find epsilon", parameter->save_name);

    if ( fscanf(saved, " repsilon %lf", &parameter->repsilon) != 1 )
        error_f("Can't find repsilon", parameter->save_name);

    printf("Do you wish to specify a maximum number of sweeps? (y/n) ");
    scanf("%c", &dummy_char);

    if ( dummy_char == 'y' || dummy_char == 'Y' )
    {
        printf("(Enter maximum number of sweeps: ");
        scanf("%d", &parameter->test_sweeps);
    }
    else
        parameter->test_sweeps = 0;

    /* Allocate the network and read in the saved weights */

    node->network = allocate(parameter);

    current = node->network;

    for ( i = 0; i < parameter->layers; i++ )
    {
        if ( i > 0 )
        {
            neuron = current->layer;

            for ( k = 0; k < parameter->per_layer[i]; k++, neuron++ )
            {
                weight = neuron->weight;

                while ( weight < neuron->weight + parameter->per_layer[i - 1] )
                {
                    if ( fscanf(saved, "%lf", &weight->w) == EOF )
                    {
                        printf("layer %d, weight %d.\n", i, k);
                        error_f("Can't find weight", parameter->save_name);
                    }

                    weight->old_dw = 0.0;
                    weight->hold_delta_w = 0.0;

                    if ( VERBOSE )
                        printf("%lf\n", weight->w);

                    weight++;
                }
            }
        }

        current++;
    }

    if ( fscanf(saved, " sample in %s", parameter->sample_in.name) != 1 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);
    parameter->sample_in.type = 'a';

    if ( fscanf(saved, " sample out %s", parameter->sample_out.name) != 1 )
        error_f("Can't find name of sample output file for teaching",
            parameter->sample_out.name);
    parameter->sample_out.type = 'a';

    parameter->sample_in_cases = parse(parameter->sample_in);

    if ( parameter->sample_in_cases <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);

    if ( parameter->sample_in_cases != parse(parameter->sample_out) )
        error_f("Number of input cases doesn't match number of output",
            parameter->sample_out.name);

    if ( fscanf(saved, " data in %s", parameter->data_in.name) != 1 )
        error_f("Can't find name of execute input file",
            parameter->data_in.name);
    parameter->data_in.type = 'a';

    parameter->data_in_cases = parse(parameter->data_in);

    if ( parameter->data_in_cases <= 0 )
        error_f("Can't find name of execute input file",
            parameter->data_in.name);

    if ( fscanf(saved, " data out %s", parameter->data_out.name) != 1 )
        error_f("Can't find name of execute output file",
            parameter->data_out.name);

    strcpy(parameter->data_out.name, parameter->data_in.name);

    if ( !QUIET )
    {
        printf(
            "\nIncoming input file = %s (incoming expected file = %s\n",
            parameter->sample_in.name, parameter->sample_out.name);
        printf(
            "\nExecution input file = %s.\n",
            parameter->data_in.name);
    }

    if ( fscanf(saved, " start time %d", &node->start_time) != 1 )
        error_f("Can't find start time", parameter->save_name);

    fclose(saved);

    /* Assign the parts of the data structure to the retrieved parameters
     * and network.
     */

    node->parameter = parameter;

    return(node);
}

/* The following routine is used in the input and output learning routines. */

I_S *
get_data(file_data)
DATA_FILE file_data;
{
    int     h;
    int     i;
    int     k;
    int     discarded;

    double  dummy_num;

    char    dummy[80];
    char    dummy_char;

    I_S     *i_s_n;
    I_S     *work;

    FILE    *file;

    if ( (file = fopen(file_data.name, "r")) == NULL )
        error_f("Can't open file for input", file_data.name);

    /* Scan comment at start of file */
    if ( fscanf(file, "%[^{]", dummy) == EOF )
        error_f("Can't find first start token", file_data.name);

    if ( (i_s_n = (I_S *) malloc(file_data.i_o_cases * file_data.neurons * sizeof(I_S))) == NULL )
        error_m("I_S", "get_data");

    work = i_s_n;

    for ( i = 0; i < file_data.i_o_cases; i++ )
    {
        if ( fscanf(file, " %c", &dummy_char) != 1 || dummy_char != '{' )
            error_f("Can't get data item", file_data.name);

        for ( k = 0; k < file_data.neurons; k++, work++ )
        {
            if ( fscanf(file, "%lf", &dummy_num) != 1 )
                error_f("Can't get data item", file_data.name);
            work->x = dummy_num;
        }
    }

    discarded = 0;

    while ( fscanf(file, "%lf", &dummy_num) == 1 )
    {
        discarded++;
        if ( VERBOSE )
            printf("File %s: Case %d: discarded = %d: is %lf.\n",
                file_data.name, i, discarded, dummy_num);
    }

    fclose(file);

    return(i_s_n);
}
