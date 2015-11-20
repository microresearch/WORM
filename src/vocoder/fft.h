/* $Id: fft.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

/* (C) Copyright 1993 by Steven Trainoff.  Permission is granted to make
* any use of this code subject to the condition that all copies contain
* this notice and an indication of what has been changed.
*/
#ifndef _FFT_
#define _FFT_

#include "spt.h"

#ifndef REAL
#define REAL float		/* Precision of data */
#endif

void fft(REAL (*x)[2], int n);
void invfft(REAL (*x)[2], int n);
void normalize_fft(REAL (*x)[2], int n);

void realfftmag(REAL *data, int n);
void normalize_fftn(REAL (*x)[2], int ndim, int *dim);

unsigned bitrev(unsigned, int);	/* Bit reversal routine */
int getindex(int, int, int, int *);
int ipow(int, int);
int ilog2(int);



#endif /* _FFT_ */
