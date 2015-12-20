#include <stdio.h>
#include "audio.h"
//#include "slputils.c"

#define k 14		/* Number of coefficients */

/* LPCSYN.C									*/
/* Reads an excitation signal from errfile into x_in, and vectors of k LPC	*/
/* coefficients from infile. Uses linear prediction to model the signal	*/
/* and writes it to outfile.							*/


int runitlpc(void)
{
	short int *e_in, *x_out;
	char *coeffs;
	int frame, i, j, n, prev, next, *length, n_frames;
	float data[81], *xms, *c, *lp, coeff;


	//	e_in = signal_in(errfile,length);
	n_frames = (*length)/80;
	//	fid = fopen(coeffs,"rb");

	c = (float *) malloc((*length)*k*sizeof(float)); // TODO!

	/*	for (frame=0;frame<n_frames;frame++) {
		for (j=0;j<k;j++) {
			fread(&coeff,sizeof(float),1,fid);
			c[(frame+1)*80*k-k+j] = coeff;
		}
	}
	fclose(fid);*/

/* For the first LPC frame, use the first analysis vector for every sample	*/

	for (i=0;i<=78;i++) {
		for (j=0;j<=k-1;j++) {
			c[i*k+j] = c[79*k+j];
		}
	}

	/* For frames 1..n_frames, interpolate the intermediate LPC vectors		*/ //TODO - skip this!
	for (frame=1;frame<n_frames;frame++) {
		prev = frame*k*80-k;
		next = prev+k*80;
		for (i=0;i<79;i++) {
			for (j=0;j<=k-1;j++) {
				c[prev+k+k*i+j] = c[prev+j]+((float) i/80.0)*(c[prev+j]-c[next+j]);
			}
		}
	}

/* Model x_out									*/
	lp = (float *) malloc(*length*sizeof(float));	/* predicted signal	*/ // TODO
	x_out = (short int *) malloc(*length*sizeof(short int)); // TODO
 
	for (i=k;i<(*length);i++) {
		j = (i-1)*k;
		lp[i] = 0;
		for (n=0;n<=k-1;n++)
			lp[i] = lp[i] -c[j+n]*lp[i-n-1];
		lp[i] = -lp[i];
		x_out[i] = (short int) lp[i]+e_in[i];	
		lp[i] += e_in[i];
	}
	//	signal_out(length,x_out,outfile);
        return 0;
}

