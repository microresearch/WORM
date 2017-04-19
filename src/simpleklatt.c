/* from docs/klatt simple klatt */

// also uses parwave.c

#include "audio.h"
#include "stdlib.h"
#include "stdint.h"
#include <stdio.h>
#include "parwave.h"
#include "worming.h"
#include "resources.h"

/* for default sampled glottal excitation waveform */

#define NUMBER_OF_SAMPLES 100
//#define SAMPLE_FACTOR 0.00001 // this is too low - hardcoded in parwave

  int16_t *iwave;
  int16_t isam;
  int16_t icount;
  int16_t nmspf_def;
  klatt_global_ptrr globals;
//  klatt_frame_ptrr frame;
  unsigned char high_byte;
  unsigned char low_byte;
  flag raw_flag;
  flag raw_type;

static int16_t frame[40];
extern float exy[64];
extern float _selx, _sely, _selz;

  static const int16_t natural_samples[NUMBER_OF_SAMPLES]=
  {
    -310,-400,530,356,224,89,23,-10,-58,-16,461,599,536,701,770,
    605,497,461,560,404,110,224,131,104,-97,155,278,-154,-1165,
    -598,737,125,-592,41,11,-247,-10,65,92,80,-304,71,167,-1,122,
    233,161,-43,278,479,485,407,266,650,134,80,236,68,260,269,179,
    53,140,275,293,296,104,257,152,311,182,263,245,125,314,140,44,
    203,230,-235,-286,23,107,92,-91,38,464,443,176,98,-784,-2449,
    -1891,-1045,-1600,-1462,-1384,-1261,-949,-730
  };

// check parameters: see klattparams - could be good to have contour morphing (snap on trigger)
// param0 is fund freq, param39 is volume



static const int16_t mins[40] __attribute__ ((section (".flash"))) = {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};

static const int16_t maxs[40] __attribute__ ((section (".flash"))) = {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

static const int16_t range[40] __attribute__ ((section (".flash"))) ={3800, 70, 1100, 960, 2450, 960, 3799, 960, 3799, 960, 3799, 960, 3799, 1960, 280, 960, 280, 960, 70, 55, 80, 24, 80, 40, 80, 960, 80, 960, 80, 960, 80, 960, 80, 960, 80, 1960, 80, 80, 70, 60};

klatt_global_tt globale;


void simpleklatt_init(void){

  //  straightwormy=addworm(10.0f,10.0f,100.0f, 100.0f, straightworm);

  globals=&globale;

  //  globals = (klatt_global_ptrr)malloc(sizeof(klatt_global_tt));
  //  frame = (klatt_frame_ptrr)malloc(sizeof(klatt_frame_tt));
  //  framer framezz[40];
  //  frame_init(globals,simpleklattset.val); 

  globals->synthesis_model = 1; // all_parallel
 globals->samrate = 32000;
 globals->glsource = 2; // 1=impulsive 2=glottal impulse 3=sampled as above
 globals->natural_samples = natural_samples;
 globals->num_samples = NUMBER_OF_SAMPLES;
//  globals->sample_factor = (float) SAMPLE_FACTOR;
 nmspf_def = 10;
  globals->nfcascade = 0;
  globals->f0_flutter = 0;

unsigned char y;

// or this could be 32 for audio.c frame

 globals->nspfr = (globals->samrate * nmspf_def) / 100; // was / 1000// number of samples per frame = 320000 /1000 = 320
simple_parwave_init(globals);
}

void generate_exy_frame(int16_t* frame){
unsigned char y;

    for (y=0;y<40;y++){
      frame[y]=mins[y] + (range[y]*(1.0f-exy[y])); // TODO: floated loggy!
    }
      }

int16_t simpleklatt_get_sample(){
  u8 x=0;
  static int16_t samplenumber=0;
  int16_t sampel;

  sampel=single_single_parwave(globals, frame);

  samplenumber++;
  if (samplenumber>globals->nspfr*_selz) { // greater than what???? 320 samples - this can be our selz???? 
      // end of frame so...????
    samplenumber=0;
    simpleklatt_newsay();
    }
    return sampel;
}

void simpleklatt_newsay(){
  // generate the frame from our exy -> frame
  generate_exy_frame(frame);
  frame_init(globals,frame); 
    if (globals->f0_flutter != 0)
      flutter(globals,frame);  
    globals->ns=0;
}


