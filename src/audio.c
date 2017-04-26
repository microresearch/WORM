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

// from oldaudio:

////7 sp0256 modes: sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256bend, sp0256sing

// changed to logspeed for pitch

static const wormer sp0256er={0, 0.3125f, sp0256_get_sample, sp0256_newsay, 0, 0};
static const wormer sp0256TTSer={0, 0.3125f, sp0256_get_sampleTTS, sp0256_retriggerTTS, 0, 1};
static const wormer sp0256singer={0, 0.3125f, sp0256_get_sample_sing, sp0256_newsay, 0, 0};
static const wormer sp0256vocaboneer={0, 0.3125f, sp0256_get_samplevocabbankone, sp0256_newsayvocabbankonea, 0, 0}; // wrapped newsay
static const wormer sp0256vocabtwoer={0, 0.3125f, sp0256_get_samplevocabbanktwo, sp0256_newsayvocabbanktwoa, 0, 0};
static const wormer sp02561219er={0, 0.3125f, sp0256_get_sample1219, sp0256_newsay1219, 0, 0};
static const wormer sp0256bender={14, 0.3125f, sp0256_get_samplebend, sp0256_newsaybend, 2, 0}; // as extra samplerate mode with trigger as newsay

// 8 votrax modes: votrax, votraxTTS, votraxgorf, votraxwow, votraxwowfilterbend, votrax_param, votrax_bend, votraxsing

static const wormer votraxer={0, 1.25f, votrax_get_sample, votrax_newsay, 0, 0};
static const wormer votraxTTSer={0, 1.25f, votrax_get_sampleTTS, votrax_retriggerTTS, 0, 1};
static const wormer votraxgorfer={0, 1.25f, votrax_get_samplegorf, votrax_newsaygorfr, 0, 0}; 
static const wormer votraxwower={0, 1.25f, votrax_get_samplewow, votrax_newsaywowr, 0, 0};
static const wormer votraxwowfilterbender={0, 1.25f, votrax_get_samplewow_bendfilter, votrax_newsaywow_bendfilterr, 0, 0}; 
static const wormer votraxbender={8, 1.25f, votrax_get_sample_bend, votrax_newsay_bendr, 2, 0}; // as extra samplerate mode with trigger as newsay

static const wormer votraxparamer={6, 1.25f, votrax_get_sample_rawparam, votrax_newsay_bendr, 1, 0}; // exy-raw .. newsay is never used here
static const wormer votraxsinger={0, 1.25f, votrax_get_sample_sing, votrax_newsay_sing, 0, 0};

// 18 tms modes: tms, tmsphon, tmsTTS, tmsbendlength, tmslowbit, tmsraw5100, tmsraw5200, tmsraw5220, tmsbend5100, tmsbend5200, tms5100pitchtablebend, tms5200pitchtablebend, tms5100ktablebend, tms5200ktablebend, tms5100kandpitchtablebend, tms5200kandpitchtablebend,  tmssing, tmsphonsing, 

static const wormer tmser={0, 0.25f, tms_get_sample, tms_newsay, 0, 0};

// 10 sam modes: sam_banks0, sam_banks1, sam_TTS, sam_TTSs, sam_phon, sam_phons, sam_phonsing, sam_xy, sam_param, sam_bend

static const wormer sambanks0er={0, 1.0f, sam_get_sample_banks0a, sam_newsay_banks0, 0, 0}; 
static const wormer sambanks1er={0, 1.0f, sam_get_sample_banks1a, sam_newsay_banks0, 0, 0}; //???

static const wormer samTTSer={0, 1.0f, sam_get_sample_TTSa, sam_newsay_TTS, 0, 1}; 
static const wormer samTTSser={0, 1.0f, sam_get_sample_TTSsa, sam_newsay_TTS, 0, 1}; 

static const wormer samphoner={15, 1.0f, sam_get_sample_phona, sam_newsay_banks0, 1, 0};  // newsay is not called
static const wormer samphonser={15, 1.0f, sam_get_sample_phonsa, sam_newsay_banks0, 1, 0};  // newsay is not called
static const wormer samphonsinger={15, 1.0f, sam_get_sample_phonsinga, sam_newsay_banks0, 1, 0};  // newsay is not called

static const wormer samxyer={0, 1.0f, sam_get_sample_xya, sam_newsay_banks0, 0, 0};  // pitch on x speed on y

static const wormer samparamer={3, 1.0f, sam_get_sample_parama, sam_newsay_banks0, 1, 0};  // newsay is not called
static const wormer sambender={239, 1.0f, sam_get_sample_benda, sam_newsay_banks0, 1, 0};  // newsay is not called

//((((((((((((((((((
  //OLD INDEX//0sp0256, 1sp0256TTS, 2sp0256vocabone, 3sp0256vocabtwo, 4sp0256_1219, 5sp0256bend, /// 6votrax, 7votraxTTS, 8votraxgorf, 9votraxwow, 10votraxwowfilterbend, 11votrax_param, 12votrax_bend, // 13tms, 14tmsphone, 15tmsTTS, 16tmsbendlength, 17tmslowbit, 18tmsraw5100, 19tmsraw5200, 20tmsraw5220, 21tmsbend5100, 22tmsbend5200, 23tms5100pitchtablebend, 24tms5200pitchtablebend, 25tms5100ktablebend, 26tms5200ktablebend, 27tms5100kandpitchtablebend, 28tms5200kandpitchtablebend, 29sam_banks0, 30sam_banks1, 31sam_TTS, 32sam_TTSs, 33sam_phon, 34,sam_phons, 35sam_phonsing, 36sam_xy, 37sam_param, 38sam_bend, 39digitalker, 40digitalker_sing, 41digitalker_bendpitchvals, 42sp0256sing, 43votraxsing, 44tmssing, 45tmsphonsing, /// 46simpleklatt, 47nvp, 48klatt, 49klattTTS, 50nvpvocab, 51klattsingle, 52klattvocab, 53rsynthy, 54rsynthelm, 55rsynthsingle, 56nvpvocabsing, 57rsynthsing, 58klattsinglesing, 59klattvocabsing, 60tubes=tube.c



// start to add for testings BELOW:

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
//static const wormer waveer={0, 1.0f, wave_get_sample, wave_newsay, 0, 0};

static const wormer composter={0, 1.0f, compost_get_sample, compost_newsay, 0, 0};
static const wormer compostfrer={0, 1.0f, compost_get_sample_frozen, compost_newsay, 0, 0};

static const wormer *wormlist[]={&tuber, &tubsinger, &tubbender, &tubrawer, &composter, &digitalker, &tubxyer, &nvper, &klatter, &sp0256er, &sp0256TTSer, &sp0256singer, &sp0256vocaboneer, &sp0256vocabtwoer, &sp02561219er, &sp0256bender, &votraxer, &votraxTTSer, &votraxgorfer, &votraxwower, &votraxwowfilterbender, &votraxbender, &votraxparamer, &votraxsinger, &tmser, &sambanks0er, &sambanks1er, &samTTSer, &samTTSser, &samphoner, &samphonser, &samphonsinger, &samxyer, &samparamer, &sambender};

  // list: 0&tuber, 1&tubsinger, 2&tubbender, 3&tubrawer, 4&composter, 5&digitalker, 6&tubxyer, 7&nvper, 8&waveer, 9&klatter, 10sp0256er, 11&sp0256TTSer, 12&sp0256singer, 13&sp0256vocaboneer, 14&sp0256vocabtwoer, 15&sp02561219er, 16&sp0256bender 17&votraxer, 18&votraxTTSer, 19&votraxgorfer, 20&votraxwower, 21&votraxwowfilterbender, 22&votraxbender, 23&votraxparamer, 24&votraxsinger, 25&tmser, 26&sambanks0er, 27&sambanks1er, 28&samTTSer, 29&samTTSser, 30&samphoner, 31&samphonser, 32&samphonsinger, 33&samxyer, 34&samparamer, 35&sambender

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

  _intmode=27; //TESTY!

  if (trigger==1) wormlist[_intmode]->newsay();   // first trigger from mode-change pulled out from below

  if (wormlist[_intmode]->xy==0) samplerate_simple(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio);
  else if (wormlist[_intmode]->xy==1)
    samplerate_simple_exy(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent);
  else 
    samplerate_simple_exy_trigger(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent);

  // copy sample buffer into audio_buffer as COMPOST as long as we are NOT COMPOSTING!
  if (_intmode!=4 || _intmode!=66){// TODO - whatever is compost modes 64 or??? - at  moment test with 4
    for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc++]=mono_buffer[x];
    if (cc>AUDIO_BUFSZ) cc=0;
    }
  }
  
  if (wormlist[_intmode]->TTS){ // so this doesn't change as fast as generators
    doadc();
    u8 xax=_sely*19.0f; 
    u8 selz=_selz*68.0f; 
    MAXED(xax,15);
    MAXED(selz,63);
    xax=15-xax; // inverted
    selz=63-selz;
    TTSinarray[xax]=mapytoascii[selz];
    }

  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}
#endif
