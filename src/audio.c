/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

/*- LACH knobs left/right from top (pcb and lach=test) = 
0/mode-(2..3)
1/selX.egX-(3..2) 
2/speed-(0..4) 
3/end-(4..1)
4/trigthresh/vocoderfreq/othersel.eg/selY-(1..0)*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#include "audio.h"
#include "effect.h"
#include "klatt_phoneme.h"
//#include "mdavocoder.h"
#include "vocode.h"
#include "vocoder/vocode.h"
#include "scformant.h"
#include "braidworm.h"
#include "voicform.h"
#include "lpcansc.h"
#include "parwave.h"
#include "nvp.h"
#include "lpc.h"
#include "saml.h"
#include "holmes.h"
#include "sp0256.h"
#include "biquad.h"
#include "tms5200x.h"
#include "tube.h"
#include "channelv.h"
#include "svf.h"
#include "wvocoder.h"

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

arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate

extern __IO uint16_t adc_buffer[10];
extern int16_t* buf16;
//extern u8* datagenbuffer;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], sample_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

//for vocoder
//int16_t modulator_sample_buffer[256];
//int16_t carrier_sample_buffer[256];
//int16_t output_sample_buffer[256];

#define float float32_t

float samplepos=0;//=genstruct->samplepos;
int16_t samplel;//=genstruct->lastsample;
int16_t lastval;//=genstruct->prevsample;

//extern mdavocal mdavocall;
//extern mdavocoder mdavocod;
extern VocoderInstance* vocoderr;

/*
Formlet *formy;
Formant *formanty;
*/

#define THRESH 32000
#define THRESHLOW 30000

u16 sp0256(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // MODEL GENERATOR: TODO is speed and interpolation options DONE
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=UPSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=sp0256_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 return size;
};

u16 tubes(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // MODEL GENERATOR: TODO is speed and interpolation options DONE
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=UPSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tube_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=tube_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 return size;
};


u16 sammy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  //  float xx,xxx;
  //  float tmpbuffer[32];

  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate
//samplespeed=1.0f;

     // test lowpass/biquad:
     /*
     int_to_floot(outgoing,tmpbuffer,32);
    for (x=0;x<32;x++){
    xxx=tmpbuffer[x];
    xx=BiQuad(xxx,newB); 
    tmpbuffer[x]=xx;
      }
    floot_to_int(outgoing,tmpbuffer,32);
     */

 
   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (x<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[x]=samplel;

       // TEST trigger: 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (x<size){
       while (howmany==0)	 howmany=(sam_get_sample(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       // TEST trigger: 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
       }

  // refill back counter etc.
 return size;
}

u16 tms5220talkie(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // MODEL GENERATOR: TODO is speed and interpolation options
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(lpc_get_sample()<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 lpc_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=(lpc_get_sample()<<6)-32768; 

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 lpc_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 return size;
};

float flinbuffer[MONO_BUFSZ];
float flinbufferz[MONO_BUFSZ];
float floutbuffer[MONO_BUFSZ];
float floutbufferz[MONO_BUFSZ];

u16 fullklatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params SELZ is pitch bend commentd now OUT
  //  klattrun(outgoing, size);
  u8 xx=0;
     while (xx<size){
       outgoing[xx]=klatt_get_sample();
       xx++;
     }
};

u16 tms5200mame(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // MODEL GENERATOR: TODO is speed and interpolation options
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(tms5200_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tms5200_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=(tms5200_get_sample());//<<6)-32768; 

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tms5200_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 return size;
};



u16 simpleklatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params - break down as above in tms, spo256 - pull out size
  dosimpleklattsamples(outgoing, size);
};


u16 channelv(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params - break down as above in tms, spo256 - pull out size
  dochannelv(incoming,outgoing, size);
};

// testing various vocoder and filter implementations:

u16 testvoc(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params - break down as above in tms, spo256 - pull out size
//  dochannelv(incoming,outgoing, size);
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32];
  int_to_floot(incoming,voicebuffer,size);
  dochannelvexcite(carrierbuffer,32);

  //  runVocoder(vocoderr, voicebuffer, carrierbuffer, otherbuffer, size);
  //void runSVFtest_(SVF* svf, float* incoming, float* outgoing, u8 band_size){
  //  runBANDStest_(voicebuffer, otherbuffer, size);

  Vocoder_Process(voicebuffer, carrierbuffer, otherbuffer, size);

  floot_to_int(outgoing,otherbuffer,size);
};



//u16 LPCanalyzer(genny* genstruct, int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int testE, float delta, int inNumSamples) {
  // convert in to float
  // exciter=indriver to float
  /* for (u8 x=0;x<size;x++){
	    flinbufferz[x]=(float)((rand()%65536)-32768)/32768.0f;
	    }*/
//        int_to_floot(incoming,flinbuffer,size);
//	LPCAnalyzer_next(NULL, flinbuffer, floutbuffer, 10, size); //poles=10 - CROW TEST!
    // out from float to int
//    floot_to_int(mono_buffer,floutbuffer,size);
//};




void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *(src++);
		sz--;
		*(rdst++) = *(src++);
		//		*(rdst++) = 0;
		sz--;
	}
}

inline void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}

/*extern u8 trigger;
extern u16 generated;
extern u16 writepos;
extern u8 mode; /// ????? TODO do we need these?
*/

u8 mode;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  //  static u8 framecount=0, tmpcount=0;
  //  static float eaten=0;
  //  static float samplepos=0.0f; 
  float samplespeed;
  //  static float samplepos=0.0f;
  u8 x;
  //  u16 readpos;
  u8 speedy;
  //  float xx,yy;

  //  u16 ending=AUDIO_BUFSZ;
  speedy=(adc_buffer[SPEED]>>6)+1;
  samplespeed=4.0f/(float)(speedy); // what range this gives? - 10bits>>6=4bits=16 so 8max skipped and half speed

  //  samplespeed=1.0f;

  // splitting input

  for (x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  mode=8; // checked=0,1,2,3,4,5,6 ALL 

  u16 (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={tms5220talkie,fullklatt,sp0256,simpleklatt,sammy,tms5200mame,tubes, channelv,testvoc};//,klatt,rawklatt,SAM,tubes,channelvocoder,vocoder};

  // we don't really need to return size?

  x=generators[mode](sample_buffer,mono_buffer,samplespeed,sz/2); 


  /*    for (x=0;x<sz/2;x++){ // STRIP_OUT
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      samplepos+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
      }*/


#ifdef TEST
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else // left is out on our WORM BOARD!
  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
#endif
}
