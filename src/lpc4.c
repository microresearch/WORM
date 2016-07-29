#include "audio.h"
#include <math.h>
#include "arm_const_structs.h"

extern uint16_t adc_buffer[10];

#define BLOCK_SIZE 32
#define	P_MAX	10	/* order p of LPC analysis, typically 8..14 */

// what is samplerate

typedef float LPCfloat;
static LPCfloat inputty[32];

static const LPCfloat window32[32]={0.000019, 0.000088, 0.000318, 0.001015, 0.002934, 0.007748, 0.018718, 0.041390, 0.083793, 0.155316, 0.263593, 0.409601, 0.582778, 0.759205, 0.905585, 0.989041, 0.989041, 0.905585, 0.759205, 0.582778, 0.409601, 0.263593, 0.155316, 0.083793, 0.041390, 0.018718, 0.007748, 0.002934, 0.001015, 0.000318, 0.000088, 0.000019};
// gaussian window of size 128 generated below in lpcforlap.c

static LPCfloat input[32];
static LPCfloat output[32];
static LPCfloat delay[128];
static LPCfloat coeff[32];

///source= Impulse.ar(delaytime.reciprocal); 
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

void lpc_preemphasis(LPCfloat * x, int len, LPCfloat alphaxx )
{
  u8 jj;
  for(jj = len - 1; jj > 0; jj-- )
      x[jj] = x[jj] - alphaxx * x[jj-1];////y[k]=x[k]-0.95x[k-1]
}

void calculatepraatPoles(){
	int i = 1; // For error condition at end
	int m = P_MAX; int nx=BLOCK_SIZE;
	LPCfloat gain; LPCfloat r[32], rc[32];

	LPCfloat  *x = inputty;
	for (i = 1; i <= m + 1; i++) {
		for (int j = 1; j <= nx - i + 1; j++) {
			r[i] += x[j] * x[j + i - 1];
		}
	}
	if (r[1] == 0.0) {
		i = 1; /* ! */ goto end;
	}
	coeff[1] = 1; coeff[2] = rc[1] = - r[2] / r[1];
	gain = r[1] + r[2] * rc[1];
	for (i = 2; i <= m; i++) {
		LPCfloat s = 0.0;
		for (int j = 1; j <= i; j++) {
			s += r[i - j + 2] * coeff[j];
		}
		rc[i] = - s / gain;
		for (int j = 2; j <= i / 2 + 1; j++) {
			LPCfloat at = coeff[j] + rc[i] * coeff[i - j + 2];
			coeff[i - j + 2] += rc[i] * coeff[j];
			coeff[j] = at;
		}
		coeff[i + 1] = rc[i]; gain += rc[i] * s;
		if (gain <= 0) {
			goto end;
		}
	}
end:
	i--;
	//	printf("COEFF[0]= %f\n", coeff[0]);
	for (int j = 1; j <= i; j++) {
		coeff[j] = coeff[j + 1];
		//		printf("COEFF[j]= %f j= %d\n", coeff[j],j);
	}
}

void calculateresOutput(LPCfloat * input, LPCfloat * target, int num) {
int j; int i;
int basepos,posnow;

 for(i=1; i<=num; ++i) { // was 0 and < for both loops but now checking praat
   LPCfloat sum=0.0f;//input[i]; // as in praat?
   int m = i > P_MAX ? P_MAX : i - 1;
   for(j=1; j<=m; j++) {
     sum+=coeff[j]*input[i-j];
   }
   //   target[i]= sum;//-last[basepos];
 }
}

void calculateiirOutput(LPCfloat * input, LPCfloat * target, int num) {
int j; int i;
int basepos,posnow;

 for(i=1; i<=num; ++i) { // was 0 and < for both loops but now checking praat
  LPCfloat sum=input[i]; // as in praat?
		int m = i > P_MAX ? P_MAX : i - 1;

		for(j=1; j<=m; j++) {
		  sum-=coeff[j]*input[i-j];
		}
		//		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below
		//		last[startpos+i]=input[i];
		target[i]= sum;//-last[basepos];
		input[i]=sum;
		//		printf("sum %f , sample %f\n", sum, input[i]);
		//		printf("coeef %f \\\\ \n",coeff[j]);
	}
}

void LPCAnalysis_update(LPCfloat * newinput, LPCfloat * output, int numSamples) {
  u8 i;
  //  lpc_preemphasis(newinput,numSamples,0.95f);
  	for (i=0;i<numSamples;i++) {
	  //	  newinput[i]*=window32[i];
	  inputty[i]= newinput[i];
	  }
		calculatepraatPoles(); // this calculates the coeffs so...

		calculateresOutput(newinput, output, numSamples);
		//			calculateiirOutput(output, output, numSamples);
}

void zeroAll() {
  int i;
  //  P_MAX=10;
  for (i=0; i<BLOCK_SIZE;++i) {
    inputty[i]= 0.0f;
  }

  for (i=0; i<P_MAX;++i) {
    coeff[i]= 0.0f;
  }
}

void LPCAnalyzer_init() {
  zeroAll();
}
