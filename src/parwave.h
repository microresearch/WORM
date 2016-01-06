/* An implementation of a Klatt cascade-parallel formant synthesizer.
 *
 * Copyright (C) 2011 Reece H. Dunn
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

#ifndef KLATT_PARWAVE_H
#define KLATT_PARWAVE_H

/** @brief The type of synthesis model to use.
  */


#define NPAR		 40        /* Number of control parameters */
#define MAX_SAM          20000     /* Maximum sample rate */
#define TRUE             1
#define FALSE            0

/** @brief The voicing source generator to use for glottal waveforms.
  */

typedef char flag;

/** @brief A klatt resonator.
  */
typedef struct
{
  float a;
  float b;
  float c;
  float p1; /* value at time period |n-1|. */
  float p2; /* value at time period |n-2| */
} resonator_t, *resonator_ptr;

/** @brief Parameters, global data and resonators used by the klatt synthesizer.
  */
typedef struct
{
  unsigned char synthesis_model; /* cascade-parallel or all-parallel */
  unsigned int samrate;     /* Number of output samples per second           */
  unsigned int FLPhz ;      /* Frequeny of glottal downsample low-pass filter */
  unsigned int BLPhz ;      /* Bandwidth of glottal downsample low-pass filter */
  unsigned int nfcascade;   /* Number of formants in cascade vocal tract    */
  unsigned char glsource;    /* Type of glottal source */
  int f0_flutter;   /* Percentage of f0 flutter 0-100 */
  flag quiet_flag;  /* set to TRUE for error messages */
  unsigned int nspfr;       /* number of samples per frame */
  unsigned int nper;        /* Counter for number of samples in a pitch period */
  unsigned int ns;
  unsigned int T0;          /* Fundamental period in output samples times 4 */
  unsigned int nopen;       /* Number of samples in open phase of period    */
  unsigned int nmod;        /* Position in period to begin noise amp. modul */
  unsigned int nrand;       /* Varible used by random number generator      */
  float pulse_shape_a;  /* Makes waveshape of glottal pulse when open   */
  float pulse_shape_b;  /* Makes waveshape of glottal pulse when open   */
  float onemd;
  float decay;
  float amp_bypas; /* AB converted to linear gain              */
  float amp_voice; /* AVdb converted to linear gain            */
  float par_amp_voice; /* AVpdb converted to linear gain       */
  float amp_aspir; /* AP converted to linear gain              */
  float amp_frica; /* AF converted to linear gain              */
  float amp_breth; /* ATURB converted to linear gain           */
  float amp_gain0; /* G0 converted to linear gain              */
  int num_samples; /* number of glottal samples */
  float sample_factor; /* multiplication factor for glottal samples */
  int *natural_samples; /* pointer to an array of glottal samples */
  unsigned int original_f0; /* original value of f0 not modified by flutter */

  resonator_t rnpp; /* internal storage for resonators */
  resonator_t r1p;
  resonator_t r2p;
  resonator_t r3p;
  resonator_t r4p;
  resonator_t r5p;
  resonator_t r6p;
  resonator_t r1c;
  resonator_t r2c;
  resonator_t r3c;
  resonator_t r4c;
  resonator_t r5c;
  resonator_t r6c;
  resonator_t r7c;
  resonator_t r8c;
  resonator_t rnpc;
  resonator_t rnz;
  resonator_t rgl;
  resonator_t rlp;
  resonator_t rout;
} klatt_global_tt, *klatt_global_ptrr;
  
/** @brief The audio characteristics of an audio frame.
  */
typedef struct
{
  unsigned int F0hz10; /* Voicing fund freq in Hz                          */        
  unsigned int AVdb;   /* Amp of voicing in dB,            0 to   70       */        
  unsigned int F1hz;   /* First formant freq in Hz,        200 to 1300     */        
  unsigned int B1hz;   /* First formant bw in Hz,          40 to 1000      */        
  unsigned int F2hz;   /* Second formant freq in Hz,       550 to 3000     */        
  unsigned int B2hz;   /* Second formant bw in Hz,         40 to 1000      */        
  unsigned int F3hz;   /* Third formant freq in Hz,        1200 to 4999    */        
  unsigned int B3hz;   /* Third formant bw in Hz,          40 to 1000      */        
  unsigned int F4hz;   /* Fourth formant freq in Hz,       1200 to 4999    */        
  unsigned int B4hz;   /* Fourth formant bw in Hz,         40 to 1000      */        
  unsigned int F5hz;   /* Fifth formant freq in Hz,        1200 to 4999    */        
  unsigned int B5hz;   /* Fifth formant bw in Hz,          40 to 1000      */        
  unsigned int F6hz;   /* Sixth formant freq in Hz,        1200 to 4999    */        
  unsigned int B6hz;   /* Sixth formant bw in Hz,          40 to 2000      */        
  unsigned int FNZhz;  /* Nasal zero freq in Hz,           248 to  528     */        
  unsigned int BNZhz;  /* Nasal zero bw in Hz,             40 to 1000      */        
  unsigned int FNPhz;  /* Nasal pole freq in Hz,           248 to  528     */        
  unsigned int BNPhz;  /* Nasal pole bw in Hz,             40 to 1000      */        
  unsigned int ASP;    /* Amp of aspiration in dB,         0 to   70       */        
  unsigned int Kopen;  /* # of samples in open period,     10 to   65      */        
  unsigned int Aturb;  /* Breathiness in voicing,          0 to   80       */        
  unsigned int TLTdb;  /* Voicing spectral tilt in dB,     0 to   24       */        
  unsigned int AF;     /* Amp of frication in dB,          0 to   80       */        
  unsigned int Kskew;  /* Skewness of alternate periods,   0 to   40 in sample#/2  */
  unsigned int A1;     /* Amp of par 1st formant in dB,    0 to   80       */        
  unsigned int B1phz;  /* Par. 1st formant bw in Hz,       40 to 1000      */        
  unsigned int A2;     /* Amp of F2 frication in dB,       0 to   80       */        
  unsigned int B2phz;  /* Par. 2nd formant bw in Hz,       40 to 1000      */        
  unsigned int A3;     /* Amp of F3 frication in dB,       0 to   80       */        
  unsigned int B3phz;  /* Par. 3rd formant bw in Hz,       40 to 1000      */        
  unsigned int A4;     /* Amp of F4 frication in dB,       0 to   80       */        
  unsigned int B4phz;  /* Par. 4th formant bw in Hz,       40 to 1000      */        
  unsigned int A5;     /* Amp of F5 frication in dB,       0 to   80       */        
  unsigned int B5phz;  /* Par. 5th formant bw in Hz,       40 to 1000      */        
  unsigned int A6;     /* Amp of F6 (same as r6pa),        0 to   80       */        
  unsigned int B6phz;  /* Par. 6th formant bw in Hz,       40 to 2000      */        
  unsigned int ANP;    /* Amp of par nasal pole in dB,     0 to   80       */        
  unsigned int AB;     /* Amp of bypass fric. in dB,       0 to   80       */        
  unsigned int AVpdb;  /* Amp of voicing,  par in dB,      0 to   70       */        
  unsigned int Gain0;  /* Overall gain, 60 dB is unity,    0 to   60       */        
 } klatt_frame_tt, *klatt_frame_ptrr;

void simple_parwave(klatt_global_ptrr,klatt_frame_ptrr);
void simple_parwave_init(klatt_global_ptrr);
void dosimpleklatt(void);
void init_simpleklatt(void);

#endif
