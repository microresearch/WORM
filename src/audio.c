// license:GPL-2.0+
// copyright-holders: Martin Howse


#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#define THRESH 16000 
#define THRESHLOW 10000

#ifdef TESTING
#include "audio.h"
#include "wavetable.h"
#include "resources.h"
#else
#include "audio.h"
#include "klatt_phoneme.h"
#include "parwave.h"
#include "sam.h"
#include "holmes.h"
#include "sp0256.h"
#include "tms5200x.h"
#include "digitalker.h"
#include "nvp.h"
#include "vot.h"
#include "resources.h"
#include "rs.h"
#include "samplerate.h"
#endif

extern __IO uint16_t adc_buffer[10];
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".ccmdata")));

int16_t	left_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];

float smoothed_adc_value[5]={0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; 

extern float exy[240];
float _mode, _speed, _selx, _sely, _selz;
static float oldselx=0.5f, oldsely=0.5f, oldselz=1.0f;
static u8 _intspeed, _intmode=0, trigger=0;

#define float float32_t

#ifdef TESTING
extern Wavetable wavtable;
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
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // smoother
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
  uint16_t val=0;
  doadc();
    val=(1.0f-_selx)*1027.0f; // how can we test all others???? - add and then mod
    val+=(1.0f-_sely)*1027.0f; 
    val+=(1.0f-_selz)*1027.0f;

  float value =(float)adc_buffer[SPEED]/65536.0f; 
    smoothed_adc_value[0] += 0.1f * (value - smoothed_adc_value[0]);
    _speed=smoothed_adc_value[0];
    CONSTRAIN(_speed,0.0f,1.0f);
        val+=(1.0f-_speed)*1027.0f;


    value =(float)adc_buffer[MODE]/65536.0f; 
    smoothed_adc_value[1] += 0.01f * (value - smoothed_adc_value[1]); // 0.01f for SMOOTHER mode locking
    _mode=smoothed_adc_value[1];
    CONSTRAIN(_mode,0.0f,1.0f);
    val+=(1.0f-_mode)*1027.0f;

  //  val=val%1024;

  MAXED(val,1023);
    dowavetable(lastbuffer, &wavtable, 2.0f+(logspeed[val]*440.0f), size); // for exp/1v/oct test
  //    dowavetable(lastbuffer, &wavtable, 2.0f+(1.038673f*440.0f), size); // for exp/1v/oct test
  //    dowavetable(lastbuffer, &wavtable, 440.0f, size); // for exp/1v/oct test
  floot_to_int(outgoing,lastbuffer,size);
}  

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  static u8 triggered=0;

 float  samplespeed=1.0f;
  void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={test_wave}; 

  // splitting input
    for (u8 x=0;x<sz/2;x++){
  // we want to test trigger here??? void wave_newsay(void)
    sample_buffer[x]=*(src++); // right is input on LACH, LEFT ON EURO!
    src++;
    if (sample_buffer[x]>=THRESH && !triggered) {
      doadc();
      wave_newsay();
      triggered=1;
  }
  else if (sample_buffer[x]<THRESHLOW && triggered) triggered=0;
    //  mono_buffer[x]=rand()%32768;
  }
    
    
    generators[0](sample_buffer,mono_buffer,samplespeed,sz/2); 
  
  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}

#else

////////--->>> list of modes

int16_t compost_get_sample();
void compost_newsay();
void compost_newsay_frozen();

int16_t none_get_sample(){
  return 0;
}

void none_newsay(){
}

typedef struct wormer_ {
  u8 maxextent;
  float sampleratio;
  int16_t(*getsample)(void);
  void(*newsay)(void);
  u8 xy;
  u8 TTS;
} wormer;


static const wormer sp0256er={0, 0.3125f, sp0256_get_sample, sp0256_newsay, 0, 0};
static const wormer sp0256TTSer={0, 0.3125f, sp0256_get_sampleTTS, sp0256_retriggerTTS, 0, 1};
static const wormer sp0256singer={0, 0.3125f, sp0256_get_sample_sing, sp0256_newsay, 0, 0};
static const wormer sp0256vocaboneer={0, 0.3125f, sp0256_get_samplevocabbankone, sp0256_newsayvocabbankonea, 0, 0}; // wrapped newsay
static const wormer sp0256vocabtwoer={0, 0.3125f, sp0256_get_samplevocabbanktwo, sp0256_newsayvocabbanktwoa, 0, 0};
static const wormer sp02561219er={0, 0.3125f, sp0256_get_sample1219, sp0256_newsay1219, 0, 0};
static const wormer sp0256bender={14, 0.3125f, sp0256_get_samplebend, sp0256_newsaybend, 1, 0}; // trigger as toggle // checked exy extent

// 8 votrax modes: votrax, votraxTTS, votraxgorf, votraxwow, votraxwowfilterbend, votrax_param, votrax_bend, votraxsing

static const wormer votraxer={0, 0.8f, votrax_get_sample, votrax_newsay, 0, 0};
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
static const wormer samTTSer={0, 1.0f, sam_get_sample_TTSa, sam_newsay_TTS, 0, 1}; // pitch on x
static const wormer samTTSser={0, 1.0f, sam_get_sample_TTSsa, sam_newsay_TTS, 0, 1}; // speed on x
static const wormer samphoner={0, 1.0f, sam_get_sample_phona, sam_newsay_phon, 0, 0};  // try with newsay trigger - was exy but now has own exy
static const wormer samphonser={0, 1.0f, sam_get_sample_phonsa, sam_newsay_phon, 0, 0};  // this one is with speed of phoneme
static const wormer samphonsinger={0, 1.0f, sam_get_sample_phonsinga, sam_newsay_phonsing, 0, 0}; // and - constant pitch on z
///

static const wormer samxyer={0, 1.0f, sam_get_sample_xya, sam_newsay_banks0, 0, 0};  // pitch on x speed on y
static const wormer samparamer={3, 1.0f, sam_get_sample_parama, sam_newsay_param, 2, 0}; // all x params as x/y axis with z as selected vocab 2=exy_trigger
static const wormer sambender={239, 1.0f, sam_get_sample_benda, sam_newsay_xy, 2, 0};  // x/y bends freq data, z as selected

//39digitalker, 40digitalker_sing, 41digitalker_bendpitchvals, 

static const wormer digitalker={0, 1.0f, digitalk_get_sample, digitalk_newsay, 0, 0}; // digitalker now has resampling inside say
static const wormer digitalker_sing={0, 1.0f, digitalk_get_sample_sing, digitalk_newsay, 0, 0}; // digitalker now has resampling inside say
static const wormer digitalker_bendpitchvals={31, 1.0f, digitalk_get_sample_bendpitchvals, digitalk_newsay, 2, 0}; // digitalker now has resampling inside say

// tms modes: 

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

static const wormer tmsbend5100er={11, 0.25f, tms_get_sample_bend5100w, tms_newsay_specific5100, 2, 0}; // maxextent=11, triggerxymode
static const wormer tmsbend5200er={11, 0.25f, tms_get_sample_bend5200a, tms_newsay_specifica, 2, 0}; // maxextent=11, triggerxymode - allphons 
static const wormer tmsbend5200erx={11, 0.25f, tms_get_sample_bend5200x, tms_newsay_specificx, 2, 0}; // maxextent=11, triggerxymode - allphons 

static const wormer tms5100pitchtablebender={31, 0.25f, tms_get_sample_5100pitchtablew, tms_newsay_specific5100, 2, 0}; // maxextent=31, triggerxymode
static const wormer tms5200pitchtablebender={63, 0.25f, tms_get_sample_5200pitchtablea, tms_newsay_specifica, 2, 0}; // maxextent=63, triggerxymode - allphons
static const wormer tms5200pitchtablebenderx={63, 0.25f, tms_get_sample_5200pitchtablex, tms_newsay_specificx, 2, 0}; // maxextent=63, triggerxymode - allphons 

static const wormer tms5100ktablebender={167, 0.25f, tms_get_sample_5100ktablew, tms_newsay_specific5100, 2, 0}; // maxextent=167, triggerxymode
static const wormer tms5200ktablebender={167, 0.25f, tms_get_sample_5200ktablea, tms_newsay_specifica, 2, 0}; // maxextent=167, triggerxymode - allphons

static const wormer tms5100kandpitchtablebender={199, 0.25f, tms_get_sample_5100kandpitchtablew, tms_newsay_specific5100, 2, 0}; // maxextent=199, triggerxymode
static const wormer tms5200kandpitchtablebender={231, 0.25f, tms_get_sample_5200kandpitchtablea, tms_newsay_specifica, 2, 0}; // maxextent=231, triggerxymode - allphons
static const wormer tms5200kandpitchtablebenderx={231, 0.25f, tms_get_sample_5200kandpitchtablex, tms_newsay_specificx, 2, 0}; // maxextent=231, triggerxymode - allphons

static const wormer rsynthy={0, 0.25f, rsynth_get_sample, rsynth_newsay, 0, 0};
static const wormer rsynthelm={0, 0.25f, rsynth_get_sample_elm, rsynth_newsay_elm, 0, 0}; 
static const wormer rsynthsingle={0, 0.25f, rsynth_get_sample_single, rsynth_newsay_singlex, 0, 0};
static const wormer rsynthysing={0, 0.25f, rsynth_get_sample_sing, rsynth_newsay, 0, 0};

static const wormer klatter={0, 1.0f, klatt_get_sample, klatt_newsay, 0, 0};  // elements
static const wormer klattsingle={0, 1.0f, klatt_get_sample_single, klatt_newsay_single, 0, 0};
static const wormer klattvocab={0, 1.0f, klatt_get_sample_vocab, klatt_newsay_vocab, 0, 0};
static const wormer klattsinglesing={0, 1.0f, klatt_get_sample_single_sing, klatt_newsay_single, 0, 0};
static const wormer klattvocabsing={0, 1.0f, klatt_get_sample_vocab_sing, klatt_newsay_vocab, 0, 0};

static const wormer simpleklatter={38, 0.5f, simpleklatt_get_sample, simpleklatt_newsay, 2, 0}; // TODO: which kind of trigger mode?
static const wormer nvper={0, 0.5f, nvp_get_sample, nvp_newsay, 0, 0};
static const wormer nvpvocabsing={0, 0.5f, nvp_get_sample_vocab_sing, nvp_newsay_vocab_trigger, 0, 0};
static const wormer nvpvocaber={0, 0.5f, nvp_get_sample_vocab, nvp_newsay_vocab_trigger, 0, 0};

static const wormer composter={0, 1.0f, compost_get_sample, compost_newsay, 0, 0};
static const wormer compostfrer={0, 1.0f, compost_get_sample, compost_newsay_frozen, 0, 0};

static const wormer *wormlist[]={&tmser, &tmslowbiter, &tmssinger, &tmsbendlengther, &tmsphoner, &tmsphonsinger, &tmsttser, &tmsraw5100er, &tmsraw5200er, &tmsraw5220er, &tmsbend5100er, &tmsbend5200er, &tmsbend5200erx, &tms5100pitchtablebender, &tms5200pitchtablebender, &tms5200pitchtablebenderx, &tms5100ktablebender, &tms5200ktablebender, &tms5100kandpitchtablebender, &tms5200kandpitchtablebender, &tms5200kandpitchtablebenderx, &sp0256er, &sp0256singer, &sp0256TTSer, &sp0256vocaboneer, &sp0256vocabtwoer, &sp02561219er, &sp0256bender, &votraxer, &votraxTTSer, &votraxgorfer, &votraxwower, &votraxwowfilterbender, &votraxbender, &votraxparamer, &votraxsinger, &sambanks0er, &sambanks1er, &samTTSer, &samTTSser, &samphoner, &samphonser, &samphonsinger, &samxyer, &samparamer, &sambender, &digitalker, &digitalker_sing, &digitalker_bendpitchvals, &rsynthy, &rsynthelm, &rsynthsingle, &rsynthysing, &klatter, &klattsingle, &klattsinglesing, &klattvocab, &klattvocabsing, &simpleklatter, &nvper, &nvpvocaber, &nvpvocabsing, &composter, &compostfrer};

#define MODEF 66.0f // float 68.0f
#define MODEFC 65.0f // float 68.0f
#define MODET 63 // mode top 63 // 61 for no COMPOST
#define COMPOST 62
#define COMPOSTF 63

// compost

static u16 comp_counter=0;
static u16 ccc=0;
static u8 freezer=0;

static inline void doadc_compost(){
  // exy stays anyway as it is
  _selx=oldselx;
  _sely=oldsely;
  _selz=oldselz;
}

static int16_t delay_buffer[2] = { 0 }; 
#define DELAY_SIZE 6

static void new_data(int16_t data)
{
  delay_buffer[0] = delay_buffer[1];
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
  u16 startx=(1.0f-_selx)*32767.0f;
  u16 endy=(1.0f-_sely)*32767.0f;
  signed char dir;

  if (freezer==1){ // UNFROZEN
  // generate into audio_buffer based on selz mode and struct list
    /*  float value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // try to smooth it!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f); */ // selz is already done in samplerate
  oldcompost=compostmode;
  _selz=1.0f-_selz; // invert
  compostmode= _selz*MODEFC; 
  MAXED(compostmode, MODET-2); // NUMMODES-2 for composts
  doadc_compost();

  if (oldcompost!=compostmode )
    {
      wormlist[compostmode]->newsay();
    }

  //wormlist[compostmode]->sampleratio <= 1.0f
  float factor=wormlist[compostmode]->sampleratio;

  if (time_now>32768){
    int_time=0; 
    time_now-=32768.0f;
  }

    alpha = time_now - (float)int_time;
    audio_buffer[ccc++] = ((float)delay_buffer[DELAY_SIZE-5] * alpha) + ((float)delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
    if (ccc>AUDIO_BUFSZ-1) ccc=0;

  time_now += factor;
  last_time = int_time;
  int_time = time_now;
  while(last_time<int_time)      {
    int16_t val=wormlist[compostmode]->getsample();
    new_data(val);
    last_time += 1;
  }
  } // end of freezer  
  if (startx>endy){
    dir=-1;
    if (comp_counter<=endy) comp_counter=startx; // swopped round
  }
  else {
    dir=1;
    if (comp_counter>=endy) comp_counter=startx;
  }

  int16_t sample=audio_buffer[comp_counter%32768];
  comp_counter+=dir;
  return sample;
}

void compost_newsay_frozen(){ 
  freezer^=1; // toggles freezer
}


void compost_newsay(){ //TODO - restart compost mode

  
  freezer=1; // always unfrozen
  // just reset counter to start 
  doadc();
/*  u16 startx=_selx*32767.0f;
  u16 endy=_sely*32767.0f;

  if (startx>endy){
    comp_counter=startx; // backwards
  }
  else if (startx<=endy){
    comp_counter=startx;//
  }
*/

  _selz=1.0f-_selz; // invert
  u8 compostmode= _selz*MODEFC; 
  MAXED(compostmode, MODET-2); // NUMMODES-2 for composts
  doadc_compost();

  wormlist[compostmode]->newsay();
}

/////////////////---------------------------------------------------------

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  int16_t sample;
  float samplespeed;
  float value;
  u16 samplespeedref;
  static u16 cc;
  u8 oldmode; 
  u8 triggered=0;
  static u8 retrigger=0;
  static u8 firsttime=0;
  value =(float)adc_buffer[SPEED]/65536.0f; 
  smoothed_adc_value[0] += 0.1f * (value - smoothed_adc_value[0]); // smooth
  _speed=smoothed_adc_value[0];
  CONSTRAIN(_speed,0.0f,1.0f);
  _speed=1.0f-_speed;

  value =(float)adc_buffer[MODE]/65536.0f; 
  smoothed_adc_value[1] += 0.01f * (value - smoothed_adc_value[1]); // 0.01f for SMOOTHER mode locking
  _mode=smoothed_adc_value[1];
  CONSTRAIN(_mode,0.0f,1.0f);
  _mode=1.0f-_mode; // invert
    oldmode=_intmode;
  _intmode=_mode*MODEF;
  _intmode=60; //TESTY
  MAXED(_intmode, MODET); 
  trigger=0; 

  // TESTY: OUT COMMENT BELOW
  /*
     if (oldmode!=_intmode) {// IF there is a modechange!
    trigger=1; // for now this is never/always called TEST
    // if we are not leaving compost - if we are entering compost ???
    if ((intmode== COMPOST || intmode== COMPOSTF) && oldmode!=COMPOST && oldmode!=COMPOSTF){
    doadc();
    oldselx=_selx;
    oldsely=_sely;
    oldselz=_selz;
    }
    }
  */
    if (firsttime==0){ // we can leave this so is always called first
      trigger=1;
      firsttime=1;
      }

    samplespeedref=_speed*1028.0f;
    MAXED(samplespeedref, 1023);
    samplespeed=logspeed[samplespeedref];  
    //    samplespeed=1.0f; // TESTY CHECK!!!
    // how can we avoid trigger crossing boundary - this should work
  for (u8 x=0;x<sz/2;x++){ /// sz/2=128/2-64 = /2=32
    sample=*(src++);
    src++;
    if (retrigger==0 && sample> THRESH) {
      triggered=1;
      retrigger=1;
      break; // ???
    }
    if (sample<THRESHLOW) retrigger=0;
  }

  if (trigger==1) wormlist[_intmode]->newsay();   // first trigger from mode-change pulled out from below

  if (wormlist[_intmode]->xy==0) samplerate_simple(mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio, triggered);
  else if (wormlist[_intmode]->xy==1)
    samplerate_simple_exy(mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent, triggered);
  else 
    samplerate_simple_exy_trigger(mono_buffer, samplespeed, sz/2, wormlist[_intmode]->getsample, wormlist[_intmode]->newsay , wormlist[_intmode]->sampleratio, wormlist[_intmode]->maxextent, triggered);

  if (_intmode!=COMPOST && _intmode!=COMPOSTF){
    for (u8 x=0;x<sz/2;x++) {
      audio_buffer[cc++]=mono_buffer[x];
    if (cc>AUDIO_BUFSZ-1) cc=0;
    }
  }
  
  if (wormlist[_intmode]->TTS==1){ 
    doadc();
    u8 xax=_sely*19.0f; 
    u8 selz=_selz*67.0f; 
    MAXED(xax,15);
    MAXED(selz,63);
    xax=15-xax; // inverted
    selz=63-selz;
    TTSinarray[xax]=mapytoascii[selz];
    }

  audio_comb_stereo(sz, dst, mono_buffer,left_buffer);
}
#endif
