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
  uint16_t samrate;     /* Number of output samples per second           */
  uint16_t FLPhz ;      /* Frequeny of glottal downsample low-pass filter */
  uint16_t BLPhz ;      /* Bandwidth of glottal downsample low-pass filter */
  uint16_t nfcascade;   /* Number of formants in cascade vocal tract    */
  unsigned char glsource;    /* Type of glottal source */
  int16_t f0_flutter;   /* Percentage of f0 flutter 0-100 */
  flag quiet_flag;  /* set to TRUE for error messages */
  uint16_t nspfr;       /* number of samples per frame */
  uint16_t nper;        /* Counter for number of samples in a pitch period */
  uint16_t ns;
  uint16_t T0;          /* Fundamental period in output samples times 4 */
  uint16_t nopen;       /* Number of samples in open phase of period    */
  uint16_t nmod;        /* Position in period to begin noise amp. modul */
  uint16_t nrand;       /* Varible used by random number generator      */
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
  int16_t num_samples; /* number of glottal samples */
  float sample_factor; /* multiplication factor for glottal samples */
  int16_t *natural_samples; /* pointer to an array of glottal samples */
  uint16_t original_f0; /* original value of f0 not modified by flutter */

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

void simple_parwave(klatt_global_ptrr,int16_t*);
void simple_parwave_init(klatt_global_ptrr);
void dosimpleklatt(void);
void simpleklatt_init(void);
void single_parwave(klatt_global_ptrr globals, int16_t* frame, u8 newframe, u16 samplenumber,u8 x, int16_t* outgoing);
void dosimpleklattsamples(int16_t* outgoing, u8 size);

#endif
