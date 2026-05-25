/* FILE = error.c */

/* setup standard functions */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

/* Include the following as this file can reference external variables */
#include "net_type.h"
#include "fishNET.h"
#include "learn.h"
#include "show.h"

/* This routine prints an error message passed to it then exits. */

void
error(message)
char *message;
{
    fprintf(stderr, "ERROR: %s\n", message);
    exit(-1);
}

/* This is the routine that is used to print out an error message and a file.
 * Both parameters are char *.
 */

void
error_f(message, file_name)
char *message;
char *file_name;
{
    fprintf(stderr, "ERROR: %s in file %s.\n", message, file_name);
    exit(-1);
}

/* use this one specifically for memory allocation errors. */

void
error_m(type, function)
char    *type;
char    *function;
{
    fprintf(stderr, "OUT OF SPACE trying to allocate %s in function %s.\n",
        type, function);
    exit(-1);
}

/* This is the interrupt handler for controls termination of the program. */

void
interrupt_handler(sig)
int sig;
{
    signal( SIGINT, SIG_IGN );
    fprintf( stderr, "Saving network after control-c detected.\n" );
    ext_parameter->test_sweeps = ext_b_ptr;
    store_network( ext_network, ext_parameter );
    return;
}

/* This is the interrupt handler for floating point error termination of
 * the program.
 */

void
sfpe_handler(sig)
int sig;
{
    signal( SIGFPE, SIG_IGN );
    fprintf( stderr, "FLOATING POINT STATUS: error encountered.\n" );
    ext_parameter->test_sweeps = ext_b_ptr;
    store_network( ext_network, ext_parameter );
    return;
}
