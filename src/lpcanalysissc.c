#include "audio.h"
#include <math.h>
#include "arm_const_structs.h"

extern uint16_t adc_buffer[10];


/*
 *  LPCAnalysis.h
 *
 *  Created by Nicholas Collins on 10/09/2009.
 *  Copyright 2009 Nicholas M Collins. All rights reserved.
 *
 */

// test for own use! TEST

#include "crow_lpc_coeffs.h"

struct crowz{
  const float* crowy;
  int crowlength;
};

struct crowz crow_coeffz[2];

//own_crow_coeffs, single_crow_coeffs

typedef float LPCfloat;

int windowsize;
float * windowfunction; //can add later if start cross fading consecutive windows with hop
float * inputty;
int numpoles;
//static int pos;
float * coeff;
float * last;
static float G; //gain;

//matrix calculations at double resolution to help avoid errors?
//storage for Levinson-Durbin iteration to calculate coefficients
static LPCfloat * R;
static LPCfloat * preva;
static LPCfloat * a;

//recalculate poles based on recent window of input
void calculatePoles() {

	//can test for convergence by looking for 1-((Ei+1)/Ei)<d

	int i, j;
	LPCfloat sum;

	//safety
	if(numpoles<1) numpoles=1;
	if(numpoles>windowsize) numpoles=windowsize;

	//printf("p? %d",p);

	//calculate new LPC filter coefficients following (Makhoul 1975) autocorrelation, deterministic signal, Durbin iterative matrix solver

	//float R[21];//autocorrelation coeffs;
	//float preva[21];
	//float a[21];
	LPCfloat E, k;

	//faster calculation of autocorrelation possible?

	for(i=0; i<=numpoles; ++i) {
		sum=0.0;

		for (j=0; j<= windowsize-1-i; ++j)
			sum+= inputty[j]*inputty[j+i];

		R[i]=sum;
	}

	E= R[0];
	k=0;

	if(E<0.00000000001) {

		//zero power, so zero all coeff
		for (i=0; i<numpoles;++i)
			coeff[i]=0.0;
//
//		latesterror= E;
		G=0.0;
		//printf("zero power %f\n", E);
		return;

	};

	//rescaling may help with numerical instability issues?
//	float mult= 1.0/E;
//	for(i=1; i<=numpoles; ++i)
//		R[i]= R[i]*mult;
//
	for(i=0; i<=(numpoles+1); ++i) {
		a[i]=0.0;
		preva[i]=0.0; //CORRECTION preva[j]=0.0;
	}


//	for(i=0; i<numpoles; ++i) {
//		printf("sanity check a %f R %1.15f ",a[i+1], R[i]);
//	}
//	printf("\n");


	LPCfloat prevE= E;

	for(i=1; i<=numpoles; ++i) {

		sum=0.0;

		for(j=1;j<i;++j)
			sum+= a[j]*R[i-j];

		k=(-1.0*(R[i]+sum))/E;

		a[i]=k;

		for(j=1;j<=(i-1);++j)
			a[j]=preva[j]+(k*preva[i-j]);

		for(j=1;j<=i;++j)
			preva[j]=a[j];

		E= (1-k*k)*E;

		//printf("E check %f %d k was %f\n", E,i,k);

		//check for instability; all E must be greater than zero
		if(E<0.00000000001) {

			//zero power, so zero all coeff

			//leave coeff as previous values
			//for (j=0; j<numpoles;++j)
//				coeff[j]=0.0;

//			latesterror= E;
			//printf("early return %1.15f %d\n", E,i);
			return;

		};
	}

	G= sqrtf(E);

	//	latesterror= E;

	//solution is the final set of a
	for(i=0; i<numpoles; ++i) {
		//coeff[numpoles-1-i]=a[i+1];
		coeff[i]=a[i+1];
		//printf("a %f R %f",a[i+1], R[i]);
	}

	//MUST CHECK gain?

}


void calculateOutput(float * source, float * target, int startpos, int num) {
	int i,j;

	int basepos,posnow;

	for(i=0; i<num; ++i) {

		basepos= startpos+i+windowsize-1; //-1 since coefficients for previous values starts here

		float sum=0.0;

		for(j=0; j<numpoles; ++j) {
		  posnow= (basepos-j)%windowsize;

			//where is pos used?
		  sum += last[posnow]*coeff[j]; //was coeff i
		}
		
		//			G=1.0f; // TEST!

		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below

		last[startpos+i]=sum;

		//ZXP(out)=
		//		target[i]+= sum*windowfunction[startpos+i];
		target[i]+= sum;
		//		target[i]=source[i];
	}

}


void zeroAll() {

  int i;

  numpoles=10;

  //  pos=0;

  for (i=0; i<windowsize;++i) {
    inputty[i]= 0.0;
    coeff[i]=0.0;
    last[i]=0.0;
  }

  int half= windowsize>>1;

  float mult,value;

    //rectangular window
    for (i=0; i<windowsize;++i) {
      windowfunction[i]= 1.0;
    }


  //for (i=0; i<blocksize;++i) {
  //			output[i]= 0.0;
  //		}

    //  latesterror=0.0;

  G=0.0; //gain starts at zero;
}


void LPCAnalysisinit(int _windowsize) {
  windowsize=_windowsize;
  windowfunction=(float *)malloc(windowsize*sizeof(float));
  inputty=(float *)malloc(windowsize*sizeof(float));
  coeff=(float *)malloc(windowsize*sizeof(float));
  last=(float *)malloc(windowsize*sizeof(float));
  R=(float *)malloc(windowsize*sizeof(float));
  preva=(float *)malloc(windowsize*sizeof(float));
  a=(float *)malloc(windowsize*sizeof(float));
  zeroAll();

  crow_coeffz[0].crowy=own_crow_coeffs;
  crow_coeffz[0].crowlength=10296;
  crow_coeffz[1].crowy=single_crow_coeffs;
  crow_coeffz[1].crowlength=10450;


  //  pos=0;
};




	//latest value of p is number of poles for fitting; if -1 adapt using criteria
	//assess if tonal or transient via error?
	//source is driver signal for filtering, newinput is data arriving for analysis

/*
 *  LPCAnalyzer.cpp
 *  xSC3ExtPlugins-Universal
 *
 *  Created by Nick Collins on 10/04/2009.
 *  Copyright 2009 Nick Collins. All rights reserved.
 *
 */

//also can do test to return convergence point; clue to transient vs tonal?
//toggle to freeze or not on current filter coefficients?

void LPCAnalyzer_init() {

  int windowsize= 32; // ???

	if((windowsize & 0x01))
		windowsize= windowsize+1;
	if(windowsize>1024) windowsize=1024;

	LPCAnalysisinit(windowsize);
}


//blocksize MUST be less than or equal to windowsize

void update(float * newinput, float * newsource, float * output, int numSamples, int p) {

  int i, pos=0;
  static u16 countex=0;
  u8 select=adc_buffer[SELY]>>11;

	/*			for (i=0; i<numSamples;++i) {

			inputty[pos++]= newinput[i];

			}*/

		calculateOutput(newsource, output, 0, numSamples);

			//	calculateOutput(newsource, output, windowsize, 0);
	
		//update
		if (adc_buffer[SELX]<2000){ //TODO: freeze!
		  numpoles=p;
		//				calculatePoles(); // TEST!
				
			for(i=0; i<numpoles; i++) {
			  // switching co_effs -> 

			  //			  crow_coeffz[0].crowy=own_crow_coeffs;
			  //			  crow_coeffz[0].crowlength=10296;


			  coeff[i]=crow_coeffz[select].crowy[countex]; // exact length
			  countex++;
			  if (countex==crow_coeffz[select].crowlength) countex=0;
			}
			G=crow_coeffz[select].crowy[countex];
			//			G=1.0f;
			  countex++;
			  if (countex==crow_coeffz[select].crowlength) countex=0;
			//			counter+=numpoles;
				
		}

	//return output;
}

void LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int inNumSamples) {

  //	float * inoriginal= IN(0);
  //	float * indriver= IN(1); // we have 2 inputs????

	/*	int p= (int)ZIN0(3);
	int testE= (int)ZIN0(4);
	LPCfloat delta= (LPCfloat)ZIN0(5);*/

	//	float * out= OUT(0);

	for (int i=0; i<inNumSamples; ++i) {
		out[i]= 0.0;
	}

	update(inoriginal, indriver, out, inNumSamples, p);
}




