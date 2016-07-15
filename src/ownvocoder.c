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

*/

// from svf.c

static float coefficients[]={-0.0282793973573, 0.00959297446756, -0.0163167758833, 0.0230064882424, -0.0393651013703, 0.00641386905172, -0.0334160506508, 0.00544619053794, -0.0574054032856, 0.00934663014085, -0.0487363845568, 0.00793888326107, -0.0779545431939, 0.0126823781741, -0.0661927558613, 0.0107763298717, -0.0942366298986, 0.015321870747, -0.0800287301336, 0.013023431221, -0.114286604369, 0.0185679468016, -0.097072311225, 0.0157895175849, -0.138288264423, 0.0224478486855, -0.117484032941, 0.0190998615243, -0.167253501531, 0.0271218210932, -0.14213102646, 0.023094469639, -0.202167149801, 0.0327441793954, -0.171861964199, 0.027910744163, -0.244389527147, 0.0395277773228, -0.207852602085, 0.0337403624366, -0.29500629759, 0.0476391521105, -0.251056229345, 0.0407420466369, -0.355824872964, 0.0573583693292, -0.303060665772, 0.0491839233527, -0.428878649093, 0.0689993382983, -0.365679453184, 0.0593844544377, -0.55499261433, 0.0890290943739, -0.474238488247, 0.0772216105788, -0.800710156926, 0.127936456871, -0.687889474666, 0.113347474541, -0.883201326334, 0.283602362272, -0.98933441725, 0.859117052889}; // total 64 is 16*4 yes

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

const u8 kNumBands=16;
#define false 0
typedef u8 bool;

static float kFollowerGain; 

typedef struct EnvelopeFollower {  
float attack_;
float decay_;
float envelope_;
float peak_;
float freeze_;
} EnvelopeFollower;

void EnvF_Init(EnvelopeFollower* env) {
  kFollowerGain = sqrtf(kNumBands);
  env->envelope_ = 0.0f;
  env->freeze_ = false;
  env->attack_ = env->decay_ = 0.1f;
  env->peak_ = 0.0f;
};
  
void EnvF_set_attack(EnvelopeFollower* env, float attack) {
  env->attack_ = attack;
}
  
void EnvF_set_decay(EnvelopeFollower* env, float decay) {
  env->decay_ = decay;
}
  
void EnvF_set_freeze(EnvelopeFollower* env, bool freeze) {
  env->freeze_ = freeze;
}
  
void EnvF_Process(EnvelopeFollower* env, const float* in, float* out, size_t size) {
  float envelope = env->envelope_;
  float attack = env->freeze_ ? 0.0f : env->attack_;
  float decay = env->freeze_ ? 0.0f : env->decay_;
  float peak = 0.0f;
  while (size--) {
    float error = fabsf(*in++ * kFollowerGain) - envelope;
    envelope += (error > 0.0f ? attack : decay) * error;
    if (envelope > peak) {
      peak = envelope;
    }
    *out++ = envelope;
  }
  env->envelope_ = envelope;
  float error = peak - env->peak_;
  env->peak_ += (error > 0.0f ? 0.5f : 0.1f) * error;
}
  
inline float EnvF_peak(EnvelopeFollower* env) { return env->peak_; }

EnvelopeFollower follower_[16];

////

void VOCODER_init(){

  // init the svf stuff

for (u8 i = 0; i < 16; i++) {
for (u8 pass = 0; pass < 2; ++pass) {
  SVF_Init(&modulator_filter_bank_.band_[i].svf[pass]);
  SVF_Init(&carrier_filter_bank_.band_[i].svf[pass]);
  set_f_fq(&modulator_filter_bank_.band_[i].svf[pass],coefficients[(i*4)+(pass * 2)],coefficients[(i*4)+(pass * 2)+1]);
  set_f_fq(&carrier_filter_bank_.band_[i].svf[pass],coefficients[(i*4)+(pass * 2)],coefficients[(i*4)+(pass * 2)+1]);
    }
}

// init gains

  for (u8 x=0;x<16;x++){
    previous_gain_[x].carrier=0.0f;
    previous_gain_[x].vocoder=0.0f;
    gain_[x].carrier=0.0f;
    gain_[x].vocoder=0.0f;
  }

// init follower

  for (u8 i = 0; i < 16; ++i) {
    EnvF_Init(&follower_[i]);
  }


}

void runVOCODERtest_(float* voice, float* carrier, float* outgoing, u8 band_size){

  // set followers

  // adjust gain

  // filter and follow gains as in wvocoder - in that case lots is about skew of ...

  // maybe can re-do without up and down delay/ samples

}
