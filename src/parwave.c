/* An implementation of a Klatt cascade-parallel formant synthesizer.
 *
 * Copyright (C) 2011-2013 Reece H. Dunn
 * (c) 1993,94 Jon Iles and Nick Ing-Simmons
 *
 * A re-implementation in C of Dennis Klatt's Fortran code, originally by:
 *
 *     Jon Iles (j.p.iles@cs.bham.ac.uk)
 *     Nick Ing-Simmons (nicki@lobby.ti.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "parwave.h"

#define getrandom(min,max) ((rand()%(int)(((max)+1)-(min)))+(min))
//#define getrandom(min,max) 0

void flutter(klatt_global_ptrr,int16_t*);
static float sampled_source(klatt_global_ptrr);
static float impulsive_source(klatt_global_ptrr);
static float natural_source(klatt_global_ptrr);
static void pitch_synch_par_reset(klatt_global_ptrr,int16_t*);
static float gen_noise(klatt_global_ptrr);
static float DBtoLIN(u16);
//static void frame_init(klatt_global_ptrr,int16_t*);
static float resonator(resonator_ptr, float);
static float antiresonator(resonator_ptr, float);
static void setabc(u16,u16,resonator_ptr,klatt_global_ptrr);
static void setzeroabc(u16,u16,resonator_ptr,klatt_global_ptrr);

/** @brief A generic resonator.
  *
  * Internal memory for the resonator is stored in the globals structure.
  */
static float resonator(resonator_ptr r, float input)
{
  float x = r->a * input + r->b * r->p1 + r->c * r->p2;
 r->p2 = r->p1;
 r->p1 = x;
 return x;
}

/** @brief A generic anti-resonator.
  *
  * The code is the same as resonator except that a,b,c need to be set with
  * setzeroabc() and we save inputs in p1/p2 rather than outputs. There is
  * currently only one of these - "rnz".
  */
static float antiresonator(resonator_ptr r, float input)
{
  float x = r->a * input + r->b * r->p1 + r->c * r->p2;
 r->p2 = r->p1;
 r->p1 = input;
 return x;
}

/** @brief Add F0 flutter.
  *
  * See "Analysis, synthesis and perception of voice quality variations among
  * female and male talkers" D.H. Klatt and L.C. Klatt JASA 87(2) February 1990.
  *
  * Flutter is added by applying a quasi-random element constructed from three
  * slowly varying sine waves.
  */

void flutter(klatt_global_ptrr globals, int16_t* frame)
{
  static int time_count;
  float delta_f0;
  float fla,flb,flc,fld,fle;

  fla = (float) globals->f0_flutter / 50;
  flb = (float) globals->original_f0 / 100;
  flc = sinf(2*M_PI*12.7f*time_count);
  fld = sinf(2*M_PI*7.1f*time_count);
  fle = sinf(2*M_PI*4.7f*time_count);
  delta_f0 =  fla * flb * (flc + fld + fle) * 10;
  frame[0] = frame[0] + (u16) delta_f0;
  time_count++;
}

/** @brief Allows the use of a glottal excitation waveform sampled from a real voice.
  */
static float sampled_source(klatt_global_ptrr globals)
{
  int itemp;
  float ftemp;
  float result;
  float diff_value;
  int current_value;
  int next_value;
  float temp_diff;

  if(globals->T0!=0)
  {
    ftemp = (float) globals->nper;
    ftemp = ftemp / globals->T0;
    ftemp = ftemp * globals->num_samples;
    itemp = (int) ftemp;

    temp_diff = ftemp - (float) itemp;
  
    current_value = globals->natural_samples[itemp];
    next_value = globals->natural_samples[itemp+1];

    diff_value = (float) next_value - (float) current_value;
    diff_value = diff_value * temp_diff;

    result = globals->natural_samples[itemp] + diff_value;
    //    result = result * globals->sample_factor;
    result = result * 2.0f;
    //    printf("xxx %f",result);
  }
  else
  {
    result = 0;
  }
  return(result);
}


void single_parwave(klatt_global_ptrr globals, int16_t *frame, u8 newframe, u16 samplenumber,u8 x, int16_t *outgoing){
    float noise;
    u8 n4;
    float out = 0.0f;
    float frics;
    float glotout;
    float aspiration;
    float par_glotout;
    float voice;
    float sourc;
    static float glotlast=0.0f;
    static float vlast=0.0f;

    
    if (newframe==1){
      frame_init(globals,frame); 
    if (globals->f0_flutter != 0)
      flutter(globals,frame);  
    globals->ns=0;
    }

    /* Get low-passed random number for aspiration and frication noise */

    noise = gen_noise(globals);

    /*    
      Amplitude modulate noise (reduce noise amplitude during
      second half of glottal period) if voicing simultaneously present.
    */

    if (globals->nper > globals->nmod) 
    {
      noise *= (float) 0.5;
    }

    /* Compute frication noise */

    frics = globals->amp_frica * noise;

    /*  
      Compute voicing waveform. Run glottal source simulation at 4 
      times normal sample rate to minimize quantization noise in 
      period of female voice.
    */

    for (n4=0; n4<4; n4++) 
    {
      switch(globals->glsource)
      {
      case 1:
	voice = impulsive_source(globals);
	break;
      case 2:
	voice = natural_source(globals);	
	break;
      case 3:
	voice = sampled_source(globals);
	break;
      }

      /* Reset period when counter 'nper' reaches T0 */

      if (globals->nper >= globals->T0) 
      {
	globals->nper = 0;
	pitch_synch_par_reset(globals,frame);
      }

      /*        
	Low-pass filter voicing waveform before downsampling from 4*samrate
	to samrate samples/sec.  Resonator f=.09*samrate, bw=.06*samrate 
      */

      voice = resonator(&(globals->rlp),voice);

      /* Increment counter that keeps track of 4*samrate samples per sec */

      globals->nper++;
    }

    /*
      Tilt spectrum of voicing source down by soft low-pass filtering, amount
      of tilt determined by TLTdb
    */


    voice = (voice * globals->onemd) + (vlast * globals->decay);
    vlast = voice;


    /* 
      Add breathiness during glottal open phase. Amount of breathiness 
      determined by parameter Aturb Use nrand rather than noise because 
      noise is low-passed. 
    */


    if (globals->nper < globals->nopen) 
    {
      voice += globals->amp_breth * globals->nrand;
    }


    /* Set amplitude of voicing */

    glotout = globals->amp_voice * voice;
    par_glotout = globals->par_amp_voice * voice;


    /* Compute aspiration amplitude and add to voicing source */


    aspiration = globals->amp_aspir * noise;
    glotout += aspiration;
  

    par_glotout += aspiration;


    if(globals->synthesis_model != 1) // paralllel
    {
      /*
       * Cascade vocal tract, excited by laryngeal sources.
       * Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
       */
      float rnzout = antiresonator(&(globals->rnz),glotout);
      float casc_next_in = resonator(&(globals->rnpc),rnzout);

      switch (globals->nfcascade)
      {
      case 8:  casc_next_in = resonator(&(globals->r8c),casc_next_in);
      case 7:  casc_next_in = resonator(&(globals->r7c),casc_next_in);
      case 6:  casc_next_in = resonator(&(globals->r6c),casc_next_in);
      case 5:  casc_next_in = resonator(&(globals->r5c),casc_next_in);
      case 4:  casc_next_in = resonator(&(globals->r4c),casc_next_in);
      case 3:  casc_next_in = resonator(&(globals->r3c),casc_next_in);
      case 2:  casc_next_in = resonator(&(globals->r2c),casc_next_in);
      case 1:  out          = resonator(&(globals->r1c),casc_next_in);
      }
    }

    /* Excite parallel F1 and FNP by voicing waveform */

    /*  
      Standard parallel vocal tract Formants F6,F5,F4,F3,F2, 
      outputs added with alternating sign. Sound sourc for other 
      parallel resonators is frication plus first difference of 
      voicing waveform. 
    */

    out += resonator(&(globals->r1p),par_glotout);
    out += resonator(&(globals->rnpp),par_glotout);

    sourc = frics + par_glotout - glotlast;
    glotlast = par_glotout;

    out = resonator(&(globals->r6p),sourc) - out;
    out = resonator(&(globals->r5p),sourc) - out;
    out = resonator(&(globals->r4p),sourc) - out;
    out = resonator(&(globals->r3p),sourc) - out;
    out = resonator(&(globals->r2p),sourc) - out;

    out = globals->amp_bypas * sourc - out;

    out = resonator(&(globals->rout),out);

    //    out = out * globals->amp_gain0;  /* Convert back to integer */
    out = out * 32768.0f; // TODO - gain????
    if (out < -32767) out = -32767;
    if (out > 32767) out = 32767;
   
    outgoing[x]=(int)out;
    //    outgoing[x]=(int)voice * globals->amp_gain0;
    //    outgoing[x]=(int)noise;

  globals->ns++;
}


int16_t single_single_parwave(klatt_global_ptrr globals, int16_t *frame){
    float noise;
    u8 n4;
    float out = 0.0f;
    float frics;
    float glotout;
    float aspiration;
    float par_glotout;
    float voice;
    float sourc;
    static float glotlast=0.0f;
    static float vlast=0.0f;

    /* Get low-passed random number for aspiration and frication noise */

    noise = gen_noise(globals);

    /*    
      Amplitude modulate noise (reduce noise amplitude during
      second half of glottal period) if voicing simultaneously present.
    */

    if (globals->nper > globals->nmod) 
    {
      noise *= (float) 0.5f;
    }

    /* Compute frication noise */

    frics = globals->amp_frica * noise;

    /*  
      Compute voicing waveform. Run glottal source simulation at 4 
      times normal sample rate to minimize quantization noise in 
      period of female voice.
    */

    for (n4=0; n4<4; n4++) 
    {
      switch(globals->glsource)
      {
      case 1:
	voice = impulsive_source(globals);
	break;
      case 2:
	voice = natural_source(globals);	
	break;
      case 3:
	voice = sampled_source(globals);
	break;
      }

      /* Reset period when counter 'nper' reaches T0 */

      if (globals->nper >= globals->T0) 
      {
	globals->nper = 0;
	pitch_synch_par_reset(globals,frame);
      }

      /*        
	Low-pass filter voicing waveform before downsampling from 4*samrate
	to samrate samples/sec.  Resonator f=.09*samrate, bw=.06*samrate 
      */

      voice = resonator(&(globals->rlp),voice);

      /* Increment counter that keeps track of 4*samrate samples per sec */

      globals->nper++;
    }

    /*
      Tilt spectrum of voicing source down by soft low-pass filtering, amount
      of tilt determined by TLTdb
    */


    voice = (voice * globals->onemd) + (vlast * globals->decay);
    vlast = voice;


    /* 
      Add breathiness during glottal open phase. Amount of breathiness 
      determined by parameter Aturb Use nrand rather than noise because 
      noise is low-passed. 
    */


    if (globals->nper < globals->nopen) 
    {
      voice += globals->amp_breth * globals->nrand;
    }


    /* Set amplitude of voicing */

    glotout = globals->amp_voice * voice;
    par_glotout = globals->par_amp_voice * voice;


    /* Compute aspiration amplitude and add to voicing source */


    aspiration = globals->amp_aspir * noise;
    glotout += aspiration;
  

    par_glotout += aspiration;


    if(globals->synthesis_model != 1) // paralllel
    {
      /*
       * Cascade vocal tract, excited by laryngeal sources.
       * Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
       */
      float rnzout = antiresonator(&(globals->rnz),glotout);
      float casc_next_in = resonator(&(globals->rnpc),rnzout);

      switch (globals->nfcascade)
      {
      case 8:  casc_next_in = resonator(&(globals->r8c),casc_next_in);
      case 7:  casc_next_in = resonator(&(globals->r7c),casc_next_in);
      case 6:  casc_next_in = resonator(&(globals->r6c),casc_next_in);
      case 5:  casc_next_in = resonator(&(globals->r5c),casc_next_in);
      case 4:  casc_next_in = resonator(&(globals->r4c),casc_next_in);
      case 3:  casc_next_in = resonator(&(globals->r3c),casc_next_in);
      case 2:  casc_next_in = resonator(&(globals->r2c),casc_next_in);
      case 1:  out          = resonator(&(globals->r1c),casc_next_in);
      }
    }

    /* Excite parallel F1 and FNP by voicing waveform */

    /*  
      Standard parallel vocal tract Formants F6,F5,F4,F3,F2, 
      outputs added with alternating sign. Sound sourc for other 
      parallel resonators is frication plus first difference of 
      voicing waveform. 
    */

    out += resonator(&(globals->r1p),par_glotout);
    out += resonator(&(globals->rnpp),par_glotout);

    sourc = frics + par_glotout - glotlast;
    glotlast = par_glotout;

    out = resonator(&(globals->r6p),sourc) - out;
    out = resonator(&(globals->r5p),sourc) - out;
    out = resonator(&(globals->r4p),sourc) - out;
    out = resonator(&(globals->r3p),sourc) - out;
    out = resonator(&(globals->r2p),sourc) - out;

    out = globals->amp_bypas * sourc - out;

    out = resonator(&(globals->rout),out);

    out = out * globals->amp_gain0;  /* Convert back to integer */

    if (out < -32767) out = -32767;
    if (out > 32767) out = 32767;
    //    out=rand()%32768;

    //    outgoing[x]=(int)out;
    //    outgoing[x]=(int)voice * globals->amp_gain0;
    //    outgoing[x]=(int)noise;
    
  globals->ns++;
  return (int)out;
}


/** @brief Initialise all parameters used in parwave.
  *
  * This sets resonator internal memory to zero.
  */
void simple_parwave_init(klatt_global_ptrr globals)
{
  globals->FLPhz = (950 * globals->samrate) / 10000;
  globals->BLPhz = (630 * globals->samrate) / 10000;
  setabc(globals->FLPhz,globals->BLPhz,&(globals->rlp),globals);
  globals->nper = 0;
  globals->T0 = 0;
  globals->nopen = 0;
  globals->nmod = 0;

  globals->rnpp.p1=0;
  globals->r1p.p1=0;
  globals->r2p.p1=0;
  globals->r3p.p1=0;
  globals->r4p.p1=0;
  globals->r5p.p1=0;
  globals->r6p.p1=0;
  globals->r1c.p1=0;
  globals->r2c.p1=0;
  globals->r3c.p1=0;
  globals->r4c.p1=0;
  globals->r5c.p1=0;
  globals->r6c.p1=0;
  globals->r7c.p1=0;
  globals->r8c.p1=0;
  globals->rnpc.p1=0;
  globals->rnz.p1=0;
  globals->rgl.p1=0;
  globals->rlp.p1=0;
  globals->rout.p1=0;
  
  globals->rnpp.p2=0;
  globals->r1p.p2=0;
  globals->r2p.p2=0;
  globals->r3p.p2=0;
  globals->r4p.p2=0;
  globals->r5p.p2=0;
  globals->r6p.p2=0;
  globals->r1c.p2=0;
  globals->r2c.p2=0;
  globals->r3c.p2=0;
  globals->r4c.p2=0;
  globals->r5c.p2=0;
  globals->r6c.p2=0;
  globals->r7c.p2=0;
  globals->r8c.p2=0;
  globals->rnpc.p2=0;
  globals->rnz.p2=0;
  globals->rgl.p2=0;
  globals->rlp.p2=0;
  globals->rout.p2=0;
}

/** @brief Use parameters from the input frame to set up resonator coefficients.
  */
void frame_init(klatt_global_ptrr globals, int16_t* frame)
{
  globals->original_f0 = frame[0] / 10;

  /*  frame[1]  = frame[1] - 7;
  if (frame[1] < 0)   
  {
    frame[1] = 0;
    }*/

  globals->amp_aspir = DBtoLIN(frame[18]) * 0.05f;
  globals->amp_frica = DBtoLIN(frame[22]) * 0.25f;
  globals->par_amp_voice = DBtoLIN(frame[38]);
  globals->amp_bypas = DBtoLIN(frame[37]) * 0.05f;
  /*  frame[39] = frame[39] - 3;
  if (frame[39] <= 0) 
  {
    frame[39] = 57;
    }*/
  globals->amp_gain0 = DBtoLIN(frame[39]);

  /* Set coefficients of variable cascade resonators */
  /*
  if (globals->nfcascade >= 8)    
  {
    if (globals->samrate >= 16000){
      setabc(7500,600,&(globals->r8c),globals);
    else
      globals->nfcascade = 6;
  }
  if (globals->nfcascade >= 7)    
  {
    //    if (globals->samrate >= 16000){
      setabc(6500,500,&(globals->r7c),globals);
    //    else
    //      globals->nfcascade = 6;
    //  }
  if (globals->nfcascade >= 6)    
  {
    setabc(frame[12],frame[13],&(globals->r6c),globals);
  }
  if (globals->nfcascade >= 5)    
  {
    setabc(frame[10],frame[11],&(globals->r5c),globals);
  }
  (
  setabc(frame[8],frame[9],&(globals->r4c),globals);
  setabc(frame[6],frame[7],&(globals->r3c),globals);
  setabc(frame[4],frame[5],&(globals->r2c),globals);
  setabc(frame[2],frame[3],&(globals->r1c),globals);

  /* Set coeficients of nasal resonator and zero antiresonator */
 
  setabc(frame[16],frame[17],&(globals->rnpc),globals);
  setzeroabc(frame[14],frame[15],&(globals->rnz),globals);
  
  /* Set coefficients of parallel resonators, and amplitude of outputs */

  setabc(frame[2],frame[25],&(globals->r1p),globals);
  globals->r1p.a *= DBtoLIN(frame[24]) * 0.4f;
  setabc(frame[16],frame[17],&(globals->rnpp),globals);
  globals->rnpp.a *= DBtoLIN(frame[36]) * 0.6f;
  setabc(frame[4],frame[27],&(globals->r2p),globals);
  globals->r2p.a *= DBtoLIN(frame[26]) * 0.15f;
  setabc(frame[6],frame[29],&(globals->r3p),globals);
  globals->r3p.a *= DBtoLIN(frame[28]) * 0.06f;
  setabc(frame[8],frame[31],&(globals->r4p),globals);
  globals->r4p.a *= DBtoLIN(frame[30]) * 0.04f;
  setabc(frame[10],frame[33],&(globals->r5p),globals);
  globals->r5p.a *= DBtoLIN(frame[32]) * 0.022f;
  setabc(frame[12],frame[35],&(globals->r6p),globals);
  globals->r6p.a *= DBtoLIN(frame[34]) * 0.03f;
  
  /* output low-pass filter */

  setabc((u16)0.0f,(u16)(globals->samrate/2),&(globals->rout),globals);
}

/** @brief Generate the glottal waveform from an impulse source.
  *
  * Generate a low pass filtered train of impulses as an approximation of a
  * natural excitation waveform. Low-pass filter the differentiated impulse
  * with a critically-damped second-order filter, time constant proportional
  * to Kopen.
  */
static float impulsive_source(klatt_global_ptrr globals)
{
  static float doublet[] = {0.0,13000000.0f,-13000000.0f};
  static float vwave;

  if (globals->nper < 3) 
  {
    vwave = doublet[globals->nper];
  }
  else 
  {
    vwave = 0.0f;
  }
  
  return(resonator(&(globals->rgl),vwave));
}

/** @brief Generate the glottal waveform from a natural (sampled) source.
  *
  * Vwave is the differentiated glottal flow waveform, there is a weak
  * spectral zero around 800 Hz, magic constants a,b reset pitch
  * synchronously.
  */
static float natural_source(klatt_global_ptrr globals)
{
  float lgtemp;
  static float vwave;

  if (globals->nper < globals->nopen) 
  {
    globals->pulse_shape_a -= globals->pulse_shape_b;
    vwave += globals->pulse_shape_a;
    lgtemp=vwave * 0.028f;

    return(lgtemp);
  }
  else 
  {
    vwave = 0.0f;
    return(0.0f);
  }
}

/** @brief Reset selected parameters pitch-synchronously.
  */
static void pitch_synch_par_reset(klatt_global_ptrr globals, int16_t* frame)
{
  u16 temp;
  float temp1;
  static u16 skew;
  /*
   * Constant B0 controls shape of glottal pulse as a function
   * of desired duration of open phase N0. (Note that N0 is
   * specified in terms of 40,000 samples/sec of speech.)
   *
   * Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3.
   *
   * If the radiation characterivative, a temporal derivative
   * is folded in, and we go from continuous time to discrete
   * integers n:
   *
   *     dV/dt = vwave[n]
   *           = sum over i=1,2,...,n of { a - (i * b) }
   *           = a n  -  b/2 n**2
   *
   * where the  constants a and b control the detailed shape
   * and amplitude of the voicing waveform over the open
   * potion of the voicing cycle "nopen".
   *
   * Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3.
   *
   * Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
   * meaning as nopen gets bigger, V has bigger peak proportional to n.
   *
   * Thus, to generate the table below for 40 <= nopen <= 263:
   *
   *     B0[nopen - 40] = 1920000 / (nopen * nopen)
   */
  static short B0[224] = 
  {
    1200,1142,1088,1038, 991, 948, 907, 869, 833, 799, 768, 738, 710, 683, 658,
    634, 612, 590, 570, 551, 533, 515, 499, 483, 468, 454, 440, 427, 415, 403,
    391, 380, 370, 360, 350, 341, 332, 323, 315, 307, 300, 292, 285, 278, 272,
    265, 259, 253, 247, 242, 237, 231, 226, 221, 217, 212, 208, 204, 199, 195,
    192, 188, 184, 180, 177, 174, 170, 167, 164, 161, 158, 155, 153, 150, 147,
    145, 142, 140, 137, 135, 133, 131, 128, 126, 124, 122, 120, 119, 117, 115, 
    113,111, 110, 108, 106, 105, 103, 102, 100, 99, 97, 96, 95, 93, 92, 91, 90,
    88, 87, 86, 85, 84, 83, 82, 80, 79, 78, 77, 76, 75, 75, 74, 73, 72, 71,
    70, 69, 68, 68, 67, 66, 65, 64, 64, 63, 62, 61, 61, 60, 59, 59, 58, 57, 
    57, 56, 56, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48,
    47, 47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42, 41, 41, 41, 41, 40, 40,
    39, 39, 38, 38, 38, 38, 37, 37, 36, 36, 36, 36, 35, 35, 35, 35, 34, 34,33,
    33, 33, 33, 32, 32, 32, 32, 31, 31, 31, 31, 30, 30, 30, 30, 29, 29, 29, 29,
    28, 28, 28, 28, 27, 27
  };

  if (frame[0] > 0) 
  {
    /* T0 is 4* the number of samples in one pitch period */

    globals->T0 = (40 * globals->samrate) / frame[0];


    globals->amp_voice = DBtoLIN(frame[1]);

    /* Duration of period before amplitude modulation */

    globals->nmod = globals->T0;
    if (frame[1] > 0) 
    {
      globals->nmod >>= 1;
    }

    /* Breathiness of voicing waveform */

    globals->amp_breth = DBtoLIN(frame[20]) * 0.1f;

    /* Set open phase of glottal period where  40 <= open phase <= 263 */

    globals->nopen = 4 * frame[19];

    if ((globals->glsource == 1) && (globals->nopen > 263))    
    {
      globals->nopen = 263;
    }

    if (globals->nopen >= (globals->T0-1)) 
    {
      //      fprintf(stderr, "Warning: glottal open period cannot exceed T0, truncated as was %d T0=%d \n",globals->nopen,globals->T0);
      globals->nopen = globals->T0 - 2;
      /*      if(globals->quiet_flag == FALSE)
      {
	fprintf(stderr, "Warning: glottal open period cannot exceed T0, truncated \n");
	}*/
    }

    if (globals->nopen < 40) 
    {
      /* F0 max = 1000 Hz */
      globals->nopen = 40;    
    }


    /* Reset a & b, which determine shape of "natural" glottal waveform */

    globals->pulse_shape_b = B0[globals->nopen-40];
    globals->pulse_shape_a = (globals->pulse_shape_b * globals->nopen) * 0.333f;

    /* Reset width of "impulsive" glottal pulse */

    temp = globals->samrate / globals->nopen;

    setabc((u16)0,temp,&(globals->rgl),globals);

    /* Make gain at F1 about constant */

    temp1 = globals->nopen *.00833f;
    globals->rgl.a *= temp1 * temp1;
    
    /*
      Truncate skewness so as not to exceed duration of closed phase
      of glottal period.
    */


    temp = globals->T0 - globals->nopen;
    if (frame[23] > temp) 
    {
      frame[23] = temp;
    }
    if (skew >= 0) 
    {
      skew = frame[23];
    }
    else 
    {
      skew = - frame[23];
    }

    /* Add skewness to closed portion of voicing period */


    globals->T0 = globals->T0 + skew;
    skew = - skew;

  }

  else 
  {
    globals->T0 = 4;                     /* Default for f0 undefined */
    globals->amp_voice = 0.0f;
    globals->nmod = globals->T0;
    globals->amp_breth = 0.0f;
    globals->pulse_shape_a = 0.0f;
    globals->pulse_shape_b = 0.0f;
  }

  /* Reset these pars pitch synchronously or at update rate if f0=0 */

  if ((globals->T0 != 4) || (globals->ns == 0)) 
  {
    /* Set one-pole low-pass filter that tilts glottal source */

    globals->decay = (0.033f * frame[21]);

    if (globals->decay > 0.0f) 
    {
      globals->onemd = 1.0f - globals->decay;
    }
    else 
    {
      globals->onemd = 1.0f;
    }
  }
}

/** Convert formant freqencies and bandwidth into resonator difference equation constants.
  *
  * @param f   Frequency of resonator in Hz
  * @param bw  Frequency of resonator in Hz
  */
static void setabc(u16 f, u16 bw, resonator_ptr rp, klatt_global_ptrr globals)
{
 float r;

 r = expf(-M_PI / globals->samrate * bw);
 rp->c = -(r * r);
 rp->b = r * cosf(2.0f * M_PI / globals->samrate * f) * 2.0f;
 rp->a = 1.0 - rp->b - rp->c;
}

/** @brief Convert formant freqencies and bandwidth into anti-resonator difference equation constants.
  *
  * @param f   Frequency of resonator in Hz
  * @param bw  Frequency of resonator in Hz
  */
static void setzeroabc(u16 f, u16 bw, resonator_ptr rp, klatt_global_ptrr globals)
{
 float r;

 /* First compute ordinary resonator coefficients */
 r = expf(-M_PI / globals->samrate * bw);
 rp->c = -(r * r);
 rp->b = r * cosf(2.0 * M_PI / globals->samrate * -f) * 2.0f;
 rp->a = 1.0 - rp->b - rp->c;

 if (f != 0) /* prevent a', b' and c' going to INF! */
 {
  /* Now convert to antiresonator coefficients (a'=1/a, b'=b/a, c'=c/a) */
  rp->a = 1.0f / rp->a;
  rp->c *= -rp->a;
  rp->b *= -rp->a;
 }
}

/** @brief Random number (noise) generator.
  *
  * @return a number between -8191 and +8191.
  *
  * Noise spectrum is tilted down by soft low-pass filter having a pole near 
  * the origin in the z-plane, i.e. output = input + (0.75 * lastoutput) 
  */
static float gen_noise(klatt_global_ptrr globals)
{
  u16 temp;
  static float nlast;

  temp = (u16) getrandom(-8191,8191);
  globals->nrand = (u16) temp;
  nlast = globals->nrand + (0.75f * nlast);

  return(nlast);
}

/** @brief Convert from decibels to a linear scale factor.
  *
  * Conversion table, db to linear:
  *
  *     87 dB --> 32767
  *     86 dB --> 29491 (1 dB down = 0.5**1/6)
  *     ...
  *     81 dB --> 16384 (6 dB down = 0.5)
  *     ...
  *     0 dB -->      0
  *
  * The just noticeable difference for a change in intensity of a vowel
  * is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
  * steps.
  */
static float DBtoLIN(u16 dB)
{
  float lgtemp;
  static const float amptable[88] = 
  {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 6.0f, 7.0f,
    8.0f, 9.0f, 10.0f, 11.0f, 13.0f, 14.0f, 16.0f, 18.0f, 20.0f, 22.0f, 25.0f, 28.0f, 32.0f,
    35.0f, 40.0f, 45.0f, 51.0f, 57.0f, 64.0f, 71.0f, 80.0f, 90.0f, 101.0f, 114.0f, 128.0f,
    142.0f, 159.0f, 179.0f, 202.0f, 227.0f, 256.0f, 284.0f, 318.0f, 359.0f, 405.0f,
    455.0f, 512.0f, 568.0f, 638.0f, 719.0f, 811.0f, 911.0f, 1024.0f, 1137.0f, 1276.0f,
    1438.0f, 1622.0f, 1823.0f, 2048.0f, 2273.0f, 2552.0f, 2875.0f, 3244.0f, 3645.0f, 
    4096.0f, 4547.0f, 5104.0f, 5751.0f, 6488.0f, 7291.0f, 8192.0f, 9093.0f, 10207.0f, 
    11502.0f, 12976.0f, 14582.0f, 16384.0f, 18350.0f, 20644.0f, 23429.0f,
    26214.0f, 29491.0f, 32767.0f
  };

  if ((dB < 0) || (dB > 87))
  {
      return(0);
  }            

  lgtemp=amptable[dB] * .001f;
  return(lgtemp);
}
