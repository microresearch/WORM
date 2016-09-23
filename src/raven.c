/* what do we have here?

- resonances with twotube and onetube delay line (finish float there-DONE)

- raven syrinx models: Mindlin, IF (double only?), gardner

TODO: dooble->floot and port tests, port in latest FLETCHER from raven_model_latestmari.py when we have working

split to raven_lap.c and raven.c ... NO - back now just do as define

- but watch pow->powf etc....

//

play -t raw --bits 16 --channels 1 --encoding signed-integer --rate 96000 testraven.pcm  

IF- double only so only on laptop???: ./raven 100 100 10 100 but need to tweak these

 // params=pressure, k1, m1, d1

Gardner: ./raven 11000 6000 0 0  ... testraven.pcm only // decays????

Mindlin: ./raven 1000 12000 1000 0 > testplot // oscillates

spring versions: ./raven 0 0 0 0 > testplot - no pcm... coupling Ug question

find also: James L. Flanagan & L.L. Landgraf (1968): "Self-oscillating source for vocal-tract synthesizers."

--- Cataldo paper references:

1- Flanagan and Landgraf (1968) - forced mass/damper/spring... as below but how to oscillate? - Titze (1980) - parameters for above
2- IF (1972) 
3- Gardner et al (2001) - variations in pressure

*/

#ifdef LAP
#include <stdio.h>
#include "math.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#ifdef FLOATER
typedef float ourfloat;
#else
typedef double ourfloat;
#endif
#define PI            3.1415927
FILE *fo;
#else
#include "audio.h"
#include "stdio.h"
#include <math.h>
typedef float ourfloat;
extern __IO uint16_t adc_buffer[10];
#define FLOATER
#endif

ourfloat integrate(ourfloat old_value, ourfloat new_value, ourfloat period);

/// TWOTUBES: SC

 // or maybe replace with pluck.c code  from JOS: https://ccrma.stanford.edu/~jos/pmudw/pluck.c
 int d1length=7; // *trachea = 7 samples = 1st K = (0.3838-3.141) / (0.3838+3.141) = -2.7572 / 3.525 = -0.782*
// so wider we open beak = (0.3838 - WIDER) / (0.3838 + WIDER) = -LARGER / +LARGER = towards -1.0
 int d2length=2; // //*beak length=20mm = 2 samples   2nd K = (3.141 - 3.141) / (3.141 + 3.141) = 0 ????*

 // delay = round( L * fs / c); = say for 70mm = 0.07 * 32000 (here) / 347.23   // speed of sound (m/sec)

 ourfloat lossfactor=0.99f;

 ourfloat delay1right[80]; //->d1length of ourfloats
 ourfloat delay1left[80];//=>d1length of ourfloats

 ourfloat delay2right[80]; //->d1length of ourfloats
 ourfloat delay2left[80];//=>d1length of ourfloats

 void donoise(ourfloat *out, int numSamples){
   int x;
   for (x=0;x<numSamples;x++){
     ourfloat xx=(ourfloat) ( 2.0f * rand() / (RAND_MAX * 1.0f) - 1.0f);
     out[x]=xx;
   }
 }

void do_impulse(ourfloat* out, int numSamples, int freq){ //- so for 256 samples we have freq 125 for impulse
     // from Impulse->LFUGens.cpp
   int i;
   static ourfloat phase =0.0f;
   ourfloat z, freqinc;
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


 void RavenTube_init(void){
	 int i;

	 //initialise to zeroes!
	 for (i=0; i<d1length; ++i) {
		 delay1right[i]= 0.0f;
		 delay1left[i]= 0.0f;
		 delay2right[i]= 0.0f;
		 delay2left[i]= 0.0f;

	 }
 }

 ourfloat f1in= 0.0f;
 ourfloat f1out= 0.0f;
 ourfloat f2in= 0.0f;
 ourfloat f2out= 0.0f;

 int d1rightpos= 0;
 int d1leftpos= 0;
 int d2rightpos= 0;
 int d2leftpos= 0;

void SingleRavenTube_next(ourfloat *inn, ourfloat *outt, int inNumSamples) {
  int i;
  ourfloat * in = inn;//= IN(0);
  ourfloat loss= 1.0f;//lossfactor; // 0.99f
 ourfloat * d1right= delay1right;
 ourfloat * d1left= delay1left;

	 for (i=0; i<inNumSamples; ++i) {
	   // single tube 
		 ourfloat d1rightout= d1right[d1rightpos];
		 ourfloat d1leftout= d1left[d1leftpos];
		  outt[i]=d1rightout;
		  f1out= -(loss*(0.5f*f1in+0.5f*d1rightout)); // added minus which is for open tube! REFLECTION and LPF
		  f1in= d1rightout;

		 //calculate inputs of all delays
		  d1right[d1rightpos]= in[i]+loss*d1leftout;
		  d1left[d1leftpos]= f1out; // losses here?

	   d1rightpos= (d1rightpos+1)%d1length;
	   d1leftpos= (d1leftpos+1)%d1length;
	 }
}

void RavenTube_next(ourfloat *inn, ourfloat *outt, int inNumSamples) {

	 int i;
	 ourfloat * in = inn;//= IN(0);
#ifdef LAP
ourfloat k= -0.782f;// (ourfloat)ZIN0(1); //scattering coefficient updated at control rate?
 k=0.9f;
#else
ourfloat k = 1.0f-((ourfloat)(adc_buffer[SELZ])/2048.0f); // -1 to +1.0 - far left = open=higher res?
#endif
	 //	 ourfloat k = -0.8f;
 ourfloat loss= 1.0f;//lossfactor; // 0.99f
	 ourfloat * d1right= delay1right;
	 ourfloat * d1left= delay1left;
	 ourfloat * d2right= delay2right;
	 ourfloat * d2left= delay2left;


	 for (i=0; i<inNumSamples; ++i) {

		 //update outs of all delays
		 ourfloat d1rightout= d1right[d1rightpos];
		 ourfloat d1leftout= d1left[d1leftpos];
		 ourfloat d2rightout= d2right[d2rightpos];
		 ourfloat d2leftout= d2left[d2leftpos];
		  outt[i]=d2rightout;
		 //update all filters ????
		  f1out= loss*0.5*(f1in+d1leftout);
		  //		  f1out= loss*d1leftout; // why f1in here as last f1in and same question below? - this was a low pass
		 f1in= d1leftout;

		 f2out= -(loss*(0.5f*f2in+0.5f*d2rightout)); // added minus which is for open tube! REFLECTION
		 f2in= d2rightout;

		 //calculate inputs of all delays
		 d1right[d1rightpos]= in[i]+f1out;
		 d2right[d2rightpos]= d1rightout*(1.0f+k)+ ((-k)*d2leftout); // losses here? 
		 //- checked against http://www.music.mcgill.ca/~gary/courses/2015/618/week8/node19.html
		 d2left[d2leftpos]= f2out;
		 d1left[d1leftpos]= d1rightout*k+ ((1.0f-k)*d2leftout); // losses here?
		 //- checked against http://www.music.mcgill.ca/~gary/courses/2015/618/week8/node19.html

		 d1rightpos= (d1rightpos+1)%d1length;
		 d2rightpos= (d2rightpos+1)%d2length;
		 d1leftpos= (d1leftpos+1)%d1length;
		 d2leftpos= (d2leftpos+1)%d2length;
	 }
 }


 #define OURFLOAT_TO_SHORT(x) ((int)((x)*32768.0))

// from pluck.c

 typedef struct _DelayLine {
ourfloat *data; // max size of delayline
     unsigned char length;
     ourfloat *pointer;
     ourfloat *end;
 } DelayLine;

static DelayLine *initDelayLine(int len) {
//DelayLine dll;
//DelayLine *dl = &dll;
DelayLine *dl = (DelayLine *)calloc(len, sizeof(DelayLine));
	 dl->data = (ourfloat *)calloc(len, len * sizeof(ourfloat));
     dl->length = len;
     dl->pointer = dl->data;
     dl->end = dl->data + len - 1;
     return dl;
 }


 inline static void setDelayLine(DelayLine *dl, ourfloat *values, ourfloat scale) {
     int i;
     for (i=0; i<dl->length; i++)
	 dl->data[i] = scale * values[i];
 }

 /* lg_dl_update(dl, insamp);
  * Places "nut-reflected" sample from upper delay-line into
  * current lower delay-line pointer location (which represents
  * x = 0 position).  The pointer is then incremented (i.e. the
  * wave travels one sample to the left), turning the previous
  * position into an "effective" x = L position for the next
  * iteration.
  */
 static inline void lg_dl_update(DelayLine *dl, ourfloat insamp) {
     register ourfloat *ptr = dl->pointer;
     *ptr = insamp;
	 ptr++;
     if (ptr > dl->end)
	 ptr = dl->data;
     dl->pointer = ptr;
 }

 /* rg_dl_update(dl, insamp);
  * Decrements current upper delay-line pointer position (i.e.
  * the wave travels one sample to the right), moving it to the
  * "effective" x = 0 position for the next iteration.  The
  * "bridge-reflected" sample from lower delay-line is then placed
  * into this position.
  */
 static inline void rg_dl_update(DelayLine *dl, ourfloat insamp) {
     register ourfloat *ptr = dl->pointer;    
	 ptr--;
     if (ptr < dl->data)
	 ptr = dl->end;
	 *ptr = insamp;
     dl->pointer = ptr;
 }

 /* dl_access(dl, position);
  * Returns sample "position" samples into delay-line's past.
  * Position "0" points to the most recently inserted sample.
  */
 static inline ourfloat dl_access(DelayLine *dl, int position) {
     ourfloat *outloc = dl->pointer + position;
     while (outloc < dl->data)
	 outloc += dl->length;
     while (outloc > dl->end)
	 outloc -= dl->length;
     return *outloc;
 }

 /*
  *  Right-going delay line:
  *  -->---->---->--- 
  *  x=0
  *  (pointer)
  *  Left-going delay line:
  *  --<----<----<--- 
  *  x=0
  *  (pointer)
  */

 /* rg_dl_access(dl, position);
  * Returns spatial sample at location "position", where position zero
  * is equal to the current upper delay-line pointer position (x = 0).
  * In a right-going delay-line, position increases to the right, and
  * delay increases to the right => left = past and right = future.
  */
 static inline ourfloat rg_dl_access(DelayLine *dl, int position) {
     return dl_access(dl, position);
 }

 /* lg_dl_access(dl, position);
  * Returns spatial sample at location "position", where position zero
  * is equal to the current lower delay-line pointer position (x = 0).
  * In a left-going delay-line, position increases to the right, and
  * delay DEcreases to the right => left = future and right = past.
  */
 static inline ourfloat lg_dl_access(DelayLine *dl, int position) {
     return dl_access(dl, position);
 }

 static DelayLine *upper_rail,*lower_rail;

 static inline ourfloat bridgeReflection(ourfloat insamp) {
     static ourfloat state = 0; /* filter memory */
     /* Implement a one-pole lowpass with feedback coefficient = 0.5 */
     /* outsamp = 0.5 * outsamp + 0.5 * insamp */
//      ourfloat outsamp = (state >> 1) + (insamp >> 1);
ourfloat outsamp = 0.5f * state + 0.5f * insamp;
state = insamp; // or outsamp?
     return  outsamp;
 }


 void single_tube_init(int len){
     int i, rail_length = len;
     upper_rail = initDelayLine(rail_length);
     lower_rail = initDelayLine(rail_length);
 }

void single_tube_next(ourfloat *inn, ourfloat *outt, int inNumSamples, int length) {
     ourfloat yp0,ym0,ypM,ymM;
     int i;
     for (i=0;i<inNumSamples;i++){

     /* Output at pickup location */
       outt[i]  = rg_dl_access(upper_rail, length-1);
//outt[i] += lg_dl_access(lower_rail, length-1);
#ifdef LAP
//int iii=outt[i]*3276.0;
//     fwrite(&iii,2,1,fo);
#endif
     //    outsamp1 = lg_dl_access(lower_rail, pickup_loc);
     //	outsamp += outsamp1;

       ym0 = lg_dl_access(lower_rail, 1);     /* Sample traveling into "bridge" */ // bridge is base of trachea - lower rail
       ypM = rg_dl_access(upper_rail, length - 1); /* Sample to "nut" */ // was -2 why???? nut is mouth/air/beak - upper rail

       ymM = -bridgeReflection(ypM);                    /* Inverting upper rail */
       yp0 = ym0+inn[i];  /* lower rail + input */
       //       printf("xxxxxx %f\n", ymM);
//     yp0=-(ym0*0.9f)+inn[i];

     /* String state update */
          rg_dl_update(upper_rail, yp0);  // was yp0 /* Decrement pointer and then update */
          lg_dl_update(lower_rail, ymM); /* Update and then increment pointer */
     }


 }

 /*
  *  based on balloon1.cpp - IF model... only works on lap so...

 See http://www.dei.unipd.it/~avanzini/downloads/paper/avanzini_eurosp01_revised.pdf - measurements

  */

 //#ifdef LAP
 ourfloat computeSample(ourfloat pressure_in);
 void clearOld();

 ourfloat ps;		//subglottal pressure
static ourfloat r1;		//damping factor
static ourfloat r2;
 ourfloat m1;		//mass
 ourfloat m2;
 ourfloat k1;		//spring constant
 ourfloat k2;
 ourfloat k12;		//coupling spring constant
 ourfloat d1;		//glottal width
 ourfloat d2;
 ourfloat lg;		//glottal length
 ourfloat aida;		//nonlinearity coefficient
 ourfloat S;			//subglottal surface area
 ourfloat Ag01;		//nominal glottal area, with mass at rest position
 ourfloat Ag02;
 ourfloat pm1Prev;	//pressure at previous time step
 ourfloat pm2Prev;
 ourfloat x1Prev;	//displacement at previous time step
 ourfloat x1PrevPrev;//displacement at previous time step to the previous one
 ourfloat x2Prev;
 ourfloat x2PrevPrev;
 ourfloat gain;		//after-market gain
 ourfloat uPrev;		//previous flow value
 ourfloat Fs;		//calculation sampling rate, not actual audio output sample rate

#ifdef FLOATER

 void init(){

   // adapt these settings for potential raven voice

   // from MATLAB code: also there is pressure envelope there

   /*

 p=0;        %relative output pressure, 
 rho = 1.14; %kg/m^3 mass density
 v = 1.85e-5; %N*s/m^2 greek new: air shear viscosity
 lg = 1.63e-2; %m glottal length

 twod = 3e-5; %m, glottal width 2d1
 d1=twod/2; %1.5000e-005
 d2=d1;
 m = 4.4e-5/90; %kg, glottal mass // why /90 - otherwise accords for human glottis
 m1=m; %4.8889e-007
 m2=m;
 k12=0.04; %coupling spring constant
 k = 0.09; %N/m, spring constant
 k1=k;
 k2=k1;
 aida=1000000.01; %non-linearity factor
 r = 0.0001*sqrt(m*k); %damper constant, N*s/m
 r1=r*1;
 r2=r1; %2.0976e-008
 %Ag0 = 5e-6; %m^2 glottal area at rest = 5mm^2=5e-6
 Ag0 = 5e-9; %m^2 glottal area at rest
 S = 5e-5; %m^2 output area (vocal tract end)
 %S = 5e-4; %m^2 output area (vocal tract end)

   */

   /* raven details (see kahrs.pdf and zacarelli)

 kahrs: 

 from zacarelli we have:

 stiffness (g ms−2)	k1, k2	22.0×10−3
 damping constant (g ms−1)	r1, r2	1.2×10−3
 coupling constant (g ms−2)	kc	6.0×10−3

 but not sure how to convert between????

 m1/m2=glottal mass - 3.848451000647498e-6 - 0.00000384 /90

 k1/k2-spring constant N/m - 3.11 ???
 k12=coupling spring constant ???

 d1/d2=glottal width 2dl /2??? diameter is 7mm  say 2mm now or is this *thickness?* 1e-4 - from fletcher is 100 micrometer
 r1/r2 = 0.0001*sqrt(m*k); %damper constant, N*s/m - 1.386e-7 // but depends on K spring constant can vary 0.0001

 Ag0 = 5e-9; %m^2 glottal area at rest - 2mm say at rest= 3.14mm 3.14e-6
 S = 5e-5; %m^2 output area (vocal tract end) - 20mm diameter BEAK 314mm = 0.000314

 lg= 1.63e-2; %m glottal length - say 7mm=7e-3

    */
d1 =0.0008f;
d2 =0.0008f;

r1=0.00000980665f; // damper constant depends on k and mass r = 0.0001*sqrt(m*k); %damper constant, N*s/m Avanzini has 0.1 * sqrt(m*k)  m0.000044 * 20 = 0.002966
// above 0.001 is no sound
	r2 =r1;
m1 =1e-6; // -7 or -5 for -5 we would have for r1 and as k1=0.09 = 
	m2 =m1;
	k1 =0.014f;
	k2 =k1;
	k12=0.00005f;
	//		aida =10000000.0f;
	aida =1.0f;
	d1 =1.5e-2f;
	d2 =d1;

	lg =0.007f;
	gain=400.0f;
	S=0.0005f;
	Ag01=3e-7f;
	Ag02=3e-7f; 

	x1Prev=0.0f;
	x1PrevPrev=0.0f;
	x2Prev=0.0f;
	x2PrevPrev=0.0f;
	pm1Prev=0.0f;
	pm2Prev=0.0f;
	uPrev=0.0f;
	ps=0.0f;
	Fs=32000.0f; // try at 96000 NON!
  
}


int rtick(ourfloat *buffer, int bufferSize, ourfloat pressureIn) {
  ourfloat *samples = (ourfloat *) buffer;

/*	bsynth->setM1(5e-8*(ourfloat)[(id)dataPointer m1In]);
	bsynth->setM2(5e-8*(ourfloat)[(id)dataPointer m2In]);
	bsynth->setR1(5e-9*(ourfloat)[(id)dataPointer r1In]);
	bsynth->setR2(5e-9*(ourfloat)[(id)dataPointer r2In]);
	bsynth->setK1(1e-3*(ourfloat)[(id)dataPointer k1In]);
	bsynth->setK2(1e-3*(ourfloat)[(id)dataPointer k2In]);
	bsynth->setD1(1.5e-7*(ourfloat)[(id)dataPointer d1In]);
	bsynth->setD2(1.5e-7*(ourfloat)[(id)dataPointer d2In]);
	bsynth->setK12(1e-4*(ourfloat)[(id)dataPointer k12In]);
	bsynth->setLg(1.3e-4*(ourfloat)[(id)dataPointer lgIn]);
	bsynth->setAida(1.1e-4*(ourfloat)[(id)dataPointer aidaIn]);
	bsynth->setS(5.5e-7*(ourfloat)[(id)dataPointer SIn]);
	bsynth->setAg01(5.1e-10*(ourfloat)[(id)dataPointer Ag01In]);
	bsynth->setAg02(5.1e-10*(ourfloat)[(id)dataPointer Ag02In]);
	bsynth->setGain((ourfloat)[(id)dataPointer gainIn]);
	bsynth->setFs((ourfloat)[(id)dataPointer FsIn]);
*/

/* pressure waveform:

%Set pressure waveform envelope
for n=1:N
if n<5
%ps(n)=MAXps;
ps(n)=0;
elseif n<7
ps(n)=0;
%ps(n)=MAXps;
elseif n<T1 // T1 is N/90 - N is number of iterations/samples
ps(n)=ps(n-1)+MAXps/T1; // MAXps is 400
elseif n<=T2 // T2 is N/80 
    ps(n)=ps(n-1);
else
    ps(n)=ps(n-1)+(MINps-MAXps)/(N-T2); // MINps is 30
*/

	int i; ourfloat lastp;
	for (i=0; i<bufferSize; i++ ) {
	  //	  *samples++ = computeSample(pressureIn);
	  //	  printf("%f  ",computeSample(pressureIn));

	  /*	  	 	  if (i<5) pressureIn=0;
	  else if (i< (32000/90)) pressureIn=lastp+ 400/(32000/90);
	  else if (i<= (32000/80)) pressureIn=lastp;
	  else pressureIn=lastp-360.0f/(32000.0f-(32000.0/80.0));
	  */
	  // constant pressure
//	  pressureIn=300; // 0.3 kPa after Fletcher

			  signed int s16=(signed int)(computeSample(pressureIn)*32768.0);
	  *samples++=(ourfloat)(computeSample(pressureIn));
#ifdef LAP
	  //	  printf("%d\n",s16);
	  //	  fwrite(&s16,2,1,fo);
#endif
   lastp=pressureIn;
   //   fwrite(&s16,2,1,fo);

   //*samples++ = [(id)dataPointer amp] * bsynth->tick();
	}

	return 0;
};

ourfloat computeSample(ourfloat pressure_in){
  ourfloat T=1.0f/Fs;
  ourfloat rho = 1.14f; 
  ourfloat rhosn = rho*0.69f;
  ourfloat hfrho=rho/2.0f;
  ourfloat v = 1.85e-5f;
  ourfloat twvd1lg=12.0f*v*d1*lg*lg;
  ourfloat twvd2lg=12.0f*v*d2*lg*lg;
  ourfloat Ag012lg=Ag01/2.0f/lg;
  ourfloat Ag022lg=Ag02/2.0f/lg;
  ourfloat lgd1=lg*d1;
  ourfloat lgd2=lg*d2;
  ourfloat m1T=m1/T/T;
  ourfloat m2T=m2/T/T;
  ourfloat r1T=r1/T;
  ourfloat r2T=r2/T;
  ourfloat C11=k1*(1.0f+aida*x1Prev*x1Prev);
  ourfloat C12=k2*(1.0f+aida*x2Prev*x2Prev);
  ourfloat C21=k1*(1.0f+aida*(x1Prev+Ag012lg)*(x1Prev+Ag012lg));
  ourfloat C22=k2*(1.0f+aida*(x2Prev+Ag022lg)*(x2Prev+Ag022lg));
  ourfloat alpha1=lgd1*pm1Prev;
  ourfloat alpha2=lgd2*pm2Prev;
  ourfloat beta1=m1T*(x1PrevPrev-2.0f*x1Prev);
  ourfloat beta2=m2T*(x2PrevPrev-2.0f*x2Prev);
  ourfloat gamma1=-r1T*x1Prev;
  ourfloat gamma2=-r2T*x2Prev;
  ourfloat delta1=Ag012lg*C21;
  ourfloat delta2=Ag022lg*C22;
  ourfloat lambda1=-k12*x2Prev;
  ourfloat lambda2=-k12*x1Prev;
  ourfloat x1=0.0f;
  ourfloat x2=0.0f;
  ourfloat pm1=0.0f;
  ourfloat pm2=0.0f;
  ourfloat A1=0.0f;
  ourfloat A2=0.0f;
  ourfloat A1n2=0.0f;
  ourfloat A1n3=0.0f;
  ourfloat A2n2=0.0f;
  ourfloat A2n3=0.0f;
  ourfloat a=0.0f;
  ourfloat b=0.0f;
  ourfloat c=0.0f;
  ourfloat det=0.0f;
  ourfloat flow1=0.0f;
  ourfloat flow2=0.0f;
  ourfloat udif1=0.0f;
  ourfloat udif2=0.0f;
  ourfloat u=0.0f;
  ourfloat g1=0.0f;
  ourfloat g2=0.0f;
  ourfloat g4=0.0f;
  ourfloat g5=0.0f;
  ourfloat pm1b=0.0f;
  ourfloat pm2b=0.0f;


  if  (x1Prev>=-Ag012lg){
    x1=(alpha1-beta1-gamma1-lambda1)/(m1T+r1T+C11+k12);
  }
  else {
    x1=(alpha1-beta1-gamma1-lambda1-delta1)/(m1T+r1T+C21+k12);
  }

  if  (x2Prev>=-Ag022lg){
    x2=(alpha2-beta2-gamma2-lambda2)/(m2T+r2T+C12+k12);
  }
  else{
    x2=(alpha2-beta2-gamma2-lambda2-delta2)/(m2T+r2T+C22+k12);
  }

  A1=Ag01+lg*x1;
  A2=Ag02+lg*x2;
    
  if (A1<=0.0f)
    {A1=0.1e-8f;}
    

  if (A2<=0.0f)
    {A2=0.1e-8f;}
   
	
	
  A1n2=A1*A1; 
  A1n3=A1n2*A1;

  A2n2=A2*A2; 
  A2n3=A2n2*A2;
	
  a= (rhosn/A1n2)+hfrho*(1.0f/A2n2-1/A1n2)+hfrho/A2n2*(2.0f*A2/S*(1.0f-A2/S));
  b= twvd1lg/A1n3+twvd2lg/A2n3;
  c= -pressure_in;

  det=b*b-4.0f*a*c;

  if (det>=0.0f){
    flow1=(-b+sqrtf(det))/(2.0f*a);
    flow2=(-b-sqrtf(det))/(2.0f*a);
  }
  else{
    flow1=(-b)/(2.0f*a);
    flow2=(-b)/(2.0f*a);
  }

  udif1=fabsf(flow1-uPrev);
  udif2=fabsf(flow2-uPrev);

  if (udif1<udif2){
    u=flow1;
  }
  else{
    u=flow2;
  }

  //u=max(flow1,flow2);



  g1=rhosn*u*u/A1n2;
  g2=twvd1lg*u/A1n3;
  g4=twvd2lg*u/A2n3;
  g5=hfrho*u*u/A2n2*(2.0f*A2/S*(1.0f-A2/S));

  pm1=pressure_in-g1-g2/2.0f;
  pm2=g5+g4/2.0f;

  if (x1>=-Ag012lg){
    
    pm1b=(m1T*(x1-2.0f*x1Prev+x1PrevPrev)+r1T*(x1-x1Prev)+ k1*x1*(1.0f+aida*x1*x1) +k12*(x1-x2))/(lgd1);
    pm1=pm1/2.0f+pm1b/2.0f;
        
	
    if (x2>=-Ag022lg){

      pm2b=(m2T*(x2-2.0f*x2Prev+x2PrevPrev)+r2T*(x2-x2Prev)+ k2*x2*(1.0f+aida*x2*x2) -k12*(x1-x2))/(lgd2);
      pm2=pm2/2.0f+pm2b/2.0f;
    }
    else{
                 pm2=pressure_in;
               pm1=pressure_in;
    }
  }     
  else{
       pm1=pressure_in;
       pm2=0.0f;
  }

    if (pm1>pressure_in)
    	{pm1=pressure_in;}
    
    if (pm2>pressure_in)
           {pm2=pressure_in;}
    
      if (pm1<0.0f)
	//	   pm1(n)=abs(pm1(n));
	{pm1=fabsf(pm1);}
 

  if (pm2<0.0f)
    {pm2=0.0f;}
    
  //  if (u<0.0)
  //    {u=0.0;}
		
    pm1Prev=pm1;
  pm2Prev=pm2;
  x1PrevPrev=x1Prev;
  x1Prev=x1;
  x2PrevPrev=x2Prev;
  x2Prev=x2;
  uPrev=u;
  ps=pressure_in;
//printf("UUUU %f pressure-in %f\n", u, pressure_in);
  return gain*u;
}

void clearOld()
{
x1Prev=0.0f;
x1PrevPrev=0.0f;
x2Prev=0.0f;
x2PrevPrev=0.0f;
pm1Prev=0.0f;
pm2Prev=0.0f;
uPrev=0.0f;
ps=0.0f;
}
#else
 void init(){

   // adapt these settings for potential raven voice

   // from MATLAB code: also there is pressure envelope there

   /*

 p=0;        %relative output pressure, 
 rho = 1.14; %kg/m^3 mass density
 v = 1.85e-5; %N*s/m^2 greek new: air shear viscosity
 lg = 1.63e-2; %m glottal length

 twod = 3e-5; %m, glottal width 2d1
 d1=twod/2; %1.5000e-005
 d2=d1;
 m = 4.4e-5/90; %kg, glottal mass // why /90 - otherwise accords for human glottis
 m1=m; %4.8889e-007
 m2=m;
 k12=0.04; %coupling spring constant
 k = 0.09; %N/m, spring constant
 k1=k;
 k2=k1;
 aida=1000000.01; %non-linearity factor
 r = 0.0001*sqrt(m*k); %damper constant, N*s/m
 r1=r*1;
 r2=r1; %2.0976e-008
 %Ag0 = 5e-6; %m^2 glottal area at rest = 5mm^2=5e-6
 Ag0 = 5e-9; %m^2 glottal area at rest
 S = 5e-5; %m^2 output area (vocal tract end)
 %S = 5e-4; %m^2 output area (vocal tract end)

   */

   /* raven details (see kahrs.pdf and zacarelli)

 kahrs: 

 from zacarelli we have:

 stiffness (g ms−2)	k1, k2	22.0×10−3
 damping constant (g ms−1)	r1, r2	1.2×10−3
 coupling constant (g ms−2)	kc	6.0×10−3

 but not sure how to convert between????

 m1/m2=glottal mass - 3.848451000647498e-6 - 0.00000384 /90

 k1/k2-spring constant N/m - 3.11 ???
 k12=coupling spring constant ???

 d1/d2=glottal width 2dl /2??? diameter is 7mm  say 2mm now or is this *thickness?* 1e-4 - from fletcher is 100 micrometer
 r1/r2 = 0.0001*sqrt(m*k); %damper constant, N*s/m - 1.386e-7 // but depends on K spring constant can vary 0.0001

 Ag0 = 5e-9; %m^2 glottal area at rest - 2mm say at rest= 3.14mm 3.14e-6
 S = 5e-5; %m^2 output area (vocal tract end) - 20mm diameter BEAK 314mm = 0.000314

 lg= 1.63e-2; %m glottal length - say 7mm=7e-3

    */
d1 =0.0008;
d2 =0.0008;

r1=0.00000980665; // damper constant depends on k and mass r = 0.0001*sqrt(m*k); %damper constant, N*s/m Avanzini has 0.1 * sqrt(m*k)  m0.000044 * 20 = 0.002966
// above 0.001 is no sound
	r2 =r1;
m1 =1e-6; // -7 or -5 for -5 we would have for r1 and as k1=0.09 = 
	m2 =m1;
	k1 =0.014;
	k2 =k1;
	k12=0.00005;
	//	aida =10000000.0;
		aida =1.0;
	d1 =1.5e-2;
	d2 =d1;

	lg =0.007;
	gain=400.0;
	S=0.0005;
	Ag01=3e-7;
	Ag02=3e-7; 

	x1Prev=0.0;
	x1PrevPrev=0.0;
	x2Prev=0.0;
	x2PrevPrev=0.0;
	pm1Prev=0.0;
	pm2Prev=0.0;
	uPrev=0.0;
	ps=0.0;
	Fs=32000.0; // try at 96000  
}


int rtick(ourfloat *buffer, int bufferSize, ourfloat pressureIn) {
  ourfloat *samples = (ourfloat *) buffer;

/*	bsynth->setM1(5e-8*(ourfloat)[(id)dataPointer m1In]);
	bsynth->setM2(5e-8*(ourfloat)[(id)dataPointer m2In]);
	bsynth->setR1(5e-9*(ourfloat)[(id)dataPointer r1In]);
	bsynth->setR2(5e-9*(ourfloat)[(id)dataPointer r2In]);
	bsynth->setK1(1e-3*(ourfloat)[(id)dataPointer k1In]);
	bsynth->setK2(1e-3*(ourfloat)[(id)dataPointer k2In]);
	bsynth->setD1(1.5e-7*(ourfloat)[(id)dataPointer d1In]);
	bsynth->setD2(1.5e-7*(ourfloat)[(id)dataPointer d2In]);
	bsynth->setK12(1e-4*(ourfloat)[(id)dataPointer k12In]);
	bsynth->setLg(1.3e-4*(ourfloat)[(id)dataPointer lgIn]);
	bsynth->setAida(1.1e-4*(ourfloat)[(id)dataPointer aidaIn]);
	bsynth->setS(5.5e-7*(ourfloat)[(id)dataPointer SIn]);
	bsynth->setAg01(5.1e-10*(ourfloat)[(id)dataPointer Ag01In]);
	bsynth->setAg02(5.1e-10*(ourfloat)[(id)dataPointer Ag02In]);
	bsynth->setGain((ourfloat)[(id)dataPointer gainIn]);
	bsynth->setFs((ourfloat)[(id)dataPointer FsIn]);
*/

/* pressure waveform:

%Set pressure waveform envelope
for n=1:N
if n<5
%ps(n)=MAXps;
ps(n)=0;
elseif n<7
ps(n)=0;
%ps(n)=MAXps;
elseif n<T1 // T1 is N/90 - N is number of iterations/samples
ps(n)=ps(n-1)+MAXps/T1; // MAXps is 400
elseif n<=T2 // T2 is N/80 
    ps(n)=ps(n-1);
else
    ps(n)=ps(n-1)+(MINps-MAXps)/(N-T2); // MINps is 30
*/

	int i; ourfloat lastp;
	for (i=0; i<bufferSize; i++ ) {
	  //	  *samples++ = computeSample(pressureIn);
	  //	  printf("%f  ",computeSample(pressureIn));

	  /*	  	 	  if (i<5) pressureIn=0;
	  else if (i< (32000/90)) pressureIn=lastp+ 400/(32000/90);
	  else if (i<= (32000/80)) pressureIn=lastp;
	  else pressureIn=lastp-360.0f/(32000.0f-(32000.0/80.0));
	  */
	  // constant pressure
//	  pressureIn=300; // 0.3 kPa after Fletcher

			  signed int s16=(signed int)(computeSample(pressureIn)*32768.0);
	  *samples++=(ourfloat)(computeSample(pressureIn));
	  //	  printf("%d\n",s16);
	  //     fwrite(&s16,2,1,fo);
   lastp=pressureIn;
   //   fwrite(&s16,2,1,fo);

   //*samples++ = [(id)dataPointer amp] * bsynth->tick();
	}

	return 0;
};


ourfloat computeSample(ourfloat pressure_in){
  ourfloat T=1.0/Fs;
  ourfloat rho = 1.14; 
  ourfloat rhosn = rho*0.69;
  ourfloat hfrho=rho/2.0;
  ourfloat v = 1.85e-5;
  ourfloat twvd1lg=12.0*v*d1*lg*lg;
  ourfloat twvd2lg=12.0*v*d2*lg*lg;
  ourfloat Ag012lg=Ag01/2.0/lg;
  ourfloat Ag022lg=Ag02/2.0/lg;
  ourfloat lgd1=lg*d1;
  ourfloat lgd2=lg*d2;
  ourfloat m1T=m1/T/T;
  ourfloat m2T=m2/T/T;
  ourfloat r1T=r1/T;
  ourfloat r2T=r2/T;
  ourfloat C11=k1*(1+aida*x1Prev*x1Prev);
  ourfloat C12=k2*(1+aida*x2Prev*x2Prev);
  ourfloat C21=k1*(1+aida*(x1Prev+Ag012lg)*(x1Prev+Ag012lg));
  ourfloat C22=k2*(1+aida*(x2Prev+Ag022lg)*(x2Prev+Ag022lg));
  ourfloat alpha1=lgd1*pm1Prev;
  ourfloat alpha2=lgd2*pm2Prev;
  ourfloat beta1=m1T*(x1PrevPrev-2*x1Prev);
  ourfloat beta2=m2T*(x2PrevPrev-2*x2Prev);
  ourfloat gamma1=-r1T*x1Prev;
  ourfloat gamma2=-r2T*x2Prev;
  ourfloat delta1=Ag012lg*C21;
  ourfloat delta2=Ag022lg*C22;
  ourfloat lambda1=-k12*x2Prev;
  ourfloat lambda2=-k12*x1Prev;
  ourfloat x1=0.0;
  ourfloat x2=0.0;
  ourfloat pm1=0.0;
  ourfloat pm2=0.0;
  ourfloat A1=0.0;
  ourfloat A2=0.0;
  ourfloat A1n2=0.0;
  ourfloat A1n3=0.0;
  ourfloat A2n2=0.0;
  ourfloat A2n3=0.0;
  ourfloat a=0.0;
  ourfloat b=0.0;
  ourfloat c=0.0;
  ourfloat det=0.0;
  ourfloat flow1=0.0;
  ourfloat flow2=0.0;
  ourfloat udif1=0.0;
  ourfloat udif2=0.0;
  ourfloat u=0.0;
  ourfloat g1=0.0;
  ourfloat g2=0.0;
  ourfloat g4=0.0;
  ourfloat g5=0.0;
  ourfloat pm1b=0.0;
  ourfloat pm2b=0.0;


  if  (x1Prev>=-Ag012lg){
    x1=(alpha1-beta1-gamma1-lambda1)/(m1T+r1T+C11+k12);
  }
  else {
    x1=(alpha1-beta1-gamma1-lambda1-delta1)/(m1T+r1T+C21+k12);
  }

  if  (x2Prev>=-Ag022lg){
    x2=(alpha2-beta2-gamma2-lambda2)/(m2T+r2T+C12+k12);
  }
  else{
    x2=(alpha2-beta2-gamma2-lambda2-delta2)/(m2T+r2T+C22+k12);
  }

  A1=Ag01+lg*x1;
  A2=Ag02+lg*x2;
    
  if (A1<=0.0f)
    {A1=0.1e-8;}
    

  if (A2<=0.0f)
    {A2=0.1e-8;}
	
  A1n2=A1*A1; 
  A1n3=A1n2*A1;

  A2n2=A2*A2; 
  A2n3=A2n2*A2;
	
  a= (rhosn/A1n2)+hfrho*(1.0/A2n2-1/A1n2)+hfrho/A2n2*(2.0*A2/S*(1.0-A2/S));
  b= twvd1lg/A1n3+twvd2lg/A2n3;
  c= -pressure_in;

  det=b*b-4.0*a*c;

  if (det>=0.0f){
    flow1=(-b+sqrt(det))/(2.0*a);
    flow2=(-b-sqrt(det))/(2.0*a);
  }
  else{
    flow1=(-b)/(2.0*a);
    flow2=(-b)/(2.0*a);
  }

  udif1=fabs(flow1-uPrev);
  udif2=fabs(flow2-uPrev);

  if (udif1<udif2){
    u=flow1;
  }
  else{
    u=flow2;
  }

  //u=max(flow1,flow2);



  g1=rhosn*u*u/A1n2;
  g2=twvd1lg*u/A1n3;
  g4=twvd2lg*u/A2n3;
  g5=hfrho*u*u/A2n2*(2.0*A2/S*(1.0-A2/S));

  pm1=pressure_in-g1-g2/2.0;
  pm2=g5+g4/2.0;

  if (x1>=-Ag012lg){
    
    pm1b=(m1T*(x1-2.0*x1Prev+x1PrevPrev)+r1T*(x1-x1Prev)+ k1*x1*(1.0+aida*x1*x1) +k12*(x1-x2))/(lgd1);
    pm1=pm1/2.0+pm1b/2.0;
        
	
    if (x2>=-Ag022lg){

      pm2b=(m2T*(x2-2.0*x2Prev+x2PrevPrev)+r2T*(x2-x2Prev)+ k2*x2*(1.0+aida*x2*x2) -k12*(x1-x2))/(lgd2);
      pm2=pm2/2.0+pm2b/2.0;
    }
    else{
                 pm2=pressure_in;
               pm1=pressure_in;
    }
  }     
  else{
       pm1=pressure_in;
       pm2=0.0;
  }

    if (pm1>pressure_in)
    	{pm1=pressure_in;}
    
    if (pm2>pressure_in)
           {pm2=pressure_in;}
    
      if (pm1<0.0)
	//	   pm1(n)=abs(pm1(n));
	{pm1=fabs(pm1);}
 

  if (pm2<0.0)
    {pm2=0.0;}
    
  //  if (u<0.0)
  //    {u=0.0;}
		
    pm1Prev=pm1;
  pm2Prev=pm2;
  x1PrevPrev=x1Prev;
  x1Prev=x1;
  x2PrevPrev=x2Prev;
  x2Prev=x2;
  uPrev=u;
  ps=pressure_in;
//printf("UUUU %f pressure-in %f\n", u, pressure_in);
  return gain*u;
}

void clearOld()
{
x1Prev=0.0;
x1PrevPrev=0.0;
x2Prev=0.0;
x2PrevPrev=0.0;
pm1Prev=0.0;
pm2Prev=0.0;
uPrev=0.0;
ps=0.0;
}
#endif
//#endif


//////////////////////// MINDLIN - from Laje, Gardner and Mindlin:

typedef struct mindlin {
ourfloat x, xprime,xdobleprime, k, b, c, f0, T, p0;
}Mindlin;

Mindlin syrinxM;

void init_mindlin(Mindlin* mind, ourfloat b, ourfloat k, ourfloat c){
mind->b=b;
mind->k=k;
mind->c=c;
mind->x=0.01f;
mind->p0=100000.0f;
mind->xdobleprime=0.0f;
mind->f0=2000.0f;
mind->T = 1.0f/32000.0f;

}

static ourfloat alpha=0.0f;
static ourfloat beta=0.0f;
static ourfloat gammaa=1500.0f;

ourfloat calc_xdobleprime_mindlin(Mindlin *mind){
// from pseudocode: result/0?/=xdobleprime+k*xprime + (c*x)^2 * xprime * xprime - b * xprime + f0;
// xdobleprime = - (k*x + c *x^2 * xprime * xprime - b * xprime + f0);

//return - (mind->k*mind->x + mind->c* pow(mind->x,2) * mind->xprime * mind->xprime - mind->b * mind->xprime + mind->f0);

// from paper: xdobleprime= -k*x-c*x^2*xprime+b*xprime-f0;

//return -mind->k*mind->x - mind->c*powf(mind->x,2) * mind->xprime + mind->b *mind->xprime-mind->f0; // ./raven 1000 1000 10 0 > testplot matches p52

// more from mindlin 2013 - this seems to work with correct parameters and includes air pressure business?

// xdobleprime= - alpha*gamma^2 - beta*gamma^2*x - gamma^2*x^3 - gamma*x^2*xprime + gamma^2*x^2 -gamma*x*xprime

 return  - alpha*powf(gammaa,2) - beta*powf(gammaa,2)*mind->x - powf(gammaa,2)*powf(mind->x,3) - gammaa*powf(mind->x,2)*mind->xprime + powf(gammaa,2)*powf(mind->x,2) - gammaa*mind->x*mind->xprime;

// 

// from book: -k*x + b*xprime - c*x^2*xprime


//  return -(mind->k*mind->x) + (mind->b *mind->xprime) - (mind->c*powf(mind->x,2) * mind->xprime); // minus  mind->f0;

// from: http://www.scholarpedia.org/article/Models_of_birdsong_%28physics%29

//return -mind->k * mind->x - (mind->b + pow(mind->c + mind->x,2) - mind->p0)* mind->xprime - mind->f0;

}

ourfloat mindlin_oscillate(Mindlin* mind){
int iii;  
ourfloat newxdobleprime = calc_xdobleprime_mindlin(mind);
ourfloat newxprime = mind->xprime + integrate(mind->xdobleprime,newxdobleprime,mind->T);
mind->xdobleprime = newxdobleprime;
mind->x += integrate(mind->xprime,newxprime,mind->T);
#ifdef LAP
//printf("%f\n", mind->x);
// iii=1600.0f*mind->x;
// fwrite(&iii,2,1,fo);
#endif
mind->xprime = newxprime;
return mind->x;
}

////////////////////// GARDNER 2001

typedef struct gardner {
  ourfloat x, xprime,oldxprime,xdobleprime, K, Pb, a0, b0, t, M, K_scale, K_scalex, Pb_scalex, D, D2, Pb_scale, T, freq, ofreq;
}Gardner;

Gardner syrinx;

void init_gardner(Gardner* gd, int16_t kk, int16_t pb){
gd->x=0.0f;
gd->xprime=1.0f;
gd->xdobleprime = 0.0f;
gd->K =  (ourfloat)kk; // between 11k and 12k something happens... - and now???
gd->Pb = (ourfloat)pb;

// kk is say 5 N/cm3 - 500000 // pb is say 5 kPa = 50000 g/s 

/*
	upper_labia = 0.02 #cm
	lower_labia = 0.04 #cm
	t_constant = .00015 #s # phenomenological constant Titze!
	mass = .005 #g/cm3
	K=restitution_constant = 200000 #g*cm/s2cm3 // this is K- IF= approx 200 kdyn/cm3 ??? // paper 0-8 N/cm3

8 Newtons is 800 kdyn 1 newton = 100 000 dyne

	D_coefficient = 5 #dynes*s/cm3
	D2_coefficient = .01 #dyne*s/cm5 .001
	bronchial_pressure = 10000 #g/(cm*s2) # 0-3 kPa

1 pascal is 1 N/m2 = 1 kg /ms2 =  say 2 kPa = 2 N/m2 2000 kPa=20000 g/s
1 pascal is 0.01 g/cm^2 so 2000 kpa=20 g/cm2
*/

//#constants
 gd->a0 = 0.02f;
 gd->b0 = 0.04f;
 gd->t = 0.00015f;
 gd->M = 0.005f;
 gd->K_scale = 1.0f;
 gd->K_scalex = 1000.0f;
 gd->D = 5.0f;
 gd->D2 = 0.001f;
 gd->Pb_scale = 1.0f;
 gd->Pb_scalex = 1000.0f;
 gd->T = 1.0f/96000.0f;
 gd->freq = 10.0f;
 gd->ofreq= 20.0f;
}

ourfloat dvx (ourfloat t, ourfloat x, ourfloat v)
{
Gardner* gd=&syrinx;
  ourfloat value;
  value=((gd->Pb*(((gd->a0 - gd->b0) +(2.0f*gd->t*v))/(x+gd->b0+(gd->t*v)))) -(gd->K*x) - (gd->D2*powf(v,3)) - (gd->D*v))/gd->M;
  return value;
}


ourfloat calc_xdobleprime_gardner(Gardner *gd, int i){
  //  ourfloat a = gd->a0 + gd->x + (gd->t*gd->xprime);
  //  ourfloat b = gd->b0 + gd->x - (gd->t*gd->xprime);
  //  ourfloat Pf = gd->Pb*(1 - (a/b));
//  printf("%f\n",gd->x);
  int iii=(ourfloat)gd->x*32768.0f;
#ifdef LAP
  //  printf("%f\n",gd->x);
  //  fwrite(&iii,2,1,fo);
#endif
  //		return (Pf - (self.K*self.x) - (self.D2*math.pow(self.xprime,3)) - (self.D*self.xprime))/self.M
  //    return (Pf - (gd->K*gd->x) - (gd->D*gd->xprime))/gd->M;
  //  return (Pf - (gd->K*gd->x) - (gd->D2*pow(gd->oldxprime,3)) - (gd->D*gd->xprime))/gd->M;
  return ((gd->Pb*(((gd->a0 - gd->b0) +(2.0f*gd->t*gd->xprime))/(gd->x+gd->b0+(gd->t*gd->xprime))))-(gd->K*gd->x) - (gd->D2*powf(gd->xprime,3)) - (gd->D*gd->xprime))/gd->M;
}


void gardner_oscillate(Gardner* gd, int i){
  ourfloat newxdobleprime = calc_xdobleprime_gardner(gd,i);
  ourfloat newxprime = gd->xprime + integrate(gd->xdobleprime,newxdobleprime,gd->T);
    gd->xdobleprime = newxdobleprime;
    gd->x += integrate(gd->xprime,newxprime,gd->T);
    //    fwrite(&gd->x,2,1,fo);

gd->oldxprime=gd->xprime;
  gd->xprime = newxprime;
  //  gd->K+= gd->K_scale;
  //  gd->Pb+= gd->Pb_scale;
  //      gd->K = gd->K_scale*sin((2*PI*gd->T*gd->freq*i)) + gd->K_scalex;
		  //		gd->K = gd->K_scale*2.0
      //    gd->Pb = gd->Pb_scale*cos((2*PI*gd->T*gd->ofreq*i) + PI) + gd->Pb_scalex;
		  //               gd->Pb = gd->Pb_scale
  //   gd->freq += 0.01;
  //   gd->ofreq+=0.01;
}

//// simple spring undriven

static ourfloat x = 0.0f;
ourfloat xprime = 1.0f;
ourfloat xdobleprime = 0.0f;
ourfloat M = 0.005f;
ourfloat R = 5.0f;
ourfloat K = 200000.0f;
ourfloat T = 1.0/96000.0f;


ourfloat integrate(ourfloat old_value, ourfloat new_value, ourfloat period){
  return (old_value + new_value)*(period/2.0f);
}

ourfloat flintegrate(ourfloat old_value, ourfloat new_value, ourfloat period){
  return (old_value + new_value)*(period/2.0f);
}

ourfloat old_integrate(ourfloat old_value, ourfloat new_value, ourfloat period){
  return new_value*period;
}

ourfloat old_flintegrate(ourfloat old_value, ourfloat new_value, ourfloat period){
  return old_value + new_value*period;
}

ourfloat Fx=0.0f;
ourfloat xo=0.000001f;
ourfloat ago=0.000005f;
ourfloat l=0.0018f;

// for runge-kutta spring

ourfloat mass, stiffness, damping, pulse;
ourfloat time;
ourfloat position;
ourfloat velocity;

static ourfloat lastx;

ourfloat calc_xdobleprime(int i){
  ourfloat value;
  //  if (x>xo)  Fx=stiffness/12 * powf(x,2);
  //  else Fx=500.0f*0.002;
  //  printf("\n x: %f xo: %f\n", x, xo);

  //  if (x>xo)  Fx=stiffness/5.0f * pow(x,2);
  //  else if (t>0.05 && x<xo) {
  //      Fx=l/2.0*8.0*98.0*l*0.002;
  //  }
  //    else Fx=stiffness/5.0f * pow(x,2);

  // Forcing function from Cataldo - Flanagan Landgraf

  ourfloat ps=783.0f;
  ourfloat ago=0.00005f; // glottal area
  //  ourfloat l=0.0018; //length  we have above
  ourfloat airdensity= 1.225f; // kg/m3
  //      ourfloat ug= ;//acoustic volume velocity - but is not constant? - relates to vocal tract? impedance*pressure?

  ourfloat ag=ago + l*x; // as x is already oldx
    //    ourfloat pb=0.5*airdensity*pow(abs(ug),2))*pow(ag,-2);  // we just ignore ug
    ourfloat pb=0.5f*airdensity*powf(ag,-2);
  //  ourfloat pb=0.5f*airdensity*powf(abs(lastx),2)*powf(ag,-2);
  ourfloat p1=(ps - 1.37f*pb);
  ourfloat p2=-0.5f*pb;

    Fx=0.5f * (p1 + p2)*(l*0.002f); // break out
  //  Fx=6.0f*powf(lastx,2);
  lastx=x;


  //    Fx=stiffness/5.0f * pow(x,2);
  //  Fx=0.008f;
  //  Fx=sin(t*1000.0);

  //  if (i>0.05f && i<0.06f) pulse=1.0f;
  //  else pulse=0.0f;
  //    value=(0.01f-damping*v-stiffness*x)/mass;
      value=(0.01f*pulse+Fx-damping*xprime-stiffness*x)/mass;
  //  value=(Fx- damping*v - stiffness*x)/mass; 
  //  value=(1-pow(x,2))*v-x; // van der pol - this is working
  //  return ( -stiffness / mass ) * x - ( damping / mass ) * v;
  //    return (Fx- ((damping*v) + (stiffness*x)))/mass; 
    //    printf("\nFx: %f x: %f\n",Fx,x);
  return value;
}


ourfloat dx (ourfloat t, ourfloat x, ourfloat v){ // derivative of x - displacement
	return v;
}


ourfloat dvy (ourfloat t, ourfloat x, ourfloat v)
{
  ourfloat value;
  //  if (x>xo)  Fx=stiffness/12 * powf(x,2);
  //  else Fx=500.0f*0.002;
  //  xo=-ago/l;
  //  printf("\n x: %f xo: %f\n", x, xo);

  //  if (x>xo)  Fx=stiffness/5.0f * pow(x,2);
  //  else if (t>0.05 && x<xo) {
  //      Fx=l/2.0*8.0*98.0*l*0.002;
  //  }
  //    else Fx=stiffness/5.0f * pow(x,2);

  // Forcing function from Cataldo - Flanagan Landgraf

  ourfloat ps=783.0f;
  ourfloat ago=0.00005f; // glottal area
  //  ourfloat l=0.0018; //length  we have above
  ourfloat airdensity= 1.225f; // kg/m3 
 //      ourfloat ug= ;//acoustic volume velocity - but is not constant? - relates to vocal tract? impedance*pressure?

  ourfloat ag=ago + l*lastx;
    //    ourfloat pb=0.5*airdensity*pow(abs(ug),2))*pow(ag,-2);
  //      ourfloat pb=0.5f*airdensity*powf(abs(lastx),2)*powf(ag,-2);
  //        ourfloat pb=0.5f*airdensity*powf(ag,-2);
  // ourfloat p1=(ps - 1.37f*pb);
  //  ourfloat p2=-0.5f*pb;

  //    Fx=0.5f * (p1 + p2)*(l*0.002f); // break out
  Fx=0.12f*v; // this is velocity and makes it oscillate - v or lastv
  //Fx=0.0f;
      lastx=v;

  //    Fx=stiffness/5.0f * pow(x,2);
  //  Fx=0.008f;
  //  Fx=sin(t*1000.0);

  if (t>0.05f && t<0.06f) pulse=1.0f;
  else pulse=0.0f;
  //    value=(0.01f-damping*v-stiffness*x)/mass;
      value=(0.01f*pulse+Fx-damping*v-stiffness*x)/mass;
  //  value=(Fx- damping*v - stiffness*x)/mass; 
  //  value=(1-pow(x,2))*v-x; // van der pol - this is working
  //  return ( -stiffness / mass ) * x - ( damping / mass ) * v;
  //    return (Fx- ((damping*v) + (stiffness*x)))/mass; 
    //    printf("\nFx: %f x: %f\n",Fx,x);
  return value;
}

ourfloat rk4 (ourfloat (*dv)(ourfloat t, ourfloat x, ourfloat v), ourfloat t, ourfloat h )
{
	// step 1
	ourfloat x = position;
	ourfloat v = velocity;

	ourfloat dx1 = dx ( t, x, v );
	ourfloat dv1 = dv ( t, x, v );

	// step 2
	x = position + ( h / 2.0f ) * dx1;
	v = velocity + ( h / 2.0f ) * dv1;

	ourfloat dx2 = dx ( t, x, v );
	ourfloat dv2 = dv ( t, x, v );

	// step 3
	x = position + ( h / 2.0f ) * dx2;
	v = velocity + ( h / 2.0f ) * dv2;

	ourfloat dx3 = dx ( t, x, v );
	ourfloat dv3 = dv ( t, x, v );

	// step 4
	x = position + h * dx3;
	v = velocity + h * dv3;

	ourfloat dx4 = dx ( t, x, v );
	ourfloat dv4 = dv ( t, x, v );

	// now combine the derivative estimates and
	// compute new state
	position = position + ( h / 6.0f ) * ( dx1 + dx2 * 2.0f + dx3 * 2.0f + dx4 );
	velocity = velocity + ( h / 6.0f ) * ( dv1 + dv2 * 2.0f + dv3 * 2.0f + dv4 );
#ifdef LAP
	int iii=position*32768.0f;
	//	fwrite(&iii,2,1,fo);
	//	printf("%d\n",iii);
#endif
	// returns new time
	return t + h;
}



void spring_oscillate(int i){
ourfloat newxdobleprime = calc_xdobleprime(i);
ourfloat newxprime = xprime + flintegrate(xdobleprime,newxdobleprime,T);
xdobleprime = newxdobleprime;
x += flintegrate(xprime,newxprime,T);
#ifdef LAP
// int iii=x*32768.0f;
 // fwrite(&iii,2,1,fo);
printf("%f\n",x);
#endif
xprime = newxprime;
}

#ifdef LAP
void main(int argc, char *argv[]){
  int xx,lenny=8,freq=200;
  fo = fopen("testraven.pcm", "wb");

  init();
  ourfloat buffer[320000], otherbuffer[320000];  // try now varying some parameters each second:
ourfloat f=(ourfloat)atoi(argv[1]);
ourfloat ff=(ourfloat)atoi(argv[2]);
ourfloat fff=(ourfloat)atoi(argv[3]);
ourfloat ffff=(ourfloat)atoi(argv[4]);
init_gardner(&syrinx, f, ff);
init_mindlin(&syrinxM, f, ff, fff);
 single_tube_init(ffff); // delay
 RavenTube_init();

//for (lenny=0;lenny<2;lenny++){

// test for runge kutta - spring only

 position = 0.0f;
 velocity = 0.0f;
 mass = ff/100.0f; // = .1e-3 - all kg and m
 stiffness = f/100.0f; // was 30.0f or 0.47 when calculated according to cataldo
 damping = 2.0f*sqrt(mass*stiffness)*0.1f;
 //  damping=0.0015;
 time = 0.0f;
  d1length=ffff;

  int samplerun=3200;

   for (x=0;x<10;x++){
        for (xx=0;xx<samplerun;xx++){
     ///        time=rk4(dvy, time,0.00002); // 44k samplerate
    //                spring_oscillate(xx);
    //         gardner_oscillate(&syrinx,xx);
	  buffer[xx]=mindlin_oscillate(&syrinxM);
     //      do_impulse(buffer,3200,freq);
         }

	RavenTube_next(buffer, otherbuffer,samplerun);
	//	         SingleRavenTube_next(buffer, otherbuffer,samplerun);
	//	single_tube_next(buffer, otherbuffer,samplerun, ffff);

   for (xx=0;xx<samplerun;xx++){
      int iii=32768.0f*otherbuffer[xx];
      int iiii=32768.0f*buffer[xx];
      printf("%d %d\n",iiii,iii);
      fwrite(&iii,2,1,fo);
   }

 
   //for latest mindlin-> optimum values

alpha+=0.001f;
beta=0.25f;
 }

 fclose(fo);

//RavenTube_next(buffer, 32000);

//}


  // we need some kind of input?

  /*  for (x=0;x<100;x++){
    single_tube_init(lenny);
    donoise(buffer,3200);
    single_tube(buffer, 3200, lenny);
    lenny++;
    //    freq+=5;
    }*/

 // pressure, k1, m1, d1

 /* k1=ff; k2=k1; */
 /* m1=fff; m2=m1; */
 /* d1=ffff; d2=d1; */
 /* r1=0.0001*sqrtf(m1*k1); */
 /* r2=r1; */


 // for (x=0;x<100;x++){
 //  rtick(buffer, 96000, f);
 /*         k1=k1+0.001; */
 /*   m1=m1+0.001; */
 /* 	k2=k1; */
 /* 	m2=m1; */
 /* r1=0.0001*sqrtf(m1*k1); */
 /* r2=r1; */

 /*     d1+=0.00001; */
 /*     d2=d1; */
     //     OneTube_next(buffer, 32000);
     //  	clearOld();
	//     	OneTube_init();
 //	 }
}
#endif

