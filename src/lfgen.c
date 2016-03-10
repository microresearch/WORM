// LF - reverted to PC - add to write wav or just raw file and test/list different sets of parameters ofr voice types etc.

#include <errno.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>

typedef unsigned int u16;

#define MY_PI 3.14159265359

#define TRUE 1
#define FALSE 0

double noiseRemainder;
double noiseAdd;
double noiseSample;

double noiseAmount = 0.025;
double noiseDuration = 0.5;
double noiseStart = 0.75;


// Get LF waveform coefficients
double t;
double LFcurrentSample;
double LFcurrentSample1;
double period; //= fxs->_period;
int dataLength; //= fxs->_dataLength;
double te; //= fxs->_te;
double Eo; //= fxs->_Eo;
double alpham; //= fxs->_alpha;
double wg; //= fxs->_wg;
double Ee; // = fxs->_Ee;
double epsilon; //= fxs->_epsilon;
double ta,tppp; //= fxs->_ta;
double tc; //= fxs->_tc;
int k=0; //= fxs->_k;

// basic voice

double _tcVal = 1.0;
double _teVal = 0.780;
double _tpVal = 0.6;
double _taVal = 0.028;
double vocalTension = 0.0;
int noiseOn = FALSE;

// FRY?
/*
double _tcVal = 1.0;
double _teVal = 0.251;
double _tpVal = 0.19;
double _taVal = 0.008;
double noiseOn = FALSE;
double vocalTension = 0.0;
*/

/* others

FRY:
double _tcVal = 1.0;
double _teVal = 0.251;
double _tpVal = 0.19;
double _taVal = 0.008;
double _noiseOn = FALSE;
double _vocalTension = 0.0;

FALSETTO:
double _tcVal = 1.0;
double _teVal = 0.770;
double _tpVal = 0.570;
double _taVal = 0.133;
double _noiseOn = TRUE;
double _noiseAmount = 0.015;
double _noiseDuration = 0.5;
double _noiseStart = 0.75;
double _vocalTension = 0.0;

BREATHY:
double _noiseOn = TRUE;
double _noiseDuration = 0.5;
double _noiseStart = 0.75;
double _noiseAmount = 0.025;
double _teVal = 0.756;
double _tpVal = 0.529;
double _taVal = 0.082;

MODAL:
double _noiseOn = FALSE;
double _teVal = 0.575;
double _tpVal = 0.457;
double _taVal = 0.028;

*/

void createLFInput(void);

void main(void){
  int x;


/* from (void)createLFInput(void)

 _period = period;
_dataLength = dataLength;
_Eo = Eo;
_Ee = Ee;
_alpha = alpha;
_wg = wg;
_epsilon = epsilon;
_tc = tc;
_te = te;
_tp = tp;
_ta = ta;
*/

// parameters from LFinput are: alpham, epsilon - wg and datalength are simple but all changes with F0!!!
// so we will have lookups for alpham and epsilon

createLFInput();

// Main LF-waveform calculation performed here: IN LOOP!

// say for one second at 32000

// open file and write samples to it...

FILE* fo = fopen("testlfgen.pcm", "wb");


for (x=0;x<320;x++){

  /*if (k>dataLength) {
k = k - dataLength;
}*/

// ROSENBERG:
  signed int s16;
  float sample;
 float T=1/120.0;
 float fs=32000;
 float pulselength=T*fs;
 float N2=pulselength*0.5;
 float N1=0.5*N2;
 int kkk;
 for (kkk=0;kkk<N1;kkk++){
   //    gn(n)=0.5*(1-cos(pi*(n-1)/N1));
   LFcurrentSample1=0.5*(1-cos(MY_PI*(kkk-1)/N1));
   s16=(signed int)(LFcurrentSample1*32000.0);
   //   printf("%d\n",s16);
   fwrite(&s16,2,1,fo);
 }
 for (kkk=N1;kkk<N2;kkk++){
   //    gn(n)=cos(pi*(n-N1)/(N2-N1)/2);
   LFcurrentSample1=cos(MY_PI*(kkk-N1)/(N2-N1)/2);
   s16=(signed int)(LFcurrentSample1*32000.0);
   //   printf("%d\n",s16);
   fwrite(&s16,2,1,fo);
 }
 for (kkk=N1;kkk<N2;kkk++){
   LFcurrentSample1==0.0;
   s16=(signed int)(LFcurrentSample1*10.0);
   //   printf("%d\n",s16);
   fwrite(&s16,2,1,fo);
 }


/*
%Rosenberg Pulse
%this function accepts fundamental frequency of the glottal signal and 
%the sampling frequency in hertz as input and returns one period of 
%the rosenberg pulse at the specified frequency.
%N2 is duty cycle of the pulse, from 0 to 1.
%N1 is the duration of the glottal opening as a fraction of the 
%total pulse, from 0 to 1.
function[gn]=rosenberg(N1,N2,f0,fs)
T=1/f0;     %period in seconds
pulselength=floor(T*fs);    %length of one period of pulse
%select N1 and N2 for duty cycle
N2=floor(pulselength*N2);
N1=floor(N1*N2);
gn=zeros(1,N2);
%calculate pulse samples
for n=1:N1-1
    gn(n)=0.5*(1-cos(pi*(n-1)/N1));
end
for n=N1:N2
    gn(n)=cos(pi*(n-N1)/(N2-N1)/2);
end
gn=[gn zeros(1,(pulselength-N2))];
*/

/*

t = (double)k*period/(double)dataLength;

if (t<te) {
LFcurrentSample = Eo*(exp(alpham*t)) * sin(wg*t);
//printf("wg %f alpha %f\n",wg, alpha);
}

if (t>=te) {
LFcurrentSample = -((Ee)/(epsilon*ta))*(exp(epsilon*(t-te)) - exp(-epsilon*(tc-te)));
}

if (t>tc) {
LFcurrentSample = 0.0;
}

// adding noise here?

if (noiseOn == TRUE) {
noiseSample = rand() % 200;
noiseSample = noiseSample - 100;
noiseSample = noiseSample/100;
noiseAdd = noiseSample*noiseAmount;

 noiseRemainder = (noiseStart + noiseDuration) - dataLength;

if (noiseStart + noiseDuration < dataLength) {
if (k >= noiseStart + noiseDuration){
LFcurrentSample1 = LFcurrentSample;
}
if (k < noiseStart) {
LFcurrentSample1 = LFcurrentSample;
}
if (k > noiseStart & k <= noiseStart +
noiseDuration) {
LFcurrentSample1 = LFcurrentSample+noiseAdd;
}
}
if (noiseStart + noiseDuration >= dataLength){
if (k > noiseRemainder && k < noiseStart) {
LFcurrentSample1 = LFcurrentSample;
}
if (k <= noiseRemainder || k >= noiseStart)
{
LFcurrentSample1 = LFcurrentSample+noiseAdd;
}
}
 }

if (noiseOn == FALSE) {
LFcurrentSample1 = LFcurrentSample;
}

*/
//printf("SAMPLE %f\n",LFcurrentSample);

// if (LFcurrentSample>32768.0) LFcurrentSample=32768.0;
// if (LFcurrentSample<-32768.0) LFcurrentSample=-32768.0;

//   printf("%d\n",s16);
/* unsigned int s16=(unsigned int)(LFcurrentSample1*1.0);
 unsigned char c = (unsigned)s16 & 255;
 fwrite(&c, 1, 1, fo);
 c = ((unsigned)s16 / 256) & 255;
 fwrite(&c, 1, 1, fo);*/
 
 //k += 1;
  }


}

// This function calculates LF equation coefficients based on currently selected voice type
void createLFInput(void){
double f0;
int Fs;
int overSample;
double areaSum, area1, area2;
double optimumArea;
double epsilonTemp, epsilonDiff, epsilonOptimumDiff;
double tn,tb;

Fs = 32000;
f0 = 100.0;
overSample = 1000;
period = 1/f0;
Ee = 1.0;
tc = _tcVal;
te = _teVal;
tppp = _tpVal;
ta = _taVal;

te = te + te*vocalTension;
tppp = tppp + tppp*vocalTension;
ta = ta - ta*vocalTension;

tc=tc*period;
te=te*period;
tppp=tppp*period;
ta=ta*period;

//These if statements prevent timing parameter values from going outside of their range with respect to other values.
if (te <= tppp) {
te = tppp + tppp*0.01;
}
if (te >= (tc-ta)){
te = tc-ta - (tc-ta)*0.01;
 }

// over sample values (can omit this section if it impacts real time operation)
period = period/overSample;
tc = tc/overSample;
te = te/overSample;
tppp = tppp/overSample;
ta = ta/overSample;
// dependant timing parameters
tn = te - tppp;
tb = tc - te;
// angular frequency of sinusoid section
wg = MY_PI/tppp;
// maximum negative peak value
Eo = Ee;
// epsilon and alpha equation coefficients

/*
areaSum = 1.0;
optimumArea = 1e-14;
epsilonDiff = 10000.0;
epsilonOptimumDiff = 0.1;
epsilonTemp = 1/ta;
// solve iteratively for epsilon
while (abs(epsilonDiff)>epsilonOptimumDiff) {
epsilon = (1/ta)*(1-exp(-epsilonTemp*tb));
epsilonDiff = epsilon - epsilonTemp;
epsilonTemp = epsilon;
if (epsilonDiff<0) {
epsilonTemp = epsilonTemp + (abs(epsilonDiff)/100);
}
if (epsilonDiff>0) {
epsilonTemp = epsilonTemp - (abs(epsilonDiff)/100);
}
}

//printf("Epislon %f\n", epsilon);

// iterate through area balance to get Eo and alpha to give area1 + area2 = 0
//while ((areaSum< -optimumArea)||(areaSum>optimumArea)) {

while (areaSum > optimumArea){

  //printf("Ee %f Eo %f wg %f te %f Areasum %f\n",Ee,Eo,wg,te,area2);

alpham = ( clog(-Ee/(Eo*sin(wg*te))))/te; // problem is here! NaN for log of negative
//alpha = ((-Ee/(Eo*sin(wg*te))))/te;

//printf("al %f xx %f xx %f\n",alpha, te,sin(wg*te) );

area1 = ( Eo*exp(alpham*te)/(sqrt(alpham*alpham+wg*wg))) * (sin(wg*te-atan(wg/alpham))) + (Eo*wg/(alpham*alpham+wg*wg));

area2 = ( -(Ee)/(epsilon*epsilon*(ta))) * (1 - exp(-epsilon*tb*(1+epsilon*tb)));
areaSum = area1 + area2;


if (areaSum>0.0) {
Eo = Eo - 1e5*areaSum;
}
if (areaSum<0.0) {
Eo = Eo + 1e5*areaSum;
}
}
*/

// alpham=938077.0;
// epsilon=12500000.0;

 alpham=305670.0;
 epsilon=3355800.0;


//calculate length of waveform in samples
//if (_pitchSlide == FALSE) {
dataLength = floor(overSample*Fs*period);
//}
//else{
//dataLength = _dataLength;
//}
//calculate length of waveform in samples without overSample
//
//dataLength = floor(Fs*period);
// pass all variables needed for waveform calculation to effectState

 printf("period=%f, length= %d, alpham = %f epsilon= %f \n", period, dataLength, alpham,epsilon);

//printf("period=%f, length= %d, tc = %f, te = %f, tp = %f, ta = %f wg = %f \n", period, dataLength, tc, te, tp,ta, wg);

 }
