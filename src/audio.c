/*
 * audio.c - justttt the callback 

*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#include "audio.h"
#include "effect.h"
#include "klatt_phoneme.h"
#include "mdavocoder.h"
#include "vocode.h"
#include "vocoder/vocode.h"
#include "scformant.h"
#include "braidworm.h"
#include "voicform.h"
#include "lpcansc.h"
#include "parwave.h"
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
#include "digitalker.h"
#include "nvp.h"
#include "vosim.h"
#include "samplerate.h"
#include "ntube.h"
#include "wavetable.h"
#include "worming.h"
#include "vot.h"
//#include "raven.h"

/*
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

*/

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


arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate

extern __IO uint16_t adc_buffer[10];
extern int16_t* buf16;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], sample_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

float smoothed_adc_value[5]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // SELX, Y, Z, SPEED
static adc_transform transform[5] = {
  {MODE, 0, 0.1f, 33.0f}, // only multiplier we use except speed!
  {SELX, 0, 0.1f, 32.0f},
  {SELY, 0, 0.1f, 32.0f},
  {SELZ, 0, 0.1f, 32.0f},
  {SPEED, 1, 0.1f, 8.0f} // what was former speed range?
};

extern unsigned char m_rome[256];
extern float exy[64];
float _mode, _speed, _selx, _sely, _selz;
u8 _intspeed, _intmode=0, trigger=0;
u8 TTS=0;

enum adcchannel {
  MODE_,
  SELX_,
  SELY_,
  SELZ_,
  SPEED_
};

#define float float32_t

float samplepos=0;//=genstruct->samplepos;
int16_t samplel;//=genstruct->lastsample;
int16_t lastval;//=genstruct->prevsample;

#define THRESH 16000
#define THRESHLOW 10000

// for TTS

static const unsigned char mapytoascii[]  __attribute__ ((section (".flash"))) ={32, 32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122}; // total 64 and starts with 2 spaces SELY=0-63

char TTSinarray[65];

/// start with SP0256 sets

void sp0256(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // NEW model - ALLOPHONES
  TTS=0;
  // added trigger
  if (trigger==1) sp0256_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=sp0256_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
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
};

void sp0256_1219(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
TTS=0;

  // adding trigger
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=sp0256_get_sample1219();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsay1219(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample1219();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsay1219(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void sp0256TTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=1;
  if (trigger==1) sp0256_retriggerTTS(); // selector is in newsay
    
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sampleTTS();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 //	  sp0256_newsayTTS(); // selector is in newsay
	 sp0256_retriggerTTS();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sampleTTS();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 //	 sp0256_newsayTTS(); // selector is in newsay
	 sp0256_retriggerTTS();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void sp0256vocabone(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  if (trigger==1) sp0256_newsayvocabbankone(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_samplevocabbankone();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	  sp0256_newsayvocabbankone(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_samplevocabbankone();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsayvocabbankone(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void sp0256vocabtwo(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  if (trigger==1) sp0256_newsayvocabbanktwo(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_samplevocabbanktwo();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	  sp0256_newsayvocabbanktwo(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_samplevocabbanktwo();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsayvocabbanktwo(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void sp0256raw(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  // write into m_rome
    u16 xaxis=_selx*257.0f;
  MAXED(xaxis,255);
  xaxis=255-xaxis;
  u16 val=(_sely*257.0f);
  MAXED(val,255);
  val=255-val;
  m_rome[xaxis]=val; 
    
  if (trigger==1) sp0256_newsayraw(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sampleraw();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	  sp0256_newsayraw(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sampleraw();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsayraw(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void sp0256bend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  u8 xaxis=_selx*14.0f; //0-16 for r, but now 14 params
  MAXED(xaxis,13);
  xaxis=13-xaxis;
  exy[xaxis]=1.0f-_sely; // no multiplier and inverted here
    
  if (trigger==1) sp0256_newsaybend(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_samplebend();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	  sp0256_newsaybend(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_samplebend();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 sp0256_newsaybend(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

u8 toggled=1;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  float samplespeed;
  static u16 cc;
  u8 oldmode=255;
  
  for (u8 x=0;x<5;x++){
  float value=(float)adc_buffer[transform[x].whichone]/65536.0f; 

  if (transform[x].flip) {
      value = 1.0f - value;
    }
  smoothed_adc_value[x] += transform[x].filter_coeff * (value - smoothed_adc_value[x]);
  }

  _speed=smoothed_adc_value[SPEED_];
  CONSTRAIN(_speed,0.0f,1.0f);
  _selx=smoothed_adc_value[SELX_];
  CONSTRAIN(_selx,0.0f,1.0f);
  _sely=smoothed_adc_value[SELY_];
  CONSTRAIN(_sely,0.0f,1.0f);
  _selz=smoothed_adc_value[SELZ_];
  CONSTRAIN(_selz,0.0f,1.0f);
  _mode=smoothed_adc_value[MODE_];
  CONSTRAIN(_mode,0.0f,1.0f);
  oldmode=_intmode;
  _intmode=_mode*transform[MODE_].multiplier; //0=32 CHECKED!
  MAXED(_intmode, 31);
  trigger=0;
  _intmode=6; // check extents
  if (oldmode!=_intmode) trigger=1; 
  samplespeed=_speed*transform[SPEED_].multiplier;

  // splitting input
  for (u8 x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256raw, sp0256bend}; // 0-6 modes

  generators[_intmode](sample_buffer,mono_buffer,samplespeed,sz/2); 

  // copy sample buffer into audio_buffer as COMPOST
  if (!toggled){
  for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc]=mono_buffer[x];
    cc++;
    if (cc>AUDIO_BUFSZ) cc=0;
  }
  }

  // remapping for TTS modes - TODO!
  if (TTS){
    u8 selx=_selx*65.0f; // TODO!
    u8 sely=_sely*65.0f;
    MAXED(selx,63);
    MAXED(sely,63);
    selx=63-selx;
    sely=63-sely;
    TTSinarray[selx]=mapytoascii[sely];
  }

#ifdef TEST
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else // left is out on our WORM BOARD!
  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
#endif
}
