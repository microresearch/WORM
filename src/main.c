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
#include "mdavocoder.h"
#include "vocode.h"
#include "biquad.h"
#include "effect.h"
#include "PV_vocoder.h"
#include "say.h"
#include "klatt_phoneme.h"
#include "nsynth.h"
#include "elements.h"
#include "holmes.h"
#include "saml.h"
#include "tube.h"
#include "LPC/lpc.h"
#include "parwave.h"
#include "sp0256.h"
#include "tms5200x.h"
#include "channelv.h"
#include "svf.h"
#include "wvocoder.h"
#include "digitalker.h"
#include "nvp.h"
#include "samplerate.h"
#include "braidworm.h"
#include "voicform.h"
#include "scformant.h"
#include "ntube.h"
#include "lpcansc.h"
#include "wavetable.h"
#include "wavetables.h"
#include "worming.h"
#include "vot.h"

//#include "raven.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[5];

#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

extern int errno;

const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,  44, 8, 0,  54, 16, 0,  20, 8, 0,  1, 6, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  44, 6, 0,  44, 8, 0,  44, 8, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what?

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what? - this one doesn;t crackle

u8 test_elm[51]={28, 10, 0, 47, 6, 0, 40, 8, 0, 2, 8, 0, 3, 1, 0, 4, 2, 0, 1, 6, 0, 1, 6, 0, 20, 8, 0, 53, 9, 0, 1, 6, 0, 1, 6, 0, 25, 12, 0, 54, 16, 0, 1, 6, 0, 1, 6, 0}; // "help me sir"

// ELM_LEN is 48 = 16 phonemes

float exy[64]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

extern Wavetable wavtable;
extern wormy myworm;
extern char TTSinarray[17];

void main(void)
{
  int16_t x;

  for (x=0;x<16;x++){
    TTSinarray[x]=97+x;
  }



  // all generator inits
  sp0256_init();
  sp0256_initbend();
  votrax_init();
  LPCAnalyzer_init4();
  //  LPCAnalyzer_init(); // for raven voice
  wavetable_init(&wavtable, plaguetable_simplesir, 328); // now last arg as length of table=less than 512 
  //  sample_rate_init();
  tms_init();


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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  //printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* Infinite loop */
  while (1)
    {
    }
}
#endif

#if 1
/* exception handlers - so we know what's failing */
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
