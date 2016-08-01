#include <math.h>
#include <stdlib.h>
#include <sndfile.h>
#include "forlap.h"

// 44100 so 0.005 seconds would be 220 samples - close to 256 

/* fix on parameters-below, check coeff calc and filter calc (delay), printfs in lpcana, fitlpc !!!

 1- all coeffs from 3 methods are now the same (in praat list of coeffs starts with 1)
 1.5- same results with double and float. so coeffs seems okay
 1.6- why are there different results in praat (crossover? pre-emph and windowing might all be different(

 2- delay and filter is the issue - return to SC code:

but filter of source using coeffs always works but NOT inverse filter on source?

 3- check filter and pre-emph - only works with pre-emph and predict from fitlpc - blocksize and chunksize issue to fix
///////TODO from here
 4- porting

 5- look at other LPC code

*/

#define BLOCK_SIZE 128
#define CHUNKSIZE 32
#define	P_MAX	10	/* order p of LPC analysis, typically 8..14 */

// what is samplerate

typedef float LPCfloat;
static const int windowsize=BLOCK_SIZE; // say up to 1024
static LPCfloat inputty[129];

static LPCfloat window128[1024]={0.000003, 0.000007, 0.000012, 0.000020, 0.000031, 0.000045, 0.000066, 0.000094, 0.000132, 0.000184, 0.000254, 0.000346, 0.000470, 0.000633, 0.000846, 0.001125, 0.001485, 0.001950, 0.002544, 0.003300, 0.004256, 0.005455, 0.006953, 0.008810, 0.011098, 0.013900, 0.017308, 0.021428, 0.026375, 0.032277, 0.039273, 0.047510, 0.057144, 0.068335, 0.081248, 0.096044, 0.112883, 0.131909, 0.153256, 0.177034, 0.203323, 0.232174, 0.263593, 0.297542, 0.333931, 0.372615, 0.413388, 0.455984, 0.500077, 0.545278, 0.591145, 0.637184, 0.682857, 0.727594, 0.770803, 0.811880, 0.850228, 0.885265, 0.916443, 0.943263, 0.965282, 0.982134, 0.993531, 0.999279, 0.999279, 0.993531, 0.982134, 0.965282, 0.943263, 0.916443, 0.885265, 0.850228, 0.811880, 0.770803, 0.727594, 0.682857, 0.637184, 0.591145, 0.545278, 0.500077, 0.455984, 0.413388, 0.372615, 0.333931, 0.297542, 0.263593, 0.232174, 0.203323, 0.177034, 0.153256, 0.131909, 0.112883, 0.096044, 0.081248, 0.068335, 0.057144, 0.047510, 0.039273, 0.032277, 0.026375, 0.021428, 0.017308, 0.013900, 0.011098, 0.008810, 0.006953, 0.005455, 0.004256, 0.003300, 0.002544, 0.001950, 0.001485, 0.001125, 0.000846, 0.000633, 0.000470, 0.000346, 0.000254, 0.000184, 0.000132, 0.000094, 0.000066, 0.000045, 0.000031, 0.000020, 0.000012, 0.000007, 0.000003};

// gaussian window of varying sizes are generated below

static LPCfloat lasted[128];
static LPCfloat delay[128];
static LPCfloat coeff[32];
static LPCfloat G; //gain;

static LPCfloat R[32];
static int pos=0;

///source= Impulse.ar(delaytime.reciprocal); 
// 
void do_impulse(float *out, int numSamples, uint16_t freq){ //- so for 256 samples we have freq 125 for impulse
    // from Impulse->LFUGens.cpp
  int i;
  static float phase =0.0f;
  float z, freqinc;
  freqinc=0.00003125 * freq;

  for (i=0; i<numSamples;++i) {
    if (phase >= 1.f) {
      phase -= 1.f;
      z = 1.f;
    } else {
      z = 0.f;
    }
    phase += freqinc; // punch in freq is freqmul=1/32000 = 0.00003125 * 1000 (32000/32) = 0.03125
    out[i]=z;
  }
}

//(DelayN.ar(input,delaytime, delaytime)- LPCAnalyzer.ar(input,source,1024,MouseX.kr(1,256))).poll(10000)
void do_delay(float *in, float *out, uint16_t delaytime, int numSamples){
  // delay up to buffersize =512
  static uint16_t dpointer=0;
  for (int i=0;i<numSamples;i++){
    out[i]=delay[dpointer];
  // place sample in buffer
    delay[dpointer++]=in[i];
    if (dpointer==delaytime) dpointer=0;
}
}

void lpc_preemphasis(LPCfloat * x, int len, LPCfloat alpha )
{
    for( int i = len - 1; i > 0; i-- )
      x[i] = x[i] - alpha * x[i-1];////y[k]=x[k]-0.95x[k-1]
}


void calculateDurbPoles(){ // into coeffs
  int i, j;  LPCfloat r,sum;

	// this is as autocorrelation

	for(i=0; i<=P_MAX; ++i) {
	  coeff[i]=0.0f;
		sum=0.0;
		for (j=0; j<= windowsize-1-i; ++j) sum+= inputty[j]*inputty[j+i];
		R[i]=sum;
		//		printf("SUM: %f,,,,\n", R[i]);
	}

    LPCfloat error = R[0];

    for (i = 0; i < P_MAX; i++) {
      r = -R[i + 1];
      for (j = 0; j < i; j++) r -= coeff[j] * R[i - j];
      r /= error;
			coeff[i] = r;
			for (j = 0; j < i/2; j++) {
				LPCfloat tmp  = coeff[j];
				coeff[j]     += r * coeff[i-1-j];
				coeff[i-1-j] += r * tmp;
				///				printf("iiii %d %d\n",j, i-1-j);
			}
			if (i % 2) coeff[j] += coeff[j] * r;

error *= 1.0 - r * r;
 }

//	G= sqrtf(error); .// TODO!
}

void zeroAll() {
  int i;
  //  P_MAX=10;
  for (i=0; i<windowsize;++i) {
    inputty[i]= 0.0f;
    //    last[i]=0.0f;
    lasted[i]=0.0f;
    //    lastnew[i]=0.0;
  }

  for (i=0; i<P_MAX;++i) {
    coeff[i]= 0.0f;
  }
  G=0.0f; //gain starts at zero;
}

void LPCAnalyzer_init() {
  zeroAll();
}

// residual= sum += source[i-j]*coeff[j];
// IIR= source[i] -= source[i-j]*coeff[j];  	//		y[i] -= a[j] * y[i - j];

float predict(long order,long length,float *data,float *coeffs, float * errur)
{
    long i,j;
    float power=0.0,error,tmp;
    static float Zs[P_MAX] = {0.0};
//    short shortError;       //  Use this if want error to be soundfile

    for (i=0;i<length;i++)     {         //  0 to hopsize??????????
        tmp = 0.0;
	for (j=0;j<order;j++)  tmp += Zs[j]*coeffs[j];
	for (j=order-1;j>0;j--) Zs[j] = Zs[j-1];
	Zs[0] = data[i];
        error = data[i] - tmp;
	errur[i]=error;
	//	printf("error: %f - data %f xx",tmp, data[i]); 
	//        fwrite(&error,4,1,resFile);
//        shortError = error;      //  Use these if want error to be soundfile
//        fwrite(&shortError,2,1,resFile);
	power += error * error;
    }
    return sqrt(power) / length;  
}

void calculateOutput(LPCfloat * source, LPCfloat * target, int startpos, int num) {
  u8 j; int i;
	int basepos,posnow;
//G=1.0; // TESTY!

	for(i=0; i<num; ++i) {

		basepos= startpos+i+windowsize-1; //-1 since coefficients for previous values starts here
		LPCfloat sum=0.0;
		for(j=0; j<P_MAX; ++j) {
		  posnow= (basepos-j)%windowsize;
		  sum += lasted[posnow]*coeff[j]; 
		}
		sum= source[i]-sum; //scale factor G calculated by squaring energy E below TODO - if we use this from coeffs
		lasted[startpos+i]=sum;
		target[i]= sum;
	}
}

void LPC_cross(LPCfloat * newinput, LPCfloat *newsource, LPCfloat * output, int numSamples) {

  // cross newinput as LPC analysis with newsource as residual...
  int i;
  int left= windowsize-pos;

  // test with
	do_impulse(newsource, numSamples, 200);

	if(numSamples>=left) {
		lpc_preemphasis(newinput,numSamples,0.97);

		for (i=0; i<left;++i) {
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		calculateDurbPoles(); // this calculates the coeffs so... - these all give same results
		pos=0;
		predict(P_MAX,left,newinput,coeff,newsource); // this gives the error signal into newsource
		int remainder= numSamples-left;

			for (i=0; i<remainder;++i) {
			  float temp= newinput[left+i]*window128[pos]; //where are we in window 
			  inputty[pos++]=temp;
			  newinput[i]=temp;
		}
			calculateOutput(newsource, output+left, pos-remainder, remainder);
	} else {
		lpc_preemphasis(newinput,numSamples,0.97);
		for (i=0; i<numSamples;++i) {
		  //			inputty[pos++]= newinput[i];
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		calculateOutput(newsource, output, pos-numSamples, numSamples);
	}
}

void LPC_residual(LPCfloat * newinput, LPCfloat * output, int numSamples) { // error signal into out

  int i;
  int left= windowsize-pos;

	if(numSamples>=left) {
		lpc_preemphasis(newinput,numSamples,0.97);

		for (i=0; i<left;++i) {
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		calculateDurbPoles(); // this calculates the coeffs so... - these all give same results
		pos=0;
		predict(P_MAX,left,newinput,coeff,output); // this gives the error signal into newsource
		int remainder= numSamples-left;

			for (i=0; i<remainder;++i) {
			  //	inputty[pos++]= newinput[left+i];
			  float temp= newinput[left+i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
			//		calculateresOutput(newinput, output+left, pos-remainder, remainder);
			//		calculateOutput(tt, newsource+left, pos-remainder, remainder);
			predict(P_MAX,remainder,newinput,coeff,output+left); // this gives the error signal into newsource
	} else {
		lpc_preemphasis(newinput,numSamples,0.97);
		for (i=0; i<numSamples;++i) {
		  //			inputty[pos++]= newinput[i];
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		predict(P_MAX,numSamples,newinput,coeff,output); // this gives the error signal into newsource
	}
}

void LPCAnalysis_update(LPCfloat * newinput, LPCfloat *newsource, LPCfloat * output, int numSamples, int p) {

  int i; float tt[128];
	int left= windowsize-pos;
	do_impulse(tt, numSamples, 200);

	if(numSamples>=left) {
		lpc_preemphasis(newinput,numSamples,0.97);

		for (i=0; i<left;++i) {
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		calculateDurbPoles(); // this calculates the coeffs so... - these all give same results
//		calculatePraatPoles(); // this calculates the coeffs so...
//		calculatePoles(); // this calculates the coeffs so... - which one is fastest? timing?
		pos=0;
		predict(P_MAX,left,newinput,coeff,newsource); // this gives the error signal into newsource
		int remainder= numSamples-left;

			for (i=0; i<remainder;++i) {
			  //	inputty[pos++]= newinput[left+i];
			  float temp= newinput[left+i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
			//		calculateresOutput(newinput, output+left, pos-remainder, remainder);
			//		calculateOutput(tt, newsource+left, pos-remainder, remainder);
			predict(P_MAX,remainder,newinput,coeff,newsource+left); // this gives the error signal into newsource
	} else {
		lpc_preemphasis(newinput,numSamples,0.97);
		for (i=0; i<numSamples;++i) {
		  //			inputty[pos++]= newinput[i];
		  float temp= newinput[i]*window128[pos]; //where are we in window 
		  inputty[pos++]=temp;
		  newinput[i]=temp;
		}
		///		calculateresOutput(newinput, output, pos-numSamples, numSamples);
		//		calculateOutput(tt, newsource, pos-numSamples, numSamples);
		predict(P_MAX,numSamples,newinput,coeff,newsource); // this gives the error signal into newsource
	}

}

void lpctimer(LPCfloat *in){
  int i;
  for (i=0; i<128;++i) {
    inputty[i]=in[i];
  }

  for (i=0;i<10000;i++){
    //		calculateDurbPoles(); // this calculates the coeffs so... - these all give same results
    //    calculatePraatPoles(); // this calculates the coeffs so...
//		calculatePoles(); // this calculates the coeffs so... - which one is fastest? timing?
  }

}

void main(int argc, char * argv []){

  // read in wav file(?) into float and output coefficients as array
	char 		*progname, *infilename, *outfilename ;
	SNDFILE	 	*infile = NULL ;
	SNDFILE		*outfile = NULL ;
	SF_INFO	 	sfinfo ;
	
	float pout[1024];

	infilename = argv [1] ;

	// for guassian window!

	double imid = 0.5 * (128 + 1), edge = exp (-12.0);
	for (long i = 1; i <= 128; i++) {
	  float xx = (exp (-48.0 * (i - imid) * (i - imid) / (128 + 1) / (128 + 1)) - edge) / (1 - edge);
	  //	  printf("%f, ",xx);
	}


	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		} ;

	if (! (outfile = sf_open("output.wav", SFM_WRITE, &sfinfo)))
	  {   printf ("Not able to open output file %s.\n", "output.wav") ;
	    sf_perror (NULL) ;
	    //	    return  1 ;
	  } ;

		printf ("# Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;

	LPCAnalyzer_init();

	// read in 32 samples and print coeffs to test
	//	float buf [BLOCK_SIZE] ;
	int k, m, readcount,count=0;
	static float input[1280]; 
	static float output[1280];

	while ((readcount = sf_readf_float (infile, input,CHUNKSIZE)) > 0)
	//			while ((readcount = sf_readf_double (infile, input, BLOCK_SIZE)) > 0)
	{	
	  //	  LPCAnalysis_update(input, pout, output, CHUNKSIZE, P_MAX);//
	  //	  LPC_cross(input, pout, output, CHUNKSIZE);
	  //	  printf("SAMPLE %f", input[16]);

	  LPC_residual(input,output, CHUNKSIZE);
	  sf_writef_float (outfile, output, readcount) ;
	  //		sf_writef_double (outfile, output, readcount) ;
		count++;
		} ;

	sf_close (infile) ;
	sf_close (outfile) ;
}


