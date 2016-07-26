/*
 *  based on balloon1.cpp

See http://www.dei.unipd.it/~avanzini/downloads/paper/avanzini_eurosp01_revised.pdf

 */
#include <stdio.h>
#include "math.h"

// forlap: 

// port below, and one tube for raven trachea - from twotube to onetube SC code: SLUGens.cpp
// or maybe replace with pluck.c code  from JOS: https://ccrma.stanford.edu/~jos/pmudw/pluck.c

////output= TwoTube.ar(input, scatteringcoefficient,lossfactor,d1length,d2length);

// setting up

FILE *fo;

int d1length=3; // what is d1length measured in??? relates to length in mm, speed sound and samplerate I guess...
int d2length=4; // what is d1length measured in??? relates to length in mm, speed sound and samplerate I guess...

// delay = round( L * fs / c); = say for 70mm = 0.07 * 32000 (here) / 347.23   // speed of sound (m/sec)

float lossfactor=0.9;

float delay1right[7]; //->d1length of floats
float delay1left[7];//=>d1length of floats

float delay2right[7]; //->d1length of floats
float delay2left[7];//=>d1length of floats


void OneTube_init(void){
	int i;

	//initialise to zeroes!
	for (i=0; i<d1length; ++i) {
		delay1right[i]= 0.0;
		delay1left[i]= 0.0;
		delay2right[i]= 0.0;
		delay2left[i]= 0.0;

	}
}

float f1in= 0.0;
float f1out= 0.0;
float f2in= 0.0;
float f2out= 0.0;

int d1rightpos= 0;
int d1leftpos= 0;
int d2rightpos= 0;
int d2leftpos= 0;


void OneTube_next(float *inn, int inNumSamples) {

	int i;

	//value to store
	float * in = inn;//= IN(0);
	//	float * out;//= OUT(0);
	float k= 0.9;// (float)ZIN0(1); //scattering coefficient updated at control rate?
	float loss= lossfactor;

	float * d1right= delay1right;
	float * d1left= delay1left;
	float * d2right= delay2right;
	float * d2left= delay2left;

	//have to store filter state around loop; probably don't need to store output, but oh well

	for (i=0; i<inNumSamples; ++i) {

		//update outs of all delays
		float d1rightout= d1right[d1rightpos];
		float d1leftout= d1left[d1leftpos];
		float d2rightout= d2right[d2rightpos];
		float d2leftout= d2left[d2leftpos];

		//output value
		//		out[i]=d1rightout;


		signed int s16=(signed int)(d1rightout*32768.0);
		//		signed int s16=(signed int)(in[i]*32768.0);
		//		printf("d1out %d in  %d \n",s16, (signed int)(in[i]*32768.0));
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
		//		d1left[d1leftpos]= d1rightout*k+ ((1-k)*d2leftout);
		d1left[d1leftpos]= d1rightout*k;//+ ((1-k)*d2leftout); FIX!

		//update delay line position pointers

		d1rightpos= (d1rightpos+1)%d1length;
		d2rightpos= (d2rightpos+1)%d2length;
		d1leftpos= (d1leftpos+1)%d1length;
		d2leftpos= (d2leftpos+1)%d2length;
	}
}

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

//	r1 =2.0976e-8;
	r1 =1.2e-3;
//	r2 =2.0976e-8;
	r2 =1.2e-3;
	m1=3.848e-6;
//	m1 =5.8889e-6;
	m2 =3.848e-6;

	k1 =0.1;
	k2 =0.1;
	//	k2 =0.5;
//	k12=0.04;
	k12=0.04;
	//	aida =10000000.0;
	aida =0.000001;
//	d1 =1.5e-5;
	d1 =0.00005;
	d2 =0.00005;

	lg =0.007;
	gain=400.0;
	S=0.000314;
	Ag01=3.14e-6;
	Ag02=3.14e-6; 


	x1Prev=0.0;
	x1PrevPrev=0.0;
	x2Prev=0.0;
	x2PrevPrev=0.0;
	pm1Prev=0.0;
	pm2Prev=0.0;
	uPrev=0.0;
	ps=0.0;
	Fs=32000.0;

  /// ballon

  /*
//	r1 =2.0976e-8;
	r1 =2.0976e-8;
//	r2 =2.0976e-8;
	r2 =2.0976e-8;
//	m1 =4.8889e-7;
	m1 =5.8889e-6;
//	m2 =4.8889e-7;
	m2 =5.8889e-6;
	k1 =0.5;
//	k2 =0.09;
	k2 =0.5;
//	k12=0.04;
	k12=0.04;
//	aida =10000000.0;
	aida =0.000001;
//	d1 =1.5e-5;
	d1 =1.5e-5;
	d2 =1.5e-5;
	lg =0.0163;
	gain=1000.0;
	S=5e-5;
	Ag01=5e-9;
	Ag02=5e-9;
	x1Prev=0.0;
	x1PrevPrev=0.0;
	x2Prev=0.0;
	x2PrevPrev=0.0;
	pm1Prev=0.0;
	pm2Prev=0.0;
	uPrev=0.0;
	ps=0.0;
	Fs=48000.0;
  */
}


int rtick(float *buffer, int bufferSize, double pressureIn) {
  float *samples = (float *) buffer;

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

	  /*	 	  if (i<5) pressureIn=0;
	  else if (i< (32000/90)) pressureIn=lastp+ 400/(32000/90);
	  else if (i<= (32000/80)) pressureIn=lastp;
	  else pressureIn=lastp+(30-400)/(32000-(32000/80));

	  */

	  // constant pressure
	  pressureIn=300; // 0.3 kPa after Fletcher

	  //   signed int s16=(signed int)(computeSample(pressureIn)*32768.0);
	  *samples++=(float)(computeSample(pressureIn));
   //   printf("%d\n",s16);
   //   fwrite(&s16,2,1,fo);
   lastp=pressureIn;
   //*samples++ = [(id)dataPointer amp] * bsynth->tick();
	}

	return 0;
};

void main(void){
  int x;
  fo = fopen("testraven.pcm", "wb");

  init();
  float buffer[32000];  // try now varying some parameters each second:

  OneTube_init();

  for (x=0;x<10;x++){
  rtick(buffer, 32000, 300.0);
          k1=k1+0.01;
  //    m1=m1+0.00001;
  	k2=k1;
	//    m2=m1;
    r1=0.0001*sqrt(m1*k1);
    r2=r1;

    //    d1+=0.00001;
    //    d2=d1;
    OneTube_next(buffer, 32000);
    clearOld();
  OneTube_init();
  }
}

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
    flow1=(-b+sqrt(det))/(2*a);
    flow2=(-b-sqrt(det))/(2*a);
  }
  else{
    flow1=(-b)/(2*a);
    flow2=(-b)/(2*a);
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
      //           pm2=pressure_in;
      //         pm1=pressure_in;
    }
  }     
  else{
    //    pm1=pressure_in;
    //   pm2=0.0;
  }

  /*  if (pm1>pressure_in)
    //	{pm1=pressure_in;}
    
    if (pm2>pressure_in)
      //     {pm2=pressure_in;}
    
      if (pm1<0.0)
	//   pm1(n)=abs(pm1(n));
        {pm1=0.0;}
  */

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

