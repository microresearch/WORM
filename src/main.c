
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

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

static const float freq[5][5] __attribute__ ((section (".flash"))) = {
      {600, 1040, 2250, 2450, 2750},
      {400, 1620, 2400, 2800, 3100},
      {250, 1750, 2600, 3050, 3340},
      {400, 750, 2400, 2600, 2900},
      {350, 600, 2400, 2675, 2950}
  };

static const float qqq[5][5] __attribute__ ((section (".flash"))) = {
    {14.424072,21.432398,29.508234,29.453644,30.517193},
    {14.424072,29.213112,34.623474,33.661621,37.268467},
    {6.004305,28.050945,37.508934,36.667324,40.153900},
    {14.424072,13.522194,34.623474,31.257082,34.863983},
{12.620288,10.816360,34.623474,32.158768,35.465038}
  };

static const float mull[5][5] __attribute__ ((section (".flash"))) = {
    {1, 0.44668359215096, 0.35481338923358, 0.35481338923358, 0.1},
    {1, 0.25118864315096, 0.35481338923358, 0.25118864315096, 0.12589254117942},
    {1, 0.031622776601684, 0.15848931924611, 0.079432823472428, 0.03981071705535},
    {1, 0.28183829312645, 0.089125093813375, 0.1, 0.01},
    { 1, 0.1, 0.025118864315096, 0.03981071705535, 0.015848931924611}
  };

//u16 *buf16;

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

extern int16_t audio_buffer[AUDIO_BUFSZ];
u16 sin_data[256] __attribute__ ((section (".ccmdata")));  // sine LUT Array


// effects tests

//mdavocoder *mdavocod;
arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate


extern int errno;

volatile u16 readpos=0;
volatile u8 mode=0;
volatile u8 trigger=0;
volatile u8 maintrigger=0;
volatile u16 generated=0;
volatile u16 writepos=0;

static const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62};

u8 test_elm[30]={54, 16, 0, 24, 15, 0, 1, 6, 0, 1, 6, 0, 44, 8, 0, 54, 16, 0, 20, 8, 0, 1, 6, 0, 1, 6, 0, 1, 6, 0};

void main(void)
{
  int32_t samplepos;
  u16 count,x,xx;
  u8 oldmode=0;

  // effects init

  //  mdavocod=(mdavocoder *)malloc(sizeof(mdavocoder));
  //  mdaVocoder_init(mdavocod);

  float Fc,Q,peakGain;
  const float Fs=32000.0f;// TODO
  float a0,a1,a2,b1,b2,norm,V,K;
  float *state[5][5];

  for (xx=0;xx<5;xx++){// five formants INIT

  for (x=0;x<5;x++){
    Fc=freq[xx][x];
    Q=qqq[xx][x];
    K = tanf(M_PI * Fc / Fs);
    norm = 1 / (1 + K / Q + K * K);
    a0 = K / Q * norm;
    a1 = 0;
    a2 = -a0;
    b1 = 2 * (K * K - 1) * norm;
    b2 = (1 - K / Q + K * K) * norm;

    coeffs[xx][x][0]=a0*mull[xx][x]; coeffs[xx][x][1]=a1*mull[xx][x]; coeffs[xx][x][2]=a2*mull[xx][x]; coeffs[xx][x][3]=-b1*mull[xx][x]; coeffs[xx][x][4]=-b2*mull[xx][x];
      /// can also just mult coeffs????
    state[xx][x] = (float*)malloc(4*sizeof(float));
    arm_biquad_cascade_df1_init_f32(&df[xx][x],1,coeffs[xx][x],state[xx][x]);
  }
  }


  static const float32_t pi= 3.141592;
  float32_t w;
  float32_t yi;
  float32_t phase=0;
  int sign_samp,i;
  w= 2*pi;
  w= w/256;

  for (i = 0; i <= 256; i++)
    {
      yi= 32767*sinf(phase); // was 2047???
      phase=phase+w;
      sign_samp=32767+yi;     // dc offset
      sin_data[i]=sign_samp; // write value into array
    }

  ADC1_Init((uint16_t *)adc_buffer);
  Codec_Init(32000); 
  delay();
  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);
  init_synth();

  while(1)
    {

      oldmode=mode;    
      mode=adc_buffer[MODE]>>7; // 12 bits to say 32 modes (5 bits)

  // if there is a change in mode do something?
  if (oldmode!=mode){
    maintrigger=1;
  }

  if (maintrigger==1) {writepos=0;trigger=1;}

  mode=3;

  switch(mode){
  case 0:// rsynth/klatt-single phoneme
           if (trigger==1){
	     trigger=0;
	     u8 phonemm=phoneme_prob_remap[(adc_buffer[SELX]>>5)]; // 7bits=128 %69//6=64
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
      float freqwency = (float)(adc_buffer[SELX]+100);//1500.0f; 
      float cycles = (float)((adc_buffer[SELY]>>8)+1);
      float decay = 0.5f; //TODO!
      pair xx=demandVOSIM_SC(writepos,freqwency,cycles,decay); 
      generated=xx.generated;
      writepos=xx.writepos;
    }
    break;
  
    // now readpos is back to one now that we have written something 
    if (maintrigger==1) {
      readpos=0;
      maintrigger=0;
}
 
  }
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
