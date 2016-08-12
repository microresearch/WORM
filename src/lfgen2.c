/* 

TODO: finish shimmer, all needs cleaning so does arbitrary numbers of samples in each case

LF is from: gm.py

Rosenberg is from lfgen.c

KLGLOTT88 is from glottalair.py

shimmer is from err flowgen_shimmer.c

 */

#include <errno.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/times.h>

typedef unsigned int u16;

#define PI 3.14159265359
#define TRUE 1
#define FALSE 0

FILE* fo;// = fopen("testlfgen.pcm", "wb");

signed short truncate(float aux)
{
  signed short i;
  if(aux > 32767) i = 32767;
  else if(aux < -32767) i = -32767;
  else i = (signed short) ceil(aux);

  return i;

}


typedef struct PAR {
  float dur,		/* duration of the created file */
	jitter,
	cq,
	K,		/* speed of closure */
	Fg,		/* glottal formant; Fg = 1/(2.T2) */
	F0,		/* Fg > F0 */
	DC,		/* DC flow (% of max amplitude) */
	noise;		/* pow(10, arg.noise/10) */

  long fs;		/* sampling rate */
   int amp,             /* maximum amplitude */
       NoiseDistWidth;	/* noise = uniform(0,...,NoiseDistWidth) */
  float Kvar,Shimmer;           /* speed closure variation, Shimmer */
} PAR;

struct PAR parry;

static int i, j, k,			/* general use */
      P,			/* nominal Pitch period */
      T,			/* real Pitch period (jittered) */
      T2,			/* = 1/(w.Fg) */
      T3,			/* point in the closing phase where flow = 0 */
      T4,			/* instant where the rising flow = DCflow */
      w[500];			/* vector for additive white noise */

static unsigned long nSamples, CountSamples;
static float aux,
	x_pow,
	w_pow,
  J, jitter,Shimmer,                      /* random jitter */
	DeltaPer[2] = {0, 0};
static float S, DeltaShimmer[2] = {0, 0};

void shimmer_init(PAR* param){
  // fill in param values
  srandom(time(NULL));
  P = T = (int) ( (float) param->fs/param->F0);
}

void do_shimmer(int* x, PAR* param, int numsamples){
    if(param->jitter != -1 && jitter != 0.0) {

      DeltaPer[1] = DeltaPer[0];
      do {
	J = (  random() / (RAND_MAX * 10000.0) ) * 40000.0 * param->jitter -
	    2.0*param->jitter;

	DeltaPer[0] = DeltaPer[1]*(2.0 + J)/(2.0 - J) +
		      2.0*P*J/(2.0 - J);

	T = (signed short) ceil((float) P + DeltaPer[0]);
      } while ( (float) T > (float) 1.2*P || (float) T < (float) 0.8*P);
    }

    float Amplitude;

   if(param->Shimmer != -1 && Shimmer != 0.0) {
   DeltaShimmer[1] = DeltaShimmer[0];
      do {
	float epsilon =  ((float)random()) / RAND_MAX;
	
	S =  epsilon  * 4.0 * param->Shimmer - 2.0*param->Shimmer;

	DeltaShimmer[0] = DeltaShimmer[1]*(2.0 + S)/(2.0 - S) +
		      2.0*param->amp*S/(2.0 - S);

	Amplitude = ((float) param->amp + DeltaShimmer[0]);
      } while (  (Amplitude > (float) 1.8*param->amp) || (Amplitude < (float) 0.2*param->amp));
      //	printf("%5.2f \n", S);
	
    }
    else
    {
	Amplitude = (float) param->amp;
    }

    /* generate glottal flow */
    T2 = ceil(0.5*param->cq*P);
    for(i=0; i<T2; i++) {
      x[i] = ceil( Amplitude * 0.5*(1.0 - cos(PI*i/T2) ));
      if(x[i] < param->DC) {
	x[i] = param->DC;
	T4 = i;
      }
    }
    float Knew = param->K * ( 1 + 2 * param->Kvar *  ( ( (1.0 * random())/RAND_MAX )  - 0.5));
    for(i=T2; i<2*T2; i++) {
      x[i] = ceil( (float) Amplitude*(Knew*cos(PI*(i-T2)/T2) - Knew + 1.0));
      if(x[i] < param->DC) break;
    }

    T3 = i;

    for(i=T3; i<T; i++) {
      x[i] = param->DC;
    }

    if(param->noise != -1) {
      aux = 0.0;
      for(i=T4; i<T3; i++) {
	aux += (float) x[i]*x[i];
      }
      x_pow = aux/((float) T3 - T4);


      aux = 1.0 + ((float) T3 - T4)/((float) T);
      param->NoiseDistWidth = sqrt(12*aux*(x_pow)/param->noise);

      aux = 0.0;
      for(i=0; i<T4; i++) {

	w[i] = (signed short) ceil( ((  1.0 * random()) / RAND_MAX )* param->NoiseDistWidth - param->NoiseDistWidth/2.0);

	aux += (float) w[i]*w[i];
	x[i] = truncate( (float) x[i] + w[i]);
      }
      for(i=T3; i<T; i++) {

	w[i] = (signed short) ceil( ( ( 1.0*random() )/RAND_MAX )* param->NoiseDistWidth - param->NoiseDistWidth/2.0);
	aux += (float) w[i]*w[i];
	x[i] = truncate( (float) x[i] + w[i]);
      }
      w_pow = aux/( (float) T);

      //      printf("SNRdb = %5.2f\n", 10.0*log10(x_pow/w_pow));

    }
}


/////////////////////////////////////

typedef struct {
  int samplingRate;// # Hertz
  int freq,n;// # F0, given in Hertz (T0 = 1 / F0)
  float CQ;// # closed quotient
  float amp;//, # amplitude of voicing
  float a, b, period, T0, OQ, openPhaseCorrection, amplitudeCorrectionFactor,totalOffset;// what else?
} klglott;

float doklglott(klglott* self){
  //      # puts out the 1st derivative of the glottal flow wave
  float valOut = 0.0f;

  if (self->n < (self->T0 * self->OQ * self->samplingRate)){
  //          # open phase
    float tmp = (float)self->n / (float)(self->samplingRate);
    valOut = 2.0 * self->a * (float)self->n / self->samplingRate - 3.0 * self->b * tmp * tmp;
    //          #valOut = self->a * self->n * self->n - self->b * self->n * self->n * self->n
    valOut -= self->openPhaseCorrection;
    valOut /= self->amplitudeCorrectionFactor;
  }
    self->n += 1;
    if (self->n >= (self->T0 * self->samplingRate))  self->n = 0;  
  return valOut;
}

void setfreq(klglott* self, int freq){
  self->freq = freq;
  self->period = 1.0 / self->freq;
  self->T0 = self->period;// * (float)(self->samplingRate);
    self->OQ = 1.0 - self->CQ;
    self->a = 0.0;
    self->b = 0.0;
	  //        self->calculateParams() ->>>>>>>>>>>>>>>
    self->a = 27.0 * self->amp / (4.0 * self->OQ * self->OQ * self->T0);
    self->b = 27.0 * self->amp / (4.0 * self->OQ * self->OQ * self->OQ * self->T0 * self->T0);

    self->n = 0.0;
      
    self->totalOffset = 0.0;
    self->openPhaseCorrection = 0.0;
    self->amplitudeCorrectionFactor = 1.0;
    int framesPerPeriod = (int)(self->period * (float)(self->samplingRate));
    int i;
    for (i=0;i<framesPerPeriod;i++) self->totalOffset += doklglott(self);
    self->openPhaseCorrection = self->totalOffset / (self->OQ * (float)(framesPerPeriod));
    self->n = 0.0;
    self->amplitudeCorrectionFactor = (float)(self->samplingRate) / (float)(freq);
}

void klglott88_init(klglott* self, int samplingRate, int freq, float CQ, float amp){
  self->samplingRate=samplingRate;
  self->amp=amp;
  self->CQ=CQ;
  setfreq(self, freq);
} 

void pulse_lf(int howmany, double *pulse, double T0, double Te, double alpha, double omega, double eps){
  //    Assumes E_e = -1.
  int i; int n=howmany; int s16;
  for (i=0;i<(int)(Te/T0*n+0.5);i++){
    double t = (double)(i)/n*T0;
    pulse[i] = -(exp(alpha*t)  * sin(omega*t)     /       exp(alpha*Te) / sin(omega*Te));
    //    printf("%f\n", pulse[i]);
    s16=pulse[i]*32768.0;
   fwrite(&s16,2,1,fo);
  }
	
  for (i=(int)(Te/T0*howmany+0.5);i<n;i++){
    double t = (double)(i)/n*T0;
    pulse[i] = -((exp(-(t-Te)*eps) - exp(-(T0-Te)*eps)) /            (1.0 - exp(-(T0-Te)*eps)));
    //    printf("%f\n", pulse[i]);
    s16=pulse[i]*32768.0;
   fwrite(&s16,2,1,fo);
  }
}

double lf_alpha(double tp, double te, double epsilon, double T0){
/*    """
    Given the three timing parameters and a starting point, uses
    Newton-Raphson to find a value of alpha for the Liljencrants-Fant
    glottal pulse shape.
    """*/
  double tc = T0; double alpha=0.0f;
  double omega = PI / tp; int i;
  for (i=0;i<5;i++){
    double expy = exp(-epsilon*(tc-te));
    double esin = exp(alpha*te)*sin(omega*te);
        double f = (            alpha            - omega/tan(omega*te)            + omega/esin            - (alpha*alpha + omega*omega)*((tc-te)*expy/(1.0-expy) - 1.0/epsilon)	     );
        double fd = (            1.0            - omega*te/esin            - 2.0*alpha*((tc-te)*expy/(1.0-expy) - 1.0/epsilon)            );
        alpha -= f/fd;
  }
	return alpha;
}

double lf_epsilon(double te, double ta, double T0){
  /*    """
    Given Ta, uses Newton-Raphson to find epsilon in an LF model.
    """*/
  double tce = T0 - te; int i;
  double epsilon = 1.0/ta;
  for (i=0;i<5;i++){
    double f = 1.0 - exp(-epsilon * tce) - ta * epsilon;    
    double fd = tce * exp(-epsilon * tce) - ta;
    epsilon -= f/fd;
  }
  return epsilon;
      }

void main(){
  int z;
  fo = fopen("testlfgen.pcm", "wb");
  double pulse[32000];
  double Fa= 50;// ???    
  double Rg= 1.2;//
  double Rk= 0.9; //
  double T0=0.02; // fundamental period in seconds??? can't be less than 0.001 or??? - 500 Hz - is same as Fa or????

  double     Ta = 1.0/(2.0*PI*Fa);
  double             Tp = T0/(2.0*Rg);
  double       Te = Tp*(Rk+1.0);
  double eps = lf_epsilon(Te, Ta, T0);
  double             alpha = lf_alpha(Tp, Te, eps, T0);
  double             omega = PI / Tp;

  //  printf("alpha %f omega %f eps %f\n",alpha,omega, eps);
  //  for (z=0;z<1000;z++){
  //  pulse_lf(32000*T0, pulse, T0,Te, alpha, omega, eps);
  //  }
  //}
  float recentY;
  int s16;
  klglott glotty;
  klglott88_init(&glotty, 32000, 200, 0.15, 1.0);

  for (z=0;z<32000;z++){
  float val=doklglott(&glotty);
  float y= recentY + val;
  recentY = y;
  //  printf("%f\n", y);
  s16=y*32768.0;
  fwrite(&s16,2,1,fo);
  }
}  


/*
        elif ptype == 'lf':
            # These three are all in seconds
            Ta = 1.0/(2.0*np.pi*Fa)
            Tp = T0/(2.0*Rg)
            Te = Tp*(Rk+1.0)
            eps = lf_epsilon(Te, Ta, T0)
            alpha = lf_alpha(Tp, Te, eps, T0)
            omega = np.pi / Tp
            pulse_lf(pulse, T0, Te, alpha, omega, eps)
*/
