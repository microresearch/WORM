#include "audio.h"
#include "stdio.h"
#include <math.h>
//#include "forlap.h"
#include "worming.h"

// varying wavetable implementations: params - oversampling, interpolating... length and calc frequency?

// note that BRAIDs uses multiple indexes into giant wavetable

// from tubes:

#define TWO_PI 6.28318530717958647693

#define OVERSAMPLING_OSCILLATOR 0

#define FIR_BETA                  .2
#define FIR_GAMMA                 .1
#define FIR_CUTOFF                .00000001

// TODO: FIR filter for above = use coeffs JUST for this!// and then delete doble to float etc

float filtertaps[49]={ 0.00000001,  0.00000007,  0.00000021, -0.00000007, -0.00000243, -0.00000627,  0.00000224,  0.00004326,  0.00006970, -0.00008697, -0.00043244, -0.00028644,  0.00113161,  0.00232968, -0.00061607, -0.00709116, -0.00592280,  0.01120457,  0.02472395, -0.00136324, -0.05604421, -0.05124764,  0.08785309,  0.29650419,  0.39847428,  0.29650419,  0.08785309, -0.05124764, -0.05604421, -0.00136324,  0.02472395,  0.01120457, -0.00592280, -0.00709116, -0.00061607,  0.00232968,  0.00113161, -0.00028644, -0.00043244, -0.00008697,  0.00006970,  0.00004326,  0.00000224, -0.00000627, -0.00000243, -0.00000007,  0.00000021,  0.00000007,  0.00000001}; // filter coeffs for above settings

extern __IO uint16_t adc_buffer[10];

typedef struct {
    float FIRData[49], *FIRCoef;
    int FIRPtr, numberTaps;
} TRMFIRFilter;

typedef struct _Wavetable {
  TRMFIRFilter *FIRFilter;
    const float *wavetable;
    float basicIncrement;
    float currentPosition;
int16_t length;
} Wavetable;

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


float doFIRFilter(TRMFIRFilter *filter, float input, int needOutput)
{
    if (needOutput) {
	int i;
	float output = 0.0;

	/*  PUT INPUT SAMPLE INTO DATA BUFFER  */
	filter->FIRData[filter->FIRPtr] = input;

	/*  SUM THE OUTPUT FROM ALL FILTER TAPS  */
	for (i = 0; i < filter->numberTaps; i++) {
	    output += filter->FIRData[filter->FIRPtr] * filter->FIRCoef[i];
	    filter->FIRPtr = iincrement(filter->FIRPtr, filter->numberTaps);
	}

	/*  DECREMENT THE DATA POINTER READY FOR NEXT CALL  */
	filter->FIRPtr = ddecrement(filter->FIRPtr, filter->numberTaps);

	/*  RETURN THE OUTPUT VALUE  */
	return output;
    } else {
	/*  PUT INPUT SAMPLE INTO DATA BUFFER  */
	filter->FIRData[filter->FIRPtr] = input;

	/*  ADJUST THE DATA POINTER, READY FOR NEXT CALL  */
	filter->FIRPtr = ddecrement(filter->FIRPtr, filter->numberTaps);

	return 0.0;
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

extern wormy myworm;

inline static void WORMWavetableIncrementPosition(Wavetable *wavetable, float frequency)
{
  //    wavetable->currentPosition = mod0(wavetable->currentPosition + (frequency * wavetable->basicIncrement));
  float speed=(float)adc_buffer[SELX]/40960.0f; 
  u8 param=adc_buffer[SELY]>>6; 
  float wm=wormonefloat(&myworm, speed, param, (float)wavetable->length);
  wavetable->currentPosition = wm;//mod0(wm,wavetable->length);
}


#if OVERSAMPLING_OSCILLATOR
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, int16_t length)  //  2X oversampling oscillator
{
    int i, lowerPosition, upperPosition;
    float interpolatedValue, sample;

    for (int ii = 0; ii < length; ii++) {
    for (i = 0; i < 2; i++) {
        //  First increment the table position, depending on frequency
        WavetableIncrementPosition(wavetable, frequency / 2.0);

        //  Find surrounding integer table positions
        lowerPosition = (int)wavetable->currentPosition;
upperPosition = mod0(lowerPosition + 1, , wavetable->length);

        //  Calculate interpolated table value
        interpolatedValue = (wavetable->wavetable[lowerPosition] +
                             ((wavetable->currentPosition - lowerPosition) *
                              (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

        //  Put value through FIR filter
	sample = doFIRFilter(wavetable->FIRFilter, interpolatedValue, i);
    }
    outgoing[ii]=sample;
    //  Since we decimate, take only the second output value
    }
}
#else
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, int16_t length)  //  Plain oscillator
{
    int lowerPosition, upperPosition;
    for (int ii = 0; ii < length; ii++) {

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


void dowormwavetable(float* outgoing, Wavetable *wavetable, float frequency, int16_t length)  //  Plain oscillator
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


void wavetable_init(Wavetable* wavtable, const float *tableitself, int16_t length){ // need to declare wavetable struct and ourtable we use
  wavtable->wavetable=tableitself;
  wavtable->basicIncrement=(float)length/32000.0f; // 32000 for real thing - test lap is 8000
  wavtable->currentPosition=0.0;
  wavtable->length=length;
  //  wavtable.FIRFilter = TRMFIRFilterCreate(FIR_BETA, FIR_GAMMA, FIR_CUTOFF);
  TRMFIRFilter firfilt;
  firfilt.FIRCoef=filtertaps;
  firfilt.numberTaps=49; firfilt.FIRPtr=0;
  wavtable->FIRFilter=&firfilt;
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
