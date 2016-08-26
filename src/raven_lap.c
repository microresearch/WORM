/* what do we have here?

- resonances with twotube and onetube delay line

- raven syrinx models: Mindlin, IF (dooble only?), gardner

TODO: dooble->floot and port tests, port in latest FLETCHER

split to raven_lap.c and raven.c 

we want to look at resonances imposed on plague model by raven

*/

#include <stdio.h>
#include "math.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

double integrate(double old_value, double new_value, double period);
double old_integrate(double old_value, double new_value, double period);

#define PI            3.1415927

 // forlap: 

 // port below, and one tube for raven trachea - from twotube to onetube SC code: SLUGens.cpp
 // or maybe replace with pluck.c code  from JOS: https://ccrma.stanford.edu/~jos/pmudw/pluck.c

 ////output= TwoTube.ar(input, scatteringcoefficient,lossfactor,d1length,d2length);

 // setting up

 FILE *fo;

 int d1length=7; // *trachea = 7 samples = 1st K = (0.3838-3.141) / (0.3838+3.141) = -2.7572 / 3.525 = -0.782*
 int d2length=2; // //*beak length=20mm = 2 samples   2nd K = (3.141 - 3.141) / (3.141 + 3.141) = 0 ????*


 // delay = round( L * fs / c); = say for 70mm = 0.07 * 32000 (here) / 347.23   // speed of sound (m/sec)

 double lossfactor=0.99;

 //just need delay1 for one TUBE!!

 double delay1right[8]; //->d1length of doubles
 double delay1left[8];//=>d1length of doubles

 double delay2right[8]; //->d1length of doubles
 double delay2left[8];//=>d1length of doubles

 void donoise(short *out, int numSamples){
   int x;
   for (x=0;x<numSamples;x++){
     double xx=(double) ( 2.0 * rand() / (RAND_MAX + 1.0) - 1.0 );
     out[x]=xx*8000.0;
   }
 }


 void do_impulse(short* out, int numSamples, int freq){ //- so for 256 samples we have freq 125 for impulse
     // from Impulse->LFUGens.cpp
   int i;
   static double phase =0.0f;
   double z, freqinc;
   freqinc=0.00003125 * freq;

   for (i=0; i<numSamples;++i) {
     if (phase >= 1.f) {
       phase -= 1.f;
       z = 1.f;
     } else {
       z = 0.f;
     }
     phase += freqinc; // punch in freq is freqmul=1/32000 = 0.00003125 * 1000 (32000/32) = 0.03125
     out[i]=z*32768.0f;
     //    fwrite(&out[i],2,1,fo);

   }
 }


 void RavenTube_init(void){
	 int i;

	 //initialise to zeroes!
	 for (i=0; i<d1length; ++i) {
		 delay1right[i]= 0.0;
		 delay1left[i]= 0.0;
		 delay2right[i]= 0.0;
		 delay2left[i]= 0.0;

	 }
 }

 double f1in= 0.0;
 double f1out= 0.0;
 double f2in= 0.0;
 double f2out= 0.0;

 int d1rightpos= 0;
 int d1leftpos= 0;
 int d2rightpos= 0;
 int d2leftpos= 0;

 void RavenTube_next(double *inn, int inNumSamples) {

	 int i;

	 //value to store
	 double * in = inn;//= IN(0);
	 //	double * out;//= OUT(0);
	 double k= -0.782;// (double)ZIN0(1); //scattering coefficient updated at control rate?
	 double loss= lossfactor;

	 // easier stick with originals

	 double * d1right= delay1right;
	 double * d1left= delay1left;
	 double * d2right= delay2right;
	 double * d2left= delay2left;

	 //have to store filter state around loop; probably don't need to store output, but oh well

	 for (i=0; i<inNumSamples; ++i) {

		 //update outs of all delays
		 double d1rightout= d1right[d1rightpos];
		 double d1leftout= d1left[d1leftpos];
		 double d2rightout= d2right[d2rightpos];
		 double d2leftout= d2left[d2leftpos];

		 //output value
		 //		out[i]=d1rightout;


		  signed int s16=(signed int)(d2rightout*32768.0);
		  //	 		 s16=in[i]*32768.0; // TESTY=straight OUT!
		 //		signed int s16=(signed int)(in[i]*32768.0);
		 //		 printf("ff %f \n",in[i]);
		fwrite(&s16,2,1,fo);

		 //update all filters
		 f1out= loss*0.5*(f1in+d1leftout);
		 f1in= d1leftout;

		 f2out= loss*(0.5*f2in+0.5*d2rightout);
		 f2in= d2rightout;

		 //calculate inputs of all delays
		 d1right[d1rightpos]= in[i]+f1out;
		 d2right[d2rightpos]= d1rightout*(1+k)+ ((-k)*d2leftout);
		 d2left[d2leftpos]= f2out;
		 d1left[d1leftpos]= d1rightout*k+ ((1-k)*d2leftout);
		 //		d1left[d1leftpos]= d1rightout*k;//+ ((1-k)*d2leftout); FIX!

		 //update delay line position pointers

		 d1rightpos= (d1rightpos+1)%d1length;
		 d2rightpos= (d2rightpos+1)%d2length;
		 d1leftpos= (d1leftpos+1)%d1length;
		 d2leftpos= (d2leftpos+1)%d2length;
	 }
 }


 #define DOUBLE_TO_SHORT(x) ((int)((x)*32768.0))

 typedef struct _DelayLine {
     short *data;
     int length;
     short *pointer;
     short *end;
 } DelayLine;

 static DelayLine *initDelayLine(int len) {
     DelayLine *dl = (DelayLine *)calloc(len, sizeof(DelayLine));
     dl->length = len;
     if (len > 0)
	 dl->data = (short *)calloc(len, len * sizeof(short));
     else
	 dl->data = 0;
     dl->pointer = dl->data;
     dl->end = dl->data + len - 1;
     return dl;
 }

 static void freeDelayLine(DelayLine *dl) {
     if (dl && dl->data)
		 free(dl->data);
     dl->data = 0;
     free(dl);
 }

 inline static void setDelayLine(DelayLine *dl, double *values, double scale) {
     int i;
     for (i=0; i<dl->length; i++)
	 dl->data[i] = DOUBLE_TO_SHORT(scale * values[i]);
 }

 /* lg_dl_update(dl, insamp);
  * Places "nut-reflected" sample from upper delay-line into
  * current lower delay-line pointer location (which represents
  * x = 0 position).  The pointer is then incremented (i.e. the
  * wave travels one sample to the left), turning the previous
  * position into an "effective" x = L position for the next
  * iteration.
  */
 static inline void lg_dl_update(DelayLine *dl, short insamp) {
     register short *ptr = dl->pointer;
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
 static inline void rg_dl_update(DelayLine *dl, short insamp) {
     register short *ptr = dl->pointer;    
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
 static inline short dl_access(DelayLine *dl, int position) {
     short *outloc = dl->pointer + position;
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
 static inline short rg_dl_access(DelayLine *dl, int position) {
     return dl_access(dl, position);
 }

 /* lg_dl_access(dl, position);
  * Returns spatial sample at location "position", where position zero
  * is equal to the current lower delay-line pointer position (x = 0).
  * In a left-going delay-line, position increases to the right, and
  * delay DEcreases to the right => left = future and right = past.
  */
 static inline short lg_dl_access(DelayLine *dl, int position) {
     return dl_access(dl, position);
 }

 static DelayLine *upper_rail,*lower_rail;

 static inline short bridgeReflection(int insamp) {
     static short state = 0; /* filter memory */
     /* Implement a one-pole lowpass with feedback coefficient = 0.5 */
     /* outsamp = 0.5 * outsamp + 0.5 * insamp */
     short outsamp = (state >> 1) + (insamp >> 1);
     state = outsamp;
     return outsamp;
 }


 void single_tube_init(int len){
     int i, rail_length = len;
     upper_rail = initDelayLine(rail_length);
     lower_rail = initDelayLine(rail_length);
 }

 void single_tube(short *inn, int inNumSamples, int length) {
     short yp0,ym0,ypM,ymM;
     int i;
     short outsamp, outsamp1,out;
     for (i=0;i<inNumSamples;i++){

     /* Output at pickup location */
     out  = rg_dl_access(upper_rail, length-1);
     fwrite(&out,2,1,fo);

     //    outsamp1 = lg_dl_access(lower_rail, pickup_loc);
     //	outsamp += outsamp1;

     ym0 = lg_dl_access(lower_rail, 1);     /* Sample traveling into "bridge" */ // bridge is base of trachea
     ypM = rg_dl_access(upper_rail, length - 1); /* Sample to "nut" */ // was -2 why???? nut is mouth/air/beak

     ymM = -ypM;                    /* Inverting reflection at rigid nut */
     //    yp0 = -bridgeReflection(ym0)+inn[i];  /* Reflection at yielding bridge */  // where do we add our incoming sample?
     yp0=(ym0*0.9)+inn[i];

     /* String state update */
     rg_dl_update(upper_rail, yp0);  // was yp0 /* Decrement pointer and then update */
     lg_dl_update(lower_rail, ymM); /* Update and then increment pointer */
     }


 }

 /*
  *  based on balloon1.cpp - IF model...

 See http://www.dei.unipd.it/~avanzini/downloads/paper/avanzini_eurosp01_revised.pdf - measurements

  */

 double computeSample(double pressure_in);
 void clearOld();

 double ps;		//subglottal pressure
 double r1;		//damping factor
 double r2;
 double m1;		//mass
 double m2;
 double k1;		//spring constant
 double k2;
 double k12;		//coupling spring constant
 double d1;		//glottal width
 double d2;
 double lg;		//glottal length
 double aida;		//nonlinearity coefficient
 double S;			//subglottal surface area
 double Ag01;		//nominal glottal area, with mass at rest position
 double Ag02;
 double pm1Prev;	//pressure at previous time step
 double pm2Prev;
 double x1Prev;	//displacement at previous time step
 double x1PrevPrev;//displacement at previous time step to the previous one
 double x2Prev;
 double x2PrevPrev;
 double gain;		//after-market gain
 double uPrev;		//previous flow value
 double Fs;		//calculation sampling rate, not actual audio output sample rate


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
	aida =10000000.0;
//	aida =10.0;
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
	Fs=32000.0;
  
}


int rtick(double *buffer, int bufferSize, double pressureIn) {
  double *samples = (double *) buffer;

/*	bsynth->setM1(5e-8*(double)[(id)dataPointer m1In]);
	bsynth->setM2(5e-8*(double)[(id)dataPointer m2In]);
	bsynth->setR1(5e-9*(double)[(id)dataPointer r1In]);
	bsynth->setR2(5e-9*(double)[(id)dataPointer r2In]);
	bsynth->setK1(1e-3*(double)[(id)dataPointer k1In]);
	bsynth->setK2(1e-3*(double)[(id)dataPointer k2In]);
	bsynth->setD1(1.5e-7*(double)[(id)dataPointer d1In]);
	bsynth->setD2(1.5e-7*(double)[(id)dataPointer d2In]);
	bsynth->setK12(1e-4*(double)[(id)dataPointer k12In]);
	bsynth->setLg(1.3e-4*(double)[(id)dataPointer lgIn]);
	bsynth->setAida(1.1e-4*(double)[(id)dataPointer aidaIn]);
	bsynth->setS(5.5e-7*(double)[(id)dataPointer SIn]);
	bsynth->setAg01(5.1e-10*(double)[(id)dataPointer Ag01In]);
	bsynth->setAg02(5.1e-10*(double)[(id)dataPointer Ag02In]);
	bsynth->setGain((double)[(id)dataPointer gainIn]);
	bsynth->setFs((double)[(id)dataPointer FsIn]);
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

	int i; double lastp;
	for (i=0; i<bufferSize; i++ ) {
	  //	  *samples++ = computeSample(pressureIn);
	  //	  printf("%f  ",computeSample(pressureIn));

	  	 	  if (i<5) pressureIn=0;
	  else if (i< (32000/90)) pressureIn=lastp+ 400/(32000/90);
	  else if (i<= (32000/80)) pressureIn=lastp;
	  else pressureIn=lastp-360.0f/(32000.0f-(32000.0/80.0));

	  // constant pressure
//	  pressureIn=300; // 0.3 kPa after Fletcher

	  //   signed int s16=(signed int)(computeSample(pressureIn)*32768.0);
	  *samples++=(double)(computeSample(pressureIn));
   //   printf("%d\n",s16);
   //   fwrite(&s16,2,1,fo);
   lastp=pressureIn;
   //   fwrite(&s16,2,1,fo);

   //*samples++ = [(id)dataPointer amp] * bsynth->tick();
	}

	return 0;
};

double computeSample(double pressure_in){
  double T=1/Fs;
  double rho = 1.14; 
  double rhosn = rho*0.69;
  double hfrho=rho/2;
  double v = 1.85e-5;
  double twvd1lg=12*v*d1*lg*lg;
  double twvd2lg=12*v*d2*lg*lg;
  double Ag012lg=Ag01/2/lg;
  double Ag022lg=Ag02/2/lg;
  double lgd1=lg*d1;
  double lgd2=lg*d2;
  double m1T=m1/T/T;
  double m2T=m2/T/T;
  double r1T=r1/T;
  double r2T=r2/T;
  double C11=k1*(1+aida*x1Prev*x1Prev);
  double C12=k2*(1+aida*x2Prev*x2Prev);
  double C21=k1*(1+aida*(x1Prev+Ag012lg)*(x1Prev+Ag012lg));
  double C22=k2*(1+aida*(x2Prev+Ag022lg)*(x2Prev+Ag022lg));
  double alpha1=lgd1*pm1Prev;
  double alpha2=lgd2*pm2Prev;
  double beta1=m1T*(x1PrevPrev-2*x1Prev);
  double beta2=m2T*(x2PrevPrev-2*x2Prev);
  double gamma1=-r1T*x1Prev;
  double gamma2=-r2T*x2Prev;
  double delta1=Ag012lg*C21;
  double delta2=Ag022lg*C22;
  double lambda1=-k12*x2Prev;
  double lambda2=-k12*x1Prev;
  double x1=0.0;
  double x2=0.0;
  double pm1=0.0;
  double pm2=0.0;
  double A1=0.0;
  double A2=0.0;
  double A1n2=0.0;
  double A1n3=0.0;
  double A2n2=0.0;
  double A2n3=0.0;
  double a=0.0;
  double b=0.0;
  double c=0.0;
  double det=0.0;
  double flow1=0.0;
  double flow2=0.0;
  double udif1=0.0;
  double udif2=0.0;
  double u=0.0;
  double g1=0.0;
  double g2=0.0;
  double g4=0.0;
  double g5=0.0;
  double pm1b=0.0;
  double pm2b=0.0;


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
    
  if (A1<=0)
    {A1=0.1e-25;}
    

  if (A2<=0)
    {A2=0.1e-25;}
   
	
	
  A1n2=A1*A1; 
  A1n3=A1n2*A1;

  A2n2=A2*A2; 
  A2n3=A2n2*A2;
	
  a= (rhosn/A1n2)+hfrho*(1/A2n2-1/A1n2)+hfrho/A2n2*(2*A2/S*(1-A2/S));
  b= twvd1lg/A1n3+twvd2lg/A2n3;
  c= -pressure_in;

  det=b*b-4.0*a*c;

  if (det>=0){
    flow1=(-b+sqrt(det))/(2.0*a);
    flow2=(-b-sqrt(det))/(2.0*a);
  }
  else{
    flow1=(-b)/(2.0*a);
    flow2=(-b)/(2.0*a);
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
  g5=hfrho*u*u/A2n2*(2*A2/S*(1-A2/S));

  pm1=pressure_in-g1-g2/2;
  pm2=g5+g4/2;

  if (x1>=-Ag012lg){
    
    pm1b=(m1T*(x1-2*x1Prev+x1PrevPrev)+r1T*(x1-x1Prev)+ k1*x1*(1+aida*x1*x1) +k12*(x1-x2))/(lgd1);
    pm1=pm1/2+pm1b/2;
        
	
    if (x2>=-Ag022lg){

      pm2b=(m2T*(x2-2*x2Prev+x2PrevPrev)+r2T*(x2-x2Prev)+ k2*x2*(1+aida*x2*x2) -k12*(x1-x2))/(lgd2);
      pm2=pm2/2+pm2b/2;
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
	{pm1=fabsf(pm1);}
 

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
printf("UUUU %f pressure-in %f\n", u, pressure_in);
  return gain*u;
}

void clearOld()
{
x1Prev=0;
x1PrevPrev=0;
x2Prev=0;
x2PrevPrev=0;
pm1Prev=0;
pm2Prev=0;
uPrev=0;
ps=0;
}

//////////////////////// MINDLIN - from Laje, Gardner and Mindlin:

typedef struct mindlin {
double x, xprime,xdobleprime, k, b, c, f0, T, p0;
}Mindlin;

Mindlin syrinxM;

void init_mindlin(Mindlin* mind, double b, double k, double c){
mind->b=b;
mind->k=k;
mind->c=c;
mind->x=0.01;
mind->p0=100000.0f;
mind->xdobleprime=0.0f;
mind->f0=2000.0f;
mind->T = 1.0/96000.0;

}

double calc_xdobleprime_mindlin(Mindlin *mind){
// from pseudocode: result/0?/=xdobleprime+k*xprime + (c*x)^2 * xprime * xprime - b * xprime + f0;
// xdobleprime = - (k*x + c *x^2 * xprime * xprime - b * xprime + f0);

//return - (mind->k*mind->x + mind->c* pow(mind->x,2) * mind->xprime * mind->xprime - mind->b * mind->xprime + mind->f0);

// from paper: xdobleprime= -k*x-c*x^2*xprime+b*xprime-f0;

// other paper xxdobleprime= -k*x-(B1 + B2 * x^2 - p0/=air sac pressure/) * xprime - f0

return -mind->k*mind->x - mind->c*pow(mind->x,2) * mind->xprime+mind->b *mind->xprime-mind->f0; // exp overruns

// from: http://www.scholarpedia.org/article/Models_of_birdsong_%28physics%29

//return -mind->k * mind->x - (mind->b + pow(mind->c + mind->x,2) - mind->p0)* mind->xprime - mind->f0;

}

double mindlin_oscillate(Mindlin* mind){
int iii;  
double newxdobleprime = calc_xdobleprime_mindlin(mind);
double newxprime = mind->xprime + integrate(mind->xdobleprime,newxdobleprime,mind->T);
mind->xdobleprime = newxdobleprime;
mind->x += integrate(mind->xprime,newxprime,mind->T);
printf("%f\n", mind->x);
iii=32768.0f*mind->x;
fwrite(&iii,2,1,fo);
mind->xprime = newxprime;
return mind->x;
}

////////////////////// GARDNER

typedef struct gardner {
  double x, xprime,oldxprime,xdobleprime, K, Pb, a0, b0, t, M, K_scale, K_scalex, Pb_scalex, D, D2, Pb_scale, T, freq, ofreq;
}Gardner;

Gardner syrinx;

void init_gardner(Gardner* gd, int16_t kk, int16_t pb){
gd->x=0;
gd->xprime=1.0;
gd->xdobleprime = 0.0f;
gd->K =  (double)kk; // between 11k and 12k something happens... - and now???
gd->Pb = (double)pb;

/*
	upper_labia = 0.02 #cm
	lower_labia = 0.04 #cm
	t_constant = .00015 #s # phenomenological constant Titze!
	mass = .005 #g/cm3
	K=restitution_constant = 200000 #g*cm/s2cm3 // this is K- IF= approx 200 kdyn/cm3 ??? // paper 0-8 N/cm3

8 Newtons is 800 kdyn = 800 000 dyne = 

	D_coefficient = 5 #dynes*s/cm3
	D2_coefficient = .01 #dyne*s/cm5 .001
	bronchial_pressure = 10000 #g/(cm*s2) # 0-3 kPa

1 pascal is 1 N/m2 = 1 kg /ms2 =  say 2 kPa = 2 N/m2 2000 kPa=20000 g/s
*/

//#constants
 gd->a0 = 0.02;
 gd->b0 = 0.04;
 gd->t = 0.00015;
 gd->M = 0.005;
 gd->K_scale = 1.0;
 gd->K_scalex = 1000.0;
 gd->D = 5.0;
 gd->D2 = 0.01;
 gd->Pb_scale = 1.0;
 gd->Pb_scalex = 1000.0;
 gd->T = 1/96000.0;
 gd->freq = 10.0;
 gd->ofreq= 20.0;
}

double calc_xdobleprime_gardner(Gardner *gd, int i){
  double a = gd->a0 + gd->x + (gd->t*gd->xprime);
  double b = gd->b0 + gd->x - (gd->t*gd->xprime);
  double Pf = gd->Pb*(1 - (a/b));
  printf("%f\n",gd->x);
  int iii=(double)gd->x*327680.0f;
  fwrite(&iii,2,1,fo);
  //		return (Pf - (self.K*self.x) - (self.D2*math.pow(self.xprime,3)) - (self.D*self.xprime))/self.M
  //    return (Pf - (gd->K*gd->x) - (gd->D*gd->xprime))/gd->M;
  //  return (Pf - (gd->K*gd->x) - (gd->D2*pow(gd->oldxprime,3)) - (gd->D*gd->xprime))/gd->M;
return ((gd->Pb*((gd->a0 - gd->b0) +(2*gd->t*gd->xprime)/(gd->x+gd->b0+(gd->t*gd->xprime))))-((gd->K*gd->x) + (gd->D2*pow(gd->xprime,3)) + (gd->D*gd->xprime)))/gd->M;
}


void gardner_oscillate(Gardner* gd, int i){
double newxdobleprime = calc_xdobleprime_gardner(gd,i);
double newxprime = gd->xprime + integrate(gd->xdobleprime,newxdobleprime,gd->T);
gd->xdobleprime = newxdobleprime;
gd->x += integrate(gd->xprime,newxprime,gd->T);
//fwrite(&gd->x,2,1,fo);
gd->oldxprime=gd->xprime;
gd->xprime = newxprime;
gd->K+= gd->K_scale;
gd->Pb+= gd->Pb_scale;
  //    gd->K = gd->K_scale*sin((2*PI*gd->T*gd->freq*i)) + gd->K_scalex;
		  //		gd->K = gd->K_scale*2.0
  //    gd->Pb = gd->Pb_scale*cos((2*PI*gd->T*gd->ofreq*i) + PI) + gd->Pb_scalex;
		  //               gd->Pb = gd->Pb_scale
  //   gd->freq += 0.01;
  //   gd->ofreq+=0.01;
}

//// simple spring undriven

double x = 0.0;
double xprime = 1.0;
double xdobleprime = 0.0;
double M = 0.005;
double R = 5.0;
double K = 200000.0;
double T = 1.0/96000.0;



double calc_xdobleprime(int i){
  //# M x'' + Rx' + Kx = 0
  return (- (R*xprime) - (K*x) )/M;
}

double integrate(double old_value, double new_value, double period){
  return (old_value + new_value)*(period/2.0);
}

double flintegrate(double old_value, double new_value, double period){
  return (old_value + new_value)*(period/2.0);
}

double old_integrate(double old_value, double new_value, double period){
return (old_value + new_value)*period;
}

double old_flintegrate(double old_value, double new_value, double period){
return (old_value + new_value) *period;
}

void spring_oscillate(int i){
double newxdobleprime = calc_xdobleprime(i);
double newxprime = xprime + flintegrate(xdobleprime,newxdobleprime,T);
xdobleprime = newxdobleprime;
x += flintegrate(xprime,newxprime,T);
printf("%f\n",x);
xprime = newxprime;
}

void main(int argc, char *argv[]){
  int xx,lenny=8,freq=200;
  fo = fopen("testraven.pcm", "wb");

  init();
  double buffer[320000];  // try now varying some parameters each second:
double f=(double)atoi(argv[1]);
double ff=(double)atoi(argv[2]);
double fff=(double)atoi(argv[3]);
init_gardner(&syrinx, f, ff);
init_mindlin(&syrinxM, f, ff, fff);

//RavenTube_init();

//for (lenny=0;lenny<2;lenny++){

for (xx=0;xx<3200;xx++){
  //  spring_oscillate(xx);
   gardner_oscillate(&syrinx,xx);
//buffer[xx]=mindlin_oscillate(&syrinxM);
}

//RavenTube_next(buffer, 32000);

//}
  // we need some kind of input?

  /*  for (x=0;x<100;x++){
    single_tube_init(lenny);
    //    do_impulse(buffer,320,freq);
    donoise(buffer,3200);
    single_tube(buffer, 3200, lenny);
    lenny++;
    //    freq+=5;
    }*/

//    for (x=0;x<10;x++){
//  rtick(buffer, 32000, 300.0);
//          k1=k1+0.01;
//    m1=m1+0.01;
//	k2=k1;
//	m2=m1;
//  r1=0.0001*sqrt(m1*k1);
//  r2=r1;

//      d1+=0.00001;
//      d2=d1;
//OneTube_next(buffer, 32000);
//	clearOld();
//	OneTube_init();
//  }
}


