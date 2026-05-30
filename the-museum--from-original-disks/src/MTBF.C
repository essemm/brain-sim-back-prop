#include <stdio.h>
#include <math.h>

#include <stdlib.h>

double
fact(n)
double	n;
{
	int	i;
	double	fact	= 1.0;

	if ( n == 0.0 || n == 1.0 )
		return(1.0);

	for ( i = 2; i <= (int) n; i++ )
		fact *= i;
	return(fact);
}

double
ncr(n, i)
double	n;
double	i;
{
	return(fact(n) / (fact(i) * fact(n - i)));
}

double
mtbf(n, k, tau, theta)
double 	n;
double 	k;
double	tau;
double 	theta;
{
	double	lambda;
	double	mu;

	double	top	= 0.0;
	double	bottom	= 0.0;

	int	i;

	lambda	= 1 / theta;
	mu 	= 1 / tau;

	for (i = 0; i <= ((int)n - (int)k); i++)
		top += ncr(n, (double) i) * pow(lambda/mu, (double) i);

	return (top / (k * lambda * ncr(n, k) * pow(lambda/mu, n-k)));
}

int
main()
{

	double	n, k, tau, theta;

	printf("Enter total number of units, n: ");
	scanf(" %lf", &n);

	printf("Enter min numbering of functioning units, k: ");
	scanf(" %lf", &k);

	printf("Enter mean time to repair, tau: ");
	scanf(" %lf", &tau);

	printf("Enter mean time to failure, theta: ");
	scanf(" %lf", &theta);

	printf("\n\nMean time between failure of system =\n");
	printf("\t%le\n", mtbf(n, k, tau, theta));

	return(1);
}
