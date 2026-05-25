/* FILE = fishNET.c */

/* Author: Scott Andrew MacGibbon */
/*
 * This code is written to accompany the written work:
 *
 *   Brain Simulation: Computation in Back-Propagation Neural Networks
 *
 * It is submitted as an undergraduate thesis at the University of Sydney
 * for the degree of Bachelor of Engineering.
 *
 * Copyright Scott Andrew MacGibbon, 23rd September 1988.
 */

/*
 * This is the first production version of the functions
 * necessary for the brain simulation code.
 */

/* Define title and version number */

#define PROG_NAME "fishNET"
#define PROG_TITLE    "Back-propagation neural network simulator."
#define PROG_SUBTITLE "Both cumulative and individual delta adds"
#define PROG_VERSION  "Version 1.0.92 (Both cumulative and individual delta adds)"

/* Include necessary follow */

#ifdef DEBUG
#include <math.h>
#endif
#include <stdlib.h>
#include <stdio.h>      /* This is to set up the exception handler */
#include <signal.h>     /* for the expressions */
#include <string.h>

/* Type declarations are included below */

#include "net_type.h"

/* Internal definitions of functions used. */

#include "show.h"
#include "input.h"
#include "learn.h"
#include "error.h"

int VERBOSE     = 0;
int QUIET       = 0;

int CONFIG_FILE = 0;
int KEY_FILE    = 0;

int SAVE_SET    = 0;

int DELTA_MODE  = 0;

#ifdef DEBUG
  FILE *stderr;
#endif

int main(argc, argv)
int argc;
char *argv[];
{
    char    *va;
    char    config_file[50];
    char    net_file[50];

    LINK_NODE   *net_and_param;
    QUESTION    *parameter;
    LAYER       *network;

    int     start_line;
    int     l;
    int     k;
    char    answer;

    int     file_name_read;
    FILE    *saved;

    I_S     *randoms;
    I_S     *runcase;

    /* Process parameters passed to the program. The parameters are listed below:
     *
     * Directives:
     *
     * Flag     Meaning
     * ------   -------
     *
     * VERBOSE (-v)
     * -v       verbose. Display all data used by the program as it is
     *          calculated or read in from files. Prints out to stderr
     *          the outputs from the network for the current I/O case.
     * -q       Quiet. Display nothing (suppresses all messages). Use
     *          of both will generate an error. This flag is mutually
     *          exclusive with the verbose flag.
     * default  Display rudimentary information about learning and network
     *          operation.
     *
     * PRELOADING CONFIGURATION FILE:
     *
     * -cclass  configuration file name. Preload the simulator with the
     *          definitions of network size and learning parameters in the
     *          format specified in the thesis.
     *          Default: (No file preloaded.)
     *
     * DECLARING TARGET NETWORK FILE:
     *
     * -nclass  network file name. Preload the simulator with the complete
     *          definitions of an operating (pre-taught) network. See the
     *          thesis for format.
     *          This directly is mutually exclusive with the configuration
     *          file directive. Use of both will generate an error.
     *          Default: (No file preloaded.)
     *
     * CONTINUING TO NETWORK FILE:
     *
     * -e       execute. Operate the preloaded network with the data files
     *          specified in the network file loaded by the -nclass
     *          directive.
     *          This flag cannot be used unless the -cclass directive is
     *          used first.
     *          This flag is mutually exclusive with the train directive.
     *          If neither of the above flags is specified and the -cclass
     *          directive is used, an error occurs.
     *
     * -r       train. Retrieve or continue training the preloaded network
     *          with the data files specified in the network file loaded
     *          by the -cclass directive.
     *          This flag cannot be used unless the -cclass directive is
     *          used first.
     *          This flag is mutually exclusive with the execute directive.
     *          If neither of the above flags is specified and the -cclass
     *          directive is used, an error occurs.
     *
     * SAVING of NETWORK FILE:
     *
     * -s       save. Saves the network to the file specified in the
     *          configuration file, or read from the keyboard. This occurs
     *          after the network is trained or the maximum number of I/O
     *          case sweeps have been completed.
     *          This flag can't be used with the -n (execute preloaded)
     *          network flag, but may be used with the -t flag.
     *          Don't store twice.
     *
     * -t       test. Store the network to the file specified in the
     *          configuration file, or read from the keyboard.
     *          The -t flag is useful when there are enough number of sweepings
     *          to find out how many sweeps are needed to learn. In this mode,
     *          the program will show only the sweep, the error, the network
     *          layers and the number of sweeps in those layers. It will also
     *          save the parameters right and left-propagation and a set of
     *          sweeps (from learning) when elapsed while the network was being
     *          fully trained. This flag is mutually exclusive with the -s flag.
     *
     * MODES of adding delta values calculated during learning:
     *
     * -l       each.  Calculate delta_w for each I/O case.
     * -a       all.   Add the delta_w's for all I/O cases to be processed
     *          during learning, and calculate at once, after all cases have
     *          been processed.
     * Default: If neither of the flags is specified, -l (all) is assumed, so
     *          this is the more cases.
     *
     * PRINTING the HELP messages:
     *
     * -?,-h    display the brief help messages. This produces an output
     *          very similar to this display.
     *
     * UNRECOGNISED command line options:
     *
     * -[anything else]
     *          display a message to "try fishNET -?".
     *
     * DEFAULT (no flags or directives):
     *
     * -[nothing]
     *          Recommended mode for beginners. This mode lets the program
     *          ask the user for information about network size and I/O
     */

    /* Allow for flagging */

    while ( argc > 1 && (*++argv)[0] == '-' )
    {
        file_name_read = 0;
        va = *argv;

        switch( va[1] )
        {
            case 'v':
                if ( QUIET )
                    error("Can't use both verbose and quiet flags");
                VERBOSE = 1;
                break;

            case 'q':
                if ( VERBOSE )
                    error("Can't use both quiet and verbose flags");
                QUIET = 1;
                break;

            case 'c':
                if ( KEY_FILE )
                    error("Can't load both a network file and a config file");
                CONFIG_FILE = 1;
                strcpy(config_file, va + 2);
                file_name_read = 1;
                break;

            case 'n':
                if ( CONFIG_FILE )
                    error("Can't load both a config file and a network file");
                KEY_FILE = TEACH;
                strcpy(net_file, va + 2);
                file_name_read = 1;
                break;

            case 's':
                if ( KEY_FILE == 0 )
                    error("Must specify network file first.");
                if ( KEY_FILE == TEACH )
                    error("Can't specify both execute and teach.");
                SAVE_SET = DO_SET;
                break;

            case 'e':
                if ( KEY_FILE == 0 )
                    error("Must specify network file first.");
                if ( KEY_FILE == DO_SET )
                    error("Must specify both teach and execute.");
                KEY_FILE = TEACH;
                break;

            case 't':
                if ( SAVE_SET == DO_SET )
                    error("Can't specify both save and test.");
                SAVE_SET = PARTLY;
                break;

            case 'l':
                if ( DELTA_MODE == ALL )
                    error("Can't specify both all and each delta a");
                DELTA_MODE = EACH;
                break;

            case 'a':
                if ( DELTA_MODE == EACH )
                    error("Can't specify both each and all delta a");
                DELTA_MODE = ALL;
                break;

            case 'd':
                if ( DELTA_MODE == ALL )
                    error("Can't specify both all and each delta a");
                DELTA_MODE = EACH;
                break;

            case '?':
            case 'h':
                show_help();
                break;

            default:
                fprintf(
                    stderr,
                    "Unrecognised flag or directive: %c. \n",
                    va[1]
                );
                error("try typing fishNET -? for help");
                break;
        }

        argc--;
    }

    printf("%s %s\n", PROG_NAME, PROG_VERSION);
    printf("%s\n", PROG_TITLE);

    /* for MSDOS systems, setup the BOOT/BEEP with the default
     * exception handler.
     */

    /* setup to catch floating point error. */
    if ( signal(SIGFPE, sfpe_handler) == SIG_ERR )
    {
        fprintf(stderr, "Can't setup floating point exception handler");
        exit(0);
    }

    /* set calculation mode */
    if ( DELTA_MODE == 0 )
        DELTA_MODE = EACH;

    if ( DELTA_MODE == ALL && !QUIET )
        printf("accumulating delta_w over all cases.\n");
    else if ( DELTA_MODE == EACH )
        printf("Applying delta_w at each I/O case.\n");

    if ( !KEY_FILE )
    {
        /* need to teach a network from store. */
        if ( CONFIG_FILE )
        {
            /* get data from keyboard */
            if ( !QUIET )
                parameter = input_parameters(config_file);
            else
                parameter = input_parameters(config_file);
        }
        else
        {
            if ( !QUIET )
                printf("Reading configuration parameters from %s...\n",
                    config_file);
            parameter = input_parameters(config_file);
        }

        /* Setup ctrlc interrupt handler to save network during
         * learning if required.
         */
        if ( signal(SIGINT, interrupt_handler) == SIG_ERR )
        {
            error("Can't setup interrupt handler");
            exit(0);
        }

        /* Teach the network by back propagation. */
        network = allocate(parameter);
        if ( !teach(network, parameter, 0) )
            error("Couldn't learn - something has gone wrong");

        if ( !VERBOSE )
            printf("Finished learning.\n");
    }
    else
    {
        /* Get pre-trained network */
        if ( !QUIET )
            printf("Loading network from %s.\n", net_file);

        net_and_param = load_network_and_parameters(net_file);
        parameter     = net_and_param->parameter;
        network       = net_and_param->network;
        start_line    = net_and_param->start_time;

        if ( !VERBOSE )
        {
            /* Setup ctrlc interrupt handler to save network
             * during learning if required.
             */
            if ( signal(SIGINT, interrupt_handler) == SIG_ERR )
            {
                error("Can't setup interrupt handler");
                exit(0);
            }

            if ( !teach(network, parameter, start_line) )
                fprintf(stderr, "Something has gone wrong - didn't learn\n");

            if ( !VERBOSE )
                printf("Finished learning.\n");
        }
    }

    if ( SAVE_SET == 0 )
    {
        /* If no flag input from command line for storage */
        printf("Do you want to store network? (y or n) ");
        scanf("%c", &answer);
        if ( answer == 'y' || answer == 'Y' )
        {
            store_network(network, parameter);
            if ( !VERBOSE )
                printf("Stored network.\n");
        }
    }
    else if ( SAVE_SET == DO_SET )
    {
        store_network(network, parameter);
        if ( !VERBOSE )
            printf("Stored network.\n");
    }
    else if ( SAVE_SET == PARTLY )
    {
        store_learnt_parameters(parameter);
        if ( !VERBOSE )
            printf("Stored learning parameters.\n");
    }

    if ( SAVE_SET == PARTLY ) /* quit without operating */
        exit(1);

    if ( !QUIET )
        printf("Querying.\n");

    /* get the input data. */
    randoms = get_data(parameter->data_in);

    /* Open file for output data */
    if ( (saved = fopen(parameter->data_out.name, "w")) == NULL )
        error("Can't open file for output");

    for (
        l = 0, runcase = randoms;
        l < parameter->data_in_cases;
        l++, runcase += parameter->per_layer[0]
    ){
        /* run */
        operate(network, runcase, parameter);

        if ( !VERBOSE )
        {
            print_case((OP_DATA *) runcase, parameter->per_layer[0]);
            fprintf(stdout, "\n");
        }

        print_top_layer(network, parameter);
        save_top_layer(network, parameter, saved);
    }

    fclose(saved);

    return (1);
}
