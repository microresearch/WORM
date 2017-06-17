#include <config.h>
/* def_pars.c
 */
#include <stdio.h>
#include <math.h>
#include "nsynth.h"

klatt_global_t klatt_global;

/* default values for pars array from .doc file */
klatt_frame_t def_pars = // not a const
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

	klatt_global.glsource = NATURAL;/// SAMPLE=fixed, NATURAL seems same as IMPULSIVE, TRIANGULAR-loud-leave, WAVETABLE - not implemented?
	//
	klatt_global.f0_flutter = 0.0f;
	def_pars.TLTdb=0;
	def_pars.Kskew=10;
	def_pars.F0hz10=1600;//         "Base F0 in 0.1Hz",

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




