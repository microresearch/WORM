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
extern Formlet *formy;
extern Formant *formanty;
extern Blip *blipper;
extern RLPF *RLPFer;
extern NTube tuber;
extern Wavetable wavtable;
extern wormy myworm;
extern biquad* newBB;

const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,  44, 8, 0,  54, 16, 0,  20, 8, 0,  1, 6, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  44, 6, 0,  44, 8, 0,  44, 8, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what?

//u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0}; // ELM_LEN in holmes - but why do we need extra 0>// extra what? - this one doesn;t crackle

u8 test_elm[51]={28, 10, 0, 47, 6, 0, 40, 8, 0, 2, 8, 0, 3, 1, 0, 4, 2, 0, 1, 6, 0, 1, 6, 0, 20, 8, 0, 53, 9, 0, 1, 6, 0, 1, 6, 0, 25, 12, 0, 54, 16, 0, 1, 6, 0, 1, 6, 0}; // "help me sir"

// ELM_LEN is 48 = 16 phonemes

void main(void)
{
  int16_t x;
  // all generator inits
  //      LPCAnalyzer_init();
  init_synth(); // which one? --> klatt rsynth !!!! RENAME!
  sp0256_init();
   lpc_init(); 
/*   simpleklatt_init(); */
/* sam_init(); */
/* sam_newsay(); // TEST! */
/* tms5200_init(); */
/* tms5200_newsay(); */
/* channelv_init(); */
/* tube_init(); */
/* tube_newsay(); */
/* BANDS_Init_(); */
/* Vocoder_Init(32000.0f); */
/* digitalk_init(); */
/* digitalk_newsay(0); */
/* nvp_init(); */
/* sample_rate_init(); */
/* initbraidworm(); // re_name */
/* initvoicform(); */
/*   formy=malloc(sizeof(Formlet)); */
/*   formanty=malloc(sizeof(Formant)); */
/*   blipper=malloc(sizeof(Blip)); */
/*   RLPFer=malloc(sizeof(RLPF)); */
/*   Formlet_init(formy); */
/*   Formant_init(formanty); */
/*   Blip_init(blipper); */
/*   RLPF_init(RLPFer); */
/*   NTube_init(&tuber); */
/*    wavetable_init(&wavtable, crowtable_slower, 283); // now last arg as length of table=less than 512 */
/*   //    wavetable_init(&wavtable, plaguetable_simplesir, 328); // now last arg as length of table=less than 512 */
/*   //  wavetable_init(&wavtable, table_kahrs000, 160); // now last arg as length of table=less than 512 */
/*     //  addwormsans(&myworm, 10.0f,10.0f,200.0f, 200.0f, wanderworm); */
/*   //  RavenTube_init(); */
   newBB=BiQuad_new(LPF, 1.0, 1500, 32000, 0.68); // TEST? 
        votrax_init(); 
    
 ////////
  ADC1_Init((uint16_t *)adc_buffer);
  Codec_Init(32000); 
  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);
  //  Audio_Init(); not needed

  while(1)
    {
    // testing changing test_elm
      //      u8 axis=adc_buffer[SELX]>>8; // 16*3=48
      // change element, change length? leave stress as is 0
      //      test_elm[axis*3]=phoneme_prob_remap[adc_buffer[SELY]>>6]; // how many phonemes?=64
      //      test_elm[(axis*3)+1]=(adc_buffer[SELZ]>>7)+1; // length say max 32
    

      //      oldmode=mode;    
      //      mode=adc_buffer[MODE]>>7; // 12 bits to say 32 modes (5 bits)
      //           mode=10; // TESTING

      //       if(lpc_busy() == 0) lpc_newsay(adc_buffer[SELX]>>6);   

      //    if(lpc_busy() != 0)    lpc_running(); // so just writes once otherwise gets messy...


  // if there is a change in mode do something?
  //  if (oldmode!=mode){
  //    maintrigger=1;
  //  }

	   /*  if (maintrigger==1) {writepos=0;trigger=1;} // STRIP_OUT

  switch(mode){
  case 0:// rsynth/klatt-single phoneme
           if (trigger==1){
	     trigger=0;
	     u8 phonemm=phoneme_prob_remap[(adc_buffer[SELX]>>6)]; // 7bits=128 %69//6=64
	     pair xx=klatt_phoneme(writepos,phonemm); 
	     generated=xx.generated;
	     writepos=xx.writepos;
	   }
	   break;
  case 1: // rsynth/klatt-chain of phonemes
    writepos=run_holmes(writepos); 
    break;
  case 2: // vosim free running
    writepos=runVOSIM_SC(writepos);
    break;
  case 3: // VOSIMondemand
    if (trigger==1){
      trigger=0;
      float freqwency = (float)((adc_buffer[SELX])+100);//1500.0f; 
      float cycles = (float)((adc_buffer[SELY]>>4)+2);
      float decay = ((float)(adc_buffer[SELZ])/4096.0f); // TODO as SELZ!
      pair xx=demandVOSIM_SC(writepos,freqwency,cycles,decay); 
      generated=xx.generated;
      writepos=xx.writepos;
    }
    break;
  case 9: // SAM full. no writepos though and just a simple proof here
        if (trigger==0){
    	SAMMain();
	trigger=1;
	     }     
    break;
  case 10:
    if(lpc_busy() == 0) lpc_newsay(adc_buffer[SELX]>>6);   

    if(lpc_busy() != 0)    lpc_running(); // so just writes once otherwise gets messy...
    break;
  case 19: // parwave/simpleklatt
    dosimpleklatt();
    break;

  } // cases

    // now readpos is back to one now that we have written something 
  if (maintrigger==1) {
      readpos=0;
      maintrigger=0;
  }
	   */
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
