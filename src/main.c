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
#include "worming.h"

/* DMA buffers for I2S */
__IO int16_t tx_buffer[BUFF_LEN], rx_buffer[BUFF_LEN];

/* DMA buffer for ADC  & copy */
__IO uint16_t adc_buffer[10];

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

const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?

u8 test_elm[51]={44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,  44, 8, 0,  54, 16, 0,  20, 8, 0,  1, 6, 0,  1, 6, 0,  1, 6, 0, 44, 16, 0,  14, 15, 0,  1, 6, 0,  1, 6, 0,  44, 8, 0,  44, 8, 0, 1, 8, 0}; // ELM_LEN in holmes - but why do we need extra // extra what?

const float crowtable[]={-0.283905, -0.055756, 0.206726, 0.495911, 0.636169, 0.630768, 0.330597, 0.458313, 0.337769, 0.321716, 0.154480, 0.151001, 0.140137, 0.057526, 0.132965, 0.294678, -0.039459, 0.053833, 0.190521, 0.194092, 0.362946, 0.368439, 0.407898, 0.434875, 0.309113, 0.305450, 0.217468, 0.098816, 0.233612, 0.127594, 0.140167, 0.386353, 0.420502, 0.355804, 0.382782, 0.267731, 0.079102, 0.194031, 0.197723, 0.165314, 0.167114, -0.055725, 0.073730, 0.129303, 0.195984, 0.100525, 0.095306, -0.145569, -0.215668, -0.043091, 0.001770, -0.073669, -0.307281, -0.402496, -0.672150, -0.557007, -0.722443, -0.857147, -0.867981, -0.882294, -0.880554, -0.803284, -0.724121, -0.632690, -0.438293, -0.330811, -0.224487, -0.201385, -0.258698, -0.249817, -0.246155, -0.226471, -0.422241, -0.217529, -0.096924, -0.093536, 0.079132, 0.141907, 0.165344, 0.019806, 0.159882, 0.079102, 0.267761, 0.170654, 0.203156, -0.097137, 0.070160, 0.273102, 0.061127, 0.265930, 0.328857, 0.546295, 0.864349, 0.828430, 0.654144, 0.569550, 0.469147, 0.510223, 0.413422, 0.567810, 0.539062, 0.776398, 0.600067, 0.573395, 0.468903, 0.316345, 0.098785, 0.077271, 0.077332, -0.050385, -0.312592, -0.479919, -0.334137, -0.016266, 0.131287, -0.143829, -0.366577, -0.637909, -0.513977, -0.497772, -0.368317, -0.334381, -0.499390, -0.469208, -0.303558, -0.323547, -0.023285, -0.160004, 0.079132, 0.053802, -0.044830, -0.172546, -0.161713, -0.123993, -0.346863, -0.244354, -0.447479, -0.587585, -0.460052, -0.598450, -0.321564, -0.025269};

const float ourtable[512]={0.000000, 0.012296, 0.024589, 0.036879, 0.049164, 0.061441, 0.073708, 0.085965, 0.098208, 0.110437, 0.122649, 0.134842, 0.147016, 0.159166, 0.171293, 0.183394, 0.195467, 0.207511, 0.219523, 0.231502, 0.243446, 0.255353, 0.267222, 0.279050, 0.290836, 0.302578, 0.314275, 0.325923, 0.337523, 0.349071, 0.360567, 0.372008, 0.383393, 0.394720, 0.405988, 0.417194, 0.428337, 0.439415, 0.450426, 0.461370, 0.472244, 0.483046, 0.493776, 0.504430, 0.515009, 0.525509, 0.535931, 0.546271, 0.556528, 0.566702, 0.576789, 0.586790, 0.596702, 0.606524, 0.616253, 0.625890, 0.635432, 0.644878, 0.654227, 0.663477, 0.672626, 0.681674, 0.690618, 0.699458, 0.708193, 0.716820, 0.725339, 0.733748, 0.742047, 0.750233, 0.758306, 0.766264, 0.774106, 0.781832, 0.789439, 0.796926, 0.804293, 0.811539, 0.818662, 0.825661, 0.832535, 0.839284, 0.845905, 0.852399, 0.858764, 0.864999, 0.871103, 0.877076, 0.882916, 0.888622, 0.894194, 0.899631, 0.904932, 0.910096, 0.915122, 0.920010, 0.924759, 0.929369, 0.933837, 0.938165, 0.942350, 0.946394, 0.950294, 0.954050, 0.957662, 0.961130, 0.964452, 0.967628, 0.970658, 0.973541, 0.976278, 0.978866, 0.981306, 0.983599, 0.985742, 0.987736, 0.989581, 0.991277, 0.992822, 0.994218, 0.995463, 0.996558, 0.997502, 0.998295, 0.998937, 0.999428, 0.999768, 0.999958, 0.999995, 0.999882, 0.999617, 0.999202, 0.998635, 0.997917, 0.997049, 0.996029, 0.994859, 0.993539, 0.992068, 0.990448, 0.988678, 0.986758, 0.984689, 0.982471, 0.980105, 0.977590, 0.974928, 0.972118, 0.969162, 0.966058, 0.962809, 0.959414, 0.955874, 0.952190, 0.948362, 0.944390, 0.940275, 0.936019, 0.931620, 0.927081, 0.922402, 0.917584, 0.912626, 0.907531, 0.902298, 0.896929, 0.891425, 0.885785, 0.880012, 0.874106, 0.868067, 0.861898, 0.855598, 0.849168, 0.842611, 0.835925, 0.829114, 0.822177, 0.815116, 0.807931, 0.800625, 0.793197, 0.785650, 0.777984, 0.770200, 0.762299, 0.754284, 0.746154, 0.737912, 0.729558, 0.721093, 0.712520, 0.703839, 0.695051, 0.686159, 0.677162, 0.668064, 0.658864, 0.649565, 0.640167, 0.630673, 0.621083, 0.611400, 0.601624, 0.591757, 0.581801, 0.571756, 0.561626, 0.551410, 0.541111, 0.530730, 0.520269, 0.509729, 0.499112, 0.488420, 0.477654, 0.466816, 0.455907, 0.444929, 0.433884, 0.422773, 0.411598, 0.400361, 0.389064, 0.377708, 0.366295, 0.354826, 0.343304, 0.331729, 0.320105, 0.308432, 0.296713, 0.284948, 0.273141, 0.261292, 0.249404, 0.237478, 0.225517, 0.213521, 0.201493, 0.189434, 0.177347, 0.165233, 0.153094, 0.140932, 0.128748, 0.116545, 0.104325, 0.092088, 0.079838, 0.067576, 0.055303, 0.043022, 0.030735, 0.018443, 0.006148, -0.006148, -0.018443, -0.030735, -0.043022, -0.055303, -0.067576, -0.079838, -0.092088, -0.104325, -0.116545, -0.128748, -0.140932, -0.153094, -0.165233, -0.177347, -0.189434, -0.201493, -0.213521, -0.225517, -0.237479, -0.249404, -0.261293, -0.273141, -0.284949, -0.296713, -0.308432, -0.320105, -0.331730, -0.343304, -0.354826, -0.366295, -0.377708, -0.389064, -0.400362, -0.411599, -0.422773, -0.433884, -0.444929, -0.455907, -0.466816, -0.477654, -0.488420, -0.499112, -0.509729, -0.520269, -0.530730, -0.541111, -0.551410, -0.561626, -0.571757, -0.581801, -0.591757, -0.601624, -0.611400, -0.621084, -0.630673, -0.640168, -0.649565, -0.658864, -0.668064, -0.677163, -0.686159, -0.695051, -0.703839, -0.712520, -0.721093, -0.729558, -0.737912, -0.746154, -0.754284, -0.762299, -0.770200, -0.777984, -0.785650, -0.793197, -0.800625, -0.807932, -0.815116, -0.822177, -0.829114, -0.835926, -0.842611, -0.849168, -0.855598, -0.861898, -0.868068, -0.874106, -0.880012, -0.885786, -0.891425, -0.896929, -0.902298, -0.907531, -0.912626, -0.917584, -0.922402, -0.927082, -0.931621, -0.936019, -0.940275, -0.944390, -0.948362, -0.952190, -0.955874, -0.959414, -0.962809, -0.966058, -0.969162, -0.972118, -0.974928, -0.977590, -0.980105, -0.982471, -0.984689, -0.986758, -0.988678, -0.990448, -0.992068, -0.993539, -0.994859, -0.996029, -0.997049, -0.997917, -0.998635, -0.999202, -0.999617, -0.999882, -0.999995, -0.999958, -0.999768, -0.999428, -0.998937, -0.998295, -0.997502, -0.996558, -0.995463, -0.994218, -0.992822, -0.991277, -0.989581, -0.987736, -0.985742, -0.983599, -0.981306, -0.978866, -0.976277, -0.973541, -0.970658, -0.967628, -0.964452, -0.961130, -0.957662, -0.954050, -0.950294, -0.946394, -0.942350, -0.938165, -0.933837, -0.929368, -0.924759, -0.920010, -0.915122, -0.910096, -0.904932, -0.899631, -0.894194, -0.888622, -0.882915, -0.877076, -0.871103, -0.864999, -0.858764, -0.852399, -0.845905, -0.839284, -0.832535, -0.825661, -0.818662, -0.811539, -0.804293, -0.796926, -0.789438, -0.781831, -0.774106, -0.766264, -0.758306, -0.750233, -0.742047, -0.733748, -0.725339, -0.716820, -0.708193, -0.699458, -0.690618, -0.681673, -0.672626, -0.663476, -0.654227, -0.644878, -0.635432, -0.625890, -0.616253, -0.606523, -0.596702, -0.586790, -0.576789, -0.566702, -0.556528, -0.546271, -0.535930, -0.525509, -0.515009, -0.504430, -0.493775, -0.483046, -0.472244, -0.461370, -0.450426, -0.439414, -0.428336, -0.417193, -0.405988, -0.394720, -0.383393, -0.372008, -0.360567, -0.349071, -0.337523, -0.325923, -0.314274, -0.302578, -0.290836, -0.279050, -0.267222, -0.255353, -0.243446, -0.231502, -0.219523, -0.207511, -0.195467, -0.183394, -0.171293, -0.159166, -0.147015, -0.134842, -0.122649, -0.110437, -0.098208, -0.085965, -0.073708, -0.061440, -0.049163, -0.036879, -0.024589, -0.012295, 0.000000};



void main(void)
{
  int16_t x;
  // all generator inits
LPCAnalyzer_init();
init_synth(); // which one? --> klatt rsynth !!!! RENAME!
sp0256_init();
lpc_init(); 
simpleklatt_init();
sam_init();
sam_newsay(); // TEST!
tms5200_init();
tms5200_newsay();
 channelv_init();
 tube_init();
// tube_newsay();
 BANDS_Init_();
 Vocoder_Init(32000.0f);
 digitalk_init();
 digitalk_newsay(0);
 nvp_init();
 sample_rate_init();
 initbraidworm(); // re_name
  initvoicform();
  formy=malloc(sizeof(Formlet));
  formanty=malloc(sizeof(Formant));
  blipper=malloc(sizeof(Blip));
  RLPFer=malloc(sizeof(RLPF));
  Formlet_init(formy);
  Formant_init(formanty);
  Blip_init(blipper);
  RLPF_init(RLPFer);
  NTube_init(&tuber);
  wavetable_init(&wavtable, crowtable, 142); // now last arg as length of table
  addwormsans(&myworm, 10.0f,10.0f,200.0f, 200.0f, wanderworm);

 ////////
  ADC1_Init((uint16_t *)adc_buffer);
  Codec_Init(32000); 
  I2S_Block_Init();
  I2S_Block_PlayRec((uint32_t)&tx_buffer, (uint32_t)&rx_buffer, BUFF_LEN);
  //  Audio_Init(); not needed


  //  tube_init();
    //    tube_newsay();

    //    initializeSynthesizer();
      //   synthesize();

  //  lpc_newsay(1);
  //  SAMINIT();
  // test audio fill
    /*          for (x=0;x<32768;x++){
	  audio_buffer[x]=tube_get_sample();
	  }*/
   // writepos=run_holmes(writepos); 

  /*  for (x=0;x<32767;x++){
	    audio_buffer[x]=tube_get_sample();
	    }
  */

  while(1)
    {
  
            
  // testing changing test_elm
      u8 axis=adc_buffer[SELX]>>8; // 16*3=48
      // change element, change length? leave stress as is 0
      test_elm[axis*3]=phoneme_prob_remap[adc_buffer[SELY]>>6]; // how many phonemes?=64
      test_elm[(axis*3)+1]=(adc_buffer[SELZ]>>7)+1; // length say max 32
    

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
