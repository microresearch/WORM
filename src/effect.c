// TESTING various effects

/*0-CONV/1-phase vocoder/2-bandpass vocoder/3-FFT spectral
  exchanges/4-simple formant vowels/5-further eg envelope, formant
  synth, windowing etc.*/

#include "arm_math.h"
#include "arm_const_structs.h"
#include "effect.h"
#include "vocode.h"
#include "mdavocoder.h"
#include "audio.h"
#include "biquad.h"
#include "PV_vocoder.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

extern int16_t* buf16;
//extern u8* datagenbuffer;
//extern int16_t audio_buffer[AUDIO_BUFSZ];
extern arm_biquad_casd_df1_inst_f32 df[5][5];
extern mdavocoder *mdavocod;
extern float coeffs[5][5];

void BPFSC_init (BPFSC* unit, float frequency, float bandwidth){ // init say 5 cascaded filters float* frequency, float* bandwidth
  const float mRadiansPerSample=(2 * M_PI) /48000.0f;
  unit->m_a0 = 0.f;
  unit->m_b1 = 0.f;
  unit->m_b2 = 0.f;
  unit->m_y1 = 0.f;
  unit->m_y2 = 0.f;
  unit->m_freq = frequency;
  unit->m_bw = bandwidth;

  float pfreq = unit->m_freq*mRadiansPerSample;
  float pbw   = unit->m_bw   * pfreq * 0.5;
	
  float C = 1.f / tanf(pbw);
  float D = 2.f * cosf(pfreq);
	
  float a0 = 1.f / (1.f + C);
  float b1 = C * D * a0 ;
  float b2 = (1.f - C) * a0;

  unit->m_a0 = a0;
  unit->m_b1 = b1;
  unit->m_b2 = b2;
	  }

void BPFSC_process(BPFSC *unit, int inNumSamples, float* inbuffer, float* outbuffer){

  float y0;
  float y1 = unit->m_y1;
  float y2 = unit->m_y2;
  float a0 = unit->m_a0;
  float b1 = unit->m_b1;
  float b2 = unit->m_b2;

  for (int i=0;i<inNumSamples;i++){
  y0 = inbuffer[i] + b1 * y1 + b2 * y2;
  outbuffer[i] = a0 * (y0 - y2);

  y2 = y1;
  y1 = y0;
}

  unit->m_y1 = y1;//so these are only ones which change
  unit->m_y2 = y2;
}

void Formlet_setfreq(Formlet *unit, float frequency){
  const float log001=logf(0.001);
  const float mRadiansPerSample=(2 * M_PI) /32000.0f;
  float b01,b02,b11,b12;
  float attackTime = unit->m_attackTime;
  float decayTime = unit->m_decayTime;
  float ffreq = frequency * mRadiansPerSample;
  
  float R = decayTime == 0.f ? 0.f : expf(log001/(decayTime * 32000.0f));
  float twoR = 2.f * R;
  float R2 = R * R;
  float temp, sint;
  //  arm_sin_cos_f32(57.29578 *ffreq, &sint, &temp); 
    temp=arm_cos_f32(ffreq);
  //  temp=cosf(ffreq);
  //  float cost = (twoR * cosf(ffreq)) / (1.f + R2);
  float cost = (twoR * temp) / (1.f + R2);

  b01 = twoR * cost;
  b02 = -R2;
  
  R = attackTime == 0.f ? 0.f : expf(log001/(attackTime * 32000.0f));
  twoR = 2.f * R;
  R2 = R * R;
  //cost = (twoR * cosf(ffreq)) / (1.f + R2);
  cost = (twoR * temp) / (1.f + R2);
  b11 = twoR * cost;
  b12 = -R2;

  // add slopes?

  unit->m_b01 = b01;
  unit->m_b02 = b02;
  unit->m_b11 = b11;
  unit->m_b12 = b12;
}

void Formlet_init(Formlet* unit){
 //  unit->m_freq = frequency;
  //  unit->m_bw = bandwidth;
  unit->m_attackTime = 0.01f;
  unit->m_decayTime = 0.5f;
  unit->m_y01 = 0.f;
  unit->m_y02 = 0.f;
  unit->m_y11 = 0.f;
  unit->m_y12 = 0.f;
  Formlet_setfreq(unit,1000);
}

void Formlet_process(Formlet *unit, int inNumSamples, float* inbuffer, float* outbuffer){

  float y00;
  float y10;
  float y01 = unit->m_y01;
  float y11 = unit->m_y11;
  float y02 = unit->m_y02;
  float y12 = unit->m_y12;

  float b01 = unit->m_b01;
  float b11 = unit->m_b11;
  float b02 = unit->m_b02;
  float b12 = unit->m_b12;
  float ain;

  for (u8 i=0;i<inNumSamples;i++){
  ain = inbuffer[i];
  y00 = ain + b01 * y01 + b02 * y02;
  y10 = ain + b11 * y11 + b12 * y12;
    outbuffer[i] = 0.25* ((y00 - y02) - (y10 - y12)); //was 0.25*
  //  outbuffer[i]=inbuffer[i];
  y02 = y01;
  y01 = y00;
  y12 = y11;
  y11 = y10;
  }

  unit->m_y01 = y01;
  unit->m_y02 = y02;
  unit->m_y11 = y11;
  unit->m_y12 = y12;
}


void BBandPass_init(BBandPass* unit){
	float freq = unit->m_freq = 2000.0;
	float bw = unit->m_bw = 0.1;
	float w0 = 2*M_PI * (float)freq / 32000.0;
	float sinw0 = sinf(w0);
	float alpha = sinw0 * (sinhf((0.34657359027997 * (float)bw * w0) / sinw0));
	float b0rz = 1. / (1. + alpha);
	float a0 = unit->m_a0 = alpha * b0rz;
	unit->m_a1 = 0.0f;
	unit->m_a2 = -a0;
	unit->m_b1 = cosf(w0) * 2. * b0rz;
	unit->m_b2 = (1. - alpha) * -b0rz;
	unit->m_y1 = 0.;
	unit->m_y2 = 0.;
}

void BBandPass_process(BBandPass *unit, int inNumSamples,float* inbuffer, float* outbuffer)
{
	float a0, a1, a2, b1, b2, w0, sinw0, alpha, b0rz;
	float y0, y1, y2;

	y1 = unit->m_y1;
	y2 = unit->m_y2;

	a0 = unit->m_a0;
	a1 = unit->m_a1;
	a2 = unit->m_a2;
	b1 = unit->m_b1;
	b2 = unit->m_b2;

	/*	LOOP(unit->mRate->mFilterLoops, //this is the unroll by 3

		y0 = ZXP(in) + b1 * y1 + b2 * y2;
		ZXP(out) = a0 * y0 + a1 * y1 + a2 * y2;

		y2 = ZXP(in) + b1 * y0 + b2 * y1;
		ZXP(out) = a0 * y2 + a1 * y0 + a2 * y1;

		y1 = ZXP(in) + b1 * y2 + b2 * y0;
		ZXP(out) = a0 * y1 + a1 * y2 + a2 * y0;
		);*/

	for (int i=0;i<inNumSamples;i++){
	  //	LOOP(unit->mRate->mFilterRemain,
		y0 = inbuffer[i] + b1 * y1 + b2 * y2;
		outbuffer[i] = a0 * y0 + a1 * y1 + a2 * y2;
		y2 = y1;
		y1 = y0;
	}

	unit->m_a0 = a0;
	unit->m_a1 = a1;
	unit->m_a2 = a2;
	unit->m_b1 = b1;
	unit->m_b2 = b2;
	unit->m_y1 = y1;
	unit->m_y2 = y2;
}



// 2-convolution

//r[] as zeroed first
//max function above

// watch amount of out
/*void convolvee(float *convolve1, u16 n1, float *convolve2, u16 n2, float *out)
{
  for (u16 n = 0; n < n1 + n2 - 1; n++){
      out[n]=0.0f;
      for (u16 k = 0; k < MAX(n1, n2); k++)
	out[n] += (k < n1 ? convolve1[k] : 0) * (n - k < n2 ? convolve2[n - k] : 0);
  }
  }*/


void convolve1D(float* in, float* out, int dataSize, float* kernel, int kernelSize)
{
    int i, j, k;

    // start convolution from out[kernelSize-1] to out[dataSize-1] (last)
    for(i = kernelSize-1; i < dataSize; ++i)
    {
        out[i] = 0.0f;                             // init to 0 before accumulate

        for(j = i, k = 0; k < kernelSize; --j, ++k)
            out[i] += in[j] * kernel[k];
    }

    // convolution from out[0] to out[kernelSize-2]
    for(i = 0; i < kernelSize - 1; ++i)
    {
        out[i] = 0.0f;                             // init to 0 before sum

        for(j = i, k = 0; j >= 0; --j, ++k)
            out[i] += in[j] * kernel[k];
    }
}



float bandpassx(float sample,float q, float fc, float gain){ // from OWL code - statevariable
  float f,fb,hp,bp,scale;
  static float buf0=0,buf1=0;
  f = 2.0*M_PI*fc/32000.0f;
  fb= q + q/(1.0 - f);

  hp=sample-buf0;
  bp = buf0 - buf1; 
  buf0 = buf0 + f * (hp + fb * bp); 
  buf1 = buf1 + f * (buf0 - buf1);
  return bp*gain;
}

float bandpassy(float sample,float q, float fc, float gain){ // from OWL code - statevariable
  float f,fb,hp,bp,scale;
  static float buf0=0,buf1=0;
  f = 2.0*M_PI*fc/32000.0f;
  fb= q + q/(1.0 - f);

  hp=sample-buf0;
  bp = buf0 - buf1; 
  buf0 = buf0 + f * (hp + fb * bp); 
  buf1 = buf1 + f * (buf0 - buf1);
  return bp*gain;
}



float dobandpass(float sample,float f,float fb){ // from OWL code - statevariable
  float hp,bp;

  static float buf0=0.0f,buf1=0.0f;

  hp=sample-buf0;
  bp = buf0 - buf1; 
  buf0 = buf0 + f * (hp + fb * bp); 
  buf1 = buf1 + f * (buf0 - buf1);
  return bp;
}


inline void int_to_floot(int16_t* inbuffer, float* outbuffer, u16 howmany){
  for (int n = 0; n < howmany; n++) {
    outbuffer[n]=(float32_t)(inbuffer[n])/32768.0f;
  }
}

inline void intun_to_floot(u16* inbuffer, float* outbuffer,u16 howmany){
  for (int n = 0; n < howmany; n++) {
    outbuffer[n]=((float32_t)(inbuffer[n])/32768.0f)-1.0f;
  }
}


void floot_to_int(int16_t* outbuffer, float* inbuffer,u16 howmany){
  int32_t tmp;

  for (int n = 0; n < howmany; n++) {
    tmp = inbuffer[n] * 32768.0f;
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    outbuffer[n] = (int16_t)tmp;
		}
}

void envelopefollower(int16_t* envbuffer, int16_t* inbuffer, int16_t* outbuffer){ // stick to 32 samples=48k/32 ms???
  // but env is dependent on that size

  float envout = 0.0f; int16_t env=0;
 for (int x=0;x<32;x++){
   if (abs(env)<envbuffer[x]) env=inbuffer[x];
}
 envout=(float)(abs(env))/32768.0;
 for (int x=0;x<32;x++){
   outbuffer[x]=(float)inbuffer[x]*envout;
}
} 

void doenvelopefollower(int16_t* envbuffer, u8 envsize, int16_t* inbuffer, u8 insize, int16_t* outbuffer){ // stick to 32 samples=48k/32 ms???
  // but env is dependent on that size
  static u16 xx=0;
  static int16_t env=0;
  float envout; 
  /* for (int x=0;x<envsize;x++){
   if (abs(env)<envbuffer[x]) env=envbuffer[x];
   }*/
 for (u8 x=0;x<insize;x++){
   xx++;
   if (xx>envsize) {env=0;xx=0;}
   if (abs(env)<envbuffer[x]) env=envbuffer[x];
   envout=(float)env/32768.0;
   outbuffer[x]=(float)inbuffer[x]*envout;
}
} 

 
void doringcopy(int16_t *inbuffer,int16_t *modbuffer,int16_t* outbuffer,u8 longest){
  u8 xx;
  for (xx=0;xx<longest;xx++){
    outbuffer[xx]=modbuffer[xx];
  }
}

void accumbuffer(float *tmpotherbuffer,float *tmpotherotherbuffer,u8 howmany){
  for (u8 x=0;x<howmany;x++){
    tmpotherotherbuffer[x]+=(tmpotherbuffer[x]*2.0f);
  }
}
/*
void doformantfilter(int16_t *inbuffer, int16_t *outbuffer, u8 howmany, u8 vowel){
  float tmpbuffer[BUFF_LEN/4];
  float tmpotherbuffer[BUFF_LEN/4];
  float tmpotherotherbuffer[BUFF_LEN/4];
  memset(tmpotherotherbuffer,0,32*4); 
  //  vowel=vowel>>5;  
  int_to_floot(inbuffer,tmpbuffer,howmany);///or do conv when we sort out buffers
  arm_biquad_cascade_df1_f32(&df[vowel][0],tmpbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][1],tmpbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][2],tmpbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][3],tmpbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][4],tmpbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  floot_to_int(outbuffer,tmpotherotherbuffer,howmany);
  }

void doformantfilterf(float *inbuffer, float *outbuffer, u8 howmany, u8 vowel){// vowel as 0-4
  float tmpbuffer[BUFF_LEN/4];
  float tmpotherbuffer[BUFF_LEN/4];
  float tmpotherotherbuffer[BUFF_LEN/4];
  memset(tmpotherotherbuffer,0,howmany*4); 
  vowel=(vowel>>5)%5;  
  //  int_to_floot(inbuffer,tmpbuffer,howmany);///or do conv when we sort out buffers
  arm_biquad_cascade_df1_f32(&df[vowel][0],inbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][1],inbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][2],inbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][3],inbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  arm_biquad_cascade_df1_f32(&df[vowel][4],inbuffer,tmpotherbuffer,32); 
  accumbuffer(tmpotherbuffer,tmpotherotherbuffer,howmany);
  //  floot_to_int(outbuffer,tmpotherotherbuffer,howmany);
  memcpy(outbuffer,tmpotherotherbuffer,howmany*4);//dest,src,howmany//4bytes
  }
*/
/*void do_effect(villager_effect* vill_eff){
  int32_t tmp;float tmpp; 
  float freq;
  int16_t inbuffer[32],modbuffer[32],outbuffer[32];

  float finbuffer[32],fmodbuffer[32],foutbuffer[32];
  u8 x,xx,tmpinlong,tmpmodlong,longest; // never longer than 32!

  float hp,bp;
  static float buf0=0.0f,buf1=0.0f;

  // modifier is 8 bits
  // chunk in size 32
  // pos and wrap are fixed by select/knob in audio.c!
  // pos is NOT with start!

    if (++vill_eff->del>=vill_eff->speed){
      vill_eff->del=0;

    if (vill_eff->inpos>=vill_eff->inwrap) {
      vill_eff->inpos=0;
    }
 
   if (vill_eff->modpos>=vill_eff->modwrap) {
      vill_eff->modpos=0;
    }
 
    // so copy into inbuffer

   if ((vill_eff->inpos+32)<=vill_eff->inwrap) tmpinlong=32;
    else tmpinlong=vill_eff->inwrap-vill_eff->inpos; 
    
   // same for mod...
    if ((vill_eff->modpos+32)<=vill_eff->modwrap) tmpmodlong=32;
    else tmpmodlong=vill_eff->modwrap-vill_eff->modpos;
    
    //    now copy with length as longest
    if (tmpinlong<=tmpmodlong) longest=tmpinlong;// NOW lONGEST is really shortest which is as should be
    else longest=tmpmodlong;

    //    vill_eff->whicheffect=0;// TESTY!
  switch(vill_eff->whicheffect){
  case 0: // void doformantfilterf(float *inbuffer, float *outbuffer, u8 howmany, u8 vowel){// vowel as 0-4
    // just in->out as float 

    for (xx=0;xx<tmpinlong;xx++){
        finbuffer[xx]=(float32_t)(buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768)/32768.0f;//REDO!howso?
    }
    // do effect
    doformantfilterf(finbuffer, foutbuffer, tmpinlong, vill_eff->modifier);// vowel as 0-4
    // copy outbuffer to audio and do float back

    for (xx=0;xx<tmpinlong;xx++){
    tmp = foutbuffer[xx] * 32768.0f;
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
        buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=(int16_t)tmp+32768;
    //    audio_buffer[(vill_eff->outstart+vill_eff->outpos)&32767]=audio_buffer[(vill_eff->instart+vill_eff->inpos++)&32767];
    vill_eff->outpos+=vill_eff->step;
    if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }

   break; 
  case 1: //FFT in PV from inbuffer into buf16
    //void dofftin(int16_t* inbuffer, int16_t* outbuffer){ // 32 samples
    // but must be 32 samples
    for (xx=0;xx<tmpinlong;xx++){
      inbuffer[xx]=buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768;
    }
    dofftin(inbuffer,outbuffer);
    // copy into buf16
    for (xx=0;xx<tmpinlong;xx++){
      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=outbuffer[xx]+32768;
      //      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=0;
      vill_eff->outpos+=vill_eff->step;
      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }
    break;

  case 2: // reverse FFT from values in buf16
    for (xx=0;xx<32;xx++){
      inbuffer[xx]=buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768;
    }
    dofftout(inbuffer,outbuffer);
    // copy into buf16
    for (xx=0;xx<tmpinlong;xx++){
      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=outbuffer[xx]+32768;
      vill_eff->outpos+=vill_eff->step;
      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }
    break;

  case 3:
    // how to calculate 16 x 4 bit=64 bits offsets based on 12 bit modifier
    switch(vill_eff->modifier&7){ // 
    case 0:// leave as is
      break;
    case 1:// zero it
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]=0;
    }
    break;
    case 2:// is 1
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]=1;
    }
    break;
    case 3:// is -1
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]=15;
    }
    break;
    case 4:// inc it
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]++;
    }
    break;
    case 5:// dec it
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]--;
    }
    break;
    case 6: //????
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]=vill_eff->modifier>>4;
    }
    break;
    case 7:// use buf16
    for (xx=0;xx<16;xx++){
      mdavocod->offset[xx]=datagenbuffer[xx]>>4;
    }
    break;
    }
    // to floats
    for (xx=0;xx<longest;xx++){
      finbuffer[xx]=(float32_t)(buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768)/32768.0f;//REDO! why/how?
      fmodbuffer[xx]=(float32_t)((buf16[(vill_eff->modstart+vill_eff->modpos++)&32767]-32768)/32768.0f)-1.0f;//REDO! why/how?
      //      fmodbuffer[xx]=(float32_t)(audio_buffer[(vill_eff->modstart+vill_eff->modpos++)&32767]/32768.0f);
    }

    mdaVocoderprocess(mdavocod,finbuffer, fmodbuffer, foutbuffer,longest);

    // copy outbuffer to audio and do float back

    for (xx=0;xx<longest;xx++){
      tmp = (int32_t)(foutbuffer[xx] * 32768.0f);// was foutbuffer
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=(int16_t)tmp+32768;
    vill_eff->outpos+=vill_eff->step;
    if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }

    break;

  case 4: //5--convolve:
    for (xx=0;xx<tmpinlong;xx++){
      finbuffer[xx]=(float32_t)(buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768)/32768.0f;//REDO! why/how?
    }

    x=(vill_eff->modifier&15)+1;
    for (xx=0;xx<x;xx++){
      fmodbuffer[xx]=(float32_t)(buf16[(vill_eff->modstart+vill_eff->modpos++)&32767]-32768)/32768.0f;//REDO! why/how?
    }
    //void convolve1D(float* in, float* out, int dataSize, float* kernel, int kernelSize)
    convolve1D(finbuffer, foutbuffer, tmpinlong, fmodbuffer, x);

    for (xx=0;xx<tmpinlong;xx++){
    tmp = (int32_t)(foutbuffer[xx] * 32768.0f);
    tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
    buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=(int16_t)tmp;
    vill_eff->outpos+=vill_eff->step;
    if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }
    break;

  case 5: //     6--envelope follower
    for (xx=0;xx<longest;xx++){
      inbuffer[xx]=buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768;
      modbuffer[xx]=buf16[(vill_eff->modstart+vill_eff->modpos++)&32767]-32768;
    }

    doenvelopefollower(modbuffer, (vill_eff->modifier>>4), inbuffer, tmpinlong, outbuffer);

    for (xx=0;xx<longest;xx++){
      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=outbuffer[xx]+32768;
      vill_eff->outpos+=vill_eff->step;
      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }
    break;

  case 6://      7+--windower - diff windows// also 32 ???
    for (xx=0;xx<tmpinlong;xx++){
      inbuffer[xx]=buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768;
    }

    switch((vill_eff->modifier>>5)&3){
    case 0:
    default:
      hanningprocess(inbuffer, outbuffer, tmpinlong); 
      break;
    case 1:
      hammingprocess(inbuffer, outbuffer, tmpinlong); 
      break;
    case 2:
      blackmanprocess(inbuffer, outbuffer, tmpinlong); 
      break;
    }

    for (xx=0;xx<tmpinlong;xx++){
      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=outbuffer[xx]+32768;
      vill_eff->outpos+=vill_eff->step;
      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
    }
    break;

  case 7://      8--variable bandpass based on modifier 
    freq = (float32_t)(vill_eff->modifier)/1024.0f; // mod is now 8 bits
    float fb= 0.8f + 0.8f/(1.0f - freq);

    //    xx=bandpassmod(xxx,0.9f,10.0f,1.0f); // q freq gain

    for (xx=0;xx<tmpinlong;xx++){
      tmpp=(float32_t)((buf16[(vill_eff->instart+vill_eff->inpos++)&32767]-32768))/32768.0f;//REDO! why/how?
      //      tmpp=dobandpass(tmpp, freq,fb); // from OWL code - statevariable
      hp=tmpp-buf0;
      bp = buf0 - buf1; 
      buf0 = buf0 + freq * (hp + fb * bp); 
      buf1 = buf1 + freq * (buf0 - buf1);
      
	// out here
      tmp = (int32_t)(bp * 32768.0f);
      tmp = (tmp <= -32768) ? -32768 : (tmp >= 32767) ? 32767 : tmp;
      buf16[(vill_eff->outstart+vill_eff->outpos)&32767]=(int16_t)tmp+32768;
      vill_eff->outpos+=vill_eff->step;
      if (vill_eff->outpos>vill_eff->outwrap) vill_eff->outpos=0;
      }
    break;
  }
    }
}
*/

void test_effect(int16_t* inbuffer, int16_t* outbuffer){
  //  extern VocoderInstance* vocoder; u8 x;
  float xx,xxx;
  float finbuffer[32],foutbuffer[32];
  float tmpbuffer[32];
  //  float tmpotherotherbuffer[32];
  //  float out[BUFF_LEN];
  //      int_to_floot(inbuffer,finbuffer,32);
  //      doformantfilter(inbuffer, outbuffer, 32, 0);
  //      doformantfilterf(finbuffer, foutbuffer, 32, 0);
  //        floot_to_int(outbuffer,foutbuffer,32);


  // BPFSC
  /*  int_to_floot(inbuffer,tmpbuffer);
  BPFSC_process(bpfunit,32,tmpbuffer,tmpotherbuffer);
  floot_to_int(outbuffer,tmpotherbuffer);
  */


  // env without lowpass
  //  envelopefollower(inbuffer, outbuffer);

  //mdavocoder - working
  /*    int_to_floot(inbuffer,tmpbuffer,32);
  intun_to_floot(buf16,tmpotherbuffer,32);
  mdaVocoderprocess(mdavocod,tmpbuffer, tmpotherbuffer, tmpotherotherbuffer,32);
  floot_to_int(outbuffer,tmpotherotherbuffer,32);*/
  //  floot_to_int(outbuffer,tmpotherbuffer,32);


    ///mdavocal (vocoder carrier gen???)
  
  /*
  int_to_floot(inbuffer,tmpbuffer);
  intun_to_floot(buf16,tmpotherbuffer);
  mdavocal_process(unitttt,tmpbuffer, tmpotherbuffer, tmpotherotherbuffer,32);
  floot_to_int(outbuffer,tmpotherotherbuffer);
  */


  // Formlet from SC
    /*    int_to_floot(inbuffer,tmpbuffer);
    Formlet_process(unitt,32,tmpbuffer,tmpotherbuffer);
    floot_to_int(outbuffer,tmpotherbuffer);*/

  // BBandPass from SC
    /*    int_to_floot(inbuffer,tmpbuffer);
        BBandPass_process(unit,32,tmpbuffer,tmpotherbuffer);
        floot_to_int(outbuffer,tmpotherbuffer);
    */


  // BIQUAD bandpass from biquad.c
  /*  int_to_floot(inbuffer,tmpbuffer);
    for (x=0;x<32;x++){
    xxx=tmpbuffer[x];
    xx=BiQuad(xxx,biquaddd); 
    tmpbuffer[x]=xx;
      }
      floot_to_int(outbuffer,tmpbuffer);*/

  // VOCODER:
  // convert to float
  /*  int_to_floot(inbuffer,tmpbuffer);
   intun_to_floot(buf16,tmpotherbuffer);
   runVocoder(vocoder, tmpbuffer, tmpotherbuffer, tmpbuffer, BUFF_LEN/4);
  floot_to_int(outbuffer,tmpbuffer);
  */

  // BANDPASS:

  int_to_floot(inbuffer,tmpbuffer,32);
    for (u8 x=0;x<32;x++){
    //    xxx=(float)inbuffer[x]/32768.0;
    xxx=tmpbuffer[x];
    //    xx=bandpass(xxx,0.8f,270.0f,1.0f); // q freq gain
    //    xx+=bandpass(xxx,0.8f,2300.0f,1.0f); // q freq gain
    //    xx=bandpass(xxx,0.9f,10.0f,1.0f); // q freq gain
    //xx=bandpass(xxx,0.9f,10.0f,1.0f); // q freq gain
    //xx=dobandpass(xxx,0.0019f);
    tmpbuffer[x]=xx;
      }
    floot_to_int(outbuffer,tmpbuffer,32);

  // CONVOLVE:
  /*    int_to_floot(inbuffer,tmpbuffer);
    intun_to_floot(buf16,tmpotherbuffer);
    convolvee(tmpbuffer,32,tmpotherbuffer,16,out);
  //void convolvee(float *convolve1, u16 n1, float *convolve2, u16 n2, float *out)
  floot_to_int(outbuffer,out);*/
    }


/* 1-envelope

//If you don't need different attack and decay settings, just use in->abs()->LP

// Slightly faster version of the envelope follower using one multiply form.

// attTime and relTime is in seconds

float ga = exp(-1.0f/(sampleRate*attTime));
float gr = exp(-1.0f/(sampleRate*relTime));

float envOut = 0.0f;

for( ... )
{
    // get your data into 'input'

    envIn = fabsf(input);

    if( envOut < envIn )
        envOut = envIn + ga * (envOut - envIn);
    else
        envOut = envIn + gr * (envOut - envIn);

    // envOut now contains the envelope
}

*/



