#include "audio.h"
#include "effect.h"
#include "biquad.h"

extern __IO uint16_t adc_buffer[10];


// 10 channel v[oc]oder generator

/* coeffs:

from okita: 129-209=80bw 169, 196-317=121bw 256, 294-476=182bw 385, 431-698=267bw 564, 

647-1047=400bw 847, 952-1541=589bw 1246, 1295-2085=790bw 1690, 1962-3175-1213bw 2568, 2942-4762=1820bw 3852, 4315-6984=2669bw 5650


q=freqmiddle/bandwidth but this is not what we use here

= 12dB/octave response. 

 */

// conversion as in: collated_forms.h but what do we use for biquad = Q?
// q is centre/bandwidth=169/80 = 2.112 in first instance TEST!

// but bandwidth is -3db - we can maybe morph and stretch this bandwidth

// use biquad.c as this is 12db/octave

// keep as size samples for moment

// maybe 16 channels so will change freq and Q

static biquad* newB[10];

const float channelvfreq[10]={169.0f, 256.0f, 385.0f, 564.0f, 847.0f, 1246.0f, 1690.0f, 2568.0f, 3852.0f, 5650.0f};

//const float channelvQ[10]={2.112, 2.116, 2.115, 2.117, 2.118, 2.115, 2.14, 2.117, 1.567?, 2.12};

float mult_table[10]={0.1f,0.1f,0.1f,0.1f,0.1f,0.1f,0.1f,0.1f,0.1f,0.1f};

void dochannelv(int16_t* incoming, int16_t* outgoing, u8 howmany){
  u8 x,y;
  u16 which;
  float xx,xxx;
  float tmpbuffer[32],tmpoutbuffer[32];

  // all TESTING

  //*BiQuad_reinit(biquad *b, smp_type bandwidth) // just for BPF and just for change in bandwidth

  // but somehow these need change simultaneously

  // trajectory and bounds?


  //  BiQuad_reinit(newB[2], (float)adc_buffer[SELZ]/4096.0); //from 0 far right to 1 far left

  // and Q seems inverted so higher Q gives lower resonance/flatter - RE_TEST

  /*  int_to_floot(incoming,tmpbuffer,howmany);


    for (x=0;x<howmany;x++){
	xx=BiQuad(tmpbuffer[x],newB[2]); 
	tmpoutbuffer[x]=(xx*mult_table[y]); // mix in proportion or not
    }
    floot_to_int(outgoing,tmpoutbuffer,howmany);
}
  */

  
  which=(adc_buffer[SELX]>>8)%10;// 4 bits to test
  mult_table[which]=(float)adc_buffer[SELY]/4096.0;
  //    BiQuad_reinit(newB[which], (float)adc_buffer[SELZ]/256.0);

  int_to_floot(incoming,tmpbuffer,howmany);
    for (x=0;x<howmany;x++){
      tmpoutbuffer[x]=0.0f;
      for (y=0;y<10;y++){
	xx=BiQuad(tmpbuffer[x],newB[y]); 
    // for each which we add the multed mult_table result
	tmpoutbuffer[x]+=(xx*mult_table[y]); // mix in proportion or not
      }
      //tmpoutbuffer[x]+=(xx*1.0f);
      // change mult or do trajectories...
    }
    floot_to_int(outgoing,tmpoutbuffer,howmany);
    }

void channelv_init(){
  u8 x;
// allocate and set up bandpasses (10 in total test for one)

//biquad *BiQuad_new(int type, smp_type dbGain, smp_type freq, srate, bw // gain doesn't come into it!

    for (x=0;x<10;x++){

      newB[x]=BiQuad_new(BPF, 1.0, channelvfreq[x], 32000, 0.68); // Q we can morph at will?
    }
}



