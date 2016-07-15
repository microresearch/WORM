#include "audio.h"
#include "svf.h"
#include "effect.h"
//#include "biquad.h" // do we use biquad or svf? - warps: 20 third-octave 48dB filters // biquad is 12db!!

// or we can cascase 2 biquads for 24 db/octave 

// or don't change bandwidth

// or from https://github.com/istarc/stm32/blob/master/STM32F4-Discovery_FW_V1.1.0/Libraries/CMSIS/DSP_Lib/Examples/arm_graphic_equalizer_example/arm_graphic_equalizer_example_q31.c

extern __IO uint16_t adc_buffer[10];

// IDEALLY: 16 channel vocoder with variable bandwidth on each filter follows some of warps. cross patching of channels...

/* what are the frequencies?

- from buchla 296: <100, 150, 250, 350, 500, 630, 800, 1k, 1.3k, 1.6k, 2k, 2.6k, 3.5k, 5k, 8k, >10k

- ems 2000: 140, 185, 270, 367, 444, 539, 653, 791, 958, 1161, 1406, 1703, 2064, 2700, 4000, 5388 - 30 db/octave

- ems widths=  

*/

// voice // carrier

// 16 bandpass set up for each = 32 // frequency, bandpass itself

// lowest is low pass, highest is high pass for inits

typedef struct Band{
  float gain;
  float frequency;
  float bw;
  //  biquad filterit;
  SVF svf[2]; // 2 passes
  float samples[32];
} Band; // each band

typedef struct Filterbank{  
  Band band_[17];
} Filterbank;

static Filterbank modulator_filter_bank_;
static Filterbank carrier_filter_bank_;

// gain for each band above or here ?

typedef struct BandGain {
  float carrier;
  float vocoder;
} BandGain;
  
static BandGain previous_gain_[16];
static BandGain gain_[16];

// env follower for 16 of voice

typedef struct EnvelopeFollower {  
float attack_;
float decay_;
float envelope_;
float peak_;
float freeze_;
} EnvelopeFollower;

EnvelopeFollower follower_[16];

////
