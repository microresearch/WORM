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

// seperate out crow stuff

//#include "crow_lpc_coeffs.h"

struct crowz{
  const float* crowy;
  int crowlength;
};

struct crowz crow_coeffz[2];
typedef float32_t LPCfloat;

// if we increase windowsize means we need larger: inputty _and_ last ONLY!

int windowsize;
float inputty[32];
int numpoles;
float coeff[32];
float last[32];
float corcy[32];
float reflect[32];
static float G; //gain;

static LPCfloat R[32];
static LPCfloat preva[32];
static LPCfloat a[32];


// retry levinson-durbin

/// samples size, m poles
void AutoCorrelation(float *Signal, int Samples, int m, float *CorCoeff)   
{   
   int   i, j;   
   float Normal;   
      
   for (i = 0; i <= m; i++)    
   {   
      CorCoeff[i] = 0.0;   
      for (j = 0; j < Samples - i; j++)   
          CorCoeff[i] += Signal[j] * Signal[j + i];   
   }   
   
/* Now normalizing the autocorrelation. ¹éÒ»»¯ */   
   Normal = CorCoeff[0];   
   
   for (i = 0; i <= m; i++)   
     CorCoeff[i] /= Normal;   

   }   

void DUrbin(int p, float *CorrCoeff, float *LPC, float *Reflect)   
{   
   int       i, j;   
   float     Error, alpha;   
   float    A[11];   
   
   //   if (!(A = (float *)calloc((p + 1), sizeof(float))))    
   //     eerror("DUrbin:A");   
   
   LPC[1] = 1.0;   
   Reflect[1] = -CorrCoeff[1]/CorrCoeff[0];   
   alpha = CorrCoeff[0]*(1 - Reflect[1]*Reflect[1]);
   //   if (alpha == 0)     printf("alpha == 0\n");   
      
   LPC[2] = Reflect[1];   
   
   for (i = 2; i <= p; i++)    
   {   
      Error = 0.0;   
   
      for (j = 1; j <= i; j++)   
          Error += LPC[j]*CorrCoeff[i + 1 - j];   
   
      Reflect[i] = -Error/alpha;   
      alpha *= (1 - Reflect[i]*Reflect[i]);   
      LPC[i + 1] = Reflect[i];   
   
      for (j = 2; j <= i; j++)   
         A[j] = LPC[j] + Reflect[i]*LPC[i + 2 - j];   
   
      for (j = 2; j <= i; j++)   
         LPC[j] = A[j];   
   };   
   
   LPC[0] = 1.0;   
   for (i = 1; i <= p; i++)   
   {   
       LPC[i] = LPC[i + 1];   
   }   
   //   free((char *)A);   
}   


///////////////////////////////////////////

void calculatePoles() {
  numpoles=10;

	int i, j;
	LPCfloat sum;
	if(numpoles<1) numpoles=1;
	if(numpoles>windowsize) numpoles=windowsize;
	LPCfloat E, k;

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
		G=0.0;
		return;
	};

	//rescaling may help with numerical instability issues?
	float mult= 1.0/E;
	//	for(i=1; i<=numpoles; ++i)
	//		R[i]= R[i]*mult;
//
	for(i=0; i<=(numpoles+1); ++i) {
		a[i]=0.0;
		preva[i]=0.0; //CORRECTION preva[j]=0.0;
	}
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
		if(E<0.00000000001) {
			//leave coeff as previous values
			//for (j=0; j<numpoles;++j)
//				coeff[j]=0.0;
			return;

		};
	}

	G= sqrtf(E);
	for(i=0; i<numpoles; ++i) {
		//coeff[numpoles-1-i]=a[i+1];
		coeff[i]=a[i+1];
	}
}


float randyr(){
    float lastValue=(((float)rand()/RAND_MAX)*0.2f)-0.1f;
  //  float lastValue=0.1f;
  return lastValue;
}

void calculatePredicted(float *incoming, float * target, int num) {
	int i,j;
	float oldlp; int error;
	int basepos,posnow;

	for(i=0; i<num; ++i) {
		basepos= i+windowsize-1; //-1 since coefficients for previous values starts here
		float sum=0.0;
		for(j=0; j<numpoles; ++j) {
		  posnow= (basepos-j)%windowsize;
		  sum += last[posnow]*coeff[j];  // - or + ?
		  //      for (n=0;n<=k-1;n++) lp[i] = lp[i] -c[j+n]*lp[i-n-1];  // predicted signal lp[i]= // lpcana.c
		}
		sum=randyr()-sum;
		//		oldlp=sum;
		//		error= (int) incoming[i]-sum; // this is error but is more about int conversion????
		//		sum=1-sum;
		//				sum=incoming[i]-sum;
		// 		last[i]=sum+oldlp; // add error to? but this is just in_signal
		last[i]=sum;
		target[i]= sum; 
	}
}

void calculateresidual(float *incoming, float * target, int num) {
	int i,j;
	int basepos,posnow;

	for(i=0; i<num; ++i) {
	  basepos= i+windowsize-1; //-1 since coefficients for previous values starts here
	  float sum=0.0f;
	  for(j=1; j<11; ++j) { // numpoles is 10
	    posnow= (basepos-j)%windowsize;
	    sum += last[posnow]*coeff[j];  //last[ i-j] 0,
		}
	  target[i]= sum; 
	  //	  target[i]=incoming[i]- sum;
	  last[i]=randyr();
	}
}

// LPCAnalyzer.html
///source= Impulse.ar(delaytime.reciprocal);
//(DelayN.ar(input,delaytime, delaytime)- LPCAnalyzer.ar(input,source,1024,MouseX.kr(1,256))).poll(10000)
///////************************??????

void calculateOutput(float * oldsource, float * source, float * target, int num) { // now for error or residual signal
	int i,j;
	int basepos,posnow;
	for(i=0; i<num; ++i) {
	  basepos= i+windowsize-1; //-1 since coefficients for previous values starts here
	  float sum=0.0f;
	  for(j=0; j<numpoles; ++j) {
	    posnow= (basepos-j)%windowsize;
	    sum += last[posnow]*coeff[j]; 
	  }
	  sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below
	  last[i]=sum;
	  target[i]= oldsource[i]-sum; // - delayed input by one window=32 samples MINUS new LPC
	  //				target[i]= sum;
	}
}

static float lastnew[32];

void calculatenewOutput(float * source, float * target, int num) {
	int i,j;
	int basepos,posnow;
	for(i=0; i<num; ++i) {
		basepos= i+windowsize-1; //-1 since coefficients for previous values starts here
		float sum=0.0;
		for(j=0; j<numpoles; ++j) {
		  posnow= (basepos-j)%windowsize;
		  sum += lastnew[posnow]*coeff[j]; 
		}
		sum= source[i]-sum; //scale factor G calculated by squaring energy E below
		lastnew[i]=sum;
		target[i]= sum; // why += and zeroing necessary TODO!
	}
}



void zeroAll() {
  int i;
  numpoles=10;
  for (i=0; i<windowsize;++i) {
    inputty[i]= 0.0;
    coeff[i]=0.0;
    last[i]=0.0;
    lastnew[i]=0.0;

  }

  int half= windowsize>>1;
  float mult,value;

    //rectangular window
  /*    for (i=0; i<windowsize;++i) {
      windowfunction[i]= 1.0;
      }*/


  //for (i=0; i<blocksize;++i) {
  //			output[i]= 0.0;
  //		}

    //  latesterror=0.0;

  G=0.0; //gain starts at zero;
}


void LPCAnalysisinit(int _windowsize) {
  windowsize=_windowsize;
  //  windowfunction=(float *)malloc(windowsize*sizeof(float));
  zeroAll();
  //  crow_coeffz[0].crowy=own_crow_coeffs;
  //  crow_coeffz[0].crowlength=7480;
  //  crow_coeffz[1].crowy=single_crow_coeffs;
  //  crow_coeffz[1].crowlength=6974;
};

void LPCAnalyzer_init() {

  int windowsize= 32; // ???

  if((windowsize & 0x01))
    windowsize= windowsize+1;
  if(windowsize>1024) windowsize=1024;
  LPCAnalysisinit(windowsize);
}

//blocksize MUST be less than or equal to windowsize
void update(float * newinput, float * newsource, float * output, int p,  int numSamples) {
  int i, pos=0;
  static u16 countex=0;
  u8 select=adc_buffer[SELY]>>11;

  /*  for (i=0; i<numSamples;++i) { // for input
    inputty[pos++]= newinput[i];
    }*/

  calculatenewOutput(newsource, output, numSamples);
  calculatePoles(); // TEST!


 	if (adc_buffer[SELX]<2000){ //TODO: freeze!
		  numpoles=p;
				
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
}

void LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int inNumSamples) {

	for (int i=0; i<inNumSamples; ++i) {
		out[i]= 0.0;
	}
	
	update(inoriginal, indriver, out, inNumSamples, p); // THIS ONE FOR CROWS
}

  static float oldsource[32];


void update_error(float * newinput, float * output, int p, int numSamples) {

  int i; float testy[32];
  static float phase =0.0f;
    float z;

  //    for (i=0; i<numSamples;++i) {
 

  for (i=0; i<numSamples;++i) {
    inputty[i]= newinput[i];
    //    testy[i]= randyr();
    // from Impulse->LFUGens.cpp
    if (phase >= 1.f) {
      phase -= 1.f;
      z = 1.f;
    } else {
      z = 0.f;
    }
    phase += 0.03125 ; // punch in freq is freqmul=1/32000 = 0.00003125 * 1000 (32000/32) = 0.03125
    testy[i]=z;
  }
  
  //  AutoCorrelation(newinput, 32, 10, corcy);
  //  DUrbin(10, corcy, coeff, reflect);
  //  calculateresidual(newinput, output, numSamples);
  calculateOutput(oldsource, testy,output,32);
  ///  numpoles=p;

  //    calculatenewOutput(output, output, numSamples);

  calculatePoles(); // on last set of samples

      for (i=0; i<numSamples;++i) { // frame delayed samples
    oldsource[i]= newinput[i];
  }



}
			  
void LPCAnalyzer_errorsamples(float *in, float *out, int p, int numSamples) {
  float iiin[32];
    update_error(in,out, p, numSamples);
}
