// port of elements of warps vocoder

#include "audio.h"
#include "wfilterbank.h"
#include "wvocoder.h"

extern __IO uint16_t adc_buffer[10];

#define CONSTRAIN(var, min, max) \
  if (var < (min)) { \
    var = (min); \
  } else if (var > (max)) { \
    var = (max); \
  }

#define false 0
typedef u8 bool;

static float kFollowerGain; 
const u8 kNumBands=16;

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

typedef struct BandGain {
  float carrier;
  float vocoder;
} BandGain;

static float release_time_;
static   float formant_shift_;
  
static BandGain previous_gain_[16];
static BandGain gain_[16];

static float tmp_[32]; // for some reason can't be const variables

static Filterbank modulator_filter_bank_;
static Filterbank carrier_filter_bank_;
//static Limiter limiter_;

EnvelopeFollower follower_[16];
  
void Vocoder_set_release_time(float release_time) {
  release_time_ = release_time;
}

void Vocoder_set_formant_shift(float formant_shift) {
  formant_shift_ = formant_shift;
}

//// limiter

/*
class Limiter {
 public:
  Limiter() { }
  ~Limiter() { }

  void Init() {
    peak_ = 0.5f;
  }

  void Process(
      float* in_out,
      float pre_gain,
      size_t size) {
    while (size--) {
      float s = *in_out * pre_gain;
      SLOPE(peak_, fabs(s), 0.05f, 0.00002f);
      float gain = (peak_ <= 1.0f ? 1.0f : 1.0f / peak_);
      *in_out++ = stmlib::SoftLimit(s * gain * 0.8f);
    }
  }

 private:
  float peak_;
*/

/// vocoder.c break down:

// TODO - *******filter banks*, limiter?for later? -> all compiles but neeed finish wfilterbank and add limiter if necessary
// left all decimation out for now... but means need calc new coeffs

void Vocoder_Init(float sample_rate) {
  FilterBank_Init(&carrier_filter_bank_,sample_rate);
  FilterBank_Init(&modulator_filter_bank_, sample_rate);
//  limiter_.Init();
  u8 x;
  release_time_ = 0.5f;
  formant_shift_ = 0.5f;
  
  for (x=0;x<kNumBands;x++){
    previous_gain_[x].carrier=0.0f;
    previous_gain_[x].vocoder=0.0f;
    gain_[x].carrier=0.0f;
    gain_[x].vocoder=0.0f;
  }
//  fill(&previous_gain_[0], &previous_gain_[kNumBands], zero);
//  fill(&gain_[0], &gain_[kNumBands], zero);
  
  for (u8 i = 0; i < kNumBands; ++i) {
    EnvF_Init(&follower_[i]);
  }
}

void Vocoder_Process(
    const float* modulator,
    const float* carrier,
    float* out,
    u8 size) {
  // Run through filter banks.
    FilterBank_Analyze(&modulator_filter_bank_, modulator, size);
    FilterBank_Analyze(&carrier_filter_bank_, carrier, size);
  
  // Set the attack/release release_time of envelope followers.
  //float f = 80.0f * SemitonesToRatio(-72.0f * release_time_); // what kind of figures come out here?
    float f=80.0f;
  for (int32_t i = 0; i < kNumBands; ++i) {
    float decay = f / modulator_filter_bank_.band_[i].sample_rate;
    EnvF_set_attack(&follower_[i],decay * 2.0f);
    EnvF_set_decay(&follower_[i],decay * 0.5f);
    EnvF_set_freeze(&follower_[i],release_time_ > 0.995f);
    //    f *= 1.2599f;  // 2 ** (4/12.0), a third octave.
  }
  
  // Compute the amplitude (or modulation amount) in all bands.
  formant_shift_=adc_buffer[SELY]/4096.0f;
  float formant_shift_amount = 2.0f * fabsf(formant_shift_ - 0.5f);
  formant_shift_amount *= (2.0f - formant_shift_amount);
  formant_shift_amount *= (2.0f - formant_shift_amount);
//  float envelope_increment = 4.0f * SemitonesToRatio(-48.0f * formant_shift_);
  float envelope_increment = 4.0f;
  float envelope = 0.0f;
  const float kLastBand = kNumBands - 1.0001f;
  for (u8 i = 0; i < kNumBands; ++i) {
    float source_band = envelope;
    CONSTRAIN(source_band, 0.0f, kLastBand);
    //    MAKE_INTEGRAL_FRACTIONAL(source_band); // just the INTEGER part! as _integral below!
    //  int32_t x ## _integral = static_cast<int32_t>(x);		\
    //  float x ## _fractional = x - static_cast<float>(x ## _integral);
    int32_t source_band_integral= (int32_t) source_band;
    float source_band_fractional= source_band - (float)source_band_integral;

    float a = EnvF_peak(&follower_[source_band_integral]);
    float b = EnvF_peak(&follower_[source_band_integral + 1]);
    float band_gain = (a + (b - a) * source_band_fractional); // fractional part
    float attenuation = envelope - kLastBand;
    if (attenuation >= 0.0f) {
      band_gain *= 1.0f / (1.0f + 1.0f * attenuation);
    }
    envelope += envelope_increment;

    gain_[i].carrier = band_gain * formant_shift_amount;
    gain_[i].vocoder = 1.0f - formant_shift_amount;
  }
    
  for (u8 x=0;x<32;x++) out[x]=0.0f;

    
  for (u8 i = 0; i < kNumBands; ++i) {
    u8 band_size = size;// / modulator_filter_bank_.band_[i].decimation_factor;
    const float step = 1.0f / (float)(band_size);

    float* carrierx = carrier_filter_bank_.band_[i].samples;
    float* modulatorx = modulator_filter_bank_.band_[i].samples;
    float* envelopex = tmp_;  // ??????

    EnvF_Process(&follower_[i], modulatorx, envelopex, band_size);
    
    float vocoder_gain = previous_gain_[i].vocoder;
    float vocoder_gain_increment = (gain_[i].vocoder - vocoder_gain) * step;
    float carrier_gain = previous_gain_[i].carrier;
    float carrier_gain_increment = (gain_[i].carrier - carrier_gain) * step;
    for (u8 j = 0; j < band_size; ++j) {
      carrierx[j] *= (carrier_gain + vocoder_gain * envelopex[j]);
      vocoder_gain += vocoder_gain_increment;
      carrier_gain += carrier_gain_increment;
      //      if (i==0) out[j]=carrierx[j];
      out[j]+=carrierx[j];
  }
        previous_gain_[i] = gain_[i];
  }
}  

