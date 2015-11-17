#include "audio.h"
#include <malloc.h>
#include <math.h>
#include <audio.h>
#include "stm32f4xx.h"
#include "arm_math.h"
#include "klatt_phoneme.h"


extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); 
extern __IO uint16_t adc_buffer[10];

static uint16_t syncPhaseAcc;
static uint16_t syncPhaseInc;
static uint16_t grainPhaseAcc;
static uint16_t grainPhaseInc;
static uint16_t grainAmp;
static uint8_t grainDecay;
static uint16_t grain2PhaseAcc;
static uint16_t grain2PhaseInc;
static uint16_t grain2Amp;
static uint8_t grain2Decay;

uint16_t antilogTable[] = {
  64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
  54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
  45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
  38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
};

uint16_t mapPhaseInc(uint16_t input) {
  return (antilogTable[input & 0x3f]) >> (input >> 6);
}

static float vosim;
static 	float phase=0.f;
static 	float prevtrig=0.f;
static 	u16 nCycles=1;
static 	u16 numberCurCycle=0;
static 	float prevsine;
static 	float decay=0.5f;
static 	float amp=1.0f;
const float PII = 3.1415926535f;

/*void runVOSIMaud(villager_generic* vill){
  u8 step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;
  uint16_t output; u8 value;
  
  // Smooth frequency mapping
  syncPhaseInc = mapPhaseInc(buf16[0]>>6) / 4; // 10 bits /4
  
  // Stepped mapping to MIDI notes: C, Db, D, Eb, E, F...
  //syncPhaseInc = mapMidi(analogRead(SYNC_CONTROL));
  
  // Stepped pentatonic mapping: D, E, G, A, B
  //  syncPhaseInc = mapPentatonic(buf16[1]>>6);

  grainPhaseInc  = mapPhaseInc(buf16[1]>>6) / 2;
  grainDecay     = (buf16[2]>>6) / 8;
  grain2PhaseInc = mapPhaseInc(buf16[3]>>6) / 2;
  grain2Decay    = (buf16[4]>>6) / 4;
 

  // deal with step and count and so on... 
   for (u8 xx=0;xx<vill->howmany;xx++){
     count+=step;
     if (count>start+wrap) count=start;

  syncPhaseAcc += syncPhaseInc;
  if (syncPhaseAcc < syncPhaseInc) {
    // Time to start the next grain
    grainPhaseAcc = 0;
    grainAmp = 0x7fff;
    grain2PhaseAcc = 0;
    grain2Amp = 0x7fff;
  }
  
  // Increment the phase of the grain oscillators
  grainPhaseAcc += grainPhaseInc;
  grain2PhaseAcc += grain2PhaseInc;

  // Convert phase into a triangle wave
  value = (grainPhaseAcc >> 7) & 0xff;
  if (grainPhaseAcc & 0x8000) value = ~value;
  // Multiply by current grain amplitude to get sample
  output = value * (grainAmp >> 8);

  // Repeat for second grain
  value = (grain2PhaseAcc >> 7) & 0xff;
  if (grain2PhaseAcc & 0x8000) value = ~value;
  output += value * (grain2Amp >> 8);
  buf16[count&32767]=output; 

  // Make the grain amplitudes decay by a factor every sample (exponential decay)
  grainAmp -= (grainAmp >> 8) * grainDecay;
  grain2Amp -= (grain2Amp >> 8) * grain2Decay;
   }
  vill->position=count;

}

void runflam(villager_generic* vill){ // single impulse sine*sine
  u16 out;
  float freq = (float)(100);
  //  float nCycles = (float)(10);
  //  float nDecay = (float)(0.1);
  float phaseinc = freq * 2.f * PII / 32000.0f;
  //  float numberCycles = nCycles;
  //  int number = numberCurCycle;
  //  static int count=0; int start=0, wrap=1000;
  u8 step;
  u16 count=vill->position;
  u16 start=vill->start;
  u16 wrap=vill->wrap;



   for (u8 xx=0;xx<vill->howmany;xx++){
     count+=step;
     if (count>start+wrap) count=start;
  
     float z = vosim;
     //     float trigin = (float)((rand()%65536)-32768)/32768.0f;

     if(running ){
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;

       if(prevsine > 0.f && sine < 0.f){
	 running=0;
       }

       if(prevsine < 0.f && sine > 0.f){
	 running=0;
       }

       prevsine = sine;

       phase = phase + phaseinc;

     }

     //       phase = phase + phaseinc;

     // write the output
          out = (float)z*65536.0f;
     //     printf("%d\n",out);

       buf16[count&32767]=out+32768; 
     //     printf("cnt: %d out: %d\n",count, out);
     //          printf("%d\n",out);
  }
   vill->position=count;
  //  nCycles = numberCycles;
  //  numberCurCycle = number;


}
*/

pair demandVOSIM_SC(u16 writepos,float freq,float nCycles,float nDecay){
  u16 howmany=256; // fastest we need is 32*8=256
  u16 out; 
  float phaseinc = freq * 2.f * PII / 32000.0f;
  u16 numberCycles = nCycles;
  u16 number = numberCurCycle;
 
  for (u16 xx=0;xx<howmany;xx++){

     writepos++;
     if (writepos>=AUDIO_BUFSZ) writepos=0;
  
     float z = vosim;
     float trigin = (float)((rand()%65536)-32768)/32768.0f; //TODO!

     if(phase > 0.f && number <= numberCycles ){
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;

       if(prevsine >= 0.f && sine <= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       if(prevsine <= 0.f && sine >= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       prevsine = sine;
       phase = phase + phaseinc;

     }else if(trigin > 0.f && prevtrig <= 0.f){
     //        else if(1){

       numberCycles = nCycles;
       decay = nDecay;
       amp = 1.f;
       number = 0;
       phase=0;
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;
       prevsine = sine;
       phase = phase + phaseinc;
     }else if(number >= numberCycles){
       phase = 0;
       //       vosim = 0.f;
     }
     prevtrig = trigin;

     // write the output
     out = (float)z*32768.0f;

     audio_buffer[writepos]=out; 

  }
  nCycles = numberCycles;
  numberCurCycle = number;

  pair r={256,writepos};
  return r;
}


u16 runVOSIM_SC(u16 count){
  u16 howmany=256; // fastest we need is 32*8=256
  u16 out; 
  // so we need control of freq, cycles and decay... within bounds and from SELX, SELY..
  // trigger will come from INPUT!
  /*    float freq = 1500.0f; // reference figures as these work.
  float nCycles = 32.0f;
  float nDecay = 0.5f;
  float phaseinc = freq * 2.f * PII / 32000.0f;
  */

  float freq = (float)((adc_buffer[SELX])+100);//1500.0f; 
  float nCycles = (float)((adc_buffer[SELY]>>4)+2);
  float nDecay = ((float)(adc_buffer[SELZ])/4096.0f); // TODO as SELZ!
  
  float phaseinc = freq * 2.f * PII / 32000.0f;
 

  /*  float freq = (float)(adc_buffer[2]);
    u16 nCycles = (adc_buffer[0]>>8);
    float nDecay = (float)(adc_buffer[3]/4096.0f);
  float nDecay = 0.9f;
  float phaseinc = freq * 2.f * PII / 32000.0f;
  */
  u16 numberCycles = nCycles;
  u16 number = numberCurCycle;
 
  for (u16 xx=0;xx<howmany;xx++){

     count++;
     if (count>=AUDIO_BUFSZ) count=0;
  
     float z = vosim;
     float trigin = (float)((rand()%65536)-32768)/32768.0f; //TODO!

     if(phase > 0.f && number <= numberCycles ){
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;

       if(prevsine >= 0.f && sine <= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       if(prevsine <= 0.f && sine >= 0.f){
	 number += 1;
	 amp = amp * decay;
       }

       prevsine = sine;
       phase = phase + phaseinc;

     }
     else if(trigin > 0.f && prevtrig <= 0.f){
     //        else if(1){

       numberCycles = nCycles;
       decay = nDecay;
       amp = 1.f;
       number = 0;
       phase=0;
       float sine = sinf(phase);
       vosim = (sine * sine) * amp;
       prevsine = sine;
       phase = phase + phaseinc;
     }else if(number >= numberCycles){
       phase = 0;
       //       vosim = 0.f;
     }
     prevtrig = trigin;

     // write the output
     out = (float)z*32768.0f;

     audio_buffer[count]=out; 

  }
  nCycles = numberCycles;
  numberCurCycle = number;
  return count;
}

