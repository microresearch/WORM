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
#include "resources.h"
//#include "raven.h"


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
  {SPEED, 1, 0.1f, 1027.0f} 
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

char TTSinarray[17];

///[[[[[[[[[[[[[[[[[[[[[[[[ TMS - lots of vocabs to handle - is it 8KHz = *0.25f - seems OK

void tms(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
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

void tmsphon(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay_allphon(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_allphon();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_allphon(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=tms_get_sample_allphon();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_allphon(); // selector is in newsay
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

void tmsTTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=1;
  if (trigger==1) tms_retriggerTTS(); 

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_TTS();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_retriggerTTS(); 
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=tms_get_sample_TTS();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_retriggerTTS(); 
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

void tmsbendlength(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_bendlength();
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
              samplel=tms_get_sample_bendlength();
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

void tmslowbit(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay_lowbit(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_lowbit();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_lowbit(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=tms_get_sample_lowbit();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_lowbit(); // selector is in newsay
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


void tmsraw5200(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 parammode=0;
  extent tmsraw5200extent={10,12.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) tms_newsay_raw5200(); // selector is in newsay
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
              if (parammode==0){
		u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
		MAXED(xaxis,tmsraw5200extent.max);
		xaxis=tmsraw5200extent.max-xaxis;
		exy[xaxis]=_sely;
	      }
	      if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tms_get_sample_raw5200();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              if (parammode==0){
         u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,tmsraw5200extent.max);
	 xaxis=tmsraw5200extent.max-xaxis;
	 exy[xaxis]=_sely; 
	      }
	 samplel=tms_get_sample_raw5200();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
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

void tmsraw5220(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 parammode=0;
  extent tmsraw5220extent={10,12.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) tms_newsay_raw5220(); // selector is in newsay
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
              if (parammode==0){
		u8 xaxis=_selx*tmsraw5220extent.maxplus; //0-16 for r, but now 14 params 0-13
		MAXED(xaxis,tmsraw5220extent.max);
		xaxis=tmsraw5220extent.max-xaxis;
		exy[xaxis]=_sely;
	      }
	      if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tms_get_sample_raw5220();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              if (parammode==0){
         u8 xaxis=_selx*tmsraw5220extent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,tmsraw5220extent.max);
	 xaxis=tmsraw5220extent.max-xaxis;
	 exy[xaxis]=_sely; 
	      }
	 samplel=tms_get_sample_raw5220();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
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


void tmsraw5100(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 parammode=0;
  extent tmsraw5100extent={10,12.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) tms_newsay_raw5100(); // selector is in newsay
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
              if (parammode==0){
		u8 xaxis=_selx*tmsraw5100extent.maxplus; //0-16 for r, but now 14 params 0-13
		MAXED(xaxis,tmsraw5100extent.max);
		xaxis=tmsraw5100extent.max-xaxis;
		exy[xaxis]=_sely; // no multiplier and inverted here
	      }
	      if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tms_get_sample_raw5100();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              if (parammode==0){
         u8 xaxis=_selx*tmsraw5100extent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,tmsraw5100extent.max);
	 xaxis=tmsraw5100extent.max-xaxis;
	 exy[xaxis]=_sely; // no multiplier and inverted here
	      }
	 samplel=tms_get_sample_raw5100();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 parammode^=1;
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

void tmsbend5200(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  extent tmsraw5200extent={11,13.0f};//11 here
  if (trigger==1) tms_newsay_allphon(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_bend5200();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_allphon(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       samplel=tms_get_sample_bend5200();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_allphon(); // selector is in newsay
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

void tms5100ktablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){  // vocab=0
  TTS=0;
  extent tableextent={167,170.0f};//168 for LPC table here = 0-167
  if (trigger==1) tms_newsay_specific(0); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5100ktable();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(0); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5100ktable();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(0); // selector is in newsay
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

void tms5100pitchtablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){  // vocab=0
  TTS=0;
  extent tableextent={31,33.0f};//11 here
  if (trigger==1) tms_newsay_specific(0); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5100pitchtable();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(0); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5100pitchtable();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(0); // selector is in newsay
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


///[[[[[[[[[[[[[[[[[[[[[[[[SP0256 10 KHz so -   samplespeed/=3.2;

void sp0256(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // NEW model - ALLOPHONES
  TTS=0;
  if (trigger==1) sp0256_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
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
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay
  samplespeed/=3.2;

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
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
   if (samplespeed<=1){ 
     while (xx<size){
       u8 xaxis=_selx*sp0256bendextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,sp0256bendextent.max);
       xaxis=sp0256bendextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
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
	 exy[xaxis]=_sely; // no multiplier and inverted here

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

///[[[[[[[[[[[[[[[[[[[[[[[[VOTRAX 20KHz =

void votrax(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
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
  TTS=0;
  if (trigger==1) votrax_newsaygorf(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*2.0f;

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

void votraxwow(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // 40 KHZ = 
  TTS=0;
  if (trigger==1) votrax_newsaywow(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*1.25f;

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

void votrax_param(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  extent votraxparamextent={6,7.0f};
  TTS=0;
  //  if (trigger==1) votrax_newsay_rawparam(); // selector is in newsay
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (parammode==0){
	 u8 xaxis=_selx*votraxparamextent.maxplus; // 9 params 0-8 - as int rounds down
	 MAXED(xaxis,votraxparamextent.max);
	 xaxis=votraxparamextent.max-xaxis;
	 exy[xaxis]=_sely; 
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
	 MAXED(xaxis,votraxparamextent.max); 
	 xaxis=votraxparamextent.max-xaxis;
	 exy[xaxis]=_sely; 
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
  TTS=0;
  if (trigger==1) votrax_newsay_bend(1); // selector is in newsay
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
	 u8 xaxis=_selx*votraxbendextent.maxplus; // 8 params 
	 MAXED(xaxis,votraxbendextent.max); 
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
	 MAXED(xaxis,votraxbendextent.max);
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

///[[[[[ good for testing ADC/CV ins

void test_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // how we choose the wavetable - table of tables?
  float lastbuffer[32];
  uint16_t val;
  val=_selz*1030.0f;
  MAXED(val,1023);
  val=1023-val;
  dowavetable(lastbuffer, &wavtable, 2.0f+(logspeed[val]*440.0f), size); // for exp/1v/oct test
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
  u16 samplespeedref;
  static u16 cc;
  u8 oldmode=255;
  static u8 firsttime=0;
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
 _intmode=9; 
 // if (oldmode!=_intmode) trigger=1; // for now this is never/always called TEST
 if (firsttime==0){// TEST CODE - for fake trigger
   trigger=1;
   firsttime=1;
 }
 
  samplespeedref=_speed*transform[SPEED_].multiplier;
  MAXED(samplespeedref, 1023);
  samplespeed=logspeed[samplespeedref];  
  
  // splitting input
  for (u8 x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  //  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, votrax, votraxTTS, votraxgorf, votraxwow, votrax_param, votrax_bend, lpc_error, test_wave, LPCanalyzer}; // sp0256: 0-5 modes, votrax=6 - was with LPCanalyzer

  // above is for raven

  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, votrax, votraxTTS, votraxgorf, votraxwow, votrax_param, votrax_bend, test_wave, tms, tmsphon, tmsTTS, tmsbendlength, tmslowbit, tmsraw5100, tmsraw5200, tmsraw5220, tmsbend5200, tms5100pitchtablebend, tms5100ktablebend}; // sp0256: 0-5 modes, votrax=6 - 
  //0sp0256, 1sp0256TTS, 2sp0256vocabone, 3sp0256vocabtwo, 4sp0256_1219, 5sp0256bend, /// 6votrax, 7votraxTTS, 8votraxgorf, 9votraxwow, 10votrax_param, 11votrax_bend, 12test_wave, 13tms, 14tmsphone,15tmsTTS, 16tmsbendlength, 17tmslowbit, 18tmsraw5100, 19tmsraw5200, 20tmsraw5220, 21tmsbend5200, 22tms5100pitchtablebend, 23tms5100ktablebend

  generators[_intmode](sample_buffer,mono_buffer,samplespeed,sz/2); 

  // copy sample buffer into audio_buffer as COMPOST
  if (!toggled){
  for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc]=mono_buffer[x];
    cc++;
    if (cc>AUDIO_BUFSZ) cc=0;
  }
  }

    if (TTS){
    u8 xax=_sely*18.0f; 
    u8 selz=_selz*65.0f; 
    MAXED(xax,16);
    MAXED(selz,63);
    xax=16-xax; // inverted
    selz=63-selz;
    TTSinarray[xax]=mapytoascii[selz];
    }

#ifdef TEST
  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);
#else // left is out on our WORM BOARD!
  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
#endif
}
