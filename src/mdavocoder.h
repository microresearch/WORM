#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"

typedef struct { //  bytes=3+9+(13*16*4)=12+832=844bytes!)
  u8 swap; //input channel swap
  float gain; //output level
  float thru, high; //hf thru
  float kout; //downsampled output
  u8 kval; //downsample counter
  u8 nbnd; //number of bands
  float f[16][13]; //[0-8][0 1 2 | 0 1 2 3 | 0 1 2 3 | val rate]
  u8 offset[16];
  //  float param[8];
} mdavocoder;

typedef struct {
  int32_t  track;        //track input pitch
  float pstep;        //output sawtooth inc per sample
  float pmult;        //tuning multiplier
  float sawbuf;   
  float noise;        //breath noise level
  float lenv, henv;   //LF and overall envelope
  float lbuf0, lbuf1; //LF filter buffers
  float lbuf2;        //previous LF sample
  float lbuf3;        //period measurement
  float lfreq;        //LF filter coeff
  float vuv;          //voiced/unvoiced threshold
  float maxp, minp;   //preferred period range
  float root;        //tuning reference (MIDI note 0 in Hz)
  float param[5];
} mdavocal;


void mdaVocoderprocess(mdavocoder* unit,float *input1, float *input2, float *output, int sampleFrames);
void mdaVocoder_init(mdavocoder* unit);
void mdavocal_init(mdavocal* unit);
void mdavocal_process(mdavocal *unit, float *input1, float *input2, float *output, int sampleFrames);
