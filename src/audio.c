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
  {MODE, 0, 0.1f, 33.0f}, // only multiplier we use except speed! was 0.1 for all!
  {SELX, 0, 0.1f, 32.0f},
  {SELY, 0, 0.1f, 32.0f},
  {SELZ, 0, 0.1f, 32.0f},
  {SPEED, 1, 0.1f, 8.0f} // what was former speed range?
};



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

Wavetable wavtable;
wormy myworm;


// for TTS

static const unsigned char mapytoascii[]  __attribute__ ((section (".flash"))) ={32, 32, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122}; // total 64 and starts with 2 spaces SELY=0-63

char TTSinarray[65];

///[[[[[[[[[[[[[[[[[[[[[[[[ TMS - lots of vocabs to handle

void tms(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // NEW model - ALLOPHONES
  TTS=0;
  // added trigger
  if (trigger==1) tms_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=tms_get_sample();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay(); // selector is in newsay
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


///[[[[[[[[[[[[[[[[[[[[[[[[SP0256

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
  if (trigger==1) sp0256_newsayvocabbankone(1); // selector is in newsay
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
	  sp0256_newsayvocabbankone(1); // selector is in newsay
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
	 sp0256_newsayvocabbankone(1); // selector is in newsay
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
  if (trigger==1) sp0256_newsayvocabbanktwo(1); // selector is in newsay
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
	  sp0256_newsayvocabbanktwo(1); // selector is in newsay
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
	 sp0256_newsayvocabbanktwo(1); // selector is in newsay
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
  extent sp0256bendextent={13,14.0f};
  TTS=0;
    
  if (trigger==1) sp0256_newsaybend(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed/=8.0;
   if (samplespeed<=1){ 
     while (xx<size){
       u8 xaxis=_selx*sp0256bendextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,sp0256bendextent.max);
       xaxis=sp0256bendextent.max-xaxis;
       exy[xaxis]=1.0f-_sely; // no multiplier and inverted here
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
         u8 xaxis=_selx*sp0256bendextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,sp0256bendextent.max);
	 xaxis=sp0256bendextent.max-xaxis;
	 exy[xaxis]=1.0f-_sely; // no multiplier and inverted here

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

///[[[[[[[[[[[[[[[[[[[[[[[[VOTRAX

void votrax(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  samplespeed*=0.5f;
  TTS=0;
  if (trigger==1) votrax_newsay(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=votrax_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay();
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

void votraxTTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  samplespeed*=0.5f;

  TTS=1;
  if (trigger==1) votrax_retriggerTTS(); // selector is in newsay
    
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sampleTTS();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_retriggerTTS();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=votrax_get_sampleTTS();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_retriggerTTS();
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

void votraxgorf(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  samplespeed*=0.5f;

  TTS=0;
  if (trigger==1) votrax_newsaygorf(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_samplegorf();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaygorf(1);
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=votrax_get_samplegorf();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaygorf(1);
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

void votraxwow(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  samplespeed*=0.5f;

  TTS=0;
  if (trigger==1) votrax_newsaywow(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_samplewow();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaywow(1);
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=votrax_get_samplewow();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaywow(1);
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

void votrax_param(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // NOTES - needs trigger but is this best way?
  extent votraxparamextent={6,7.0f};
  samplespeed*=0.5f;
  TTS=0;
  //  if (trigger==1) votrax_newsay_rawparam(); // selector is in newsay
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (parammode==0){
	 u8 xaxis=_selx*votraxparamextent.maxplus; // 9 params 0-8 - as int rounds down
	 MAXED(xaxis,votraxparamextent.max); // how can we test the extent for the CV in
	 xaxis=votraxparamextent.max-xaxis;
	 exy[xaxis]=1.0f-_sely; // no multiplier and inverted here
       }

       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sample_rawparam();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 triggered=1;
	 parammode^=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){

       if (parammode==0){
	 u8 xaxis=_selx*votraxparamextent.maxplus; 
	 MAXED(xaxis,votraxparamextent.max); // how can we test the extent for the CV in
	 xaxis=votraxparamextent.max-xaxis;
	 exy[xaxis]=1.0f-_sely; // no multiplier and inverted here
       }

              samplel=votrax_get_sample_rawparam();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 triggered=1;
	 parammode^=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};


void votrax_bend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  extent votraxbendextent={8,9.0f};
  samplespeed*=0.5f;
  TTS=0;
  if (trigger==1) votrax_newsay_bend(1); // selector is in newsay
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
	 u8 xaxis=_selx*votraxbendextent.maxplus; // 8 params 
	 MAXED(xaxis,votraxbendextent.max); // how can we test the extent for the CV in
	 xaxis=votraxbendextent.max-xaxis;
	 exy[xaxis]=_sely; // no multiplier and NOT inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sample_bend();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 triggered=1;
	 votrax_newsay_bend(1); // selector is in newsay
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
	 u8 xaxis=_selx*votraxbendextent.maxplus; //
	 MAXED(xaxis,votraxbendextent.max); // how can we test the extent for the CV in
	 xaxis=votraxbendextent.max-xaxis;
	 exy[xaxis]=_sely; // no multiplier and NOT inverted here
              samplel=votrax_get_sample_bend();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 triggered=1;
	 votrax_newsay_bend(1); // selector is in newsay
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};


////////////////////[[[[[[[[[[ TESTING LPC residual as excitation for sp0256

// this works but quite high volume

void lpc_error(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32], lastbuffer[32];
//(DelayN.ar(input,delaytime, delaytime)- LPCAnalyzer.ar(input,source,1024,MouseX.kr(1,256))).poll(10000)
//  do_impulse(carrierbuffer, 32, adc_buffer[SELX]>>2);
//  dowormwavetable(carrierbuffer, &wavtable, adc_buffer[SELX], size);
  int_to_floot(incoming,voicebuffer,size);
  //  LPC_cross(voicebuffer,carrierbuffer, lastbuffer,size);
  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  //  NTube_do(&tuber, otherbuffer, lastbuffer, 32);
    floot_to_int(outgoing,lastbuffer,size);
};

void sp0256_within_noLPC(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  TTS=0;
  // added trigger
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay

  float voicebuffer[32],lastbuffer[32];
  int16_t outgo[32];
  //  int_to_floot(incoming,voicebuffer,size);
  //  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  //  floot_to_int(outgo,voicebuffer,size);
  for (u8 x=0;x<size;x++) outgo[x]=(incoming[x])>>4; // say 11 bits

    u8 xx=0,x=0,readpos;
  float remainder;
    samplespeed/=8.0;
  //  samplespeed=1.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample_withLPC(outgo[xx]); // check this!
	 //	 	 samplel=outgo[xx];
	 samplepos-=1.0f;
	        }
       remainder=samplepos; 
       //       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample_withLPC(outgo[xx]);
	      //	 samplel=outgo[xx];

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
     }
};

void sp0256_within(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 

  TTS=0;
  // added trigger
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay

  float voicebuffer[32],lastbuffer[32];
  int16_t outgo[32];
  int_to_floot(incoming,voicebuffer,size);
  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  floot_to_int(outgo,lastbuffer,size);
  for (u8 x=0;x<size;x++) outgo[x]=(outgo[x])>>4; // say 11 bits

    u8 xx=0,x=0,readpos;
  float remainder;
    samplespeed/=8.0;
  //  samplespeed=1.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample_withLPC(outgo[x++]); // check this!
	 //	 	 samplel=outgo[xx];
	 samplepos-=1.0f;
	        }
       remainder=samplepos; 
       //       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample_withLPC(outgo[xx]);
	      //	 samplel=outgo[xx];

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
     }
};


///[[[[[ good for testing ADC/CV ins

void test_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // how we choose the wavetable - table of tables?
  float lastbuffer[32];
  //  dowavetable(lastbuffer, &wavtable, 2, size);
  //  dowavetable(lastbuffer, &wavtable, 2+(1024-(adc_buffer[SELZ]>>2)), size);

  dowavetable(lastbuffer, &wavtable, 2.0f + (1024.0f*(1.0f-_sely)), size);
  floot_to_int(outgoing,lastbuffer,size);
}  

//////////////[[[ crow/raven/LPC

/*
void LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int inNumSamples);

void LPCanalyzer(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float voicebuffer[32],otherbuffer[32];
       int_to_floot(incoming,voicebuffer,size);
       LPCAnalyzer_next(NULL, voicebuffer, otherbuffer, 10, size); //poles=10 - CROW TEST!
   floot_to_int(mono_buffer,otherbuffer,size);
};
*/

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
  //  smoothed_adc_value[x] = value;
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
  _intmode=0; 
  if (oldmode!=_intmode) trigger=1; 
  samplespeed=_speed*transform[SPEED_].multiplier;

  // splitting input
  for (u8 x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  //  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, votrax, votraxTTS, votraxgorf, votraxwow, votrax_param, votrax_bend, lpc_error, sp0256_within, test_wave, LPCanalyzer}; // sp0256: 0-5 modes, votrax=6 - was with LPCanalyzer

  // above is for raven
  // test_wave is just for testing purposes

  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, votrax, votraxTTS, votraxgorf, votraxwow, votrax_param, votrax_bend, lpc_error, sp0256_within_noLPC, sp0256_within, test_wave, tms}; // sp0256: 0-5 modes, votrax=6 - 
  //0sp0256, 1sp0256TTS, 2sp0256vocabone, 3sp0256vocabtwo, 4sp0256_1219, 5sp0256bend, 6votrax, 7votraxTTS, 8votraxgorf, 9votraxwow, 10votrax_param, 11votrax_bend, 12lpc_error, 13sp0256_within_noLPC, 14sp0256_within, 15test_wave, 16tms


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
    selx=63-selx; // inverted
    sely=63-sely;
    TTSinarray[selx]=mapytoascii[sely];
  }

#ifdef TEST
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else // left is out on our WORM BOARD!
  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
#endif
}
