/*
 PATH=~/sat/bin:$PATH
 PATH=~/stm32f4/stlink/flash:$PATH
 make stlink_flash
*/

#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "codec.h"
#include "i2s.h"
#include "adc.h"
#include "audio.h"
#include "effect.h"
#include "say.h"
#include "klatt_phoneme.h"
#include "nsynth.h"
#include "elements.h"
#include "holmes.h"
#include "sam.h"
#include "tube.h"
#include "sp0256.h"
#include "tms5200x.h"
#include "digitalker.h"
#include "nvp.h"
#include "ntube.h"
#include "wavetable.h"
#include "wavetables.h"
#include "worming.h"
#include "vot.h"
#include "parwave.h"

void rsynth_init(long sr, float ms_per_frame);

//#include "raven.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[5];

extern int errno;

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,  44, 8, 0,  54, 16, 0,  20, 8, 0,  1, 6, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  44, 6, 0,  44, 8, 0,  44, 8, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what?

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what? - this one doesn;t crackle

u8 test_elm[51]={28, 10, 0, 47, 6, 0, 40, 8, 0, 2, 8, 0, 3, 1, 0, 4, 2, 0, 1, 6, 0, 1, 6, 0, 20, 8, 0, 53, 9, 0, 1, 6, 0, 1, 6, 0, 25, 12, 0, 54, 16, 0, 1, 6, 0, 1, 6, 0}; // "help me sir"

// ELM_LEN is 48 = 16 phonemes

float exy[64]={0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};

extern Wavetable wavtable;
extern wormy myworm;
extern char TTSinarray[17];

char TTStester[16]={'w','o','r','m',' ','s', 'p', 'e','e','c','h',' ','w', 'o', 'r', 'm'};

void main(void)
{
  int16_t x;
  for (x=0;x<16;x++){
    TTSinarray[x]=TTStester[x];
  }

  // all generator inits
  sp0256_init();
  sp0256_initbend();
  votrax_init();
  wavetable_init(&wavtable, plaguetable_simplesir, 328); // now last arg as length of table=less than 512 
  tms_init();
  //  LPCAnalyzer_init(); // for raven voice
  sam_init(); 
  sam_newsay_banks0(); // TEST!
  digitalk_init();
  simpleklatt_init();
  simpleklatt_newsay();
  nvp_init(); 
  klatt_init();
  rsynth_init(32000, 10); //void rsynth_init(long sr, float ms_per_frame)
 ////////
  ADC1_Init((uint16_t *)adc_buffer);
  Codec_Init(32000); 
  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);
  //  Audio_Init(); not needed

  while(1)
    {
    }
}

#ifdef  USE_FULL_ASSERT

#define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))

void assert_failed(uint8_t* file, uint32_t line)
{ 
  while (1)
    {
    }
}
#endif

#if 1
void NMI_Handler(void)
{ 
  while(1){};
}

void HardFault_Handler(void)
{ 
  while(1){};
  }


void MemManage_Handler(void)
{ 
  while(1){};
}

void BusFault_Handler(void)
{ 
  while(1){};
}

void UsageFault_Handler(void)
{ 
  while(1){};
}

void SVC_Handler(void)
{ 
  while(1){};
}

void DebugMon_Handler(void)
{ 
  while(1){};
}

void PendSV_Handler(void)
{ 
  while(1){};
}
#endif
