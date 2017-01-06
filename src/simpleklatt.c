/* from docs/klatt simple klatt */

// testing now with worming interface!

#include "audio.h"
#include "stdlib.h"
#include "stdint.h"
#include <stdio.h>
#include "parwave.h"
#include "worming.h"


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

//  static int16_t frame[40];
extern __IO uint16_t adc_buffer[10];


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

// these are constraints see klatt_params - TODO as a struct - in progress

// ref here:



int16_t val[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};
int16_t mins[40]= {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};
int16_t maxs[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

int16_t dir[40]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


static wormedparamset simpleklattset={40,
    {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60},
    {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0},
{4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60}
  };

wormy* straightwormy;

void simpleklatt_init(void){

  //  straightwormy=addworm(10.0f,10.0f,100.0f, 100.0f, straightworm);

  klatt_global_tt globale;
  globals=&globale;

  //  globals = (klatt_global_ptrr)malloc(sizeof(klatt_global_tt));
  //  frame = (klatt_frame_ptrr)malloc(sizeof(klatt_frame_tt));
  //  framer framezz[40];
  //  frame_init(globals,simpleklattset.val); 

  globals->synthesis_model = 1; // all_parallel
 globals->samrate = 32000;
 globals->glsource = 2; // 2=natural
 globals->natural_samples = natural_samples;
 globals->num_samples = NUMBER_OF_SAMPLES;
//  globals->sample_factor = (float) SAMPLE_FACTOR;
  nmspf_def = 10;
  globals->nfcascade = 0;
  globals->f0_flutter = 0;

unsigned char y;

 for (y=0;y<40;y++){ // init frame
   //        frame[y]=simpleklattset.val[y];
	dir[y]=rand()%3;
	}


// or this could be 32 for audio.c frame

globals->nspfr = (globals->samrate * nmspf_def) / 1000; // number of samples per frame = 320000 /1000 = 320
simple_parwave_init(globals);
}

void generate_worm_frame(){
  // there is both speed of the worm and how many times we run frame per worm
  // eg.

  for (int16_t x=0;x<adc_buffer[SPEED]>>4; x++){
  wormvaluedint(&simpleklattset,straightwormy, 1.0, 0, 0, 10); // paramset, worm, speed/float, offsetx/float, offsety/float, wormparam
  }
}


void generate_new_frame(int16_t* frame){
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

//      frame=val;
      // copy it:
      frame[y]=val[y];
      
      }

      }

void dosimpleklattsamples(int16_t* outgoing, u8 size){
  u8 x=0;
  static short samplenumber=0;
  static u8 newframe=1;
  while(x<size){
 
    // is it a new frame? - generate new frame
    if (newframe==1){
            generate_worm_frame();
      //                  generate_new_frame(frame);
    }

    single_parwave(globals,simpleklattset.val,newframe,samplenumber,x,outgoing);
    //    outgoing[x]=rand()%32768;

    if (newframe==1) newframe=0;
    x++;
    samplenumber++;
    if (samplenumber>globals->nspfr) { // greater than what????
      // end of frame so...????
      newframe=1;
      samplenumber=0;
  }
  }
}



/////
/*
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

//*framepointers[y]=val[y];
  }

// what does output point to?

//simple_parwave(globals, frame);

}
*/
