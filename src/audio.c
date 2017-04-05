#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#ifdef TESTING
#include "audio.h"
#include "effect.h"
#include "wavetable.h"
#include "resources.h"
#else
#include "audio.h"
#include "effect.h"
#include "klatt_phoneme.h"
#include "parwave.h"
#include "sam.h"
#include "holmes.h"
#include "sp0256.h"
#include "tms5200x.h"
#include "tube.h"
#include "digitalker.h"
#include "nvp.h"
#include "ntube.h"
#include "wavetable.h"
#include "worming.h"
#include "vot.h"
#include "resources.h"
#include "rs.h"
#include "raven.h"
#include "samplerate.h"
#endif

const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?

arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate

extern __IO uint16_t adc_buffer[10];
extern int16_t* buf16;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], sample_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
extern u8 test_elm[51];
extern u8 test_elm_rsynthy[106]; // as is just phon code and length... with lat stop

float smoothed_adc_value[5]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // SELX, Y, Z, SPEED

extern float exy[64];
float _mode, _speed, _selx, _sely, _selz;
u8 _intspeed, _intmode=0, trigger=0;
u8 TTS=0;

#define float float32_t

float samplepos=0;//=genstruct->samplepos;
int16_t samplel;//=genstruct->lastsample;
int16_t lastval;//=genstruct->prevsample;

Wavetable wavtable;
#ifndef TESTING
wormy myworm;
NTube tuber;
#endif

char TTSinarray[17];

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

inline void doadc(){
  float value;
  
  value =(float)adc_buffer[SELX]/65536.0f; 
  smoothed_adc_value[2] += 0.1f * (value - smoothed_adc_value[2]);
  _selx=smoothed_adc_value[2];
  CONSTRAIN(_selx,0.0f,1.0f);

  value =(float)adc_buffer[SELY]/65536.0f; 
  smoothed_adc_value[3] += 0.1f * (value - smoothed_adc_value[3]);
  _sely=smoothed_adc_value[3];
  CONSTRAIN(_sely,0.0f,1.0f);

  value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.05f * (value - smoothed_adc_value[4]); // try to smooth it!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
}

#ifdef TESTING

void test_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // how we choose the wavetable - table of tables?
  float lastbuffer[32];
  uint16_t val;
  doadc();
  val=_selz*1030.0f; // how can we test all others????
  MAXED(val,1023);
  val=1023-val;
  dowavetable(lastbuffer, &wavtable, 2.0f+(logspeed[val]*440.0f), size); // for exp/1v/oct test
  floot_to_int(outgoing,lastbuffer,size);
}  

u8 toggled=1;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  float samplespeed;
  float value;
  u16 samplespeedref;
  static u16 cc;
  u8 oldmode=255;
  static u8 firsttime=0;
  value =(float)adc_buffer[SPEED]/65536.0f; 
  smoothed_adc_value[0] += 0.1f * (value - smoothed_adc_value[0]);
  _speed=smoothed_adc_value[0];
  CONSTRAIN(_speed,0.0f,1.0f);
  _speed=1.0f-_speed;

  value =(float)adc_buffer[MODE]/65536.0f; 
  smoothed_adc_value[1] += 0.01f * (value - smoothed_adc_value[1]); // TESTY! 0.0f for SMOOTHER mode locking
  _mode=smoothed_adc_value[1];
  CONSTRAIN(_mode,0.0f,1.0f);

  oldmode=_intmode;
  _intmode=_mode*65.0f;
  MAXED(_intmode, 63);
  trigger=0; 
  _intmode=0;
 // if (oldmode!=_intmode) trigger=1; // for now this is never/always called TEST
 if (firsttime==0){// TEST CODE - for fake trigger
   trigger=1;
   firsttime=1;
 }
 
  samplespeedref=_speed*1027.0f;
  MAXED(samplespeedref, 1023);
  samplespeed=logspeed[samplespeedref];  
  
  // splitting input
  for (u8 x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={test_wave}; 
  
  generators[_intmode](sample_buffer,mono_buffer,samplespeed,sz/2); 

  // copy sample buffer into audio_buffer as COMPOST
  if (!toggled){
  for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc]=mono_buffer[x];
    cc++;
    if (cc>AUDIO_BUFSZ) cc=0;
  }
  }

  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}

#else
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
       doadc();
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
       doadc();
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

void tmssing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_sing();
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
       doadc();
              samplel=tms_get_sample_sing();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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

void tmsphonsing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=0;
  if (trigger==1) tms_newsay_allphon(); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_allphon_sing();
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
       doadc();
              samplel=tms_get_sample_allphon_sing();
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


void tmsTTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 
  TTS=1;
  if (trigger==1) tms_retriggerTTS(); 

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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

// TODO we can add vocabs from here on but they must match 5100/5200

void tmsbend5200(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // TODO: copy/add vocabs!
  TTS=0;
  extent tmsraw5200extent={11,13.0f};//11 here
  if (trigger==1)     tms_newsay_specific(1); // TODO this will change with full vocab!
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_bend5200(1);  // TODO this will change with full vocab!
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	tms_newsay_specific(1); // TODO this will change with full vocab!
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       samplel=tms_get_sample_bend5200(1);  // TODO this will change with full vocab!
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1); // TODO this will change with full vocab!
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

void tms5100ktablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){  //  // TODO: copy/add vocabs!

  TTS=0;
  extent tableextent={167,170.0f};//168 for LPC table here = 0-167
  if (trigger==1) tms_newsay_specific(0); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5100ktable(0);
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
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5100ktable(0);
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

void tms5200ktablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){  //  // TODO: copy/add vocabs!

  TTS=0;
  extent tableextent={167,170.0f};//168 for LPC table here = 0-167
  if (trigger==1) tms_newsay_specific(1);  // TODO this will change with full vocab!

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5200ktable(0);
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1);// TODO this will change with full vocab!
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5200ktable(0);
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1);// TODO this will change with full vocab!
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


void tms5100pitchtablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){   // TODO: copy/add vocabs!
  TTS=0;
  extent tableextent={31,33.0f};//11 here
  if (trigger==1) tms_newsay_specific(0); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5100pitchtable(0);
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
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5100pitchtable(0);
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

/* // all with tms_newsay_specific - at moment 0 for 5100, 1 for 5200 to change and add*

- Xtms_get_sample_bend5100(); // 0 tmsbend5100

- Xtms_get_sample_5200pitchtable(); // vocab=1 which will change for allphons + exy = 64 - reflect this in audio.c ADD VOCABS tms5200pitchtablebend

- tms_get_sample_5100kandpitchtable(); // 0 for 5100 we have 32+168 in exy= 200 tms5100kandpitchtablebend

- tms_get_sample_5200kandpitchtable(); //  5200 is 232 tms5200kandpitchtablebend
*/

void tmsbend5100(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // TODO: copy/add vocabs!
  TTS=0;
  extent tmsraw5200extent={11,13.0f};//11 here
  if (trigger==1)     tms_newsay_specific(0);
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_bend5100(0); 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	tms_newsay_specific(0); // TODO this will change with full vocab!
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tmsraw5200extent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tmsraw5200extent.max);
       xaxis=tmsraw5200extent.max-xaxis;
       samplel=tms_get_sample_bend5100(0);  // TODO this will change with full vocab!
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(0); // TODO this will change with full vocab!
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

void tms5200pitchtablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){   // TODO: copy/add vocabs!
  TTS=0;
  extent tableextent={63,65.0f};
  if (trigger==1) tms_newsay_specific(1); // TODO this will change with full vocab!
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5200pitchtable(0);
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1); // TODO this will change with full vocab!	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5200pitchtable(0);
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1); // TODO this will change with full vocab!	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};

void tms5100kandpitchtablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){   // TODO: copy/add vocabs!
  TTS=0;
  extent tableextent={199,202.0f};//11 here
  if (trigger==1) tms_newsay_specific(0); // selector is in newsay

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5100kandpitchtable(0);
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
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5100kandpitchtable(0);
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

void tms5200kandpitchtablebend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){   // TODO: copy/add vocabs!
  TTS=0;
  extent tableextent={231,234.0f};//11 here
  if (trigger==1) tms_newsay_specific(1);  // TODO this will change with full vocab!

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       exy[xaxis]=_sely; // no multiplier and inverted here
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=tms_get_sample_5200kandpitchtable(0);
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1); // TODO this will change with full vocab!
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*tableextent.maxplus; //0-16 for r, but now 14 params 0-13
       MAXED(xaxis,tableextent.max);
       xaxis=tableextent.max-xaxis;
       samplel=tms_get_sample_5200kandpitchtable(0);
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tms_newsay_specific(1); // TODO this will change with full vocab!
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
  samplespeed/=3.2;

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

  
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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

void sp0256sing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // NEW model - ALLOPHONES
  TTS=0;
  if (trigger==1) sp0256_newsay(); // selector is in newsay
  samplespeed/=3.2;

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

  
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	  samplel=sp0256_get_sample_sing();
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
       doadc();
       samplel=sp0256_get_sample_sing();
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
       doadc();
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
       doadc();
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
  samplespeed/=3.2;
    
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
    samplespeed/=3.2;

  u8 xx=0,readpos;
  float remainder;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed/=3.2;

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed/=3.2;
    
  if (trigger==1) sp0256_newsaybend(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed=samplespeed*1.25f;

  if (trigger==1) votrax_newsay(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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

void votraxsing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  samplespeed=samplespeed*1.25f;

  if (trigger==1) votrax_newsay_sing(); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sample_sing();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay_sing();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
              samplel=votrax_get_sample_sing();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay_sing();
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
  samplespeed=samplespeed*1.25f;
  TTS=1;
  if (trigger==1) votrax_retriggerTTS(); // selector is in newsay
    
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed=samplespeed*1.25f;
  TTS=0;
  if (trigger==1) votrax_newsaygorf(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*2.0f;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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
  samplespeed=samplespeed*1.25f;
  TTS=0;
  if (trigger==1) votrax_newsaywow(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*1.25f;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();
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

void votraxwowfilterbend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // 40 KHZ = 
  samplespeed=samplespeed*1.25f;
  TTS=0;
  if (trigger==1) votrax_newsaywow_bendfilter(1); // selector is in newsay
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*1.25f;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_samplewow_bendfilter();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaywow_bendfilter(1);
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
              samplel=votrax_get_samplewow_bendfilter();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsaywow_bendfilter(1);
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
  samplespeed=samplespeed*1.25f;
  extent votraxparamextent={6,7.0f};
  TTS=0;
  //  if (trigger==1) votrax_newsay_rawparam(); // selector is in newsay
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
       doadc();

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


void votrax_bend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){// bend is gorf
  samplespeed=samplespeed*1.25f;
  extent votraxbendextent={8,9.0f};
  TTS=0;
  if (trigger==1) votrax_newsay_bend(1); // 1 is reset
  static u8 triggered=0, parammode=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
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
	 votrax_newsay_bend(1); // 1 is reset
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
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
  doadc();
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

/////// [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[ SAM 

void sam_banks0(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
 
   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_banks0(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_banks0(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       while (howmany==0)	 howmany=(sam_get_sample_banks0(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_banks0(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_banks1(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
 
   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_banks1(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_banks0(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       while (howmany==0)	 howmany=(sam_get_sample_banks1(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_banks0(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_TTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=1;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
 
   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_TTS(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_TTS(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       while (howmany==0)	 howmany=(sam_get_sample_TTS(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_TTS(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_TTSs(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=1;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
 
   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_TTSs(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_TTS(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       while (howmany==0)	 howmany=(sam_get_sample_TTSs(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_TTS(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_phon(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 parammode=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  extent samphonextent={15,18.0f};

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_phon(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; 
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }

       while (howmany==0)	 howmany=(sam_get_sample_phon(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_phons(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 parammode=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  extent samphonextent={15,18.0f};

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_phons(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; 
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }

       while (howmany==0)	 howmany=(sam_get_sample_phons(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_param(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 parammode=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  extent samphonextent={3,5.0f};

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_selx*samphonextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_sely;
	      }
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_param(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_selx*samphonextent.maxplus; 
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_sely;
	      }

       while (howmany==0)	 howmany=(sam_get_sample_param(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}


void sam_phonsing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 parammode=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  extent samphonextent={15,18.0f};

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_phonsing(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_sely*samphonextent.maxplus; 
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=_selz;
	      }

       while (howmany==0)	 howmany=(sam_get_sample_phonsing(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_bend(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 parammode=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  extent samphonextent={239,243.0f};

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_selx*samphonextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=1.0f-_sely;
	      }
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_bend(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       if (parammode==0){
	 u8 xaxis=_selx*samphonextent.maxplus; 
	 MAXED(xaxis,samphonextent.max);
	 xaxis=samphonextent.max-xaxis;
	 exy[xaxis]=1.0f-_sely;
	      }

       while (howmany==0)	 howmany=(sam_get_sample_bend(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 //	 sam_newsay_phon(); // selector is in newsay
	 parammode^=1;
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

void sam_xy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // pitch on x. speed on y
  TTS=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
 
   if (samplespeed<=1){ 
     while (x<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample_xy(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_xy(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
       while (howmany==0)	 howmany=(sam_get_sample_xy(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay_xy(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

//////////////[[[[[[[[[[[ DIGI

void digitalker(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*32.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       samplel=(digitalk_get_sample());//<<6)-32768; 
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
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

// get_sample_sing -

void digitalker_sing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*32.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample_sing());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       samplel=(digitalk_get_sample_sing());//<<6)-32768; 
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
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


// get_sample_bendpitchvals - exy to 32

void digitalker_bendpitchvals(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  float remainder;
  extent nextent={31,33.0f};
  samplespeed=samplespeed*32.0f;

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
	 u8 xaxis=_selx*nextent.maxplus; 
	 MAXED(xaxis,nextent.max);
	 xaxis=nextent.max-xaxis;
	 exy[xaxis]=_sely;
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample_bendpitchvals());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 triggered=1;
	 digitalk_newsay(); // selector is in newsay
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();

	 u8 xaxis=_selx*nextent.maxplus; 
	 MAXED(xaxis,nextent.max);
	 xaxis=nextent.max-xaxis;
	 exy[xaxis]=_sely;

	 samplel=(digitalk_get_sample_bendpitchvals());//<<6)-32768; 
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 triggered=1;
	 digitalk_newsay(); // selector is in newsay
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

///////////[[[[[[[[[[[[[[[[[[[[[ starting on klatt variants:

void simpleklatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  extent nextent={39,41.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) simpleklatt_newsay();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*nextent.maxplus;
       MAXED(xaxis,nextent.max);
       xaxis=nextent.max-xaxis;
       exy[xaxis]=_sely;
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=simpleklatt_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 simpleklatt_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
         u8 xaxis=_selx*nextent.maxplus; 
	 MAXED(xaxis,nextent.max);
	 xaxis=nextent.max-xaxis;
	 exy[xaxis]=_sely; 
	 samplel=simpleklatt_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 simpleklatt_newsay();
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

// klatt with list of phonemes in test_elm
// sel is x, y is length, z is phoneme

void klatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  extent nextent={15,17.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) klatt_newsay();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*nextent.maxplus;
       MAXED(xaxis,nextent.max); // 15*3=45 so we still leave the end
       xaxis=nextent.max-xaxis;
       u8 val=_selz*66.0f;
       MAXED(val,63);
       val=63-val;
       test_elm[xaxis*3]=phoneme_prob_remap[val]; // 64 phonemes
       test_elm[(xaxis*3)+1]=((_sely)*32.0f)+1; // length say max 32

       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       u8 xaxis=_selx*nextent.maxplus;
       MAXED(xaxis,nextent.max);
       xaxis=nextent.max-xaxis;
       u8 val=_selz*66.0f;
       MAXED(val,63);
       val=63-val;
       test_elm[xaxis*3]=phoneme_prob_remap[val]; // 64 phonemes
       test_elm[(xaxis*3)+1]=((1.0f-_sely)*33.0f)+1; // length say max 32

       samplel=klatt_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay();
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


void nvp(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) nvp_newsay();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=nvp_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=nvp_get_sample();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay();
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

void nvpvocab(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) nvp_newsay_vocab_trigger(); // resets to start of vocab
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=nvp_get_sample_vocab();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay_vocab_trigger();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=nvp_get_sample_vocab();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay_vocab_trigger();
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

void nvpvocabsing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) nvp_newsay_vocab_trigger(); // resets to start of vocab
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=nvp_get_sample_vocab_sing();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay_vocab_trigger();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=nvp_get_sample_vocab_sing();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay_vocab_trigger();
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

void klattTTS(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // probably lose this! TODO!
  TTS=0;
  //    do a restricted TTS here of just 8 characters and only letters

    u8 xax=_sely*10.0f; 
    u8 selz=_selz*30.0f; 
    MAXED(xax,7);
    MAXED(selz,25);
    xax=7-xax; // inverted
    selz=25-selz;
    TTSinarray[xax]=mapytoletters[selz];

  static u8 triggered=0;
    if (trigger==1) klatt_newsayTTS();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sampleTTS();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsayTTS();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=klatt_get_sampleTTS();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsayTTS();
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

void klattsingle(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) klatt_newsay_single();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sample_single();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_single();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=klatt_get_sample_single();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_single();
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

void klattsinglesing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) klatt_newsay_single();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sample_single_sing();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_single();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=klatt_get_sample_single_sing();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_single();
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

void klattvocabsing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) klatt_newsay_vocab();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sample_vocab_sing();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_vocab();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=klatt_get_sample_vocab_sing();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_vocab();
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

void klattvocab(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) klatt_newsay_vocab();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=klatt_get_sample_vocab();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_vocab();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=klatt_get_sample_vocab();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 klatt_newsay_vocab();
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


void rsynthy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) rsynth_newsay();
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=rsynth_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=rsynth_get_sample();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay();
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

void rsynthysing(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) rsynth_newsay();
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=rsynth_get_sample_sing();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=rsynth_get_sample_sing();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay();
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



void rsynthelm(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  extent nextent={51,53.0f};
  TTS=0;
  static u8 triggered=0;
  if (trigger==1) rsynth_newsay_elm();
  u8 xx=0,readpos;
  float remainder;
  //  samplespeed*=0.25f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();

       u8 xaxis=_selx*nextent.maxplus;
       MAXED(xaxis,nextent.max); 
       xaxis=nextent.max-xaxis;
       u8 val=_selz*86.0f;
       MAXED(val,83);
       val=83-val;
       test_elm_rsynthy[xaxis*2]=val; // xaxis must be 
       test_elm_rsynthy[(xaxis*2)+1]=(_sely*33.0f)+1; // length say max 32 - short to long left to t=right

       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=rsynth_get_sample_elm();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay_elm();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();

       u8 xaxis=_selx*nextent.maxplus;
       MAXED(xaxis,nextent.max); 
       xaxis=nextent.max-xaxis;
       u8 val=_selz*86.0f;
       MAXED(val,83);
       val=83-val;
       test_elm_rsynthy[xaxis*2]=val;
       test_elm_rsynthy[(xaxis*2)+1]=((1.0f-_sely)*33.0f)+1; // length say max 32

       samplel=rsynth_get_sample_elm();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay_elm();
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

void rsynthsingle(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
    if (trigger==1) rsynth_newsay_single();
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;
   if (samplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=rsynth_get_sample_single();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay_single();
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
	 samplel=rsynth_get_sample_single();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 rsynth_newsay_single();
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

//////////////]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
// TUBES sets

// first tubes is from: tube.c

void tubes(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed*=0.5f;

   if (samplespeed<=1.0f){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tube_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder);
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
         while (xx<size){
	   //	   outgoing[xx++]=0;
	      samplel=tube_get_sample();
       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
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
};


// test new tubes and samplerate

/* TODO:

void samplerate(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size, int16_t(*getsample)(void), void(*newsay)(void), u8 trigger, float sampleratio);

xysamplerate also with extents and so on - (we need to wrap newsay_specific)

*/

typedef struct wormer_ {
  uint16_t maxextent;
  float maxplusextent;
  float sampleratio;
  int16_t(*getsample)(void);
  void(*newsay)(void);  
} wormer;

wormer t_uber={0, 0.0f, 1.0f, tube_get_sample, tube_newsay};

void tubestest(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  if (trigger==1) tube_newsay();
  //       samplespeed*=0.5f;
       samplerate(incoming, outgoing, samplespeed, size, t_uber.getsample, t_uber.newsay, trigger, t_uber.sampleratio); // TODO: add trigger, incorporate doadc, xy version
       // dooadc only if we want a sample
       
}

// test code for both raven and ntube with worm wavetable TODO!

void test_worm_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float lastbuffer[32], otherbuffer[32];
        dowavetable(otherbuffer, &wavtable, adc_buffer[SELX], size);
  //  dowormwavetable(otherbuffer, &wavtable, adc_buffer[SELX], size);
	//  NTube_do(&tuber, otherbuffer, lastbuffer, 32);
  //donoise(otherbuffer,32);

	//	RavenTube_next(otherbuffer, lastbuffer, size);
  floot_to_int(outgoing,lastbuffer,size);
}  


//////////////]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

u8 toggled=1;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  float samplespeed;
  float value;
  u16 samplespeedref;
  static u16 cc;
  u8 oldmode=255;
  static u8 firsttime=0;
  value =(float)adc_buffer[SPEED]/65536.0f; 
  smoothed_adc_value[0] += 0.1f * (value - smoothed_adc_value[0]);
  _speed=smoothed_adc_value[0];
  CONSTRAIN(_speed,0.0f,1.0f);
  _speed=1.0f-_speed;

  value =(float)adc_buffer[MODE]/65536.0f; 
  smoothed_adc_value[1] += 0.01f * (value - smoothed_adc_value[1]); // TESTY! 0.0f for SMOOTHER mode locking
  _mode=smoothed_adc_value[1];
  CONSTRAIN(_mode,0.0f,1.0f);

  oldmode=_intmode;
  _intmode=_mode*65.0f;
  MAXED(_intmode, 63);
  trigger=0; 
  _intmode=60;
 // if (oldmode!=_intmode) trigger=1; // for now this is never/always called TEST
 if (firsttime==0){// TEST CODE - for fake trigger
   trigger=1;
   firsttime=1;
 }
 
  samplespeedref=_speed*1027.0f;
  MAXED(samplespeedref, 1023);
  samplespeed=logspeed[samplespeedref];  
  
  // splitting input
  for (u8 x=0;x<sz/2;x++){
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
  }

  // array of t_uber style we select the mode from - but what of x/y modes...

  samplerate(sample_buffer, mono_buffer, samplespeed, sz/2, t_uber.getsample, t_uber.newsay, trigger, t_uber.sampleratio);

  //  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, votrax, votraxTTS, votraxgorf, votraxwow, votraxwowfilterbend, votrax_param, votrax_bend, tms, tmsphon, tmsTTS, tmsbendlength, tmslowbit, tmsraw5100, tmsraw5200, tmsraw5220, tmsbend5100, tmsbend5200, tms5100pitchtablebend, tms5200pitchtablebend, tms5100ktablebend, tms5200ktablebend, tms5100kandpitchtablebend, tms5200kandpitchtablebend, sam_banks0, sam_banks1, sam_TTS, sam_TTSs, sam_phon, sam_phons, sam_phonsing, sam_xy, sam_param, sam_bend, digitalker, digitalker_sing, digitalker_bendpitchvals, sp0256sing, votraxsing, tmssing, tmsphonsing, simpleklatt, nvp, klatt, klattTTS, nvpvocab, klattsingle, klattvocab, rsynthy, rsynthelm, rsynthsingle, nvpvocabsing, rsynthysing, klattsinglesing, klattvocabsing, tubestest}; 
  
  //INDEX//0sp0256, 1sp0256TTS, 2sp0256vocabone, 3sp0256vocabtwo, 4sp0256_1219, 5sp0256bend, /// 6votrax, 7votraxTTS, 8votraxgorf, 9votraxwow, 10votraxwowfilterbend, 11votrax_param, 12votrax_bend, // 13tms, 14tmsphone, 15tmsTTS, 16tmsbendlength, 17tmslowbit, 18tmsraw5100, 19tmsraw5200, 20tmsraw5220, 21tmsbend5100, 22tmsbend5200, 23tms5100pitchtablebend, 24tms5200pitchtablebend, 25tms5100ktablebend, 26tms5200ktablebend, 27tms5100kandpitchtablebend, 28tms5200kandpitchtablebend, 29sam_banks0, 30sam_banks1, 31sam_TTS, 32sam_TTSs, 33sam_phon, 34,sam_phons, 35sam_phonsing, 36sam_xy, 37sam_param, 38sam_bend, 39digitalker, 40digitalker_sing, 41digitalker_bendpitchvals, 42sp0256sing, 43votraxsing, 44tmssing, 45tmsphonsing, /// 46simpleklatt, 47nvp, 48klatt, 49klattTTS, 50nvpvocab, 51klattsingle, 52klattvocab, 53rsynthy, 54rsynthelm, 55rsynthsingle, 56nvpvocabsing, 57rsynthsing, 58klattsinglesing, 59klattvocabsing, 60tubes=tube.c

  //  generators[_intmode](sample_buffer,mono_buffer,samplespeed,sz/2); 

  // copy sample buffer into audio_buffer as COMPOST
  if (!toggled){
  for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc]=mono_buffer[x];
    cc++;
    if (cc>AUDIO_BUFSZ) cc=0;
  }
  }

  if (TTS){ // so this doesn't change as fast as generators
    u8 xax=_sely*18.0f; 
    u8 selz=_selz*65.0f; 
    MAXED(xax,16);
    MAXED(selz,63);
    xax=16-xax; // inverted
    selz=63-selz;
    TTSinarray[xax]=mapytoascii[selz];
    }

  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}
#endif
