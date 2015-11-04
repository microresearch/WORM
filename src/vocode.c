#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"
#include "vocode.h"

const LADSPA_Data decay_table[] =
{
  1/100.0,
  1/100.0, 1/100.0, 1/100.0,
  1/125.0, 1/125.0, 1/125.0,
  1/166.0, 1/166.0, 1/166.0,
  1/200.0, 1/200.0, 1/200.0,
  1/250.0, 1/250.0, 1/250.0
};

/* useful macros */
#undef CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/*****************************************************************************/

VocoderInstance* instantiateVocoder(void) {
  VocoderInstance* vocoder = (VocoderInstance *)malloc(sizeof(VocoderInstance));
  u8 i, j;
  float a;
  float c;
  //  if (vocoder == NULL)
  //    return NULL;

  vocoder->SampleRate = 48000.0f;

  for (u8 x=0;x<MAX_BANDS;x++){
    vocoder->bands_out[x].oldval = 0.0f;
  }

  /* initialize bandpass information if num_bands control has changed,
     or on first run */

      for(i=0; i < 16; i++)
	{
	  memset(&vocoder->bands_formant[i], 0, sizeof(struct bandpass));

	  a = 16.0 * (float)i/16.0f;  // stretch existing bands

	  if (a < 4.0)
	    vocoder->bands_formant[i].freq = 150.0 + 420.0 * a / 4.0;
	  else
	    vocoder->bands_formant[i].freq = 600.0 * powf (1.23, a - 4.0);

	  c = vocoder->bands_formant[i].freq * 2 * M_PI / vocoder->SampleRate;
	  vocoder->bands_formant[i].c = c * c;

	  vocoder->bands_formant[i].f = 0.4/c;
	  vocoder->bands_formant[i].att =
	    1.0f/(6.0f + ((expf (vocoder->bands_formant[i].freq
			    / vocoder->SampleRate) - 1.0f) * 10.0f));

	  memcpy(&vocoder->bands_carrier[i],
		 &vocoder->bands_formant[i], sizeof(struct bandpass));
	  vocoder->bands_out[i].decay = decay_table[(int)a];

	}

  return vocoder;
}

/*****************************************************************************/

/*****************************************************************************/

// vocoder_do_bandpasses /*fold00*/
void vocoder_do_bandpasses(struct bandpass *bands, LADSPA_Data sample,
			   VocoderInstance *vocoder)
{
  u8 i;
  for (i=0; i < 16; i++)
    {
      bands[i].high1 = sample - bands[i].f * bands[i].mid1 - bands[i].low1;
      bands[i].mid1 += bands[i].high1 * bands[i].c;
      bands[i].low1 += bands[i].mid1;

      bands[i].high2 = bands[i].low1 - bands[i].f * bands[i].mid2
	- bands[i].low2;
      bands[i].mid2 += bands[i].high2 * bands[i].c;
      bands[i].low2 += bands[i].mid2;
      bands[i].y = bands[i].high2 * bands[i].att;
    }
}

/* Run a vocoder instance for a block of SampleCount samples. */

// TODO: how to swap round channels/bands

void runVocoder(VocoderInstance *vocoder, float *formant, float *carrier, float *out, unsigned int SampleCount)
{
  u8 i, j;
  //  float a;
  LADSPA_Data x;

  vocoder->portFormant=formant;
  vocoder->portCarrier=carrier;
  vocoder->portOutput=out;


  ///////////////////////////////////
  for (i=0; i < SampleCount; i++)
    {
      vocoder_do_bandpasses (vocoder->bands_carrier,
			     vocoder->portCarrier[i], vocoder);
      vocoder_do_bandpasses (vocoder->bands_formant,
			     vocoder->portFormant[i], vocoder);

      vocoder->portOutput[i] = 0.0f;
      for (j=0; j < 16; j++)
	{
	  vocoder->bands_out[j].oldval = vocoder->bands_out[j].oldval
	    + vocoder->bands_formant[j].y;
	    //	    + (fabsf (vocoder->bands_formant[j].y) - vocoder->bands_out[j].oldval) * vocoder->bands_out[j].decay;

	  x = vocoder->bands_carrier[j].y * vocoder->bands_out[j].oldval;
	  vocoder->portOutput[i] += x;
	}
      vocoder->portOutput[i] *= 16.0f;
    }
}
