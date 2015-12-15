#include <stdio.h>
#include <math.h>
#include "nrutil.h"

/* memcof.c								*/
/* Calculate LPC coefficients						*/
/* This routine is from the book Numerical Recipes in C.                */
/* (Cambridge University Press), Copyright (C)  1987-1992 by            */
/* Numerical Recipes Software.  Used by permission.  Use of this        */
/* routine other than as an integral part of the software collection    */
/* provided in accompaniment with the book Introducing Speech and       */
/* Language Processing (Cambridge University Press) requires an         */
/* additional license from Numerical Recipes Software (www.nr.com).     */
/* Further distribution in any form is prohibited.                      */

void memcof(float data[], int n, int m, float *xms, float d[])

/* Given a real vector of data[1..n], and given m, this routine returns */
/* m linear prediction coefficients as d[1..m], and returns the mean	*/
/* square discrepancy as xms						*/

{
	int k, j, i;
	float p=0.0, *wk1, *wk2, *wkm;

	wk1=vector(1,n);
	wk2=vector(1,n);
	wkm=vector(1,m);

	for (j=1; j<=n; j++)
		p += SQR(data[j]);
	*xms=p/n;
	wk1[1]=data[1];
	wk2[n-1]=data[n];
	
	for (j=2;j<=n-1;j++) {
		wk1[j]=data[j];
		wk2[j-1]=data[j];
	}

	for (k=1;k<=m;k++) {
		float num=0.0,denom=0.0;

		for (j=1;j<=(n-k);j++) {
			num += wk1[j]*wk2[j];
			denom += SQR(wk1[j])+SQR(wk2[j]);
		}
		d[k]=2.0*num/denom;
		*xms *= (1.0-SQR(d[k]));

		for (i=1;i<=(k-1);i++) 
			d[i]=wkm[i]-d[k]*wkm[k-i];

		if(k == m){
			free_vector(wkm,1,m);
			free_vector(wk2,1,n);
			free_vector(wk1,1,n);
			return;
		}

		for (i=1;i<=k;i++)wkm[i]=d[i];
		for (j=1;j<=(n-k-1);j++) {
			wk1[j] -= wkm[k]*wk2[j];
			wk2[j]=wk2[j+1]-wkm[k]*wk1[j+1];
		}
	}
	nrerror("never get here in memcof.");
}

