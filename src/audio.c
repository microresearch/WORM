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
extern Wavetable wavtable;
//wormy myworm;
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

void floot_to_int(int16_t* outbuffer, float* inbuffer,u16 howmany){
  int32_t tmp;

  for (int n = 0; n < howmany; n++) {
    tmp = inbuffer[n] * 32768.0f;
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    outbuffer[n] = (int16_t)tmp;
		}
}

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


//((((((((((((((((((
  //OLD INDEX//0sp0256, 1sp0256TTS, 2sp0256vocabone, 3sp0256vocabtwo, 4sp0256_1219, 5sp0256bend, /// 6votrax, 7votraxTTS, 8votraxgorf, 9votraxwow, 10votraxwowfilterbend, 11votrax_param, 12votrax_bend, // 13tms, 14tmsphone, 15tmsTTS, 16tmsbendlength, 17tmslowbit, 18tmsraw5100, 19tmsraw5200, 20tmsraw5220, 21tmsbend5100, 22tmsbend5200, 23tms5100pitchtablebend, 24tms5200pitchtablebend, 25tms5100ktablebend, 26tms5200ktablebend, 27tms5100kandpitchtablebend, 28tms5200kandpitchtablebend, 29sam_banks0, 30sam_banks1, 31sam_TTS, 32sam_TTSs, 33sam_phon, 34,sam_phons, 35sam_phonsing, 36sam_xy, 37sam_param, 38sam_bend, 39digitalker, 40digitalker_sing, 41digitalker_bendpitchvals, 42sp0256sing, 43votraxsing, 44tmssing, 45tmsphonsing, /// 46simpleklatt, 47nvp, 48klatt, 49klattTTS, 50nvpvocab, 51klattsingle, 52klattvocab, 53rsynthy, 54rsynthelm, 55rsynthsingle, 56nvpvocabsing, 57rsynthsing, 58klattsinglesing, 59klattvocabsing, 60tubes=tube.c

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

static const wormer votraxparamer={6, 1.25f, votrax_get_sample_rawparam, votrax_newsay_bendr, 1, 0}; // exy-raw .. newsay is not used
static const wormer votraxsinger={0, 1.25f, votrax_get_sample_sing, votrax_newsay_sing, 0, 0};

// 10 sam modes: sam_banks0, sam_banks1, sam_TTS, sam_TTSs, sam_phon, sam_phons, sam_phonsing, sam_xy, sam_param, sam_bend

static const wormer sambanks0er={0, 1.0f, sam_get_sample_banks0a, sam_newsay_banks0, 0, 0}; // pitch on x, sel phrase on y/z
static const wormer sambanks1er={0, 1.0f, sam_get_sample_banks1a, sam_newsay_banks0, 0, 0}; // speed on x, sel phrase on y/z

static const wormer samTTSer={0, 1.0f, sam_get_sample_TTSa, sam_newsay_TTS, 0, 2}; // pitch on x
static const wormer samTTSser={0, 1.0f, sam_get_sample_TTSsa, sam_newsay_TTS, 0, 2}; // speed on x

// DONE: re-done for own XY - DONE/TESTED!

static const wormer samphoner={15, 1.0f, sam_get_sample_phona, sam_newsay_phon, 0, 0};  // try with newsay trigger - was exy but now has own exy
static const wormer samphonser={15, 1.0f, sam_get_sample_phonsa, sam_newsay_phon, 0, 0};  // this one is with speed of phoneme
static const wormer samphonsinger={15, 1.0f, sam_get_sample_phonsinga, sam_newsay_phonsing, 0, 0}; // and - constant pitch on z
///

static const wormer samxyer={0, 1.0f, sam_get_sample_xya, sam_newsay_banks0, 0, 0};  // pitch on x speed on y
static const wormer samparamer={3, 1.0f, sam_get_sample_parama, sam_newsay_param, 2, 0}; // all x params as x/y axis with z as selected vocab 2=exy_trigger
static const wormer sambender={239, 1.0f, sam_get_sample_benda, sam_newsay_xy, 2, 0};  // x/y bends freq data, z as selected

//39digitalker, 40digitalker_sing, 41digitalker_bendpitchvals, 

static const wormer digitalker={0, 1.0f, digitalk_get_sample, digitalk_newsay, 0, 0}; // digitalker now has resampling inside say
static const wormer digitalker_sing={0, 1.0f, digitalk_get_sample_sing, digitalk_newsay, 0, 0}; // digitalker now has resampling inside say
static const wormer digitalker_bendpitchvals={31, 1.0f, digitalk_get_sample_bendpitchvals, digitalk_newsay, 2, 0}; // digitalker now has resampling inside say

// 18 tms modes: 

static const wormer tmser={0, 0.25f, tms_get_sample, tms_newsay, 0, 0};
static const wormer tmslowbiter={0, 0.25f, tms_get_sample_lowbit, tms_newsay_lowbit, 0, 0};
static const wormer tmssinger={0, 0.25f, tms_get_sample_sing, tms_newsay, 0, 0};
static const wormer tmsphoner={0, 0.25f, tms_get_sample_allphon, tms_newsay_allphon, 0, 0};
static const wormer tmsphonsinger={0, 0.25f, tms_get_sample_allphon_sing, tms_newsay_allphon, 0, 0};
static const wormer tmsttser={0, 0.25f, tms_get_sample_TTS, tms_retriggerTTS, 0, 1};
static const wormer tmsbendlengther={0, 0.25f, tms_get_sample_bendlength, tms_newsay, 0, 0};

// tmsraw5100, tmsraw5200, tmsraw5220, 

static const wormer tmsraw5100er={10, 0.25f, tms_get_sample_raw5100, tms_newsay_raw5100, 2, 0}; // maxextent=10, triggerxymode
static const wormer tmsraw5200er={10, 0.25f, tms_get_sample_raw5200, tms_newsay_raw5200, 2, 0}; // maxextent=10, triggerxymode
static const wormer tmsraw5220er={10, 0.25f, tms_get_sample_raw5220, tms_newsay_raw5220, 2, 0}; // maxextent=10, triggerxymode

// tms5100ktablebend, tms5200ktablebend, tms5100kandpitchtablebend, tms5200kandpitchtablebend
// bends -> we need to add vocabs for each case +1 - how to choose?

static const wormer tmsbend5100er={11, 0.25f, tms_get_sample_bend5100w, tms_newsay_specific5100, 2, 0}; // maxextent=11, triggerxymode
static const wormer tmsbend5200er={11, 0.25f, tms_get_sample_bend5200a, tms_newsay_specifica, 2, 0}; // maxextent=11, triggerxymode - allphons 

static const wormer tms5100pitchtablebender={31, 0.25f, tms_get_sample_5100pitchtablew, tms_newsay_specific5100, 2, 0}; // maxextent=31, triggerxymode
static const wormer tms5200pitchtablebender={63, 0.25f, tms_get_sample_5200pitchtablea, tms_newsay_specifica, 2, 0}; // maxextent=63, triggerxymode - allphons 

static const wormer tms5100ktablebender={167, 0.25f, tms_get_sample_5100ktablew, tms_newsay_specific5100, 2, 0}; // maxextent=167, triggerxymode
static const wormer tms5200ktablebender={167, 0.25f, tms_get_sample_5200ktablea, tms_newsay_specifica, 2, 0}; // maxextent=167, triggerxymode - allphons

static const wormer tms5100kandpitchtablebender={199, 0.25f, tms_get_sample_5100kandpitchtablew, tms_newsay_specific5100, 2, 0}; // maxextent=199, triggerxymode
static const wormer tms5200kandpitchtablebender={231, 0.25f, tms_get_sample_5200kandpitchtablea, tms_newsay_specifica, 2, 0}; // maxextent=231, triggerxymode - allphons

// TODO: we need an order for these which makes sense
// from old_audio: TO TEST and document 14/6+

static const wormer rsynthy={0, 0.25f, rsynth_get_sample, rsynth_newsay, 0, 0};
static const wormer rsynthelm={0, 0.25f, rsynth_get_sample_elm, rsynth_newsay_elm, 0, 0}; // TODO port own version of exy for elmDONE-TEST!NOT WORKING!!!
static const wormer rsynthsingle={0, 0.25f, rsynth_get_sample_single, rsynth_newsay_single, 0, 0};
static const wormer rsynthysing={0, 0.25f, rsynth_get_sample_sing, rsynth_newsay, 0, 0};

static const wormer klatter={0, 1.0f, klatt_get_sample, klatt_newsay, 0, 0};  // klatt has its own xy form - what is selz doing - check?
static const wormer klattsingle={0, 1.0f, klatt_get_sample_single, klatt_newsay_single, 0, 0};
static const wormer klattvocab={0, 1.0f, klatt_get_sample_vocab, klatt_newsay_vocab, 0, 0};
static const wormer klattsinglesing={0, 1.0f, klatt_get_sample_single_sing, klatt_newsay_single, 0, 0};
static const wormer klattvocabsing={0, 1.0f, klatt_get_sample_vocab_sing, klatt_newsay_vocab, 0, 0};
static const wormer simpleklatter={39, 1.0f, simpleklatt_get_sample, simpleklatt_newsay, 2, 0};

static const wormer nvper={0, 1.0f, nvp_get_sample, nvp_newsay, 0, 0};
static const wormer nvpvocabsing={0, 1.0f, nvp_get_sample_vocab_sing, nvp_newsay_vocab_trigger, 0, 0};
static const wormer nvpvocaber={0, 1.0f, nvp_get_sample_vocab, nvp_newsay_vocab_trigger, 0, 0};

static const wormer tuber={0, 1.0f, tube_get_sample, tube_newsay, 0, 0}; // //60tubes=tube.c=tube_get_sample
static const wormer tubsinger={0, 1.0f, tube_get_sample_sing, tube_newsay_sing, 0, 0};
static const wormer tubbender={19, 1.0f, tube_get_sample_bend, tube_newsay_bend, 1, 0}; // now we add extra parameters 
static const wormer tubrawer={19, 1.0f, tube_get_sample_raw, tube_newsay_raw, 1, 0};
static const wormer tubxyer={4, 1.0f, tube_get_sample_xy, tube_newsay_xy, 1, 0};

static const wormer composter={0, 1.0f, compost_get_sample, compost_newsay, 0, 0};
static const wormer compostfrer={0, 1.0f, compost_get_sample_frozen, compost_newsay, 0, 0};

/// TODO - these will be re-arranged in order that makes sense: klatt, tubes and compost last:

//static const wormer *wormlist[]={&sp0256er, &sp0256TTSer, &sp0256singer, &sp0256vocaboneer, &sp0256vocabtwoer, &sp02561219er, &sp0256bender, &votraxer, &votraxTTSer, &votraxgorfer, &votraxwower, &votraxwowfilterbender, &votraxbender, &votraxparamer, &votraxsinger, &sambanks0er, &sambanks1er, &samTTSer, &samTTSser, &samphoner, &samphonser, &samphonsinger, &samxyer, &samparamer, &sambender, &tmser, &tmslowbiter, &tmssinger, &tmsphoner, &tmsphonsinger, &tmsttser, &tmsbendlengther, &tmsraw5100er, &tmsraw5200er, &tmsraw5220er, &tmsbend5100er, &tmsbend5200er, &tms5100pitchtablebender, &tms5200pitchtablebender, &tms5100ktablebender, &tms5200ktablebender, &tms5100kandpitchtablebender, &tms5200kandpitchtablebender, &digitalker, &digitalker_sing, &digitalker_bendpitchvals, &rsynthy, &rsynthelm, &rsynthsingle, &rsynthysing, &klatter, &klattsingle, &klattsinglesing, &klattvocab, &klattvocabsing, &simpleklatter, &nvper, &nvpvocaber, &nvpvocabsing, &tuber, &tubsinger, &composter, &compostfrer};

// removed: &tubbender, &tubrawer, &tubxyer, 

// this is the shorter list of 43 modes 0-42 with composter as 42

static const wormer *wormlist[]={&sp0256er, &sp0256TTSer, &sp0256vocaboneer, &sp0256vocabtwoer, &sp02561219er, &sp0256bender, &votraxer, &votraxTTSer, &votraxgorfer, &votraxwower, &votraxwowfilterbender, &votraxbender, &votraxparamer, &sambanks0er, &sambanks1er, &samTTSer, &samphoner, &samphonser,
&samxyer, &samparamer, &sambender, &tmser, &tmsphoner, &tmsttser, &tmsbendlengther, &tmsraw5100er, &tmsraw5200er, &tmsraw5220er, &tmsbend5100er, &tmsbend5200er, &tms5100kandpitchtablebender, &tms5200kandpitchtablebender, &digitalker, &digitalker_bendpitchvals, &rsynthy, &rsynthelm, &rsynthsingle, &klatter, &klattsingle, &klattvocab, &nvper, &nvpvocaber, &composter};

  // list: 
//0&sp0256er, 1&sp0256TTSer, 2&sp0256singer, 3&sp0256vocaboneer, 4&sp0256vocabtwoer, 5&sp02561219er, 6&sp0256bender, 7&votraxer, 8&votraxTTSer, 9&votraxgorfer, 10&votraxwower, 11&votraxwowfilterbender, 12&votraxbender, 13&votraxparamer, 14&votraxsinger, 15&sambanks0er, 16&sambanks1er, 17&samTTSer, 18&samTTSser, 19&samphoner, 20&samphonser, 21&samphonsinger, 22&samxyer, 23&samparamer, 24&sambender, 25&tmser, 26&tmslowbiter, 27&tmssinger, 28&tmsphoner, 29&tmsphonsinger, 30&tmsttser, 31&tmsbendlengther, 32&tmsraw5100er, 33&tmsraw5200er, 34&tmsraw5220er, 35&tmsbend5100er, 36&tmsbend5200er, 37&tms5100pitchtablebender, 38&tms5200pitchtablebender, 39&tms5100ktablebender, 40&tms5200ktablebender, 41&tms5100kandpitchtablebender, 42&tms5200kandpitchtablebender, 43&digitalker, 44&digitalker_sing, 45&digitalker_bendpitchvals, 46&rsynthy 47&rsynthelm, 48&rsynthsingle, 49&rsynthysing, 50&klatter,51&klattsingle, 52&klattsinglesing, 53&klattvocab, 54&klattvocabsing, 55&simpleklatter, 56&nvper, 57&nvpvocaber, 58&nvpvocabsing, 59&tuber, 60&tubsinger, 61&composter, 62&compostfrer};


static int16_t comp_counter=0;
static u16 cc=0;

static inline void doadc_compost(){
  // exy stays anyway as it is or?
  _selx=oldselx;
  _sely=oldsely;
  _selz=oldselz;
}

static int16_t delay_buffer[8] = { 0 }; 
#define DELAY_SIZE 6

static void new_data(int16_t data)
{
    for (u8 ii=0;ii<DELAY_SIZE-5;ii++)	delay_buffer[ii] = delay_buffer[ii+1];
    delay_buffer[DELAY_SIZE-5] = data;
}


int16_t compost_get_sample(){

  float alpha;
  static float time_now=0.0f;
  long last_time;
  static long int_time=0;
  static u8 triggered=0;
  
 static u8 oldcompost=255, compostmode=255;
 doadc();
  u16 startx=(1.0f-_selx)*32768.0f;
  u16 endy=(1.0f-_sely)*32768.0f;
  signed char dir=1;

  // generate into audio_buffer based on selz mode and struct list
  float value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // try to smooth it!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
  oldcompost=compostmode;
  compostmode= _selz*45.0f; // as mode - adapt for excluding compost_mode TODO!
  MAXED(compostmode, 41); // NUMMODES!!!
  //if mode change do a newsay or not?
  //  compostmode=5; // testy - here we just fix
  doadc_compost();
  if (oldcompost!=compostmode ) wormlist[compostmode]->newsay();

  // TODO: we need to re-sample so is not so fast in some cases
  //wormlist[compostmode]->sampleratio <= 1.0f
  float factor=0.5f*wormlist[compostmode]->sampleratio;

  if (time_now>32768){
    int_time=0; // preserve???
    time_now-=32768.0f;
  }

    alpha = time_now - (float)int_time;
    audio_buffer[cc++] = ((float)delay_buffer[DELAY_SIZE-5] * alpha) + ((float)delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
    //audio_buffer[cc++]=wormlist[compostmode]->getsample();

  time_now += factor;
  last_time = int_time;
  int_time = time_now;
  while(last_time<int_time)      {
    //    doadc();
    int16_t val=wormlist[compostmode]->getsample();
    new_data(val);
    last_time += 1;
  }

    
  if (cc>AUDIO_BUFSZ) cc=0;

  // resets at start or end - newsay will reset in full
  if (startx>endy){
    dir=-1;
    if (comp_counter<=endy) comp_counter=startx; // swopped round
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
  u16 startx=(1.0f-_selx)*32768.0f;
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
  smoothed_adc_value[1] += 0.01f * (value - smoothed_adc_value[1]); // TESTY! 0.01f for SMOOTHER mode locking
  _mode=smoothed_adc_value[1];
  CONSTRAIN(_mode,0.0f,1.0f);

  oldmode=_intmode;
  _intmode=_mode*47.0f;
  MAXED(_intmode, 42); //TODO: there are 0-42 = 43 modes now
  trigger=0; 
  if (oldmode!=_intmode) {// IF there is a modechange!
  trigger=1; // for now this is never/always called TEST
  doadc();
  oldselx=_selx;
  oldsely=_sely;
  oldselz=_selz;
  }
  
  /* if (firsttime==0){// TEST CODE - for fake trigger - replace with above
   trigger=1;
   firsttime=1;
   }*/

 // _speed=0.8f;
 samplespeedref=_speed*1028.0f;
 MAXED(samplespeedref, 1023);
 samplespeed=logspeed[samplespeedref];  
  
  for (u8 x=0;x<sz/2;x++){ /// sz/2=128/2-64 = /2=32
    sample_buffer[x]=*(src++); 
    src++;
  }

  //  _intmode=63; //TESTY!

  if (trigger==1) wormlist[_intmode]->newsay();   // first trigger from mode-change pulled out from below

  if (wormlist[_intmode]->xy==0) samplerate_simple(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio);
  else if (wormlist[_intmode]->xy==1)
    samplerate_simple_exy(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent);
  else 
    samplerate_simple_exy_trigger(sample_buffer, mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent);

  // copy sample buffer into audio_buffer as COMPOST as long as we are NOT COMPOSTING!
  //  if (_intmode!=61 || _intmode!=62){// TODO - whatever is compost modes 64 or??? 
  if (_intmode!=42){// TODO - whatever is compost modes 64 or??? 
    for (u8 x=0;x<sz/2;x++) {
    audio_buffer[cc++]=mono_buffer[x];
    if (cc>AUDIO_BUFSZ) cc=0;
    }
  }
  
  if (wormlist[_intmode]->TTS==1){ 
    doadc();
    u8 xax=_sely*19.0f; 
    u8 selz=_selz*68.0f; 
    MAXED(xax,15);
    MAXED(selz,63);
    xax=15-xax; // inverted
    selz=63-selz;
    TTSinarray[xax]=mapytoascii[selz];
    }

  else if (wormlist[_intmode]->TTS==2){ // for SAM TTS which doesn't like spaces 
    doadc();
    u8 xax=_sely*19.0f; 
    u8 selz=_selz*64.0f; 
    MAXED(xax,15);
    MAXED(selz,61);
    xax=15-xax; // inverted
    selz=61-selz;
    TTSinarray[xax]=mapytoascii[selz+2];
    }


  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}
#endif
