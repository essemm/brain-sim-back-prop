/* FILE = error.c */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "net_type.h"

#include "fishnet.h"
#include "show.h"
#include "learn.h"

/* This routine prints an error message passed to it then exits.
 */

void
error(char *message)
{
	fprintf(stderr, "ERROR: %s\n", message);
	exit(-1);
}

/* This is the routine that is used to print out an error message and a file
 * name. Both parameters are char *.
 */

void
error_f(char *message, char *file_name)
{
	fprintf(stderr, "ERROR: %s in file %s.\n", message, file_name);
	exit(-1);
}

/* and this one is specifically for memory allocation errors.
 */

void
error_m(char *type, char *function)
{
	fprintf
	(
		stderr,
		"RUN OUT OF SPACE trying to allocate %s in function %s.\n",
		type, function
	);
	exit(-1);
}

/* This is the interrupt handler for control+c termination of the program.
 */

void
interrupt_handler(int sig)
{
	(void)sig;
	signal(SIGINT, SIG_IGN);
	fprintf(stderr, "Saving network after control-c detected.\n");
	ext_parameter->max_sweeps = *ext_t_ptr;
	store_network(ext_network, ext_parameter);
	exit(0);
}

/* This is the interrupt handler for floating point error termination of
 * the program.
 */

void
fpe_handler(int sig)
{
	(void)sig;
	signal(SIGFPE, SIG_IGN);
	fprintf(stderr, "Saving network after floating point error.\n");
	ext_parameter->max_sweeps = *ext_t_ptr;
	store_network(ext_network, ext_parameter);
	exit(0);
}
