#include "arm_math.h"
#include "arm_const_structs.h"
#include "audio.h"
#include "sintable.h"

#define TWO_PI  6.283185

//formantfrequency=phonemeParameters[index][partial][0];
// formantgain= phonemeParameters[index][partial][2];
// formantradius= phonemeParameters[index][partial][1];
// noisegain= phonemeGains[index][1];
// voicegain= phonemeGains[index][0];

// SELX, SELY, 

/*
 PARAMETER HEADER:

   *excitation:*
pitchenv
voicegain=gainenv in set_frequency
noisegain, noiserate, randomgain
vibrato: vibratoGain0-1.0, vibratoRate???

   *filters*
formantfrequency -> multiplies
//formantgain
//formantradius (these from phoneme desc)

notes from glossolalia:
      voice.controlChange ( 4, offset + random() % 16 );= choose random phoneme of 16 so no noiseyyy?

      voice.setUnVoiced ( random() % 5 == 0 ? 0.2 : 0.0); = noiseEnv_.setTarget(nGain)
*/

typedef struct 
{
 unsigned char  dirty_;
  float frequency_;
   float radius_;
  float gain_;
 float startFrequency_;
 float startRadius_;
 float startGain_;
 float targetFrequency_;
 float targetRadius_;
 float targetGain_;
 float deltaFrequency_;
 float deltaRadius_;
 float deltaGain_;
 float sweepState_;
 float sweepRate_;
  float inputs_[3], outputs_[3];
  float a_[3],b_[3];
} filters_;

// voice/unvoiced - unvoiced need reducing DONE

const float phonemeGains[32][2]  __attribute__ ((section (".flash"))) =
  {{1.0, 0.0},    // eee
   {1.0, 0.0},    // ihh
   {1.0, 0.0},    // ehh
   {1.0, 0.0},    // aaa

   {1.0, 0.0},    // ahh
   {1.0, 0.0},    // aww
   {1.0, 0.0},    // ohh
   {1.0, 0.0},    // uhh

   {1.0, 0.0},    // uuu
   {1.0, 0.0},    // ooo
   {1.0, 0.0},    // rrr
   {1.0, 0.0},    // lll

   {1.0, 0.0},    // mmm
   {1.0, 0.0},    // nnn
   {1.0, 0.0},    // nng
   {1.0, 0.0},    // ngg

   {0.0, 0.4},    // fff
   {0.0, 0.4},    // sss
   {0.0, 0.4},    // thh
   {0.0, 0.4},    // shh

   {0.0, 0.4},    // xxx
   {0.0, 0.1},    // hee
   {0.0, 0.1},    // hoo
   {0.0, 0.1},    // hah

   {1.0, 0.1},    // bbb
   {1.0, 0.1},    // ddd
   {1.0, 0.1},    // jjj
   {1.0, 0.1},    // ggg

   {1.0, 0.4},    // vvv
   {1.0, 0.4},    // zzz
   {1.0, 0.4},    // thz
   {1.0, 0.4}     // zhh
  };

const float phonemeParameters[32][4][3]  __attribute__ ((section (".flash"))) =
  {{  { 273, 0.996,  10},       // eee (beet)
      {2086, 0.945, -16}, 
      {2754, 0.979, -12}, 
      {3270, 0.440, -17}},
   {  { 385, 0.987,  10},       // ihh (bit)
      {2056, 0.930, -20},
      {2587, 0.890, -20}, 
      {3150, 0.400, -20}},
   {  { 515, 0.977,  10},       // ehh (bet)
      {1805, 0.810, -10}, 
      {2526, 0.875, -10}, 
      {3103, 0.400, -13}},
   {  { 773, 0.950,  10},       // aaa (bat)
      {1676, 0.830,  -6},
      {2380, 0.880, -20}, 
      {3027, 0.600, -20}},
     
   {  { 770, 0.950,   0},       // ahh (father)
      {1153, 0.970,  -9},
      {2450, 0.780, -29},
      {3140, 0.800, -39}},
   {  { 637, 0.910,   0},       // aww (bought)
      { 895, 0.900,  -3},
      {2556, 0.950, -17},
      {3070, 0.910, -20}},
   {  { 637, 0.910,   0},       // ohh (bone)  NOTE::  same as aww (bought)
      { 895, 0.900,  -3},
      {2556, 0.950, -17},
      {3070, 0.910, -20}},
   {  { 561, 0.965,   0},       // uhh (but)
      {1084, 0.930, -10}, 
      {2541, 0.930, -15}, 
      {3345, 0.900, -20}},
    
   {  { 515, 0.976,   0},       // uuu (foot)
      {1031, 0.950,  -3},
      {2572, 0.960, -11},
      {3345, 0.960, -20}},
   {  { 349, 0.986, -10},       // ooo (boot)
      { 918, 0.940, -20},
      {2350, 0.960, -27},
      {2731, 0.950, -33}},
   {  { 394, 0.959, -10},       // rrr (bird)
      {1297, 0.780, -16},
      {1441, 0.980, -16},
      {2754, 0.950, -40}},
   {  { 462, 0.990,  +5},       // lll (lull)
      {1200, 0.640, -10},
      {2500, 0.200, -20},
      {3000, 0.100, -30}},
     
   {  { 265, 0.987, -10},       // mmm (mom)
      {1176, 0.940, -22},
      {2352, 0.970, -20},
      {3277, 0.940, -31}},
   {  { 204, 0.980, -10},       // nnn (nun)
      {1570, 0.940, -15},
      {2481, 0.980, -12},
      {3133, 0.800, -30}},
   {  { 204, 0.980, -10},       // nng (sang)    NOTE:: same as nnn
      {1570, 0.940, -15},
      {2481, 0.980, -12},
      {3133, 0.800, -30}},
   {  { 204, 0.980, -10},       // ngg (bong)    NOTE:: same as nnn
      {1570, 0.940, -15},
      {2481, 0.980, -12},
      {3133, 0.800, -30}},
     
   {  {1000, 0.300,   0},       // fff
      {2800, 0.860, -10},
      {7425, 0.740,   0},
      {8140, 0.860,   0}},
   {  {0,    0.000,   0},       // sss
      {2000, 0.700, -15},
      {5257, 0.750,  -3}, 
      {7171, 0.840,   0}},
   {  { 100, 0.900,   0},       // thh
      {4000, 0.500, -20},
      {5500, 0.500, -15},
      {8000, 0.400, -20}},
   {  {2693, 0.940,   0},       // shh
      {4000, 0.720, -10},
      {6123, 0.870, -10},
      {7755, 0.750, -18}},

   {  {1000, 0.300, -10},       // xxx           NOTE:: Not Really Done Yet
      {2800, 0.860, -10},
      {7425, 0.740,   0},
      {8140, 0.860,   0}},
   {  { 273, 0.996, -40},       // hee (beet)    (noisy eee)
      {2086, 0.945, -16}, 
      {2754, 0.979, -12}, 
      {3270, 0.440, -17}},
   {  { 349, 0.986, -40},       // hoo (boot)    (noisy ooo)
      { 918, 0.940, -10},
      {2350, 0.960, -17},
      {2731, 0.950, -23}},
   {  { 770, 0.950, -40},       // hah (father)  (noisy ahh)
      {1153, 0.970,  -3},
      {2450, 0.780, -20},
      {3140, 0.800, -32}},
     
   {  {2000, 0.700, -20},       // bbb           NOTE:: Not Really Done Yet
      {5257, 0.750, -15},
      {7171, 0.840,  -3}, 
      {9000, 0.900,   0}},
   {  { 100, 0.900,   0},       // ddd           NOTE:: Not Really Done Yet
      {4000, 0.500, -20},
      {5500, 0.500, -15},
      {8000, 0.400, -20}},
   {  {2693, 0.940,   0},       // jjj           NOTE:: Not Really Done Yet
      {4000, 0.720, -10},
      {6123, 0.870, -10},
      {7755, 0.750, -18}},
   {  {2693, 0.940,   0},       // ggg           NOTE:: Not Really Done Yet
      {4000, 0.720, -10},
      {6123, 0.870, -10},
      {7755, 0.750, -18}},
     
   {  {2000, 0.700, -20},       // vvv           NOTE:: Not Really Done Yet
      {5257, 0.750, -15},
      {7171, 0.840,  -3}, 
      {9000, 0.900,   0}},
   {  { 100, 0.900,   0},       // zzz           NOTE:: Not Really Done Yet
      {4000, 0.500, -20},
      {5500, 0.500, -15},
      {8000, 0.400, -20}},
   {  {2693, 0.940,   0},       // thz           NOTE:: Not Really Done Yet
      {4000, 0.720, -10},
      {6123, 0.870, -10},
      {7755, 0.750, -18}},
   {  {2693, 0.940,   0},       // zhh           NOTE:: Not Really Done Yet
      {4000, 0.720, -10},
      {6123, 0.870, -10},
      {7755, 0.750, -18}}
  };

//TODO: onepole, onezero, envelope, noise, incoming is voiced or excitation, filters...

// onepole

static float outputs, outputsx;
static float inputs;
static float b_, a_, b_1, b_0, bx_, ax_;

float doonepole(float sample){
//  inputs_[0] = gain_ * input;
  float lastFrame_ = b_ * sample - a_ * outputs;
  outputs = lastFrame_;
  return lastFrame_;
}

float doonepolex(float sample, float gain){
  sample = gain * sample;
  float lastFrame_ = bx_ * sample - ax_ * outputsx;
  outputsx = lastFrame_;
  return lastFrame_;
}


// onezero

//inputs_[0] = gain_ * input;
float doonezero(float sample){
  //  lastFrame_[0] = b_[1] * inputs_[1] + b_[0] * inputs_[0];
  float lastFrame_ = b_1 * inputs + b_0 * sample;
  inputs = sample;
  return lastFrame_;
}

// noiseEnv=envelope 

//  noiseEnv_.setRate( 0.001 );
//  noiseEnv_.setTarget( 0.0 );

static unsigned char state_=1, state_P=1, state_e=1;
static float target_=0.0, rate_=0.001;
static float target_P=0.0, rate_P=0.001;
static float target_e=0.0, rate_e=0.001;

float doenvelope(){
static float value_=0.0;
  if ( state_ ) {
    if ( target_ > value_ ) {
      value_ += rate_;
      if ( value_ >= target_ ) {
        value_ = target_;
        state_ = 0;
      }
    }
    else {
      value_ -= rate_;
      if ( value_ <= target_ ) {
        value_ = target_;
        state_ = 0;
      }
    }
  }
    return value_;
}

float doenvelopeP(){
static float value_=0.0;
  if ( state_P ) {
    if ( target_P > value_ ) {
      value_ += rate_P;
      if ( value_ >= target_P ) {
        value_ = target_P;
        state_P = 0;
      }
    }
    else {
      value_ -= rate_P;
      if ( value_ <= target_P) {
        value_ = target_P;
        state_P = 0;
      }
    }
  }
    return value_;
}

float doenvelopee(){
static float value_=0.0;
  if ( state_e ) {
    if ( target_e > value_ ) {
      value_ += rate_e;
      if ( value_ >= target_e ) {
        value_ = target_e;
        state_e = 0;
      }
    }
    else {
      value_ -= rate_e;
      if ( value_ <= target_e) {
        value_ = target_e;
        state_e = 0;
      }
    }
  }
    return value_;
}


// noise

//*samples = (float) ( 2.0 * rand() / (RAND_MAX + 1.0) - 1.0 );

// filters

 // filters_[0].setTargets( Phonemes::formantFrequency(i, 0), Phonemes::formantRadius(i, 0), pow(10.0, Phonemes::formantGain(i, 0 ) / 20.0) );

    void setTargets(filters_ *filter, float frequency, float radius, float gain)
    {
      filter->startFrequency_ = filter->frequency_; // where we find these?
  filter->startRadius_ = filter->radius_;
  filter->startGain_ = filter->gain_;

  filter->dirty_ = 1;
  filter->targetFrequency_ = frequency;
  filter->targetRadius_ = radius;
  filter->targetGain_ = gain;

  // where are start states defined? - would be minus in next three

  filter->deltaFrequency_ = frequency - filter->frequency_;

  filter->deltaRadius_ = radius - filter->radius_;
  filter->deltaGain_ = gain - filter->gain_;
  filter->sweepState_ = 0.0;
  filter->sweepRate_ = 0.001;
}

    void setResonance(filters_ *filter, float frequency, float radius )
{
  float temp, sint;
  arm_sin_cos_f32(57.29578 * (TWO_PI * frequency / 32000), &sint, &temp); 

  filter->a_[2] = radius * radius;
  //    filter->a_[1] = -2.0 * radius * cosf( TWO_PI * frequency / 32000.0f ); // samplerate
  //    filter->a_[1] = -2.0 * radius * arm_cos_f32( TWO_PI * frequency / 32000.0f ); // samplerate   
    filter->a_[1] = -2.0 * radius * temp; // samplerate
  // Use zeros at +- 1 and normalize the filter peak gain.
  filter->b_[0] = 0.5 - 0.5 * filter->a_[2];
  filter->b_[1] = 0.0;
  filter->b_[2] = -filter->b_[0];
}


    float dofilter(filters_ *filter, float input){ // and in and out
      float radius_,frequency_,gain_;
       if ( filter->dirty_ )  {
    filter->sweepState_ += filter->sweepRate_;
    if ( filter->sweepState_ >= 1.0 )   {
      filter->sweepState_ = 1.0;
      filter->dirty_ = 0;
      filter->radius_ = filter->targetRadius_;
      filter->frequency_ = filter->targetFrequency_;
      filter->gain_ = filter->targetGain_;
    }
    else {
      filter->radius_ = filter->startRadius_ + (filter->deltaRadius_ * filter->sweepState_);
      filter->frequency_ = filter->startFrequency_ + (filter->deltaFrequency_ * filter->sweepState_);
      filter->gain_ = filter->startGain_ + (filter->deltaGain_ * filter->sweepState_);
    }
    setResonance(filter, filter->frequency_, filter->radius_ ); //??? and get a b etc?
    }
      /*      if (filter->dirty_){
    setResonance(filter, filter->targetFrequency_, filter->targetRadius_ ); //??? and get a b etc?
    }*/

  filter->inputs_[0] = filter->gain_ * input;
  float lastFrame_ = filter->b_[0] * filter->inputs_[0] + filter->b_[1] * filter->inputs_[1] + filter->b_[2] * filter->inputs_[2];
  lastFrame_ -= filter->a_[2] * filter->outputs_[2] + filter->a_[1] * filter->outputs_[1];
  filter->inputs_[2] = filter->inputs_[1];
  filter->inputs_[1] = filter->inputs_[0];
  filter->outputs_[2] = filter->outputs_[1];
  filter->outputs_[1] = lastFrame_;

  return lastFrame_;
    }

filters_ filters[4];
extern __IO uint16_t adc_buffer[10];

inline float donoise(){
float xx=(float) ( 2.0 * rand() / (RAND_MAX + 1.0) - 1.0 );
 return xx;
}


// 16 bits big-endian?

const float impuls20[]  __attribute__ ((section (".flash"))) ={0.99993896, 0.95727539, 0.83560181, 0.65289307, 0.43554688, 0.21380615, 0.016571045, -0.13317871, -0.22213745, -0.24795532, -0.21890259, -0.15139771, -0.066375732, 0.015106201, 0.075805664, 0.10519409, 0.10067749, 0.067352295, 0.016052246, -0.039550781, -0.086273193, -0.11416626, -0.11846924, -0.10021973, -0.065582275, -0.023956299, 0.014282227, 0.040405273, 0.049102783, 0.039459229, 0.014984131, -0.017578125, -0.049987793, -0.074523926, -0.085754395, -0.081756592, -0.064239502, -0.038146973, -0.010162354, 0.012908936, 0.025726318, 0.025695801, 0.013397217, -0.0076904297, -0.032104492, -0.053833008, -0.067687988, -0.070648193, -0.062408447, -0.045349121, -0.023925781, -0.0035095215, 0.010986328, 0.016235352, 0.011291504, -0.0023498535, -0.021118164, -0.040283203, -0.055145264, -0.062255859, -0.060028076, -0.049316406, -0.032928467, -0.015014648, 0, 0.0085449219, 0.0086669922, 0.00054931641, -0.01361084, -0.030273438, -0.045288086, -0.055023193, -0.057159424, -0.051391602, -0.039245605, -0.023834229, -0.008972168, 0.0016479492, 0.0055541992, 0.0018920898, -0.0082702637, -0.02243042, -0.036987305, -0.048339844, -0.05380249, -0.052093506, -0.043762207, -0.030883789, -0.01675415, -0.0048217773, 0.001953125, 0.0020446777, -0.004486084, -0.016052246, -0.029663086, -0.041992188, -0.049987793, -0.051757812, -0.046905518, -0.036682129, -0.023681641, -0.011108398, -0.0020751953, 0.0012207031, -0.001953125, -0.010803223, -0.023071289, -0.035736084, -0.045684814, -0.050445557, -0.04888916, -0.041442871, -0.029968262, -0.017303467, -0.0066223145, -0.00048828125, -0.00045776367, -0.0065002441, -0.017089844, -0.029602051, -0.040924072, -0.048278809, -0.049865723, -0.045288086, -0.035675049, -0.0234375, -0.01159668, -0.0030822754, 0, -0.0030822754, -0.01159668, -0.0234375, -0.035675049, -0.045288086, -0.049865723, -0.048278809, -0.040924072, -0.029602051, -0.017089844, -0.0065002441, -0.00045776367, -0.00048828125, -0.0066223145, -0.017303467, -0.029968262, -0.041442871, -0.04888916, -0.050445557, -0.045684814, -0.035736084, -0.023071289, -0.010803223, -0.001953125, 0.0012207031, -0.0020751953, -0.011108398, -0.023681641, -0.036682129, -0.046905518, -0.051757812, -0.049987793, -0.041992188, -0.029663086, -0.016052246, -0.004486084, 0.0020446777, 0.001953125, -0.0048217773, -0.01675415, -0.030883789, -0.043762207, -0.052093506, -0.05380249, -0.048339844, -0.036987305, -0.02243042, -0.0082702637, 0.0018920898, 0.0055541992, 0.0016479492, -0.008972168, -0.023834229, -0.039245605, -0.051391602, -0.057159424, -0.055023193, -0.045288086, -0.030273438, -0.01361084, 0.00054931641, 0.0086669922, 0.0085449219, 0, -0.015014648, -0.032928467, -0.049316406, -0.060028076, -0.062255859, -0.055145264, -0.040283203, -0.021118164, -0.0023498535, 0.011291504, 0.016235352, 0.010986328, -0.0035095215, -0.023925781, -0.045349121, -0.062408447, -0.070648193, -0.067687988, -0.053833008, -0.032104492, -0.0076904297, 0.013397217, 0.025695801, 0.025726318, 0.012908936, -0.010162354, -0.038146973, -0.064239502, -0.081756592, -0.085754395, -0.074523926, -0.049987793, -0.017578125, 0.014984131, 0.039459229, 0.049102783, 0.040405273, 0.014282227, -0.023956299, -0.065582275, -0.10021973, -0.11846924, -0.11416626, -0.086273193, -0.039550781, 0.016052246, 0.067352295, 0.10067749, 0.10519409, 0.075805664, 0.015106201, -0.066375732, -0.15139771, -0.21890259, -0.24795532, -0.22213745, -0.13317871, 0.016571045, 0.21380615, 0.43554688, 0.65289307, 0.83560181, 0.95727539};

//	modulator_.setVibratoRate( 6.0 );
//	modulator_.setVibratoGain( 0.04 );
//	modulator_.setRandomGain( 0.005 );
//  vibrato_.setFrequency( 6.0 );


float vibratoGain=0.1; 
float randomGain_=0.005;
unsigned int noiseRate_;
unsigned int noiseCounter_;

#define TABLE_SIZEE 2048

float vibratoRate=6.0;

float dovibrato(){
  vibratoRate=(float)adc_buffer[SELZ]/256.0;
  static float time_=0.0f;
  while ( time_ < 0.0 )
    time_ += TABLE_SIZEE;
  while ( time_ >= TABLE_SIZEE )
    time_ -= TABLE_SIZEE;

  uint16_t iIndex_ = (unsigned int) time_;
  float alpha_ = time_ - iIndex_;
  float tmp = sintables[ iIndex_ ];
  tmp += ( alpha_ * ( sintables[ iIndex_ + 1 ] - tmp ) );

  // Increment time, which can be negative.
  time_ += vibratoRate; // what is rate? - vibratoRATE?
  return tmp;
}

float domodulate(){
  float sample, noisey;
  sample = vibratoGain * dovibrato();
  if ( noiseCounter_++ >= noiseRate_ ) {
    noisey=donoise();
    noiseCounter_ = 0;
  }
  sample += doonepolex(noisey, randomGain_);
  return sample;
}


float dosingwave(){
  float sample;
  static float time_=0.0f;

  /*  
- for singwave we need:

  Modulate modulator_;

and for mod we need:   SineWave vibrato_;
Noise noise_;
OnePole  filter_;

StkFloat newRate = pitchEnvelope_.tick();
  newRate += newRate * modulator_.tick();
  wave_.setRate( newRate );

  lastFrame_[0] = wave_.tick();
  lastFrame_[0] *= envelope_.tick();
  */

  float newRate=doenvelopeP(); // SET in init
  // modulate newrate TODO
  newRate += newRate * domodulate();

  u8 interpolate_ = 0;
  if ( fmodf( newRate, 1.0 ) != 0.0 ) interpolate_ = 1;

  while ( time_ < 0.0 )
    time_ += 256;
  while ( time_ >= 256 )
    time_ -= 256;

  if ( interpolate_ ) {
    //    sample = dinterpolate(time_,rate_);
    // sample is at fractional part
    u8 indexx=(u8)time_; u8 nextindex=indexx+1;
    float fractional=time_-(float)indexx;
    sample=impuls20[indexx];
    if (fractional!=0.0) sample+=(fractional * (impuls20[nextindex] - sample)); //     output += ( alpha * ( data_[ iIndex + nChannels_ ] - output ) );
  }
  else {
    sample = impuls20[(u8)time_]; // simply skip
  }

  // Increment time, which can be negative.
  time_ += newRate;

  //////

  sample*=doenvelopee(); // SET in voicform
  return sample;
}

void set_frequency(float frequency, float amplitude);

void dovoicform(float* incoming, float *outgoing, unsigned char howmany){

  /*  VoicForm.h :::

      SingWave *voiced_;
  Noise    noise_;
  Envelope noiseEnv_;
  FormSwep filters_[4];
  OnePole  onepole_;
  OneZero  onezero_;
  temp = onepole_.tick( onezero_.tick( voiced_->tick() ) );
  temp += noiseEnv_.tick() * noise_.tick();
  lastFrame_[0] = filters_[0].tick(temp);
  lastFrame_[0] += filters_[1].tick(temp);
  lastFrame_[0] += filters_[2].tick(temp);
  lastFrame_[0] += filters_[3].tick(temp);*/


  // set other settings defined below - eg. gains on voiced and noiseEnv and change vibrato and onepole
  // test first all set first...
  // voiced is incoming or excitation
  static u8 oldindex;
  unsigned char index=adc_buffer[SELY]>>7; // which bphoneme? of 32
  if (index!=oldindex){ // also can multiply freq
  setTargets(&filters[0],phonemeParameters[index][0][0],phonemeParameters[index][0][1], powf(10.0,phonemeParameters[index][0][2] / 20.0) );
  setTargets(&filters[1],phonemeParameters[index][1][0],phonemeParameters[index][1][1], powf(10.0,phonemeParameters[index][1][2] / 20.0) );
  setTargets(&filters[2],phonemeParameters[index][2][0],phonemeParameters[index][2][1], powf(10.0,phonemeParameters[index][2][2] / 20.0) );
  setTargets(&filters[3],phonemeParameters[index][3][0],phonemeParameters[index][3][1], powf(10.0,phonemeParameters[index][3][2] / 20.0) );

    /*  
	noisegain is phonemegain[1] and voiced is [0]
void setVoiced( StkFloat vGain ) { voiced_->setGainTarget(vGain); }; is in singwave
  //! Set the unvoiced component gain.
  void setUnVoiced( StkFloat nGain ) { noiseEnv_.setTarget(nGain); }; is our envelope target
*/
  target_=phonemeGains[index][1];
  target_e=phonemeGains[index][0]; // voice gain
  state_=1; state_e=1;
  }
  oldindex=index;

  float freqy=(float)adc_buffer[SELX]/8; // say peak 500 hz = 4096/8

  set_frequency(freqy,1.0);

  //loop over samples howmany
  for (u8 i=0;i<howmany;i++){
    //  temp = onepole_.tick( onezero_.tick( voiced_->tick() ) );
    //temp += noiseEnv_.tick() * noise_.tick();
    //    float temp=doonepole(doonezero(incoming[i])); // in original (Singwave relying on FileLoop) is modulated etc
    float temp=doonepole(doonezero(dosingwave())); // in original (Singwave relying on FileLoop) is modulated etc
    temp+=(doenvelope()*donoise());
    //    temp+=(0.0*donoise());
    //    float temp=incoming[i];
// lastframe is sample - OR IN CASCADE/parallel...
  float lastFrame_ = dofilter(&filters[0],temp);
  lastFrame_ += dofilter(&filters[1],lastFrame_);
  lastFrame_ += dofilter(&filters[2],lastFrame_);
  lastFrame_ += dofilter(&filters[3],lastFrame_);

  /*  lastFrame_ += dofilter(&filters[1],temp);
  lastFrame_ += dofilter(&filters[2],temp);
  lastFrame_ += dofilter(&filters[3],temp);*/
    outgoing[i]=lastFrame_;
  //  outgoing[i]=temp;
}
}

void set_frequency(float frequency, float amplitude){
	float temp = 1.0;
	rate_P = 256.0 * frequency / 32000.0;
	temp -= rate_P;
	if ( temp < 0) temp = -temp;
	//	pitchEnvelope_.setTarget( rate_ );
	target_P=rate_P;
	rate_P=0.001*temp;
	//pitchEnvelope_.setRate( sweepRate_ * temp );
	state_P=1;	
	// setting amplitude
	
	//  voiced_->setGainTarget( amplitude ); = singwave amp envelope_.setTarget( target ); };
	target_e=amplitude;

	  float thePole = 0.97 - (amplitude * 0.2);
	if ( thePole > 0.0 )
	  b_ = (float) (1.0 - thePole);
	else
	  b_ = (float) (1.0 + thePole);
	
	a_ = -thePole;

}

void initvoicform(){
  for ( int i=0; i<4; i++ )  {
    filters[i].sweepRate_=0.001;
     filters[i].frequency_ = 0.0;
  filters[i].radius_ = 0.0;
  filters[i].targetGain_ = 1.0;
  filters[i].targetFrequency_ = 0.0;
  filters[i].targetRadius_ = 0.0;
  filters[i].deltaGain_ = 0.0;
  filters[i].deltaFrequency_ = 0.0;
  filters[i].deltaRadius_ = 0.0;
  filters[i].sweepState_ = 0.0;
  filters[i].sweepRate_ = 0.002;
  }

  //  onezero_.setZero( -0.9 );

  // Normalize coefficients for unity gain. -0.9

  b_0 = 1.0 / ((float) 1.9); // 1.0 / 1.9
  b_1 = 0.9 * b_0; // 0.9 * 0.9

  //  onepole_.setPole( 0.9 );

    b_ = 0.1f;
    a_ = -0.9f;

    bx_ = 0.1f;
    ax_ = -0.9f;

    // initial phoneme and also what are other settings...

    unsigned char index=2; // which phoneme?

    setTargets(&filters[0],phonemeParameters[index][0][0],phonemeParameters[index][0][1], powf(10.0,phonemeParameters[index][0][2] / 20.0) );
    setTargets(&filters[1],phonemeParameters[index][1][0],phonemeParameters[index][1][1], powf(10.0,phonemeParameters[index][1][2] / 20.0) );
    setTargets(&filters[2],phonemeParameters[index][2][0],phonemeParameters[index][2][1], powf(10.0,phonemeParameters[index][2][2] / 20.0) );
    setTargets(&filters[3],phonemeParameters[index][3][0],phonemeParameters[index][3][1], powf(10.0,phonemeParameters[index][3][2] / 20.0) );

    // for singwave voice source:

    //	this->setFrequency( 75.0 );
    //	pitchEnvelope_.setRate( 1.0 );

    // farm this out to set_frequency
    set_frequency(75.0,1.0);

	noiseRate_ = (unsigned int) ( 330.0);
	noiseCounter_ = noiseRate_;
}
