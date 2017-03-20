#include <config.h>
/* def_pars.c
 */
#include <stdio.h>
#include <math.h>
#include "nsynth.h"

klatt_global_t klatt_global;

/* default values for pars array from .doc file */
klatt_frame_t def_pars =
{
#include "pars.def"
};


long samp_rate = 32000;

void klatt_init(void)
{
	//double mSec_per_frame = 10;
  float mSec_per_frame = 10; // 10? or 8? - leave as 10
  //	int impulse = 0;
  //  int impulse = 1;
	//int casc = 1;
	int casc = 6;
	klatt_global.samrate = samp_rate;
	klatt_global.quiet_flag = TRUE;
	klatt_global.glsource = IMPULSIVE;/// SAMPLE, NATURAL, IMPULSIVE, TRIANGULAR, WAVETABLE - where is sample from?
	klatt_global.f0_flutter = 0;

	//	klatt_global.quiet_flag, "Quiet - minimal messages",
	//	impulse,                 "Impulse glottal source",
	//	casc,                    "Number cascade formants",
	//	klatt_global.f0_flutter, "F0 flutter",
	//	mSec_per_frame,         "mSec per frame",
	//	def_pars.TLTdb,          "Tilt dB",
	//	def_pars.F0hz10,         "Base F0 in 0.1Hz",

	//def_pars.TLTdb=10;
		def_pars.F0hz10=1200;//         "Base F0 in 0.1Hz",
		//	def_pars.Kskew=10;
	if (casc > 0)
	{
		klatt_global.synthesis_model = CASCADE_PARALLEL;
		klatt_global.nfcascade = casc;
	}
	else
		klatt_global.synthesis_model = ALL_PARALLEL;

	//	if (impulse)
	//		klatt_global.glsource = IMPULSIVE;

	klatt_global.nspfr = (klatt_global.samrate * mSec_per_frame) / 1000;
}




