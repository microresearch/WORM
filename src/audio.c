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

extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], sample_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

float smoothed_adc_value[5]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // SELX, Y, Z, SPEED

extern float exy[64];
float _mode, _speed, _selx, _sely, _selz;
static float oldselx=0.5f, oldsely=0.5f, oldselz=1.0f;
static u8 _intspeed, _intmode=0, trigger=0;
u8 TTS=0;

#define float float32_t

float samplepos=0;//=genstruct->samplepos;
int16_t samplel;//=genstruct->lastsample;
int16_t lastval;//=genstruct->prevsample;

#ifdef TESTING
Wavetable wavtable;
wormy myworm;
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

static inline void doadc(){
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
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // try to smooth it!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
}

////////////////////////////////////////////// TESTING!

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
  //  if (!toggled){
  for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc]=mono_buffer[x];
    cc++;
    if (cc>AUDIO_BUFSZ) cc=0;
  }
  //  }

  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}

#else

////////--->>> list of modes

int16_t compost_get_sample();
int16_t compost_get_sample_frozen();
void compost_newsay();

typedef struct wormer_ {
  u8 maxextent;
  float sampleratio;
  int16_t(*getsample)(void);
  void(*newsay)(void);
  u8 xy;
  u8 TTS;
} wormer;

// start to add for testings:

static const wormer digitalker={0, 1.0f, digitalk_get_sample, digitalk_newsay, 0, 0}; // digitalker now has resampling inside say
static const wormer klatter={0, 1.0f, klatt_get_sample, klatt_newsay, 0, 0};  // klatt has its own xy form

///

static const wormer tuber={0, 1.0f, tube_get_sample, tube_newsay, 0, 0};

// these are extra tubes modes not in oldaudio+sing, bend, raw(TEST!)
static const wormer tubsinger={0, 1.0f, tube_get_sample_sing, tube_newsay_sing, 0, 0};
static const wormer tubbender={19, 1.0f, tube_get_sample_bend, tube_newsay_bend, 1, 0}; // now we add extra parameters 
static const wormer tubrawer={19, 1.0f, tube_get_sample_raw, tube_newsay_raw, 1, 0};
static const wormer tubxyer={4, 1.0f, tube_get_sample_xy, tube_newsay_xy, 1, 0};
static const wormer nvper={0, 1.0f, nvp_get_sample, nvp_newsay, 0, 0};
static const wormer waveer={0, 1.0f, wave_get_sample, wave_newsay, 0, 0};

static const wormer composter={0, 1.0f, compost_get_sample, compost_newsay, 0, 0};
static const wormer compostfrer={0, 1.0f, compost_get_sample_frozen, compost_newsay, 0, 0};

static const wormer *wormlist[]={&tuber, &tubsinger, &tubbender, &tubrawer, &composter, &digitalker, &tubxyer, &nvper, &waveer, &klatter};

  // list: 0&tuber, 1&tubsinger, 2&tubbender, 3&tubrawer, 4&composter, 5&digitalker, 6&tubxyer, 7&nvper, 8&waveer, 9&klatter};

static int16_t comp_counter=0;
static u16 cc=0;

static inline void doadc_compost(){
  // exy stays anyway as it is or?
  _selx=oldselx;
  _sely=oldsely;
  _selz=oldselz;
}

int16_t compost_get_sample(){
 static u8 oldcompost=255, compostmode=255;
 doadc();
  u16 startx=(1.0f-_selx)*32768.0f, diff;
  u16 endy=(1.0f-_sely)*32768.0f;
  signed char dir=1;

  // generate into audio_buffer based on selz mode and struct list
  float value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // try to smooth it!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
  oldcompost=compostmode;
  compostmode= _selz*65.0f; // as mode - adapt for one less excluding compost_mode TODO!
  //if mode change do a newsay or not?
  compostmode=5; // testy - here we just fix
  doadc_compost();
  if (oldcompost!=compostmode ) wormlist[compostmode]->newsay();
  audio_buffer[cc++]=wormlist[compostmode]->getsample();
  if (cc>AUDIO_BUFSZ) cc=0;

  // resets at start or end - newsay will reset in full
  if (startx>endy){
    dir=-1;
    if (comp_counter<=startx) comp_counter=startx;
  }
  else {
    dir=1;
    if (comp_counter>=endy) comp_counter=startx;
  }

  int16_t pos=comp_counter; // restricted to 32768
  int16_t sample=audio_buffer[pos];
  comp_counter+=dir;
  return sample;
}

int16_t compost_get_sample_frozen(){
 static u8 oldcompost=255, compostmode=255;
 doadc();
  u16 startx=(1.0f-_selx)*32768.0f, diff;
  u16 endy=(1.0f-_sely)*32768.0f;
  signed char dir=1;

  // resets at start or end - newsay will reset in full
  if (startx>endy){
    dir=-1;
    if (comp_counter<=startx) comp_counter=startx;
  }
  else {
    dir=1;
    if (comp_counter>=endy) comp_counter=startx;
  }

  int16_t pos=comp_counter; // restricted to 32768
  int16_t sample=audio_buffer[pos];
  comp_counter+=dir;
  return sample;
}


void compost_newsay(){ 
  // just reset counter to start 
  doadc();
  u16 startx=_selx*32768.0f;
  u16 endy=_sely*32768.0f;

  if (startx>endy){
    comp_counter=endy;
  }
  else if (startx<=endy){
    comp_counter=startx;
  }  
}


/////////////////---------------------------------------------------------

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  float samplespeed;
  float value;
  u16 samplespeedref;
  static u8 oldmode=255; 
  static u8 firsttime=0;
  value =(float)adc_buffer[SPEED]/65536.0f; 
  smoothed_adc_value[0] += 0.1f * (value - smoothed_adc_value[0]); // smooth
  _speed=smoothed_adc_value[0];
  CONSTRAIN(_speed,0.0f,1.0f);
  _speed=1.0f-_speed;

  value =(float)adc_buffer[MODE]/65536.0f; 
  smoothed_adc_value[1] += 0.05f * (value - smoothed_adc_value[1]); // TESTY! 0.01f for SMOOTHER mode locking
  _mode=smoothed_adc_value[1];
  CONSTRAIN(_mode,0.0f,1.0f);

  oldmode=_intmode;
  _intmode=_mode*65.0f;
  MAXED(_intmode, 63);
  trigger=0; 
  /* if (oldmode!=_intmode) {// IF there is a modechange!
  trigger=1; // for now this is never/always called TEST
  doadc();
  oldselx=_selx;
  oldsely=_sely;
  oldselz=_selz;
  }*/
  
 if (firsttime==0){// TEST CODE - for fake trigger - replace with above
   trigger=1;
   firsttime=1;
 }

 // _speed=0.8f;
 samplespeedref=_speed*1028.0f;
 MAXED(samplespeedref, 1023);
 samplespeed=logspeed[samplespeedref];  
  
  for (u8 x=0;x<sz/2;x++){ /// sz/2=128/2-64 = /2=32
    sample_buffer[x]=*(src++); 
    src++;
  }

  _intmode=9; // TESTY!

  if (trigger==1) wormlist[_intmode]->newsay();   // first trigger from mode-change pulled out from below

  if (wormlist[_intmode]->xy==0) samplerate_simple(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio);
  else
    samplerate_simple_exy(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent);

  // copy sample buffer into audio_buffer as COMPOST as long as we are NOT COMPOSTING!
  if (_intmode!=4 || _intmode!=66){// TODO - whatever is compost modes 64 or??? - at  moment test with 4
    for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc++]=mono_buffer[x];
    if (cc>AUDIO_BUFSZ) cc=0;
    }
  }
  
  if (wormlist[_intmode]->TTS){ // so this doesn't change as fast as generators
    doadc();
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
