/******************************************************************************
 * $Id: vocode.c,v 1.2 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1996-1999,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include "config.h"
#include "fft.h"
#include <math.h>
#include "arm_const_structs.h"
#include "vocode.h"

typedef VSHORT SAMPLE; // signed 16 bits
typedef VREAL (*COMPLEX_ARRAY)[2];

static void vocode_window(VREAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output);

//static void vocoder(SAMPLE *modulator_sample_buffer,SAMPLE *carrier_sample_buffer, SAMPLE *output_sample_buffer1);

static void sample_to_complex_array(SAMPLE *sample_array,
                                    COMPLEX_ARRAY complex_array,
                                    size_t length, SAMPLE max_magnitude);

static void sample_to_real_array(SAMPLE *sample_array, VREAL *real_array,
                                 size_t length, SAMPLE max_magnitude);

static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array, size_t length,
                                    SAMPLE max_magnitude, VREAL vocode_volume);

size_t vocode_window_length=256;
size_t vocode_window_overlap;
unsigned char vocode_band_count=16;
VREAL vocode_volume=1.0f;
VBOOL vocode_normalize=1;

static SAMPLE modulator_max_magnitude=32768, carrier_max_magnitude=32768,
              output_max_magnitude=32768;

//int16_t output_sample_buffer2[256];


/*   modulator_sample_buffer = error_malloc(sizeof(SAMPLE) * vocode_window_length); // length is 256
  carrier_sample_buffer = error_malloc(sizeof(SAMPLE) * vocode_window_length); 
  output_sample_buffer1 = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  output_sample_buffer2 = error_malloc(sizeof(SAMPLE) * vocode_window_length);
  modulator = error_malloc(sizeof(VREAL) * vocode_window_length); // is [256]float
  looped_carrier = error_malloc(sizeof(VREAL) * 2 * vocode_window_length);
  output = error_malloc(sizeof(VREAL) * 2 * vocode_window_length);
*/

// difference between carrier and looped_carrier is that looped is the complex one!

static void vocode_window(VREAL *modulator, COMPLEX_ARRAY carrier,
                          COMPLEX_ARRAY output){
  int band_no, band_length, extra_band_length;

  band_length = vocode_window_length / (vocode_band_count * 2);
  extra_band_length = vocode_window_length / 2 - band_length * (vocode_band_count - 1);

  realfftmag(modulator, vocode_window_length);
  fft(carrier, vocode_window_length);
  normalize_fft(carrier, vocode_window_length);
  
  for (band_no = 0; band_no < vocode_band_count; band_no++) {
    int i, j, k, l;
    VREAL m, c;
    
    l = (band_no == vocode_band_count - 1) ? extra_band_length : band_length;

    m = 0.0f; c = 0.0f;
    for (i = 0, j = band_no * band_length, k = vocode_window_length - j - 1;
         i < l; i++, j++, k--)
      {
        if (vocode_normalize) {
          VREAL c1 = carrier[j][0]*carrier[j][0] + carrier[j][1]*carrier[j][1],
               c2 = carrier[k][0]*carrier[k][0] + carrier[k][1]*carrier[k][1];
          c += sqrtf(c1) + sqrtf(c2);
        }
        m += modulator[j];
      }

    if (!vocode_normalize) c = 1.0f;
    if (c == 0) c = 0.0001f;

    for (i = 0, j = band_no * band_length, k = vocode_window_length - j - 1;
         i < l; i++, j++, k--) {
      output[j][0] = carrier[j][0] * m / c;
      output[j][1] = carrier[j][1] * m / c;
      output[k][0] = carrier[k][0] * m / c;
      output[k][1] = carrier[k][1] * m / c;
    }
  }

    invfft (output, vocode_window_length);
}

void vocoder(SAMPLE *modulator_sample_buffer,SAMPLE *carrier_sample_buffer, SAMPLE *output_sample_buffer1) // wihtout overlap
{
  size_t i;
  SAMPLE *output_old = output_sample_buffer1;
float modulator[256];
float looped_carrier[256][2];
float output[256][2];
  
  sample_to_real_array(modulator_sample_buffer, modulator, vocode_window_length,
                       modulator_max_magnitude);

  sample_to_complex_array(carrier_sample_buffer, looped_carrier,
                          vocode_window_length, carrier_max_magnitude);
  
    vocode_window(modulator, looped_carrier, output);
  
        complex_to_sample_array(output, output_old, vocode_window_length,
                          output_max_magnitude, vocode_volume);
    //  complex_to_sample_array(looped_carrier, output_old, vocode_window_length,
    //                    output_max_magnitude, vocode_volume); // tested OK so issue is in vocode_window

  // output is in output_old
}



static void sample_to_complex_array(SAMPLE *sample_array,
                                    COMPLEX_ARRAY complex_array,
                                    size_t length, SAMPLE max_magnitude)
{
  size_t i;
  for (i = 0; i < length; ++i)
    {
      complex_array[i][0] = sample_array[i] / (VREAL)max_magnitude;
      complex_array[i][1] = 0;
    }
}

static void sample_to_real_array(SAMPLE *sample_array, VREAL *real_array,
                                 size_t length, SAMPLE max_magnitude)
{
  size_t i;
  for (i = 0; i < length; ++i)
    {
      *real_array++ = *sample_array++ / (VREAL)max_magnitude;
    }
}

static void complex_to_sample_array(COMPLEX_ARRAY complex_array,
                                    SAMPLE *sample_array,
                                    size_t length, SAMPLE max_magnitude,
                                    VREAL vocode_volume)
{
  size_t i;
  for (i = 0; i < length; ++i)
    {
      VREAL sample = complex_array[i][0] * vocode_volume;
      if (sample < -1.0) sample = -1.0;
      else if (sample > 1.0) sample = 1.0;
      sample_array[i] = (SAMPLE)(sample * max_magnitude);
    }
}


