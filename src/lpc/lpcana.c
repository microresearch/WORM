#include <stdio.h>
#include "memcof.c"
#include "audio.h"
#define k 14		/* Number of coefficients */

/* LPCANA.C								*/
/* Reads a signal from a disk file into a variable, x_in.		*/
/* Steps through x_in, 80 frames at a time, computes k LPC coefficients	*/
/* in array c, writes the LPC residual to errfile, and the coefficients */
/* (as floats) to outfile.						*/

int runlpcana(void)
{
   short int *x_in, *e;
   char *coeffs;
   int frame, n_frames, i, j, n, prev, next, *length=0;
   float data[81], xms, d[k+1], *c, *x_in_f, *lp;

   //   x_in = signal_in(infile,length);
   n_frames = (*length)/80;

/* Make a floating-point version of x_in, called x_in_f			*/
   x_in_f = (float *) malloc((*length)*sizeof(float)); // TODO
   for (i=0;i<(*length);i++)
      x_in_f[i] = (float) x_in[i];
   c = (float *) malloc((*length)*k*sizeof(float)); // TODO

   //   fid = fopen(coeffs,"wb");

   for (frame=0;frame<n_frames;frame++) {
      for (j=0;j<=79;j++) data[j+1] = x_in_f[80*frame+j];		
      memcof(data,80,k,&xms,d);
      for (j=0;j<=k-1;j++) {
         c[(frame+1)*80*k-k+j] = d[j+1];
	 //         fwrite(&d[j+1],sizeof(float),1,fid);
      }
   }
   //   fclose(fid);

/* For the first frame, use the first analysis vector for every sample	*/

   for (i=0;i<=78;i++) {
      for (j=0;j<=k-1;j++) c[i*k+j] = c[79*k+j];
   }

/* For frames 1..n_frames, interpolate the intermediate LPC vectors 	*/
   for (frame=1;frame<n_frames;frame++) {
      prev = frame*k*80-k;
      next = prev+k*80;
      for (i=0;i<79;i++) {
        for (j=0;j<=k-1;j++) {
        c[prev+k+k*i+j] = c[prev+j]+((float) i/80.0)*(c[prev+j]-c[next+j]);
        }
      }
   }

/* Model x_in_f and calculate the error					*/
   lp = (float *) malloc((*length)*sizeof(float));  /* predicted signal */ // TODO
   e = (short int *) malloc((*length)*sizeof(short int)); // TODO

   for (i=0;i<k;i++) e[i] = 0;
   for (i=k;i<(*length);i++) {
      j = (i-1)*k;
      lp[i] = 0;

   for (n=0;n<=k-1;n++) lp[i] = lp[i] -c[j+n]*lp[i-n-1];
      lp[i] = -lp[i];

      e[i] = (short int) x_in_f[i]-lp[i];
      lp[i] += e[i]; 
   }
   //   signal_out(length,e,errfile);
   return 0;
}

