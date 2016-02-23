// LF

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

// Get LF waveform coefficients
double t;
double LFcurrentSample;
double LFcurrentSample1;
float period; //= fxs->_period;
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

// vocal fry voice type NOT
double _tcVal = 1.0;
double _teVal = 0.780;
double _tpVal = 0.6;
double _taVal = 0.028;
double vocalTension = 0.0;


void createLFInput(void);

void generateLF(void){
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

createLFInput();

// Main LF-waveform calculation performed here: IN LOOP!

for (x=0;x<32000;x++){

if (k>dataLength) {
k = k - dataLength;
}


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

//printf("SAMPLE %f\n",LFcurrentSample);

k += 1;


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
f0 = 1.0;
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
/*period = period/overSample;
tc = tc/overSample;
te = te/overSample;
tp = tp/overSample;
ta = ta/overSample;*/
// dependant timing parameters
tn = te - tppp;
tb = tc - te;
// angular frequency of sinusoid section
wg = MY_PI/tppp;
// maximum negative peak value
Eo = Ee;
// epsilon and alpha equation coefficients
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


//calculate length of waveform in samples
//if (_pitchSlide == FALSE) {
//dataLength = floor(overSample*Fs*period);
//}
//else{
//dataLength = _dataLength;
//}
//calculate length of waveform in samples without overSample
//
dataLength = floor(Fs*period);
// pass all variables needed for waveform calculation to effectState

//printf("period=%f, length= %d, tc = %f, te = %f, tp = %f, ta = %f wg = %f \n", period, dataLength, tc, te, tp,ta, wg);

 }
