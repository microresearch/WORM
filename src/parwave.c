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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "parwave.h"

#ifdef _MSC_VER
#define getrandom(min,max) ((rand()%(int)(((max)+1)-(min)))+(min))
#else
#define getrandom(min,max) ((rand()%(long)(((max)+1)-(min)))+(min))
#endif

/* function prototypes for functions private to this file */

static void flutter(klatt_global_ptr,klatt_frame_ptr);
static float sampled_source(klatt_global_ptr);
static float impulsive_source(klatt_global_ptr);
static float natural_source(klatt_global_ptr);
static void pitch_synch_par_reset(klatt_global_ptr,klatt_frame_ptr);
static float gen_noise(klatt_global_ptr);
static float DBtoLIN(long);
static void frame_init(klatt_global_ptr,klatt_frame_ptr);
static float resonator(resonator_ptr, float);
static float antiresonator(resonator_ptr, float);
static void setabc(long,long,resonator_ptr,klatt_global_ptr);
static void setzeroabc(long,long,resonator_ptr,klatt_global_ptr);

/** @brief A generic resonator.
  *
  * Internal memory for the resonator is stored in the globals structure.
  */
static float resonator(resonator_ptr r, float input)
{
 register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
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
 register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
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
static void flutter(klatt_global_ptr globals, klatt_frame_ptr frame)
{
  static int time_count;
  double delta_f0;
  double fla,flb,flc,fld,fle;

  fla = (double) globals->f0_flutter / 50;
  flb = (double) globals->original_f0 / 100;
  flc = sin(2*M_PI*12.7*time_count);
  fld = sin(2*M_PI*7.1*time_count);
  fle = sin(2*M_PI*4.7*time_count);
  delta_f0 =  fla * flb * (flc + fld + fle) * 10;
  frame->F0hz10 = frame->F0hz10 + (long) delta_f0;
  time_count++;
}

/** @brief Allows the use of a glottal excitation waveform sampled from a real voice.
  */
static float sampled_source(klatt_global_ptr globals)
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

/** @brief Converts synthesis parameters to a waveform.
  */
void parwave(klatt_global_ptr globals, klatt_frame_ptr frame, int *output)
{
  static float glotlast;
  static float vlast;

  frame_init(globals,frame);  /* get parameters for next frame of speech */

  if (globals->f0_flutter != 0)
    flutter(globals,frame);  /* add f0 flutter */


  /* MAIN LOOP, for each output sample of current frame: */

  for (globals->ns=0;globals->ns<globals->nspfr;globals->ns++) 
  {
    float noise;
    long n4;
    float out = 0.0;
    float frics;
    float glotout;
    float aspiration;
    float par_glotout;
    float voice;
    float sourc;

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
      case IMPULSIVE:
	voice = impulsive_source(globals);
	break;
      case NATURAL:
	voice = natural_source(globals);	
	break;
      case SAMPLED:
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


    if(globals->synthesis_model != ALL_PARALLEL)
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

    if (out < SHRT_MIN) out = SHRT_MIN;
    if (out > SHRT_MAX) out = SHRT_MAX;

    *output++ = (int)out;
  }
}

/** @brief Initialise all parameters used in parwave.
  *
  * This sets resonator internal memory to zero.
  */
void parwave_init(klatt_global_ptr globals)
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
static void frame_init(klatt_global_ptr globals, klatt_frame_ptr frame)
{
  globals->original_f0 = frame->F0hz10 / 10;

  frame->AVdb  = frame->AVdb - 7;
  if (frame->AVdb < 0)   
  {
    frame->AVdb = 0;
  }

  globals->amp_aspir = DBtoLIN(frame->ASP) * 0.05;
  globals->amp_frica = DBtoLIN(frame->AF) * 0.25;
  globals->par_amp_voice = DBtoLIN(frame->AVpdb);
  globals->amp_bypas = DBtoLIN(frame->AB) * 0.05;
  frame->Gain0 = frame->Gain0 - 3;
  if (frame->Gain0 <= 0) 
  {
    frame->Gain0 = 57;
  }
  globals->amp_gain0 = DBtoLIN(frame->Gain0);

  /* Set coefficients of variable cascade resonators */

  if (globals->nfcascade >= 8)    
  {
    if (globals->samrate >= 16000) /* Inside Nyquist rate? */
      setabc(7500,600,&(globals->r8c),globals);
    else
      globals->nfcascade = 6;
  }
  if (globals->nfcascade >= 7)    
  {
    if (globals->samrate >= 16000) /* Inside Nyquist rate? */
      setabc(6500,500,&(globals->r7c),globals);
    else
      globals->nfcascade = 6;
  }
  if (globals->nfcascade >= 6)    
  {
    setabc(frame->F6hz,frame->B6hz,&(globals->r6c),globals);
  }
  if (globals->nfcascade >= 5)    
  {
    setabc(frame->F5hz,frame->B5hz,&(globals->r5c),globals);
  }
  setabc(frame->F4hz,frame->B4hz,&(globals->r4c),globals);
  setabc(frame->F3hz,frame->B3hz,&(globals->r3c),globals);
  setabc(frame->F2hz,frame->B2hz,&(globals->r2c),globals);
  setabc(frame->F1hz,frame->B1hz,&(globals->r1c),globals);

  /* Set coeficients of nasal resonator and zero antiresonator */
 
  setabc(frame->FNPhz,frame->BNPhz,&(globals->rnpc),globals);
  setzeroabc(frame->FNZhz,frame->BNZhz,&(globals->rnz),globals);
  
  /* Set coefficients of parallel resonators, and amplitude of outputs */

  setabc(frame->F1hz,frame->B1phz,&(globals->r1p),globals);
  globals->r1p.a *= DBtoLIN(frame->A1) * 0.4;
  setabc(frame->FNPhz,frame->BNPhz,&(globals->rnpp),globals);
  globals->rnpp.a *= DBtoLIN(frame->ANP) * 0.6;
  setabc(frame->F2hz,frame->B2phz,&(globals->r2p),globals);
  globals->r2p.a *= DBtoLIN(frame->A2) * 0.15;
  setabc(frame->F3hz,frame->B3phz,&(globals->r3p),globals);
  globals->r3p.a *= DBtoLIN(frame->A3) * 0.06;
  setabc(frame->F4hz,frame->B4phz,&(globals->r4p),globals);
  globals->r4p.a *= DBtoLIN(frame->A4) * 0.04;
  setabc(frame->F5hz,frame->B5phz,&(globals->r5p),globals);
  globals->r5p.a *= DBtoLIN(frame->A5) * 0.022;
  setabc(frame->F6hz,frame->B6phz,&(globals->r6p),globals);
  globals->r6p.a *= DBtoLIN(frame->A6) * 0.03;
  
  /* output low-pass filter */

  setabc((long)0.0,(long)(globals->samrate/2),&(globals->rout),globals);
}

/** @brief Generate the glottal waveform from an impulse source.
  *
  * Generate a low pass filtered train of impulses as an approximation of a
  * natural excitation waveform. Low-pass filter the differentiated impulse
  * with a critically-damped second-order filter, time constant proportional
  * to Kopen.
  */
static float impulsive_source(klatt_global_ptr globals)
{
  static float doublet[] = {0.0,13000000.0,-13000000.0};
  static float vwave;

  if (globals->nper < 3) 
  {
    vwave = doublet[globals->nper];
  }
  else 
  {
    vwave = 0.0;
  }
  
  return(resonator(&(globals->rgl),vwave));
}

/** @brief Generate the glottal waveform from a natural (sampled) source.
  *
  * Vwave is the differentiated glottal flow waveform, there is a weak
  * spectral zero around 800 Hz, magic constants a,b reset pitch
  * synchronously.
  */
static float natural_source(klatt_global_ptr globals)
{
  float lgtemp;
  static float vwave;

  if (globals->nper < globals->nopen) 
  {
    globals->pulse_shape_a -= globals->pulse_shape_b;
    vwave += globals->pulse_shape_a;
    lgtemp=vwave * 0.028;

    return(lgtemp);
  }
  else 
  {
    vwave = 0.0;
    return(0.0);
  }
}

/** @brief Reset selected parameters pitch-synchronously.
  */
static void pitch_synch_par_reset(klatt_global_ptr globals, klatt_frame_ptr frame)
{
  long temp;
  float temp1;
  static long skew;
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

  if (frame->F0hz10 > 0) 
  {
    /* T0 is 4* the number of samples in one pitch period */

    globals->T0 = (40 * globals->samrate) / frame->F0hz10;


    globals->amp_voice = DBtoLIN(frame->AVdb);

    /* Duration of period before amplitude modulation */

    globals->nmod = globals->T0;
    if (frame->AVdb > 0) 
    {
      globals->nmod >>= 1;
    }

    /* Breathiness of voicing waveform */

    globals->amp_breth = DBtoLIN(frame->Aturb) * 0.1;

    /* Set open phase of glottal period where  40 <= open phase <= 263 */

    globals->nopen = 4 * frame->Kopen;

    if ((globals->glsource == IMPULSIVE) && (globals->nopen > 263))    
    {
      globals->nopen = 263;
    }

    if (globals->nopen >= (globals->T0-1)) 
    {
      fprintf(stderr, "Warning: glottal open period cannot exceed T0, truncated as was %d T0=%d \n",globals->nopen,globals->T0);
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
      if(globals->quiet_flag == FALSE)
      {
	fprintf(stderr, "Warning: minimum glottal open period is 10 samples.\n");
	fprintf(stderr, "truncated, nopen = %i\n",(int)globals->nopen);
      }
    }


    /* Reset a & b, which determine shape of "natural" glottal waveform */

    globals->pulse_shape_b = B0[globals->nopen-40];
    globals->pulse_shape_a = (globals->pulse_shape_b * globals->nopen) * 0.333;

    /* Reset width of "impulsive" glottal pulse */

    temp = globals->samrate / globals->nopen;

    setabc((long)0,temp,&(globals->rgl),globals);

    /* Make gain at F1 about constant */

    temp1 = globals->nopen *.00833;
    globals->rgl.a *= temp1 * temp1;
    
    /*
      Truncate skewness so as not to exceed duration of closed phase
      of glottal period.
    */


    temp = globals->T0 - globals->nopen;
    if (frame->Kskew > temp) 
    {
      if(globals->quiet_flag == FALSE)
      {
	fprintf(stderr, "Kskew duration=%d > glottal closed period=%d, truncate\n",
	        (int)frame->Kskew, (int)(globals->T0 - globals->nopen));
      }
      frame->Kskew = temp;
    }
    if (skew >= 0) 
    {
      skew = frame->Kskew;
    }
    else 
    {
      skew = - frame->Kskew;
    }

    /* Add skewness to closed portion of voicing period */


    globals->T0 = globals->T0 + skew;
    skew = - skew;

  }

  else 
  {
    globals->T0 = 4;                     /* Default for f0 undefined */
    globals->amp_voice = 0.0;
    globals->nmod = globals->T0;
    globals->amp_breth = 0.0;
    globals->pulse_shape_a = 0.0;
    globals->pulse_shape_b = 0.0;
  }

  /* Reset these pars pitch synchronously or at update rate if f0=0 */

  if ((globals->T0 != 4) || (globals->ns == 0)) 
  {
    /* Set one-pole low-pass filter that tilts glottal source */

    globals->decay = (0.033 * frame->TLTdb);

    if (globals->decay > 0.0) 
    {
      globals->onemd = 1.0 - globals->decay;
    }
    else 
    {
      globals->onemd = 1.0;
    }
  }
}

/** Convert formant freqencies and bandwidth into resonator difference equation constants.
  *
  * @param f   Frequency of resonator in Hz
  * @param bw  Frequency of resonator in Hz
  */
static void setabc(long int f, long int bw, resonator_ptr rp, klatt_global_ptr globals)
{
 float r;

 r = exp(-M_PI / globals->samrate * bw);
 rp->c = -(r * r);
 rp->b = r * cos(2.0 * M_PI / globals->samrate * f) * 2.0;
 rp->a = 1.0 - rp->b - rp->c;
}

/** @brief Convert formant freqencies and bandwidth into anti-resonator difference equation constants.
  *
  * @param f   Frequency of resonator in Hz
  * @param bw  Frequency of resonator in Hz
  */
static void setzeroabc(long int f, long int bw, resonator_ptr rp, klatt_global_ptr globals)
{
 float r;

 /* First compute ordinary resonator coefficients */
 r = exp(-M_PI / globals->samrate * bw);
 rp->c = -(r * r);
 rp->b = r * cos(2.0 * M_PI / globals->samrate * -f) * 2.0;
 rp->a = 1.0 - rp->b - rp->c;

 if (f != 0) /* prevent a', b' and c' going to INF! */
 {
  /* Now convert to antiresonator coefficients (a'=1/a, b'=b/a, c'=c/a) */
  rp->a = 1.0 / rp->a;
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
static float gen_noise(klatt_global_ptr globals)
{
  long temp;
  static float nlast;

  temp = (long) getrandom(-8191,8191);
  globals->nrand = (long) temp;

  nlast = globals->nrand + (0.75 * nlast);

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
static float DBtoLIN(long dB)
{
  float lgtemp;
  static float amptable[88] = 
  {
    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 6.0, 7.0,
    8.0, 9.0, 10.0, 11.0, 13.0, 14.0, 16.0, 18.0, 20.0, 22.0, 25.0, 28.0, 32.0,
    35.0, 40.0, 45.0, 51.0, 57.0, 64.0, 71.0, 80.0, 90.0, 101.0, 114.0, 128.0,
    142.0, 159.0, 179.0, 202.0, 227.0, 256.0, 284.0, 318.0, 359.0, 405.0,
    455.0, 512.0, 568.0, 638.0, 719.0, 811.0, 911.0, 1024.0, 1137.0, 1276.0,
    1438.0, 1622.0, 1823.0, 2048.0, 2273.0, 2552.0, 2875.0, 3244.0, 3645.0, 
    4096.0, 4547.0, 5104.0, 5751.0, 6488.0, 7291.0, 8192.0, 9093.0, 10207.0, 
    11502.0, 12976.0, 14582.0, 16384.0, 18350.0, 20644.0, 23429.0,
    26214.0, 29491.0, 32767
  };

  if ((dB < 0) || (dB > 87))
  {
      return(0);
  }            

  lgtemp=amptable[dB] * .001;
  return(lgtemp);
}
