#include "arm_math.h"
#include "arm_const_structs.h"
#include "audio.h"

#define TWO_PI  6.283185

// tables

// filters_[0].setTargets( Phonemes::formantFrequency(i, 0), Phonemes::formantRadius(i, 0), pow(10.0, Phonemes::formantGain(i, 0 ) / 20.0) );


//formantfrequency=phonemeParameters[index][partial][0];
// formantgain= phonemeParameters[index][partial][2];
// formantradius= phonemeParameters[index][partial][1];
// noisegain= phonemeGains[index][1];
// voicegain= phonemeGains[index][0];

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

const float phonemeGains[32][2] =
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

   {0.0, 0.7},    // fff
   {0.0, 0.7},    // sss
   {0.0, 0.7},    // thh
   {0.0, 0.7},    // shh

   {0.0, 0.7},    // xxx
   {0.0, 0.1},    // hee
   {0.0, 0.1},    // hoo
   {0.0, 0.1},    // hah

   {1.0, 0.1},    // bbb
   {1.0, 0.1},    // ddd
   {1.0, 0.1},    // jjj
   {1.0, 0.1},    // ggg

   {1.0, 1.0},    // vvv
   {1.0, 1.0},    // zzz
   {1.0, 1.0},    // thz
   {1.0, 1.0}     // zhh
  };

const float phonemeParameters[32][4][3] =
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

static float outputs;
static float inputs;
static float b_, a_, b_1, b_0;

float doonepole(float sample){
//  inputs_[0] = gain_ * input;
  float lastFrame_ = b_ * sample - a_ * outputs;
  outputs = lastFrame_;
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

unsigned char state_=0;
float target_=0.0,rate_=0.001,value_=0.0;

float doenvelope(){
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
    return value_;
    }
}

// noise

//*samples = (float) ( 2.0 * rand() / (RAND_MAX + 1.0) - 1.0 );

// filters

 // filters_[0].setTargets( Phonemes::formantFrequency(i, 0), Phonemes::formantRadius(i, 0), pow(10.0, Phonemes::formantGain(i, 0 ) / 20.0) );

    void setTargets(filters_ *filter, float frequency, float radius, float gain)
    {
  filter->dirty_ = 1;
  filter->targetFrequency_ = frequency;
  filter->targetRadius_ = radius;
  filter->targetGain_ = gain;

  // where are start states defined? - would be minus in next three

  filter->deltaFrequency_ = frequency;
  filter->deltaRadius_ = radius;
  filter->deltaGain_ = gain;
  filter->sweepState_ = 0.0;
  filter->sweepRate_ = 0.001;
}

    void setResonance(filters_ *filter, float frequency, float radius )
{
  float radius_ = radius;
  float frequency_ = frequency;

  filter->a_[2] = radius * radius;
  filter->a_[1] = -2.0 * radius * cosf( TWO_PI * frequency / 32000 ); // samplerate

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
      radius_ = filter->targetRadius_;
      frequency_ = filter->targetFrequency_;
      gain_ = filter->targetGain_;
    }
    else {
      radius_ = filter->startRadius_ + (filter->deltaRadius_ * filter->sweepState_);
      frequency_ = filter->startFrequency_ + (filter->deltaFrequency_ * filter->sweepState_);
      gain_ = filter->startGain_ + (filter->deltaGain_ * filter->sweepState_);
    }
    setResonance(filter, filter->frequency_, filter->radius_ ); //??? and get a b etc?
  }

  float inputs_ = gain_ * input;
  float lastFrame_ = filter->b_[0] * inputs_ + filter->b_[1] * filter->inputs_[1] + filter->b_[2] * filter->inputs_[2];
  lastFrame_ -= filter->a_[2] * filter->outputs_[2] + filter->a_[1] * filter->outputs_[1];
  filter->inputs_[2] = filter->inputs_[1];
  filter->inputs_[1] = filter->inputs_[0];
  filter->outputs_[2] = filter->outputs_[1];
  filter->outputs_[1] = lastFrame_;

  return lastFrame_;
    }

filters_ filters[4];


void dovoicform(float* incoming, float *outgoing, unsigned char howmany){

  // set other settings defined below - eg. gains on voiced and noiseEnv and change vibrato and onepole
  // test first all set first...
  // voiced is incoming or excitation


  //loop over samples howmany
  for (int i=0;i<howmany;i++){
//  temp = onepole_.tick( onezero_.tick( voiced_->tick() ) );
//  temp += noiseEnv_.tick() * noise_.tick();
    float temp=doonepole(doonezero(incoming[i])); // in original (Singwave relying on FileLoop) is modulated etc
    temp+=doenvelope()*(float)(rand()%32768-65536)/32678.0f; //noise env
// lastframe is sample
  float lastFrame_ = dofilter(&filters[0],temp);
  lastFrame_ += dofilter(&filters[1],temp);
  lastFrame_ += dofilter(&filters[2],temp);
  lastFrame_ += dofilter(&filters[3],temp);
  outgoing[i]=lastFrame_;
}
}


void initvoicform(){


  for ( int i=0; i<4; i++ )  filters[i].sweepRate_=0.001;
    
  //  onezero_.setZero( -0.9 );

  // Normalize coefficients for unity gain. -0.9
  b_0 = 1.0 / ((float) 1.0 - - 0.9); // 1.0 / 1.9

  b_1 = -0.9 * 0.9; // 0.9 * 0.9

  //  onepole_.setPole( 0.9 );

    b_ = 0.1f;
    a_ = -0.9f;

    //

    //    noiseEnv_.setRate( 0.001 );
    //    noiseEnv_.setTarget( 0.0 );

    // initial phoneme and also what are other settings...

    unsigned char index=0; // which phoneme?

    setTargets(&filters[0],phonemeParameters[index][0][0],phonemeParameters[index][0][1], powf(10.0,phonemeParameters[index][0][2] / 20.0) );
    setTargets(&filters[1],phonemeParameters[index][1][0],phonemeParameters[index][1][1], powf(10.0,phonemeParameters[index][1][2] / 20.0) );
    setTargets(&filters[2],phonemeParameters[index][2][0],phonemeParameters[index][2][1], powf(10.0,phonemeParameters[index][2][2] / 20.0) );
    setTargets(&filters[0],phonemeParameters[index][3][0],phonemeParameters[index][3][1], powf(10.0,phonemeParameters[index][3][2] / 20.0) );
}
