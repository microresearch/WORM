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

#define TWO_PI 6.28318530717958647693
#define TABLE_LENGTH              512
#define TABLE_MODULUS             (TABLE_LENGTH-1)

#define OVERSAMPLING_OSCILLATOR 0

#define FIR_BETA                  .2
#define FIR_GAMMA                 .1
#define FIR_CUTOFF                .00000001

typedef struct {
    float FIRData[49], *FIRCoef;
    int FIRPtr, numberTaps;
} TRMFIRFilter;



// filter calc
#define LIMIT                     200
#define BETA_OUT_OF_RANGE         1
#define GAMMA_OUT_OF_RANGE        2
#define GAMMA_TOO_SMALL           3

void rationalApproximation(double number, int *order, int *numerator, int *denominator)
{
    double fractionalPart, minimumError = 1.0;
    int i, orderMaximum, modulus = 0;


    /*  RETURN IMMEDIATELY IF THE ORDER IS LESS THAN ONE  */
    if (*order <= 0) {
	*numerator = 0;
	*denominator = 0;
	*order = -1;
	return;
    }

    /*  FIND THE ABSOLUTE VALUE OF THE FRACTIONAL PART OF THE NUMBER  */
    fractionalPart = fabs(number - (int)number);

    /*  DETERMINE THE MAXIMUM VALUE OF THE DENOMINATOR  */
    orderMaximum = 2 * (*order);
    orderMaximum = (orderMaximum > LIMIT) ? LIMIT : orderMaximum;

    /*  FIND THE BEST DENOMINATOR VALUE  */
    for (i = (*order); i <= orderMaximum; i++) {
	double ps = i * fractionalPart;
	int ip = (int)(ps + 0.5);
	double error = fabs( (ps - (double)ip)/(double)i );
	if (error < minimumError) {
	    minimumError = error;
	    modulus = ip;
	    *denominator = i;
	}
    }

    /*  DETERMINE THE NUMERATOR VALUE, MAKING IT NEGATIVE IF NECESSARY  */
    *numerator = (int)fabs(number) * (*denominator) + modulus;
    if (number < 0)
	*numerator *= (-1);

    /*  SET THE ORDER  */
    *order = *denominator - 1;

    /*  RESET THE NUMERATOR AND DENOMINATOR IF THEY ARE EQUAL  */
    if (*numerator == *denominator) {
	*denominator = orderMaximum;
	*order = *numerator = *denominator - 1;
    }
}


int maximallyFlat(double beta, double gamma, int *np, double *coefficient)
{
    double a[LIMIT+1], c[LIMIT+1], betaMinimum, ac;
    int nt, numerator, n, ll, i;


    /*  INITIALIZE NUMBER OF POINTS  */
    (*np) = 0;

    /*  CUT-OFF FREQUENCY MUST BE BETWEEN 0 HZ AND NYQUIST  */
    if ((beta <= 0.0) || (beta >= 0.5))
	return BETA_OUT_OF_RANGE;

    /*  TRANSITION BAND MUST FIT WITH THE STOP BAND  */
    betaMinimum = ((2.0 * beta) < (1.0 - 2.0 * beta)) ? (2.0 * beta) :
	(1.0 - 2.0 * beta);
    if ((gamma <= 0.0) || (gamma >= betaMinimum))
	return GAMMA_OUT_OF_RANGE;

    /*  MAKE SURE TRANSITION BAND NOT TOO SMALL  */
    nt = (int)(1.0 / (4.0 * gamma * gamma));
    if (nt > 160)
	return GAMMA_TOO_SMALL;

    /*  CALCULATE THE RATIONAL APPROXIMATION TO THE CUT-OFF POINT  */
    ac = (1.0 + cos(TWO_PI * beta)) / 2.0;
    rationalApproximation(ac, &nt, &numerator, np);

    /*  CALCULATE FILTER ORDER  */
    n = (2 * (*np)) - 1;
    if (numerator == 0)
	numerator = 1;


    /*  COMPUTE MAGNITUDE AT NP POINTS  */
    c[1] = a[1] = 1.0;
    ll = nt - numerator;

    for (i = 2; i <= (*np); i++) {
	int j;
	double x, sum = 1.0, y;
	c[i] = cos(TWO_PI * ((double)(i-1)/(double)n));
	x = (1.0 - c[i]) / 2.0;
	y = x;

	if (numerator == nt)
	    continue;

	for (j = 1; j <= ll; j++) {
	    double z = y;
	    if (numerator != 1) {
		int jj;
		for (jj = 1; jj <= (numerator - 1); jj++)
		    z *= 1.0 + ((double)j / (double)jj);
	    }
	    y *= x;
	    sum += z;
	}
	a[i] = sum * pow((1.0 - x), numerator);
    }


    /*  CALCULATE WEIGHTING COEFFICIENTS BY AN N-POINT IDFT  */
    for (i = 1; i <= (*np); i++) {
	int j;
	coefficient[i] = a[1] / 2.0;
	for (j = 2; j <= (*np); j++) {
	    int m = ((i - 1) * (j - 1)) % n;
	    if (m > nt)
		m = n - m;
	    coefficient[i] += c[m+1] * a[j];
	}
	coefficient[i] *= 2.0 / (double)n;
    }

    return 0;
}

void trim(double cutoff, int *numberCoefficients, double *coefficient)
{
    int i;

    for (i = (*numberCoefficients); i > 0; i--) {
	if (fabs(coefficient[i]) >= fabs(cutoff)) {
	    (*numberCoefficients) = i;
	    return;
	}
    }
}

TRMFIRFilter *TRMFIRFilterCreate(double beta, double gamma, double cutoff)
{
    TRMFIRFilter *newFilter;

    int i, pointer, increment, numberCoefficients;
    double coefficient[LIMIT+1];

    newFilter = (TRMFIRFilter *)malloc(sizeof(TRMFIRFilter));
    if (newFilter == NULL) {
        fprintf(stderr, "Couldn't malloc() FIRFilter.\n");
        return NULL;
    }

    /*  DETERMINE IDEAL LOW PASS FILTER COEFFICIENTS  */
    maximallyFlat(beta, gamma, &numberCoefficients, coefficient);

    /*  TRIM LOW-VALUE COEFFICIENTS  */
    trim(cutoff, &numberCoefficients, coefficient);

    /*  DETERMINE THE NUMBER OF TAPS IN THE FILTER  */
    newFilter->numberTaps = (numberCoefficients * 2) - 1;

    /*  ALLOCATE MEMORY FOR DATA AND COEFFICIENTS  */
//    newFilter->FIRData = (double *)calloc(newFilter->numberTaps, sizeof(double));
    if (newFilter->FIRData == NULL) {
        fprintf(stderr, "calloc() of FIRData failed.\n");
        free(newFilter);
        return NULL;
    }

    //    newFilter->FIRCoef = (double *)calloc(newFilter->numberTaps, sizeof(double));
    if (newFilter->FIRCoef == NULL) {
        fprintf(stderr, "calloc() of FIRCoef failed.\n");
        free(newFilter->FIRData);
        free(newFilter);
        return NULL;
    }

    /*  INITIALIZE THE COEFFICIENTS  */
    increment = -1;
    pointer = numberCoefficients;
    for (i = 0; i < newFilter->numberTaps; i++) {
	newFilter->FIRCoef[i] = coefficient[pointer];
	pointer += increment;
	if (pointer <= 0) {
	    pointer = 2;
	    increment = 1;
	}
    }

    /*  SET POINTER TO FIRST ELEMENT  */
    newFilter->FIRPtr = 0;

    /*  PRINT OUT  */
    printf("\n");
    for (i = 0; i < newFilter->numberTaps; i++)
	printf("%11.8f, ", newFilter->FIRCoef[i]);
    printf("\nTAPPPP %d\n", newFilter->numberTaps);

    return newFilter;
}

/*     newWavetable->FIRFilter = TRMFIRFilterCreate(FIR_BETA, FIR_GAMMA, FIR_CUTOFF);

typedef struct {
    double *FIRData, *FIRCoef;
    int FIRPtr, numberTaps;
} TRMFIRFilter;

TRMFIRFilter *TRMFIRFilterCreate(double beta, double gamma, double cutoff);
void TRMFIRFilterFree(TRMFIRFilter *filter);

double FIRFilter(TRMFIRFilter *filter, double input, int needOutput);
*/


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
