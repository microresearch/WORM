// license:GPL-2.0+
// copyright-holders: Martin Howse

#include "audio.h"
#include "stdio.h"
#include <math.h>
#include "wavetable.h"

#include "resources.h"
extern float _selx, _sely, _selz;
Wavetable wavtable;
Wavetable *wavetable;


#define TWO_PI 6.28318530717958647693

#define OVERSAMPLING_OSCILLATOR 0

#define FIR_BETA                  .2
#define FIR_GAMMA                 .1
#define FIR_CUTOFF                .00000001

// TODO: FIR filter for above = use coeffs JUST for this!// and then delete doble to float etc

static const float filtertaps[49]={ 0.00000001f,  0.00000007f,  0.00000021f, -0.00000007f, -0.00000243f, -0.00000627f,  0.00000224f,  0.00004326f,  0.00006970f, -0.00008697f, -0.00043244f, -0.00028644f,  0.00113161f,  0.00232968f, -0.00061607f, -0.00709116f, -0.00592280f,  0.01120457f,  0.02472395f, -0.00136324f, -0.05604421f, -0.05124764f,  0.08785309f,  0.29650419f,  0.39847428f,  0.29650419f,  0.08785309f, -0.05124764f, -0.05604421f, -0.00136324f,  0.02472395f,  0.01120457f, -0.00592280f, -0.00709116f, -0.00061607f,  0.00232968f,  0.00113161f, -0.00028644f, -0.00043244f, -0.00008697f,  0.00006970f,  0.00004326f,  0.00000224f, -0.00000627f, -0.00000243f, -0.00000007f,  0.00000021f,  0.00000007f,  0.00000001f}; // filter coeffs for above settings

//extern __IO uint16_t adc_buffer[5];


////////////SNIPPED filtercalcs -> gentable.c

inline int iincrement(int pointer, int modulus)
{
    if (++pointer >= modulus)
	return 0;

    return pointer;
}

inline int ddecrement(int pointer, int modulus)
{
    if (--pointer < 0)
return modulus - 1;

    return pointer;
}


float doFIRFilter(TRMFIRFilter *filter, float input, u8 needOutput)
{
        if (needOutput) {
	float output = 0.0f;

	filter->FIRData[filter->FIRPtr] = input;

	for (u8 i = 0; i < filter->numberTaps; i++) {
	  output += filter->FIRData[filter->FIRPtr] * filter->FIRCoef[i];
	  //output=input;
	  filter->FIRPtr = iincrement(filter->FIRPtr, filter->numberTaps);
	}

	filter->FIRPtr = ddecrement(filter->FIRPtr, filter->numberTaps);
	return output;
    } else {
	  filter->FIRData[filter->FIRPtr] = input;
	  filter->FIRPtr = ddecrement(filter->FIRPtr, filter->numberTaps);
	return 0.0f;
	}
}

//////////////////////////////


inline static float mod0(float value, int16_t length)
{
    while (value > length-1)
        value -= length;
    return value;
}

// Increments the position in the wavetable according to the desired frequency. WORMY THIS
inline static void WavetableIncrementPosition(Wavetable *wavetable, float frequency)
{
wavetable->currentPosition = mod0(wavetable->currentPosition + (frequency * wavetable->basicIncrement), wavetable->length);
}

#ifndef TESTING
//extern wormy myworm;

/*inline static void WORMWavetableIncrementPosition(Wavetable *wavetable, float frequency)
{
  //    wavetable->currentPosition = mod0(wavetable->currentPosition + (frequency * wavetable->basicIncrement));
    float speed=(float)adc_buffer[SELX]/40960.0f; 
    u8 param=adc_buffer[SELY]>>6; 
  float wm=wormonefloat(&myworm, speed, param, (float)wavetable->length);
  wavetable->currentPosition = wm;//mod0(wm,wavetable->length);
  }

void dowormwavetable(float* outgoing, Wavetable *wavetable, float frequency, u8 length)  //  Plain oscillator
{
    int lowerPosition, upperPosition;
    for (int ii = 0; ii < length; ii++) {

    //  First increment the table position, depending on frequency
    WORMWavetableIncrementPosition(wavetable, frequency);

    //  Find surrounding integer table positions
    lowerPosition = (int)wavetable->currentPosition;
    upperPosition = mod0(lowerPosition + 1, wavetable->length);

    //  Return interpolated table value
    float sample= (wavetable->wavetable[lowerPosition] +
            ((wavetable->currentPosition - lowerPosition) *
             (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

    outgoing[ii]=sample;
    }
}
*/

#endif

#if OVERSAMPLING_OSCILLATOR
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, u8 length)  //  2X oversampling oscillator
{
  int lowerPosition, upperPosition;
    float interpolatedValue, sample;

    for (u8 ii = 0; ii < length; ii++) {
    for (u8 iii = 0; iii < 2; iii++) {
        //  First increment the table position, depending on frequency
        WavetableIncrementPosition(wavetable, frequency / 2.0f);

        //  Find surrounding integer table positions
        lowerPosition = (int)wavetable->currentPosition;
	upperPosition = mod0(lowerPosition + 1, wavetable->length);

        //  Calculate interpolated table value
        interpolatedValue = (wavetable->wavetable[lowerPosition] +
                             ((wavetable->currentPosition - lowerPosition) *
                              (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

        //  Put value through FIR filter
			sample = doFIRFilter(wavetable->FIRFilter, interpolatedValue, iii);
	//		sample=interpolatedValue;
    }
    outgoing[ii]=sample;
    //  Since we decimate, take only the second output value
    }
}
#else
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, u8 length)  //  Plain oscillator
{
    int lowerPosition, upperPosition;
    for (u8 ii = 0; ii < length; ii++) {

    //  First increment the table position, depending on frequency
    WavetableIncrementPosition(wavetable, frequency);

    //  Find surrounding integer table positions
    lowerPosition = (int)wavetable->currentPosition;
    upperPosition = mod0(lowerPosition + 1, wavetable->length);

    //  Return interpolated table value
    float sample= (wavetable->wavetable[lowerPosition] +
            ((wavetable->currentPosition - lowerPosition) *
             (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

    outgoing[ii]=sample;
    }
}
#endif

void wave_newsay(void){
  wavetable=&wavtable;
  wavetable->currentPosition=0.0;
}
  
int16_t wave_get_sample(void) // what is the wavetable - newsay
{
    int lowerPosition, upperPosition;
    // set frequency
      uint16_t val=_selz*1028.0f; // how can we test all others????
      MAXED(val,1023);
      val=1023-val;

    WavetableIncrementPosition(wavetable, 2.0f+(logspeed[val]*440.0f));
    lowerPosition = (int)wavetable->currentPosition;
    upperPosition = mod0(lowerPosition + 1, wavetable->length);

    float sample= (wavetable->wavetable[lowerPosition] +
            ((wavetable->currentPosition - lowerPosition) *
             (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));
    // float to int
    int16_t tmp = sample * 32768.0f;
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    return tmp;
}

float dosinglewavetable(Wavetable *wavetable, float frequency) 
{
    int lowerPosition, upperPosition;

    //  First increment the table position, depending on frequency
    WavetableIncrementPosition(wavetable, frequency);

    //  Find surrounding integer table positions
    lowerPosition = (int)wavetable->currentPosition;
    upperPosition = mod0(lowerPosition + 1, wavetable->length);

    //  Return interpolated table value
    float sample= (wavetable->wavetable[lowerPosition] +
            ((wavetable->currentPosition - lowerPosition) *
             (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

    return sample;
}

TRMFIRFilter firfilt;


void wavetable_init(Wavetable* wavtable, const float *tableitself, int16_t length){ // need to declare wavetable struct and ourtable we use
  wavtable->wavetable=tableitself;
  wavtable->basicIncrement=(float)length/32000.0f; // 32000 for real thing - test lap is 8000
  wavtable->currentPosition=0.0;
  wavtable->length=length;
  //  wavtable.FIRFilter = TRMFIRFilterCreate(FIR_BETA, FIR_GAMMA, FIR_CUTOFF);
  firfilt.FIRCoef=filtertaps;
  firfilt.numberTaps=49; firfilt.FIRPtr=0;
  wavtable->FIRFilter=&firfilt;
  wave_newsay();
}

/*
void main(){
  //  float ourtable[512]={1.0}; // generate tabel
  Wavetable wavtable; float out[128];
  wavtable.wavetable=ourtable;
  wavtable.basicIncrement=(float)TABLE_LENGTH/8000.0f; // 32000 for real thing - test lap is 8000
  wavtable.currentPosition=0.0;
  //  wavtable.FIRFilter = TRMFIRFilterCreate(FIR_BETA, FIR_GAMMA, FIR_CUTOFF);
  TRMFIRFilter firfilt;
  firfilt.FIRCoef=filtertaps;
  firfilt.numberTaps=49; firfilt.FIRPtr=0;
  wavtable.FIRFilter=&firfilt;

  while(1){
  WavetableOscillator(out, &wavtable, 50.0f, 128);  //  Plain oscillator

  for (int x=0;x<128;x++){
    printf("%c",(char)(out[x]*128.0f));
  }
  }
}
*/
