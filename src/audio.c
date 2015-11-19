/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

/*- doublecchecked LACH knobs left/right from top (pcb and lach=test) = 0/mode-(2..3)
1/selX.egX-(3..2) 2/speed-(0..4) 3/end-(4..1)
4/trigthresh/vocoderfreq/othersel.eg/selY-(1..0)*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#include "audio.h"
#include "effect.h"
#include "klatt_phoneme.h"
#include "mdavocoder.h"
#include "vocode.h"
#include "scformant.h"

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
extern u8* datagenbuffer;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], sample_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
float flinbuffer[MONO_BUFSZ];
float floutbuffer[MONO_BUFSZ];
float floutbufferz[MONO_BUFSZ];

#define float float32_t

mdavocal *mdavocall;
mdavocoder *mdavocod;
VocoderInstance* vocoder;
Formlet *formy;
Formant *formanty;

void Audio_Init(void)
{
  float Fc,Q,peakGain;
  const float Fs=32000.0f;// TODO
  float a0,a1,a2,b1,b2,norm,V,K;
  float *state[5][5];
  uint32_t i;
  int16_t *audio_ptr;
  u16 x,xx;

	mdavocall=(mdavocal *)malloc(sizeof(mdavocal));
	mdavocal_init(mdavocall);
	mdavocod=(mdavocoder *)malloc(sizeof(mdavocoder));
	mdaVocoder_init(mdavocod);
	vocoder=instantiateVocoder();
	formy=(Formlet *)malloc(sizeof(Formlet));
	Formlet_init(formy, 110.0f);
	formanty=(Formant *)malloc(sizeof(Formant));
	Formant_init(formanty);

	/* clear the buffer */
	audio_ptr = audio_buffer;
		i = AUDIO_BUFSZ;
		while(i-- > 0)
		*audio_ptr++ = 0;

		for (xx=0;xx<5;xx++){// five formants INIT
		  for (x=0;x<5;x++){
		    Fc=freq[xx][x];
		    Q=qqq[xx][x];
		    K = tanf(M_PI * Fc / Fs);
		    norm = 1 / (1 + K / Q + K * K);
		    a0 = K / Q * norm;
		    a1 = 0;
		    a2 = -a0;
		    b1 = 2 * (K * K - 1) * norm;
		    b2 = (1 - K / Q + K * K) * norm;

		    coeffs[xx][x][0]=a0*mull[xx][x]; coeffs[xx][x][1]=a1*mull[xx][x]; coeffs[xx][x][2]=a2*mull[xx][x]; coeffs[xx][x][3]=-b1*mull[xx][x]; coeffs[xx][x][4]=-b2*mull[xx][x];
		    /// can also just mult coeffs????
		    state[xx][x] = (float*)malloc(4*sizeof(float));
		    arm_biquad_cascade_df1_init_f32(&df[xx][x],1,coeffs[xx][x],state[xx][x]);
		  }
		}
}

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

extern u8 trigger;
extern u16 generated;
extern u16 writepos;
extern u8 mode;
extern u16 readpos;

void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  static u8 framecount=0;
  static float eaten=0;
  static float samplepos=0.0f; float samplespeed;
  u16 x;
  u8 speedy;
  float xx,yy;

  // trigger to take care of but figure out basics step by step first!

  //  u16 ending=loggy[adc_buffer[END]];
  u16 ending=AUDIO_BUFSZ;
  speedy=(adc_buffer[SPEED]>>6)+1;
  samplespeed=8.0f/(float)(speedy); // what range this gives? - 10bits>>6=4bits=16 so 8max skipped and half speed
  //     samplespeed=1.0f;

  // PROCESS incoming audio and activate master_triger
  // TODO:read in audio and process for trigger
  // trigger will set samplepos=0.0f, writepos=0 and trigger=1
  // but readpos only after we have written?
 

  switch(mode){
  case 0: // rsynth/klatt-single phoneme
    for (x=0;x<sz/2;x++){
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      //                mono_buffer[x]=rand()%65536;
      if (generated<=eaten){
	eaten=0.0f;
	trigger=1;
      }
      samplepos+=samplespeed;
      eaten+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
    }
    break;
  case 1: // rsynth/klatt-chain of phonemes
    // how to update that chain - every x callbacks
    // (256=framesize*say8frames for phoneme -2048 // 32 frame
    // here=every 64 frames - divided by speedy 64/speedy
    for (x=0;x<sz/2;x++){
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      samplepos+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
    }
    framecount++;
    if (framecount>(64/speedy)){
      // update next 3 elements in phoneme chain=test_elm - so need to track elements
    }

  case 2: // start VOSIM-SC tests
    for (x=0;x<sz/2;x++){
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      samplepos+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
    }
    //    framecount++;
    break;
  case 3: // single or running VOSIM?
    for (x=0;x<sz/2;x++){
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      if (generated<=eaten){
	eaten=0.0f;
	trigger=1;
      }
      samplepos+=samplespeed;
      eaten+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
    }
    break;
  case 4: // mdavocoder - WE NEED different carriers?
        for (x=0;x<sz/2;x++){
	    src++;
	    sample_buffer[x]=*(src++); // right is input
    }
    // generate vocoder carrier and test this - mdavoc.c - why 2 inputs to this?
        int_to_floot(sample_buffer,flinbuffer,sz/2);
	mdavocall->pmult = (float)powf(1.0594631f, floor(48.0f * (((float)(adc_buffer[SELX]))/4096.0f) - 24.0f));
	mdavocal_process(mdavocall, flinbuffer, floutbuffer, sz/2);
    // vocode sample_buffer with carrier_buffer and copy to output buffer
	mdaVocoderprocess(mdavocod,flinbuffer, floutbuffer, floutbufferz,sz/2); 
        floot_to_int(mono_buffer,floutbufferz,sz/2);
        break; 
  case 5: // ladspa vocoder
        for (x=0;x<sz/2;x++){
	    src++;
	    sample_buffer[x]=*(src++); // right is input
    }
    // generate vocoder carrier and test this - mdavoc.c - why 2 inputs to this?
        int_to_floot(sample_buffer,flinbuffer,sz/2);
	mdavocall->pmult = (float)powf(1.0594631f, floor(48.0f * (((float)(adc_buffer[SELX]))/2048.0f) - 24.0f));
	mdavocal_process(mdavocall, flinbuffer, floutbuffer, sz/2);

	//	for (x=0;x<sz/2;x++){ // white noise?
	//    floutbuffer[x]=(float)(rand()%32768-65536)/32678.0f;
	//  }

    // vocode sample_buffer with carrier_buffer and copy to output buffer
	runVocoder(vocoder, flinbuffer, floutbuffer, floutbufferz, sz/2);
        floot_to_int(mono_buffer,floutbufferz,sz/2);
	break;
	
  case 6: // SELX/SELY filters in parallel!
    // ranges here: X=100-900 Y=600to2400

    xx=(((float)adc_buffer[SELX])/5.0f)+100.0f; // X=100-900
    yy=(((float)adc_buffer[SELY])/1.36f)+600.0f; // Y=600to2400

	// test single filters
	//float bandpass(float sample,float q, float fc, float gain){ // from OWL code - statevariable	
	
	for (x=0;x<sz/2;x++){
	  src++;
	  u16 income=*(src++); // right is input
	  u16 flootin=(float32_t)(income)/32768.0f;

	  float flout=bandpassx(flootin,0.8f,xx,0.8f); // q freq gain
	  float floutz=bandpassy(flootin,0.8f,yy,0.8f); // q freq gain
	  floutz+=flout; // mix coeff?
	  
	  int32_t tmp = floutz * 32768.0f;
	  tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
	  mono_buffer[x]=(int16_t)tmp;
	}
	break;
  case 7: // SELX selects different vowels for filtering. so far we have 0-4
    // could also be used with own oscillator/carriers=white noise, vibrato etc/
    // rough on transitions TODO - cross fade?
        for (x=0;x<sz/2;x++){
	    src++;
	    sample_buffer[x]=*(src++); // right is input
    }
	u8 vowel=adc_buffer[SELX]>>9;
	  doformantfilter(sample_buffer, mono_buffer, sz/2,vowel%5);
    break;
  case 8: // FORMLET - how to change and re-init these and use for vowel processing
        for (x=0;x<sz/2;x++){
	    src++;
	    sample_buffer[x]=*(src++); // right is input
    }
        int_to_floot(sample_buffer,flinbuffer,sz/2);
	Formlet_process(formy, sz/2, flinbuffer, floutbuffer);
        floot_to_int(mono_buffer,floutbuffer,sz/2);
    break;
  case 9: // free running SAM and LPC code
  case 10:
    for (x=0;x<sz/2;x++){
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      samplepos+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
    }
    break;
  case 11: // basic SC formant:
    Formant_process(formanty, adc_buffer[SELX], adc_buffer[SELY], adc_buffer[SELZ], sz/2, floutbuffer); // fundfreq: 440, formfreq: 1760, bwfreq>funfreq: 880

        floot_to_int(mono_buffer,floutbuffer,sz/2);
    break;

  } // mode end

  audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

}
