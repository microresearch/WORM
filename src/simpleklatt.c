/* from docs/klatt simple klatt */

#include "stdlib.h"
#include "stdint.h"
#include <stdio.h>
#include "parwave.h"

/* for default sampled glottal excitation waveform */

#define NUMBER_OF_SAMPLES 100
//#define SAMPLE_FACTOR 0.00001 // this is too low - hardcoded in parwave

  int *iwave;
  int isam;
  int icount;
  int nmspf_def;
  klatt_global_ptrr globals;
  klatt_frame_ptrr frame;
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

typedef struct
{
  int val;
  int min;
  int max;
} framer;

// these are constraints see klatt_params

int val[40]= {1000, 0, 497, 0, 739, 0, 2772, 0, 3364, 0, 4170, 0, 4000, 0, 0, 0, 200, 40, 0, 40, 0, 20, 0, 0, 53, 44, 79, 70, 52, 95, 44, 56, 34, 80, 0, 80, 0, 0, 27, 70};
int mins[40]= {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};
int maxs[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

int dir[40]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

unsigned int ** framepointers;

void init_simpleklatt(void){

  globals = (klatt_global_ptrr)malloc(sizeof(klatt_global_tt));
  frame = (klatt_frame_ptrr)malloc(sizeof(klatt_frame_tt));
  framer framezz[40];

unsigned int * framepointers[40] = {&frame->F0hz10, &frame->AVdb,
      &frame->F1hz,   &frame->B1hz,
      &frame->F2hz,   &frame->B2hz,
      &frame->F3hz,   &frame->B3hz,
      &frame->F4hz,   &frame->B4hz,
      &frame->F5hz,   &frame->B5hz,
      &frame->F6hz,   &frame->B6hz,
      &frame->FNZhz,  &frame->BNZhz,
      &frame->FNPhz,  &frame->BNPhz,
      &frame->ASP,    &frame->Kopen,
      &frame->Aturb,  &frame->TLTdb,
      &frame->AF,     &frame->Kskew,
      &frame->A1,     &frame->B1phz,
      &frame->A2,     &frame->B2phz,
      &frame->A3,     &frame->B3phz,
      &frame->A4,     &frame->B4phz,
      &frame->A5,     &frame->B5phz,
      &frame->A6,     &frame->B6phz,
      &frame->ANP,    &frame->AB,
	&frame->AVpdb,  &frame->Gain0};


globals->synthesis_model = 1; // all_parallel
  globals->samrate = 32000;
globals->glsource = 2; // natural
  globals->natural_samples = natural_samples;
  globals->num_samples = NUMBER_OF_SAMPLES;
//  globals->sample_factor = (float) SAMPLE_FACTOR;
  nmspf_def = 10;
  globals->nfcascade = 0;
  globals->f0_flutter = 0;

unsigned char y;
for (y=0;y<40;y++){
      framezz[y].val=val[y];
	dir[y]=rand()%3;
    }


// or this could be 32 for audio.c frame

globals->nspfr = (globals->samrate * nmspf_def) / 1000; // number of samples per frame = 320000 /1000 = 320
simple_parwave_init(globals);
}

void dosimpleklatt(void){
unsigned char y;
// put frame together from wormings

    for (y=0;y<40;y++){

      // direction change 0,1-back,2-forwards
      switch(dir[y]){
      case 0:
	// no change
	break;
      case 1:
	// forwards
	val[y]+=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]>maxs[y]) dir[y]=2;
	break;
      case 2:
	// backwards
	val[y]-=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]<mins[y]) dir[y]=1;
	break;
      }
//	printf("%d ",val[y]);
// we need to fill frame parameters:

*framepointers[y]=val[y];
  }

// what does output point to?

simple_parwave(globals, frame);

}
