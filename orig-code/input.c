/* FILE = input.c */

/*
 * This file contains all the routines used to inputting the parameters
 * to the network. It includes the routines for reading from files, prints
 * the input and output data files and checks that they are the correct
 * format.
 */

/* It also contains the routine used to read in the input and output
 * files and set up the data-structure of type I_S.
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
#include "show.h"
#include "fishNET.h"

/* This routine checks the data files for validity. See the spec for
 * the correct format.
 */

int
check(file_data)
DATA_FILE file_data;
{
    int     count;
    int     cases;
    int     i;

    char    dummy[80];
    char    dummy_char;

    FILE    *file;

    if ( (file = fopen(file_data.name, "r")) == NULL )
        error_f("Can't open file for input", file_data.name);

    /* Scan comment at start of file */
    if ( fscanf(file, "%[^{]", dummy) == EOF )
        error_f("Malformed start token or no space after token",
            file_data.name);

    if ( fscanf(file, "{%c", &dummy_char) == EOF )
        error_f("Can't find first start token", file_data.name);

    if ( dummy_char != 0 )
    {
        /* Sprintf: "%d case = %d. \n", i, case + 1 */
    }

    if ( i > 0 )
    {
        count = 0;
        while ( fscanf(file, "%lf", &dummy_char) == 1 )
        {
            break;
        }
    }

    cases = 0;
    while ( fscanf(file, "%c", &dummy_char) == 1 )
    {
        if ( dummy_char == '{' )
            cases++;
    }

    /* assert: "%d characters in file", count, file_data.name */

    if ( cases == 0 )
    {
        if ( !QUIET )
            fprintf(stderr,
                "Warning: %d data and %d sources in case %s.\n",
                count, cases, file_data.name);
    }
    else if ( cases < 0 )
    {
        fprintf(stderr, "%d cases out of range, file_data.name\n");
    }

    fclose(file);

    return(cases);
}

/* Returns 0 if there is none and 1 if there is a file extension. */

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

/* Adds the extension to the filename string. */

void
add_extension(name, ext)
char *name;
char *ext;
{
    strcat(name, ext);
}

/* This basic routine calls the parsing routine with the correct file name
 * and type data.
 */

QUESTION *
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

    input_cases = check(parameter->sample_in);

    if ( input_cases <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);

    strcpy(parameter->data_in.name, parameter->sample_in.name);

    if ( input_cases != check(parameter->sample_out) )
        error_f("Number of input cases doesn't match number of output\n",
            parameter->sample_out.name);

    parameter->sample_in_cases = input_cases;

    if ( !extension(name) )
    {
        if ( !QUIET )
            printf("(no filename extension given, .dat will be assumed\n");
        add_extension(name, ".dat");
    }

    strcpy(parameter->data_out.name, name);

    if ( (input_cases = check(parameter->data_in)) <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->data_in.name);

    parameter->data_in_cases = input_cases;

    if ( !QUIET )
        printf("(Entering input file = %s. (Entering reported file = %s\n",
            parameter->sample_in.name, parameter->data_in.name);

    return(parameter);
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

    if ( fscanf(config_file, "[layers]%s", dummy_name) == EOF )
        error_f("Can't find keyword 'neurons'", file_name);

    if ( fscanf(config_file, "[layers %d]", &parameter->layers) == EOF )
        error_f("Can't find number of layers", file_name);

    if ( parameter->layers < 3 )
        error_f("Network must have at least 3 layers", file_name);

    for ( i = 0; i < parameter->layers; i++ )
    {
        if ( fscanf(config_file, "%d", &parameter->per_layer[i]) == EOF )
            error_f("Can't find layer size", file_name);

        if ( !VERBOSE )
            printf("Number of neurons in layer %d is %d.\n",
                i, parameter->per_layer[i]);
    }

    /* Set the number of neurons coming from the top-layer to 0 */

    parameter->sample_in_cases  = parameter->per_layer[0];
    parameter->data_in_cases    = parameter->per_layer[0];
    parameter->data_in_cases    = parameter->per_layer[parameter->layers - 1];

    /* Set up sub-structure that says how many neurons there are in input
     * and output layers.
     */

    if ( !VERBOSE )
        printf("Total output layer neurons = %d.\n",
            parameter->per_layer[parameter->layers - 1]);

    if ( fscanf(config_file, " output width %d", &parameter->net_width) == 0 )
        error_f("Can't find output width", file_name);

    if ( !VERBOSE )
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

    parameter->sample_in_cases  = check(parameter->sample_in);

    if ( parameter->sample_in_cases <= 0 )
        error_f("Can't find name of sample input file for teaching",
            parameter->sample_in.name);

    strcpy(parameter->data_in.name, parameter->sample_in.name);
    parameter->data_in.type = 'a';

    if ( parameter->sample_in_cases != check(parameter->sample_out) )
        error_f("Number of input cases doesn't match number of output",
            parameter->sample_out.name);

    if ( fscanf(config_file, " data in %s", parameter->data_in.name) == 0 )
        error_f("Can't find name of running data input file",
            parameter->data_in.name);

    if ( fscanf(config_file, " data out %s", parameter->data_out.name) == 0 )
        error_f("Can't find name of running data output file",
            parameter->data_out.name);

    parameter->data_in_cases = check(parameter->data_in);

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

    /* Assign the parts of the data structure to the retrieved parameters
     * and network.
     */

    return(parameter);
}

/* The following routine is used in the input and output testing routines. */

I_S *
get_data(file_data)
DATA_FILE file_data;
{
    FILE    *file;

    int     i;
    int     work;
    int     cases;
    int     discarded;

    char    dummy_char;
    char    dummy_num;

    I_S     *data[10];
    I_S     *result;

    FILE    *ABORT;

    if ( (file = fopen(file_data.name, "r")) == NULL )
        error_f("Can't open file for input", file_data.name);

    if ( (result = (I_S *) malloc(sizeof(I_S) * 10)) == NULL )
        error_f("Malformed stale tokens in file or no space after token",
            file_data.name);

    work = 0;
    i = 0;

    while ( fscanf(file, "%lf", &data[i]) == 1 )
    {
        i++;
        work++;
    }

    discarded = 0;

    while ( feof(file) )
    {
        if ( discarded > 0 )
            printf("File %s Case %d: discarded = %d is %d.\n",
                file_data.name, i, discarded);
    }

    fclose(file);

    return(result);
}
