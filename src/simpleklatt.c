/* from docs/klatt simple klatt */

#include "parwave.h"
#include "stm32f4xx.h"
#include "audio.h"

/* for default sampled glottal excitation waveform */

#define NUMBER_OF_SAMPLES 100
#define SAMPLE_FACTOR 0.00001 // this is too low

  int *iwave;
  int isam;
  int icount;
  int nmspf_def;
  klatt_global_ptr globals;
  klatt_frame_ptr frame;
  unsigned char high_byte;
  unsigned char low_byte;
  flag raw_flag;
  flag raw_type;

  static int natural_samples[NUMBER_OF_SAMPLES]=
  {
    -310,-400,530,356,224,89,23,-10,-58,-16,461,599,536,701,770,
    605,497,461,560,404,110,224,131,104,-97,155,278,-154,-1165,
    -598,737,125,-592,41,11,-247,-10,65,92,80,-304,71,167,-1,122,
    233,161,-43,278,479,485,407,266,650,134,80,236,68,260,269,179,
    53,140,275,293,296,104,257,152,311,182,263,245,125,314,140,44,
    203,230,-235,-286,23,107,92,-91,38,464,443,176,98,-784,-2449,
    -1891,-1045,-1600,-1462,-1384,-1261,-949,-730
  };


void init_simpleklatt(void){

  globals = (klatt_global_ptr)malloc(sizeof(klatt_global_t));
  frame = (klatt_frame_ptr)malloc(sizeof(klatt_frame_t));

  globals->synthesis_model = ALL_PARALLEL;
  globals->samrate = 32000;
  globals->glsource = NATURAL;
  globals->natural_samples = natural_samples;
  globals->num_samples = NUMBER_OF_SAMPLES;
  globals->sample_factor = (float) SAMPLE_FACTOR;
  nmspf_def = 10;
  globals->nfcascade = 0;
  globals->f0_flutter = 0;
  raw_flag = FALSE;

  globals->nspfr = (globals->samrate * nmspf_def) / 1000;
  parwave_init(globals);

}
