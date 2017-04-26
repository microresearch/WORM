#include <config.h>


/* nsynth.c
 */

/* Copyright            1982                    by Dennis H. Klatt

 *      Klatt synthesizer
 *         Modified version of synthesizer described in
 *         J. Acoust. Soc. Am., Mar. 1980. -- new voicing
 *         source.
 *
 * Edit history
 * 000001 10-Mar-83 DK  Initial creation.
 * 000002  5-May-83 DK  Fix bug in operation of parallel F1
 * 000003  7-Jul-83 DK  Allow parallel B1 to vary, and if ALL_PARALLEL,
 *                      also allow B2 and B3 to vary
 * 000004 26-Jul-83 DK  Get rid of mulsh, use short for VAX
 * 000005 24-Oct-83 DK  Split off parwavtab.c, change short to int
 * 000006 16-Nov-83 DK  Make samrate a variable, use exp(), cos() rand()
 * 000007 17-Nov-83 DK  Convert to float, remove  cpsw, add set outsl
 * 000008 28-Nov-83 DK  Add simple impulsive glottal source option
 * 000009  7-Dec-83 DK  Use spkrdef[7] to select impulse or natural voicing
 *                       and update cascade F1,..,F6 at update times
 * 000010 19-Dec-83 DK  Add subroutine no_rad_char() to get rid of rad char
 * 000011 28-Jan-84 DK  Allow up to 8 formants in cascade branch F7 fixed
 *                       at 6.5 kHz, F8 fixed at 7.5 kHz
 * 000012 14-Feb-84 DK  Fix bug in 'os' options so os>12 works
 * 000013 17-May-84 DK  Add G0 code
 * 000014 12-Mar-85 DHW modify for Haskins environment
 * 000015 11-Jul-87 LG  modificiations for PC
 * 000016 20-Apr-91 ATS Modified for SPARCSTATION
 */


#ifndef LAP
#include "stm32f4xx.h"
#include "wavetable.h"
#include "audio.h"
#include "resources.h"
#else
#include "forlap.h"
#endif
#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include "nsynth.h"
#ifndef PI
#ifndef M_PI                      /* <math.h> */
#define PI               3.1415927f
#else /* M_PI */
#define PI               M_PI
#endif /* M_PI */
#endif

#define ONE 1.0F

static int natural_samples[100]= // where is this from? - from earlier rsynth
  {
    -310,-400,530,356,224,89,23,-10,-58,-16,461,599,536,701,770,
    605,497,461,560,404,110,224,131,104,-97,155,278,-154,-1165,
    -598,737,125,-592,41,11,-247,-10,65,92,80,-304,71,167,-1,122,
    233,161,-43,278,479,485,407,266,650,134,80,236,68,260,269,179,
    53,140,275,293,296,104,257,152,311,182,263,245,125,314,140,44,
    203,230,-235,-286,23,107,92,-91,38,464,443,176,98,-784,-2449,
    -1891,-1045,-1600,-1462,-1384,-1261,-949,-730
  };

static float slopet1,slopet2,Afinal,maxt1,maxt2;        /* For triangle */
static int nfirsthalf,nsecondhalf,assym,as;   /* For triangle */


typedef struct
 {
  char *name;
  float a;
  float b;
  float c;
  float p1;
  float p2;
 }
resonator_t, *resonator_ptr;

/* Various global variables */

int time_count = 0;
//yama static warnsw;                    /* JPI added for f0 flutter */
static int warnsw;                    /* JPI added for f0 flutter */

/* COUNTERS */

static long nper;                 /* Current loc in voicing period   40000 samp/s */

/* COUNTER LIMITS */

static long T0;                   /* Fundamental period in output samples times 4 */
static long nopen;                /* Number of samples in open phase of period  */
static long nmod;                 /* Position in period to begin noise amp. modul */

/* Variables that have to stick around for awhile, and thus locals
   are not appropriate 
 */

/* Incoming parameter Variables which need to be updated synchronously  */

static long F0hz10;               /* Voicing fund freq in Hz  */
static long AVdb;                 /* Amp of voicing in dB,    0 to   70  */
static long Kskew;                /* Skewness of alternate periods,0 to   40  */

/* Various amplitude variables used in main loop */

static float amp_voice;           /* AVdb converted to linear gain  */
static float amp_bypas;           /* AB converted to linear gain  */
static float par_amp_voice;       /* AVpdb converted to linear gain  */
static float amp_aspir;           /* AP converted to linear gain  */
static float amp_frica;           /* AF converted to linear gain  */
static float amp_breth;           /* ATURB converted to linear gain  */

/* State variables of sound sources */

static long skew;                 /* Alternating jitter, in half-period units  */

static float natglot_a;           /* Makes waveshape of glottal pulse when open  */
static float natglot_b;           /* Makes waveshape of glottal pulse when open  */
static float vwave;               /* Ditto, but before multiplication by AVdb  */
static float vlast;               /* Previous output of voice  */
static float nlast;               /* Previous output of random number generator  */
static float glotlast;            /* Previous value of glotout  */
static float decay;               /* TLTdb converted to exponential time const  */
static float onemd;               /* in voicing one-pole low-pass filter  */
static float minus_pi_t;          /* func. of sample rate */
static float two_pi_t;            /* func. of sample rate */


/* INTERNAL MEMORY FOR DIGITAL RESONATORS AND ANTIRESONATOR  */

static resonator_t rnpp =
{"parallel nasal pole"};
static resonator_t r1p =
{"parallel 1st formant"};
static resonator_t r2p =
{"parallel 2nd formant"};
static resonator_t r3p =
{"parallel 3rd formant"};
static resonator_t r4p =
{"parallel 4th formant"};
static resonator_t r5p =
{"parallel 5th formant"};
static resonator_t r6p =
{"parallel 6th formant"};
static resonator_t r1c =
{"cascade 1st formant"};
static resonator_t r2c =
{"cascade 2nd formant"};
static resonator_t r3c =
{"cascade 3rd formant"};
static resonator_t r4c =
{"cascade 4th formant"};
static resonator_t r5c =
{"cascade 5th formant"};
static resonator_t r6c =
{"cascade 6th formant"};
static resonator_t r7c =
{"cascade 7th formant"};
static resonator_t r8c =
{"cascade 8th formant"};
static resonator_t rnpc =
{"cascade nasal pole"};
static resonator_t rnz =
{"cascade nasal zero"};
static resonator_t rgl =
{"crit-damped glot low-pass filter"};
static resonator_t rlp =
{"downsamp low-pass filter"};
static resonator_t rout =
{"output low-pass"};

/*
 * Constant natglot[] controls shape of glottal pulse as a function
 * of desired duration of open phase N0
 * (Note that N0 is specified in terms of 40,000 samples/sec of speech)
 *
 *    Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3
 *
 *    If the radiation characterivative, a temporal derivative
 *      is folded in, and we go from continuous time to discrete
 *      integers n:  dV/dt = vwave[n]
 *                         = sum over i=1,2,...,n of { a - (i * b) }
 *                         = a n  -  b/2 n**2
 *
 *      where the  constants a and b control the detailed shape
 *      and amplitude of the voicing waveform over the open
 *      potion of the voicing cycle "nopen".
 *
 *    Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3
 *
 *    Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
 *      meaning as nopen gets bigger, V has bigger peak proportional to n
 *
 *    Thus, to generate the table below for 40 <= nopen <= 263:
 *
 *      natglot[nopen - 40] = 1920000 / (nopen * nopen)
 */
static const short natglot[224] __attribute__ ((section (".flash"))) =
{
 1200, 1142, 1088, 1038, 991, 948, 907, 869, 833, 799,
 768, 738, 710, 683, 658, 634, 612, 590, 570, 551,
 533, 515, 499, 483, 468, 454, 440, 427, 415, 403,
 391, 380, 370, 360, 350, 341, 332, 323, 315, 307,
 300, 292, 285, 278, 272, 265, 259, 253, 247, 242,
 237, 231, 226, 221, 217, 212, 208, 204, 199, 195,
 192, 188, 184, 180, 177, 174, 170, 167, 164, 161,
 158, 155, 153, 150, 147, 145, 142, 140, 137, 135,
 133, 131, 128, 126, 124, 122, 120, 119, 117, 115,
 113, 111, 110, 108, 106, 105, 103, 102, 100, 99,
 97, 96, 95, 93, 92, 91, 90, 88, 87, 86,
 85, 84, 83, 82, 80, 79, 78, 77, 76, 75,
 75, 74, 73, 72, 71, 70, 69, 68, 68, 67,
 66, 65, 64, 64, 63, 62, 61, 61, 60, 59,
 59, 58, 57, 57, 56, 56, 55, 55, 54, 54,
 53, 53, 52, 52, 51, 51, 50, 50, 49, 49,
 48, 48, 47, 47, 46, 46, 45, 45, 44, 44,
 43, 43, 42, 42, 41, 41, 41, 41, 40, 40,
 39, 39, 38, 38, 38, 38, 37, 37, 36, 36,
 36, 36, 35, 35, 35, 35, 34, 34, 33, 33,
 33, 33, 32, 32, 32, 32, 31, 31, 31, 31,
 30, 30, 30, 30, 29, 29, 29, 29, 28, 28,
 28, 28, 27, 27
};

/*
 * Convertion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                 ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                 ...
 *                                  0 dB -->     0
 *
 * The just noticeable difference for a change in intensity of a vowel
 *   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 *   steps.
 */

static const float amptable[88] __attribute__ ((section (".flash"))) =
{
 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
 0.0f, 0.0f, 0.0f, 6.0f, 7.0f,
 8.0f, 9.0f, 10.0f, 11.0f, 13.0f,
 14.0f, 16.0f, 18.0f, 20.0f, 22.0f,
 25.0f, 28.0f, 32.0f, 35.0f, 40.0f,
 45.0f, 51.0f, 57.0f, 64.0f, 71.0f,
 80.0f, 90.0f, 101.0f, 114.0f, 128.0f,
 142.0f, 159.0f, 179.0f, 202.0f, 227.0f,
 256.0f, 284.0f, 318.0f, 359.0f, 405.0f,
 455.0f, 512.0f, 568.0f, 638.0f, 719.0f,
 811.0f, 911.0f, 1024.0f, 1137.0f, 1276.0f,
 1438.0f, 1622.0f, 1823.0f, 2048.0f, 2273.0f,
 2552.0f, 2875.0f, 3244.0f, 3645.0f, 4096.0f,
 4547.0f, 5104.0f, 5751.0f, 6488.0f, 7291.0f,
 8192.0f, 9093.0f, 10207.0f, 11502.0f, 12976.0f,
 14582.0f, 16384.0f, 18350.0f, 20644.0f, 23429.0f,
 26214.0f, 29491.0f, 32767.0f
};

const char *par_name[] =
{
 "F0hz10",
 "AVdb",
 "F1hz", "B1hz",
 "F2hz", "B2hz",
 "F3hz", "B3hz",
 "F4hz", "B4hz",
 "F5hz", "B5hz",
 "F6hz", "B6hz",
 "FNZhz", "BNZhz",
 "FNPhz", "BNPhz",
 "AP",
 "Kopen",
 "Aturb",
 "TLTdb",
 "AF",
 "Kskew",
 "A1", "B1phz",
 "A2", "B2phz",
 "A3", "B3phz",
 "A4", "B4phz",
 "A5", "B5phz",
 "A6", "B6phz",
 "ANP",
 "AB",
 "AVpdb",
 "Gain0"
};

static void flutter (klatt_global_ptr globals, klatt_frame_ptr pars);
static float resonator (resonator_ptr r, float input);
static float antiresonator (resonator_ptr r, float input);
static float impulsive_source (long nper);
static float natural_source (long nper);
static void setabc (long int f, long int bw, resonator_ptr rp);
static void setabcg (long int f, long int bw, resonator_ptr rp, float gain);
static void setzeroabc (long int f, long int bw, resonator_ptr rp);
static float DBtoLIN (klatt_global_ptr globals, long int dB);
static float dBconvert (long int arg);
static int16_t clip (klatt_global_ptr globals, float input);
static void pitch_synch_par_reset (klatt_global_ptr globals,
                                         klatt_frame_ptr frame, long ns);
static void frame_init (klatt_global_ptr globals, klatt_frame_ptr frame);

/*
   function FLUTTER

   This function adds F0 flutter, as specified in:

   "Analysis, synthesis and perception of voice quality variations among
   female and male talkers" D.H. Klatt and L.C. Klatt JASA 87(2) February 1990.
   Flutter is added by applying a quasi-random element constructed from three
   slowly varying sine waves.
 */
static void flutter(klatt_global_ptr globals, klatt_frame_ptr pars)
{
	long original_f0 = pars->F0hz10 / 10.0f;
	float fla = (float) globals->f0_flutter / 50.0f;
	float flb = (float) original_f0 / 100.0f;
	float flc = sinf(2 * PI * 12.7f * time_count);
	float fld = sinf(2 * PI * 7.1f * time_count);
	float fle = sinf(2 * PI * 4.7f * time_count);
	float delta_f0 = fla * flb * (flc + fld + fle) * 10.0f;
	F0hz10 += (long) delta_f0;
}

//extern Wavetable wavtable;
//extern __IO uint16_t adc_buffer[10];


static float wave_source(long nper) {
  float res;
  //  res=dosinglewavetable(&wavtable, (adc_buffer[SPEED]>>6)+(F0hz10/16.0)); // TODO FIX is we use
  return res*2048.0f;
}

static float triangular_source(long nper) {

/*    See if glottis open */
        if (nper < nopen) {
            if (nper < nfirsthalf) {
                vwave += slopet1;
                if (vwave > maxt1)    return(maxt1);
            }
            else {
                vwave += slopet2;
                if (vwave < maxt2)    return(maxt2);
            }
            return(vwave);
        }

/*    Glottis closed */
        else {
            return(0.);
        }
}


static float impulsive_source(long nper)
{
	static float floatt[] =
		{0.0f, 13000000.0f, -13000000.0f};
	if (nper < 3)
	{
		vwave = floatt[nper];
	}
	else
	{
		vwave = 0.0f;
	}
	/* Low-pass filter the differenciated impulse with a critically-damped
    second-order filter, time constant proportional to Kopen */
	return resonator(&rgl, vwave);
}


/* Vwave is the differentiated glottal flow waveform, there is a weak
   spectral zero around 800 Hz, magic constants a,b reset pitch-synch
 */

static float natural_source(long nper)
{
	float lgtemp;
	/* See if glottis open */
	if (nper < nopen)
	{
		natglot_a -= natglot_b;
		vwave += natglot_a;
		lgtemp = vwave * 0.028f;        /* function of samp_rate ? */
		return (lgtemp);
	}
	else
	{
		/* Glottis closed */
		vwave = 0.0f;
		return (0.0f);
	}
}

// other sources - eventually all excitations but just test here simple wavetable

// from klatt in docs

static float sampled_source(long nper)
{
  int itemp;
  float ftemp;
  float result;
  float diff_value;
  int current_value;
  int next_value;
  float temp_diff;

  if(T0!=0)
  {
    ftemp = (float) nper;
    ftemp = ftemp / T0;
    ftemp = ftemp * 100.0f;
    itemp = (int) ftemp;

    temp_diff = ftemp - (float) itemp;
  
    current_value = natural_samples[itemp];
    next_value = natural_samples[itemp+1];

    diff_value = (float) next_value - (float) current_value;
    diff_value = diff_value * temp_diff;

    result = natural_samples[itemp] + diff_value;
    result = result * 0.00005f;
    //    printf("xxx %f",result);
  }
  else
  {
    result = 0.0f;
  }
  return(result);
}



/*----------------------------------------------------------------------------*/
/* Convert formant freqencies and bandwidth into
   resonator difference equation coefficents
 */
// f  :                 /* Frequency of resonator in Hz  */
// bw :                 /* Bandwidth of resonator in Hz  */
// rp :                 /* Are output coefficients  */
static void setabc(long f, long bw, resonator_ptr rp)
{
	float arg = minus_pi_t * bw;
	float r = expf(arg);              /* Let r  =  exp(-pi bw t) */
	rp->c = -(r * r);                /* Let c  =  -r**2 */
	arg = two_pi_t * f;
	rp->b = r * cosf(arg) * 2.0f;      /* Let b = r * 2*cos(2 pi f t) */
	rp->a = 1.0f - rp->b - rp->c;     /* Let a = 1.0 - b - c */
}

/* Convienience function for setting parallel resonators with gain */
// f  :                  /* Frequency of resonator in Hz  */
// bw :                 /* Bandwidth of resonator in Hz  */
// rp :                 /* Are output coefficients  */
// gain:
static void setabcg(long f, long bw, resonator_ptr rp, float gain)
{
	setabc(f, bw, rp);
	rp->a *= gain;
}

/* Convert formant freqencies and bandwidth into
 *      anti-resonator difference equation constants
 */
// f :                  /* Frequency of resonator in Hz  */
// bw :                 /* Bandwidth of resonator in Hz  */
// rp :                 /* Are output coefficients  */
static void setzeroabc(long f, long bw, resonator_ptr rp)
{
	setabc(f, bw, rp);               /* First compute ordinary resonator coefficients */
	/* Now convert to antiresonator coefficients */
	rp->a = 1.0f / rp->a;             /* a'=  1/a */
	rp->b *= -rp->a;                 /* b'= -b/a */
	rp->c *= -rp->a;                 /* c'= -c/a */
}

/*----------------------------------------------------------------------------*/


/* Convert from decibels to a linear scale factor */
static float DBtoLIN(klatt_global_ptr globals, long dB)
{
	/* Check limits or argument (can be removed in final product) */
	if (dB < 0)
		dB = 0;
	else if (dB >= 88)
	{
		dB = 87;
	}
	return amptable[dB] * 0.001f;
}

/* WHAT WERE THESE FOR ? */
#define ACOEF           0.005f
#define BCOEF           (1.0f - ACOEF)	/* Slight decay to remove dc */

static float dBconvert(long arg)
{
	return 20.0f * log10f((float) arg / 32767.0f);
}


/* Reset selected parameters pitch-synchronously */

static void pitch_synch_par_reset(klatt_global_ptr globals, klatt_frame_ptr frame, long ns)
{
	long temp;
	float temp1;
	if (F0hz10 > 0)
	{
		T0 = (40 * globals->samrate) / F0hz10;
		/* Period in samp*4 */
		amp_voice = DBtoLIN(globals, AVdb);

		/* Duration of period before amplitude modulation */
		nmod = T0;
		if (AVdb > 0)
		{
			nmod >>= 1;
		}

		/* Breathiness of voicing waveform */

		amp_breth = DBtoLIN(globals, frame->Aturb) * 0.1f;

		/* Set open phase of glottal period */
		/* where  40 <= open phase <= 263 */

		nopen = 4 * frame->Kopen; // KLSYN->             nopen = T0*((float)Kopen/100) ;  /* Was   nopen = 4 * Kopen; */

		if ((globals->glsource == IMPULSIVE) && (nopen > 263))
		{
			nopen = 263;
		}

		if (nopen >= (T0 - 1))
		{
			nopen = T0 - 2;
		}

		if (nopen < 40)
		{
			nopen = 40;                  /* F0 max = 1000 Hz */
		}

		/* Reset a & b, which determine shape of "natural" glottal waveform */

		natglot_b = natglot[nopen - 40];
		natglot_a = (natglot_b * nopen) * .333f;

		/* Reset width of "impulsive" glottal pulse */

		temp = globals->samrate / nopen;
		setabc(0L, temp, &rgl);

		/* Make gain at F1 about constant */

		temp1 = nopen * .00833f;
		rgl.a *= (temp1 * temp1);
	
/*        Reset legs of triangular glottal pulse */
            if (globals->glsource == TRIANGULAR) {
                assym = (nopen*(as-50))/100;  /* as=50 is symmetrical  CHECK */
                nfirsthalf = (nopen>>1) + assym;
                if (nfirsthalf >= nopen)    nfirsthalf = nopen -1;
                if (nfirsthalf <= 0)            nfirsthalf = 1;
                nsecondhalf = nopen - nfirsthalf;
                Afinal = -7000.;
                maxt2 = Afinal * 0.25f;
                slopet2 = Afinal / nsecondhalf;
                vwave = -(Afinal * nsecondhalf) / nfirsthalf;   /* CHECK */
                maxt1 = vwave * 0.25f;
                slopet1 = - vwave / nfirsthalf;
	    }

		/* Truncate skewness so as not to exceed duration of closed phase
		of glottal period */

		temp = T0 - nopen;
		if (Kskew > temp)
		{
			Kskew = temp;
		}
		
		if (skew >= 0)
		{
			skew = Kskew;                /* Reset skew to requested Kskew */
		}
		else
		{
			skew = -Kskew;
		}

		/* Add skewness to closed portion of voicing period */

		T0 = T0 + skew;
		skew = -skew;
	}
	else
	{
		T0 = 4;                        /* Default for f0 undefined */
		amp_voice = 0.0f;
		nmod = T0;
		amp_breth = 0.0f;
		natglot_a = 0.0f;
		natglot_b = 0.0f;
	}

	/* Reset these pars pitch synchronously or at update rate if f0=0 */

	if ((T0 != 4) || (ns == 0))
	{
		/* Set one-pole low-pass filter that tilts glottal source */
		decay = (0.033f * frame->TLTdb);	/* Function of samp_rate ? */
		if (decay > 0.0f)
		{
			onemd = 1.0f - decay;
		}
		else
		{
			onemd = 1.0f;
		}
	}
}

/* Get variable parameters from host computer,
   initially also get definition of fixed pars
 */

static void frame_init(klatt_global_ptr globals, klatt_frame_ptr frame)
{
	long Gain0;                      /* Overall gain, 60 dB is unity  0 to   60  */
	float amp_parF1;                 /* A1 converted to linear gain  */
	float amp_parFN;                 /* ANP converted to linear gain  */
	float amp_parF2;                 /* A2 converted to linear gain  */
	float amp_parF3;                 /* A3 converted to linear gain  */
	float amp_parF4;                 /* A4 converted to linear gain  */
	float amp_parF5;                 /* A5 converted to linear gain  */
	float amp_parF6;                 /* A6 converted to linear gain  */

	#if 0
	//	show_parms(globals, (int *) frame);
	#endif

	/*
    Read  speech frame definition into temp store
    and move some parameters into active use immediately
    (voice-excited ones are updated pitch synchronously
    to avoid waveform glitches).
	*/

	F0hz10 = frame->F0hz10;
	AVdb = frame->AVdb - 7;
	if (AVdb < 0)
		AVdb = 0;

	amp_aspir = DBtoLIN(globals, frame->ASP) * .05f;
	amp_frica = DBtoLIN(globals, frame->AF) * 0.25f;

	Kskew = frame->Kskew;
	par_amp_voice = DBtoLIN(globals, frame->AVpdb);

	/* Fudge factors (which comprehend affects of formants on each other?)
    with these in place ALL_PARALLEL should sound as close as 
    possible to CASCADE_PARALLEL.
    Possible problem feeding in Holmes's amplitudes given this.
	*/
	amp_parF1 = DBtoLIN(globals, frame->A1) * 0.4f;	/* -7.96 dB */
	amp_parF2 = DBtoLIN(globals, frame->A2) * 0.15f;	/* -16.5 dB */
	amp_parF3 = DBtoLIN(globals, frame->A3) * 0.06f;	/* -24.4 dB */
	amp_parF4 = DBtoLIN(globals, frame->A4) * 0.04f;	/* -28.0 dB */
	amp_parF5 = DBtoLIN(globals, frame->A5) * 0.022f;	/* -33.2 dB */
	amp_parF6 = DBtoLIN(globals, frame->A6) * 0.03f;	/* -30.5 dB */
	amp_parFN = DBtoLIN(globals, frame->ANP) * 0.6f;	/* -4.44 dB */
	amp_bypas = DBtoLIN(globals, frame->AB) * 0.05f;	/* -26.0 db */

	if (globals->nfcascade >= 8)
	{
		/* Inside Nyquist rate ? */
		if (globals->samrate >= 16000)
			setabc(7500, 600, &r8c);
		else
			globals->nfcascade = 6;
	}

	if (globals->nfcascade >= 7)
	{
		/* Inside Nyquist rate ? */
		if (globals->samrate >= 16000)
			setabc(6500, 500, &r7c);
		else
			globals->nfcascade = 6;
	}

	/* Set coefficients of variable cascade resonators */

	if (globals->nfcascade >= 6)
		setabc(frame->F6hz, frame->B6hz, &r6c);

	if (globals->nfcascade >= 5)
		setabc(frame->F5hz, frame->B5hz, &r5c);

	setabc(frame->F4hz, frame->B4hz, &r4c);
	setabc(frame->F3hz, frame->B3hz, &r3c);
	setabc(frame->F2hz, frame->B2hz, &r2c);
	setabc(frame->F1hz, frame->B1hz, &r1c);

	/* Set coeficients of nasal resonator and zero antiresonator */
	setabc(frame->FNPhz, frame->BNPhz, &rnpc);
	setzeroabc(frame->FNZhz, frame->BNZhz, &rnz);

	/* Set coefficients of parallel resonators, and amplitude of outputs */
	setabcg(frame->F1hz, frame->B1phz, &r1p, amp_parF1);
	setabcg(frame->FNPhz, frame->BNPhz, &rnpp, amp_parFN);
	setabcg(frame->F2hz, frame->B2phz, &r2p, amp_parF2);
	setabcg(frame->F3hz, frame->B3phz, &r3p, amp_parF3);
	setabcg(frame->F4hz, frame->B4phz, &r4p, amp_parF4);
	setabcg(frame->F5hz, frame->B5phz, &r5p, amp_parF5);
	setabcg(frame->F6hz, frame->B6phz, &r6p, amp_parF6);


	/* fold overall gain into output resonator */
	Gain0 = frame->Gain0 - 3;
	if (Gain0 <= 0)
	Gain0 = 57;
	/* output low-pass filter - resonator with freq 0 and BW = globals->samrate
    Thus 3db point is globals->samrate/2 i.e. Nyquist limit.
    Only 3db down seems rather mild...
	*/
	setabcg(0L, (long) globals->samrate, &rout, DBtoLIN(globals, Gain0));
}

static int16_t clip(klatt_global_ptr globals, float input)
{
	long temp = input;
	/* clip on boundaries of 16-bit word */
	if (temp < -32767)
	{
	  //		overload_warning(globals, -temp);
		temp = -32767;
	}
	else if (temp > 32767)
	{
	  //		overload_warning(globals, temp);
		temp = 32767;
	}
	return (temp);
}

/* Generic resonator function */
static float resonator(resonator_ptr r, float input)
{
	register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
	r->p2 = r->p1;
	r->p1 = x;
	return x;
}

/* Generic anti-resonator function
   Same as resonator except that a,b,c need to be set with setzeroabc()
   and we save inputs in p1/p2 rather than outputs.
   There is currently only one of these - "rnz"
 */
/*  Output = (rnz.a * input) + (rnz.b * oldin1) + (rnz.c * oldin2) */

static float antiresonator(resonator_ptr r, float input)
{
	register float x = r->a * input + r->b * r->p1 + r->c * r->p2;
	r->p2 = r->p1;
	r->p1 = input;
	return x;
}

/*
   function PARWAV

   CONVERT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
   Synthesize globals->nspfr samples of waveform and store in jwave[].
 */

void parwave(klatt_global_ptr globals, klatt_frame_ptr frame, short *jwave)
{
	long ns;
	float out = 0.0f;
	/* Output of cascade branch, also final output  */

	/* Initialize synthesizer and get specification for current speech
    frame from host microcomputer */

	frame_init(globals, frame);

	if (globals->f0_flutter != 0)
	{
		time_count++;                  /* used for f0 flutter */
		flutter(globals, frame);       /* add f0 flutter */
	}

	/* MAIN LOOP, for each output sample of current frame: */

	for (ns = 0; ns < globals->nspfr; ns++)
	{
		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75f * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++) // TODO ALL SOURCES as below
		{
			if (globals->glsource == IMPULSIVE)
			{
				/* Use impulsive glottal source */
				voice = impulsive_source(nper);
			}
			else if (globals->glsource == NATURAL)
			{
				/* Or use a more-natural-shaped source waveform with excitation
				occurring both upon opening and upon closure, stronest at closure */
				voice = natural_source(nper);
			}
			else
			  {
				voice = sampled_source(nper);
			}


/*            Modify F1 and BW1 pitch synchrounously - from parwv.c */
/*
                if (nper == nopen) {
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz,B1hz);
                    }
                    F1hzmod = 0;                // Glottis closes 
                    B1hzmod = 0;
                }
                if (nper == T0) {
                    F1hzmod = dF1hz;            // opens
                    B1hzmod = dB1hz;
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz+F1hzmod,B1hz+B1hzmod);
                    }
                }
            }
*/

			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;

		out = resonator(&rout, out);
		*jwave++ = clip(globals, out); /* Convert back to integer */
	}
}

float out;

void initparwave(klatt_global_ptr globals, klatt_frame_ptr frame){
  //  float out;

	frame_init(globals, frame);

	if (globals->f0_flutter != 0)
	{
		time_count++;                  /* used for f0 flutter */
		flutter(globals, frame);       /* add f0 flutter */
	}
	out=0.0f;
}

//    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 

unsigned int parwavesinglesample(klatt_global_ptr globals, klatt_frame_ptr frame, unsigned char ns){
		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75f * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++)
		{
		        switch(globals->glsource)
			  {
			  case IMPULSIVE:
			    /* Use impulsive glottal source */
			    voice = impulsive_source(nper);
			    break;
			  case NATURAL:
			    /* Or use a more-natural-shaped source waveform with excitation
			       occurring both upon opening and upon closure, stronest at closure */
			    voice = natural_source(nper);
			    break;
			  case SAMPLE:
			    voice = sampled_source(nper);
			    break;
			  case TRIANGULAR:
			    voice = triangular_source(nper);
			    break;
			  case WAVETABLE:
			    voice = wave_source(nper);
			  }

/*            Modify F1 and BW1 pitch synchrounously - from parwv.c */
/*
                if (nper == nopen) {
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz,B1hz);
                    }
                    F1hzmod = 0;                // Glottis closes 
                    B1hzmod = 0;
                }
                if (nper == T0) {
                    F1hzmod = dF1hz;            // opens
                    B1hzmod = dB1hz;
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz+F1hzmod,B1hz+B1hzmod);
                    }
                }
            }
*/


			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;
		out = resonator(&rout, out);//*8.0f; - why so quiet tho?
		//		*(jwave+(ns%32)) = clip(globals, out); /* Convert back to integer */
		//		u8 rr=ns%32;
		return clip(globals,out);
		//		*(jwave+ns) = rand()%32768;
}



void parwavesample(klatt_global_ptr globals, klatt_frame_ptr frame, short* jwave, unsigned char ns, unsigned char xx){
  //  float out;

		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75 * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++)
		{
			if (globals->glsource == IMPULSIVE)
			{
				/* Use impulsive glottal source */
				voice = impulsive_source(nper);
			}
			else
			{
				/* Or use a more-natural-shaped source waveform with excitation
				occurring both upon opening and upon closure, stronest at closure */
				voice = natural_source(nper);
			}

			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;
		out = resonator(&rout, out);//*8.0f; - why so quiet tho?
		//		*(jwave+(ns%32)) = clip(globals, out); /* Convert back to integer */
		//		u8 rr=ns%32;
		jwave[xx]=clip(globals,out);
		//		*(jwave+ns) = rand()%32768;
}

unsigned int new_parwave(klatt_global_ptr globals, klatt_frame_ptr frame, short *jwave, unsigned int klatthead)
{
	short ns;
	float out = 0.0;
	/* Output of cascade branch, also final output  */

	/* Initialize synthesizer and get specification for current speech
    frame from host microcomputer */

	frame_init(globals, frame);

	if (globals->f0_flutter != 0)
	{
		time_count++;                  /* used for f0 flutter */
		flutter(globals, frame);       /* add f0 flutter */
	}

	/* MAIN LOOP, for each output sample of current frame: */

	for (ns = 0; ns < globals->nspfr; ns++)
	{
		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75 * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++)
		{
			if (globals->glsource == IMPULSIVE)
			{
				/* Use impulsive glottal source */
				voice = impulsive_source(nper);
			}
			else
			{
				/* Or use a more-natural-shaped source waveform with excitation
				occurring both upon opening and upon closure, stronest at closure */
				voice = natural_source(nper);
			}

			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;
		out = resonator(&rout, out);//*8.0f; - why so quiet tho?
		//	*(jwave+klatthead) = clip(globals, out); /* Convert back to integer */
		//		*(jwave+klatthead) = rand()%32768;
				jwave[klatthead] = clip(globals, out); /* Convert back to integer */
		klatthead++;
		//		if (klatthead>=AUDIO_BUFSZ) klatthead=0;

	}
	return klatthead;
}


void parwave_init(klatt_global_ptr globals)
{
	long FLPhz = (950 * globals->samrate) / 10000;
	long BLPhz = (630 * globals->samrate) / 10000;

	minus_pi_t = -PI / globals->samrate;
	two_pi_t = -2.0f * minus_pi_t;

	setabc(FLPhz, BLPhz, &rlp);
	nper = 0;                        /* LG */
	T0 = 0;                          /* LG */

	rnpp.p1 = 0;                     /* parallel nasal pole  */
	rnpp.p2 = 0;

	r1p.p1 = 0;                      /* parallel 1st formant */
	r1p.p2 = 0;

	r2p.p1 = 0;                      /* parallel 2nd formant */
	r2p.p2 = 0;

	r3p.p1 = 0;                      /* parallel 3rd formant */
	r3p.p2 = 0;

	r4p.p1 = 0;                      /* parallel 4th formant */
	r4p.p2 = 0;

	r5p.p1 = 0;                      /* parallel 5th formant */
	r5p.p2 = 0;

	r6p.p1 = 0;                      /* parallel 6th formant */
	r6p.p2 = 0;

	r1c.p1 = 0;                      /* cascade 1st formant  */
	r1c.p2 = 0;

	r2c.p1 = 0;                      /* cascade 2nd formant  */
	r2c.p2 = 0;

	r3c.p1 = 0;                      /* cascade 3rd formant  */
	r3c.p2 = 0;

	r4c.p1 = 0;                      /* cascade 4th formant  */
	r4c.p2 = 0;

	r5c.p1 = 0;                      /* cascade 5th formant  */
	r5c.p2 = 0;

	r6c.p1 = 0;                      /* cascade 6th formant  */
	r6c.p2 = 0;

	r7c.p1 = 0;
	r7c.p2 = 0;

	r8c.p1 = 0;
	r8c.p2 = 0;

	rnpc.p1 = 0;                     /* cascade nasal pole  */
	rnpc.p2 = 0;

	rnz.p1 = 0;                      /* cascade nasal zero  */
	rnz.p2 = 0;

	rgl.p1 = 0;                      /* crit-damped glot low-pass filter */
	rgl.p2 = 0;

	rlp.p1 = 0;                      /* downsamp low-pass filter  */
	rlp.p2 = 0;

	vlast = 0;                       /* Previous output of voice  */
	nlast = 0;                       /* Previous output of random number generator  */
	glotlast = 0;                    /* Previous value of glotout  */
	warnsw = 0;
}


