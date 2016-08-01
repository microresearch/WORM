#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define twopi 6.28318530717952646f
#define PI      3.14159265358979324

const int kSineSize = 511; // was 8192 for inv sine business

unsigned bitrev(unsigned int k, int nu)
/* register unsigned k; * Number to reverse the bits of */
/* int nu;              * Number of bits k is represented by */
{
    register int i;
    register unsigned out = 0;
    
    for (i = 0; i < nu; i++) {
        out <<= 1;
        out |= k & 1;
        k >>= 1;
    }
    return(out);
}


int ilog2(int n)
{
    register int i;
    
    for (i = -1; n != 0; i++, n>>=1)
        ;
    return(i);
}

int ipow(int a, int b)
{
    register int i;
    int sum = 1;
    
    for (i = 0; i < b; i++)     
        sum *= a;
    return (sum);
}


void main(void){
  int i;
const float kBadValue = 10000000.0f; // used in the secant table for values very close to 1/0
//double sineIndexToPhase = twopi / kSineSize;
  float gInvSine[kSineSize];
  float gSine[kSineSize];
  //for (i=0; i < kSineSize; ++i) {
  //  double phase = i * sineIndexToPhase;
  //  float d = sin(phase);
  //  gSine[i] = d;
  // printf("%f, ",d);
  // printf("%d, ",i);
	double sineIndexToPhase = twopi / kSineSize;
	double pmf = (1L << 29) / twopi;
	for (i=0; i <= kSineSize; ++i) {
		double phase = i * sineIndexToPhase;
		float d = sin(phase);
		gSine[i] = d;
				//		gInvSine[i] = 1. / d;
		 }

	gInvSine[0] = gInvSine[kSineSize/2] = gInvSine[kSineSize] = kBadValue;
	int sz = kSineSize;
	int sz2 = sz>>1;
	for (i=1; i<=8; ++i) {
		gInvSine[i] = gInvSine[sz-i] = kBadValue;
		gInvSine[sz2-i] = gInvSine[sz2+i] = kBadValue;
	}
	for (i=0; i <= kSineSize; ++i) {
	  //  printf("%f, ",gInvSine[i]);
	    printf("%f, ",gSine[i]);
	}



int xx=ipow(2, ilog2(32000 / 15));
// printf("log %d",xx);

int n=256;                          /* Number of points */


//void fft_create_arrays(c, s, rev, n)
//REAL=float **c, **s;          /* Sin and Cos arrays (to be returned) */

 int nu = ilog2(128); // 256 is window_length

    /* Compute temp array of sins and cosines */
    //    *c = (REAL *)emalloc(n * sizeof(REAL));
    //    *s = (REAL *)emalloc(n * sizeof(REAL));
    //    *rev = (int *)emalloc(n * sizeof(int));
  
 //printf("%d",nu);  

 n=2048;

    for (i = 0; i < n; i++) {
        float arg = 2 * PI * i/n;
        float c = cos(arg);
        float s = sin(arg);
        int rev = bitrev(i, nu);
	//	printf("%f, ",s);
    }

    /*    StkFloat temp = 1.0 / TABLE_SIZE;
    for ( unsigned long i=0; i<=TABLE_SIZE; i++ )
      table_[i] = sin( TWO_PI * i * temp );
      }*/


for (i = 1; i < n/2; i++) {

float arg = 2.0f * PI * (float)i / (float)n; // AS LOOKUP n/2=128
float cc = cosf(arg);   /* These are different c,s than used in fft */
float ss = sinf(arg);
//printf("%f, ",ss);
}

}
