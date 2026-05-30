/* FILE = erate.c */

/* This file will take the filenames input and calculate the total
 * error between them.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "net_type.h"

#include "input.h"
#include "error.h"

#define CASES	20

int     VERBOSE;
int     QUIET;

int     CONFIG_FILE;
int     NET_FILE;

int     SAVE_NET;

int     DELTA_MODE;

main()
{
	int	i;
	int	j;

	DATA_FILE	expected;
	DATA_FILE	actual;

	I_O	*exp;
	I_O	*workexp;
	I_O	*act;
	I_O	*workact;

	OP_DATA	*item1;
	OP_DATA	*item2;

	double	E = 0.0;

	VERBOSE = 0;
	QUIET = 0;

	strcpy(expected.name, "dat\\hugerun.out");
	expected.i_o_cases 	= actual.i_o_cases	= CASES;
	expected.neurons 	= actual.neurons 	= 5;

	printf("Enter file for testing ");
	scanf(" %s", actual.name);

	exp = get_data(expected);
	act = get_data(actual);

	for 
	( 
		i = 0, workexp = exp, workact = act;
		i < CASES; 
		i++, workexp++, workact++
	)
		for
		(
			j = 0, item1 = workexp->item, item2 = workact->item;
			j < expected.neurons;
			j++, item1++, item2++
		)
			E += (item1->x - item2->x) * (item1->x - item2->x);

	fprintf(stdprn, "File = %s, E = %lf.\n", actual.name, E);
}
