#include <math.h>
#include <stdlib.h>
#include <sndfile.h>
#include "forlap.h"

// 44100 so 0.005 seconds would be 220 samples - close to 256 

#define BLOCK_SIZE 32
#define	P_MAX	10	/* order p of LPC analysis, typically 8..14 */

// what is samplerate

typedef double LPCfloat;
static const int windowsize=BLOCK_SIZE; // say up to 1024
static LPCfloat inputty[129];

static LPCfloat window128[1024]={0.000003, 0.000007, 0.000012, 0.000020, 0.000031, 0.000045, 0.000066, 0.000094, 0.000132, 0.000184, 0.000254, 0.000346, 0.000470, 0.000633, 0.000846, 0.001125, 0.001485, 0.001950, 0.002544, 0.003300, 0.004256, 0.005455, 0.006953, 0.008810, 0.011098, 0.013900, 0.017308, 0.021428, 0.026375, 0.032277, 0.039273, 0.047510, 0.057144, 0.068335, 0.081248, 0.096044, 0.112883, 0.131909, 0.153256, 0.177034, 0.203323, 0.232174, 0.263593, 0.297542, 0.333931, 0.372615, 0.413388, 0.455984, 0.500077, 0.545278, 0.591145, 0.637184, 0.682857, 0.727594, 0.770803, 0.811880, 0.850228, 0.885265, 0.916443, 0.943263, 0.965282, 0.982134, 0.993531, 0.999279, 0.999279, 0.993531, 0.982134, 0.965282, 0.943263, 0.916443, 0.885265, 0.850228, 0.811880, 0.770803, 0.727594, 0.682857, 0.637184, 0.591145, 0.545278, 0.500077, 0.455984, 0.413388, 0.372615, 0.333931, 0.297542, 0.263593, 0.232174, 0.203323, 0.177034, 0.153256, 0.131909, 0.112883, 0.096044, 0.081248, 0.068335, 0.057144, 0.047510, 0.039273, 0.032277, 0.026375, 0.021428, 0.017308, 0.013900, 0.011098, 0.008810, 0.006953, 0.005455, 0.004256, 0.003300, 0.002544, 0.001950, 0.001485, 0.001125, 0.000846, 0.000633, 0.000470, 0.000346, 0.000254, 0.000184, 0.000132, 0.000094, 0.000066, 0.000045, 0.000031, 0.000020, 0.000012, 0.000007, 0.000003};

static const LPCfloat window32[32]={0.000019, 0.000088, 0.000318, 0.001015, 0.002934, 0.007748, 0.018718, 0.041390, 0.083793, 0.155316, 0.263593, 0.409601, 0.582778, 0.759205, 0.905585, 0.989041, 0.989041, 0.905585, 0.759205, 0.582778, 0.409601, 0.263593, 0.155316, 0.083793, 0.041390, 0.018718, 0.007748, 0.002934, 0.001015, 0.000318, 0.000088, 0.000019};


// gaussian window of size 256 generated below

static LPCfloat input[1280];
static LPCfloat output[1280];

static LPCfloat last[1024];
static LPCfloat delay[260];
static LPCfloat coeff[320], prevcoeff[320];
static LPCfloat G; //gain;

static LPCfloat R[256];
static LPCfloat preva[256];
//static LPCfloat a[256];

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

void calculatepraatPoles(){
	int i = 1; // For error condition at end
	int m = P_MAX; int nx=BLOCK_SIZE;
	LPCfloat gain; LPCfloat r[320], rc[320], a[512];

	for (i=0;i<=P_MAX+1;i++){
	  r[i]=0.0f;rc[i]=0.0f;a[i]=0.0f;
		      }

	//	inputty[128]=0.0;
	//	LPCfloat  *x = inputty;

	/*	for(i=0; i<=P_MAX; ++i) {
		sum=0.0;
		for (j=0; j<= windowsize-1-i; ++j)
			sum+= inputty[j]*inputty[j+i];
		R[i]=sum;
		printf("i: %d SUM: %f,,,,\n", i, R[i]);
	}
*/

		for (i = 1; i <= m + 1; i++) {
		for (long j = 1; j <= nx - i + 1; j++) {
			r[i] += inputty[j] * inputty[j + i - 1];
			//				printf("%d ,",j + i - 1);
		}
		//			printf("%f\n",r[i]);
		}

	if (r[1] == 0.0) {
		i = 1; /* ! */ goto end;
	}
	a[1] = 1; a[2] = rc[1] = - r[2] / r[1];
	gain = r[1] + r[2] * rc[1];
	for (i = 2; i <= m; i++) {
		float s = 0.0;
		for (long j = 1; j <= i; j++) {
			s += r[i - j + 2] * a[j];
		}
		rc[i] = - s / gain;
		for (long j = 2; j <= i / 2 + 1; j++) {
			float at = a[j] + rc[i] * a[i - j + 2];
			a[i - j + 2] += rc[i] * a[j];
			a[j] = at;
		}
		a[i + 1] = rc[i]; gain += rc[i] * s;
		if (gain <= 0) {
		  goto end;
		}
	}
end:
	i--;
	for (long j = 1; j <= i; j++) {
		coeff[j] = a[j + 1];
		printf("A[j]= %f j= %d, i= %d\n", coeff[j],j,i);
	}
	if (i == m) {
		return;
	}
	//	nCoefficients = i;
	for (long j = i + 1; j <= m; j++) {
		coeff[j] = 0.0;
	}
	return; // Melder_warning ("Less coefficienst than asked for.");
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
	////////



    LPCfloat error = R[0];

//	if (R[0] == 0) {
	  //		for (i = 0; i < P_MAX; i++) ref[i] = 0; 
//		return 0; }

for (i = 0; i < P_MAX; i++) {

		/* Sum up this iteration's reflection coefficient.
		*/
r = -R[i + 1];
for (j = 0; j < i; j++) r -= coeff[j] * R[i - j];
r /= error;

		/*  Update LPC coefficients and total error.
		*/
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

  for (i = 0; i < P_MAX; i++)  printf("COEFF %f i %d\n", coeff[i], i);


	G= sqrtf(error);

}

void calculatePoles() {
  int i; int j;
	LPCfloat sum;
	LPCfloat E, k;

	// this is as autocorrelation

	for(i=0; i<=P_MAX; ++i) {
		sum=0.0;
		for (j=0; j<= windowsize-1-i; ++j)
			sum+= inputty[j]*inputty[j+i];
		R[i]=sum;
		printf("i: %d SUM: %f,,,,\n", i, R[i]);
	}
	////////

	E= R[0];
	k=0;

	if(E<0.00000000001) {

		//zero power, so zero all coeff
		for (i=0; i<P_MAX;++i)
			coeff[i]=0.0;
		G=0.0;
		return;
	};

	//rescaling may help with numerical instability issues?
	LPCfloat mult= 1.0/E;
	//	for(i=1; i<=P_MAX; ++i)
	//		R[i]= R[i]*mult;
//
	for(i=0; i<=(P_MAX+1); ++i) {
		coeff[i]=0.0;
		prevcoeff[i]=0.0; //CORRECTION prevcoeff[j]=0.0;
	}
	LPCfloat prevE= E;
	//////

	for(i=1; i<=P_MAX; i++) {
		sum=0.0;
		for(j=1;j<i;++j){
		  //			printf("iiiiii i %d j %d %d\n",i, j, i-j);
		  sum+= coeff[j]*R[i-j];
		}
		k=(-1.0*(R[i]+sum))/E;
		coeff[i]=k;
		for(j=1;j<=(i-1);++j){
			coeff[j]=prevcoeff[j]+(k*prevcoeff[i-j]);
		}
		for(j=1;j<=i;++j)
			prevcoeff[j]=coeff[j];
		E= (1-k*k)*E;
		if(E<0.00000000001) {
		  return;
		};
	}

	G= sqrtf(E);
	for(i=0; i<P_MAX; ++i) {
		coeff[i]=coeff[i+1];
	}

  for (i = 0; i < P_MAX; i++)  printf("COEFF %f i %d\n", coeff[i], i);


}

void zeroAll() {
  int i;
  //  P_MAX=10;
  for (i=0; i<windowsize;++i) {
    inputty[i]= 0.0f;
    last[i]=0.0f;
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

void calculateresOutput(LPCfloat * source, LPCfloat * target, int startpos, int num) {
  int j; int i;
	int basepos,posnow;
	G=1.0; // TESTY!

	for(i=0; i<num; ++i) {
		basepos= startpos+i+windowsize-1; //-1 since coefficients for previous values starts here
		//		LPCfloat sum=0.0;
		LPCfloat sum=source[i];
		int m = i > P_MAX ? P_MAX : i - 1;
		for(j=1; j<=m; ++j) {
		  //		  posnow= (basepos-j)%windowsize;
		  sum += source[i-j]*coeff[j];  //		  sum+=coeff[j]*input[i-j];
			//						printf("i-j %d vs posnowread %d startpos+i=%d last write \n",i-j, posnow, startpos+i);

		}
		//		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below		
		//		source[startpos+i]=source[i];
		target[i]= sum;
	}
}

void calculateiirOutput(LPCfloat * source, LPCfloat * target, int startpos, int num) {
  int j; int i;
	int basepos,posnow;
	G=1.0; // TESTY!

	for(i=0; i<num; ++i) {
		basepos= startpos+i+windowsize-1; //-1 since coefficients for previous values starts here
		LPCfloat sum=source[i];
		int m = i > P_MAX ? P_MAX : i - 1;
		for(j=1; j<=P_MAX; ++j) {
		  //			posnow= (basepos-j)%windowsize;
			sum -= source[i-j]*coeff[j];  	//		y[i] -= a[j] * y[i - j];
//		  sum+=coeff[j]*input[i-j];
		}
		//		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below		

		//		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below		
		source[i]=sum;
		target[i]= sum;
	}
}


void LPCAnalysis_update(LPCfloat * newinput, LPCfloat * output, int numSamples, int p) {

	int i;
	int left= windowsize;
		///void lpc_preemphasis(float * x, int len, float alpha )
	lpc_preemphasis(newinput,numSamples,0.95);
		//		for (i=0;i<numSamples;i++) newinput[i]*=window128[i];
	for (i=1; i<=left;++i) {// for praat
	  inputty[i]= input[i-1];//*window32[i];
		  //		  inputty[i]= 0.3f;
		  //		  printf("%d\n",i);
		}
		calculatepraatPoles(); // this calculates the coeffs so...

		calculateresOutput(newinput, output, windowsize-left, left);
}
			  
void LPCAnalyzer_cross(LPCfloat *in, LPCfloat *sourcein, LPCfloat *out, int p, int numSamples) {
  // test first with 
  //  LPCAnalysis_update(in, sourcein, out, numSamples, 10);
}


void main(int argc, char * argv []){

  // read in wav file(?) into float and output coefficients as array
	char 		*progname, *infilename, *outfilename ;
	SNDFILE	 	*infile = NULL ;
	SNDFILE		*outfile = NULL ;
	SF_INFO	 	sfinfo ;
	
	LPCfloat pout[512];

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
	float buf [BLOCK_SIZE] ;
	int k, m, readcount,count=0;

	//				while ((readcount = sf_readf_float (infile, input, BLOCK_SIZE)) > 0)
				while ((readcount = sf_readf_double (infile, input, BLOCK_SIZE)) > 0)
	{	
		LPCAnalysis_update(input, output, BLOCK_SIZE, P_MAX);//
		//calculateiirOutput(output, pout,0,BLOCK_SIZE);		//calculateiirOutput
		//	sf_writef_float (outfile, output, readcount) ;
		sf_writef_double (outfile, output, readcount) ;
		count++;
		} ;

	sf_close (infile) ;
	sf_close (outfile) ;
}


