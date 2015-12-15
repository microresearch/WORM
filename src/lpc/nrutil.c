#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#define NR_END 1
#define FREE_ARG char*

/* nrutil.c								*/

/* From W. H.Press, S. A. Teukolsky, W. T. Vettering and B. P. Flannery */
/* (1992) Numerical Recipes in C: The Art of Scientific Computing	*/
/* (2nd edition). Cambridge University Press.				*/

/* Non-copyright 							*/

/* pp. 942-7: This file only contains selected portions, however.	*/

void nrerror(char error_text[])
/* Numerical Recipes standard error handler				*/
{
	fprintf(stderr,"Numerical Recipes run-time error...\n");
	fprintf(stderr,"%s\n",error_text);
	fprintf(stderr,"...now exiting to system...\n");
	exit(1);
}

float *vector(long nl, long nh)
/* allocate a float vector with subscript range v[nl..nh]		*/
{
	float *v;

	v=(float *)malloc((size_t) ((nh-nl+1+NR_END)*sizeof(float)));
	if (!v) nrerror("allocation failure in vector()");
	return v-nl+NR_END;
}

void free_vector(float *v, long nl, long nh)
/* free a float vector allocated with vector()				*/
{
	free((FREE_ARG) (v+nl-NR_END));
}

