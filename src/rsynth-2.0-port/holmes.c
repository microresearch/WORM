#include <config.h>
/* holmes.c
 */

#define ELM_LEN 48


#include <stdio.h>
#include <ctype.h>
#include <math.h>
#ifndef LAP
#include "audio.h"
extern u8 test_elm[ELM_LEN]; // how long test_elm can be!? TEST
extern char TTSinarray[17];
#else
#include "forlap.h"
#include "resources.c"
const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?
u8 test_elm[ELM_LEN]; // how long test_elm can be!? TEST
char TTSinarray[17];

unsigned char text2speechforklatt(unsigned char input_len, unsigned char *input, unsigned char *output){
  return 0;
}

#endif
#include "nsynth.h"
#include "elements.h"
#include "darray.h"
#include "holmes.h"
#include "say.h"
#include "phfeat.h"
#include "resources.h"
#include "english2phoneme/TTS.h"

typedef struct {
  unsigned char length;
  unsigned char elements[64]; // max number of elements
} klattvocab_;


static const klattvocab_ vocab_help  __attribute__ ((section (".flash"))) ={24, 28, 10, 0, 47, 6, 0, 40, 8, 0, 2, 8, 0, 3, 1, 0, 4, 2, 0, 1, 6, 0, 1, 6, 0}; // test this!
static const klattvocab_ one  __attribute__ ((section (".flash"))) = {15, 44, 8, 0, 49, 7, 0, 21, 8, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ two  __attribute__ ((section (".flash")))= {18, 5, 6, 0, 6, 1, 0, 7, 2, 0, 57, 11, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ three  __attribute__ ((section (".flash"))) = {15, 24, 15, 0, 43, 11, 0, 53, 9, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ four __attribute__ ((section (".flash"))) = {15, 23, 12, 0, 56, 13, 0, 43, 11, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ five  __attribute__ ((section (".flash")))= {24, 23, 12, 0, 59, 7, 0, 46, 7, 0, 29, 4, 0, 30, 0, 0, 29, 4, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ six  __attribute__ ((section (".flash")))= {24, 25, 12, 0, 46, 7, 0, 8, 8, 0, 9, 1, 0, 10, 4, 0, 25, 12, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ seven  __attribute__ ((section (".flash"))) = {27, 25, 12, 0, 53, 9, 0, 29, 4, 0, 30, 0, 0, 29, 4, 0, 47, 6, 0, 21, 8, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ eight __attribute__ ((section (".flash"))) = {21, 58, 7, 0, 46, 7, 0, 5, 6, 0, 6, 1, 0, 7, 2, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ nine  __attribute__ ((section (".flash")))= {18, 21, 8, 0, 59, 7, 0, 46, 7, 0, 21, 8, 0, 1, 6, 0, 1, 6, 0};
static const klattvocab_ ten  __attribute__ ((section (".flash")))= {21, 5, 6, 0, 6, 1, 0, 7, 2, 0, 47, 6, 0, 21, 8, 0, 1, 6, 0, 1, 6, 0};

static const klattvocab_ *vocabklatt[3]={&one, &two, &three};


static u8 TTSoutarray[256];
extern const u8 phoneme_prob_remap[64];

//#if 0
//	#define AMP_ADJ 14
//#else
	#define AMP_ADJ 0
//#endif

//extern uint16_t adc_buffer[10];

extern float _selx, _sely, _selz;

int speed = 1;			//normal
// int speed = 2; 		//slow

float frac = 1.0f;

typedef struct
 {
  float v;                        /* boundary value */
  int t;                          /* transition time */
 }
slope_t;

typedef struct
 {
  slope_t p[nEparm];
 }
trans_t;

typedef struct
 {
  float a;
  float b;
  float v;
 }
filter_t, *filter_ptr;

static float filter (filter_ptr p, float v);

static float filter(filter_ptr p, float v)
{
	return p->v = (p->a * v + p->b * p->v);
}

/* 'a' is dominant element, 'b' is dominated
   ext is flag to say to use external times from 'a' rather
   than internal i.e. ext != 0 if 'a' is NOT current element.

 */

static void set_trans (slope_t * t, Elm_ptr a, Elm_ptr b, int ext, int e);

static void set_trans(slope_t *t, Elm_ptr a, Elm_ptr b, int ext, int e)
{
	int i;
	
	for (i = 0; i < nEparm; i++)
	{
		t[i].t = ((ext) ? a->p[i].ed : a->p[i].id) * speed;
	
		if (t[i].t)
			t[i].v = a->p[i].fixd + (a->p[i].prop * b->p[i].stdy) * (float) 0.01f;
		else
			t[i].v = b->p[i].stdy;
	}
}

static float linear (float a, float b, int t, int d);

/*              
   ______________ b
   /
   /
   /
   a____________/                 
   0   d
   ---------------t---------------
 */

static float linear(float a, float b, int t, int d)
{
	if (t <= 0)
		return a;
	else if (t >= d)
		return b;
	else
	{
		float f = (float) t / (float) d;
		return a + (b - a) * f;
	}
}

static float interpolate (char *w, char *p, slope_t * s, slope_t * e, float mid, int t, int d);

static float interpolate(char *w, char *p, slope_t *s, slope_t *e, float mid, int t, int d)
{
	float steady = d - (s->t + e->t);
	#ifdef DEBUG
	fprintf(stdout, "%4s %s s=%g,%d e=%g,%d m=%g,%g\n",
			w, p, s->v, s->t, e->v, e->t, mid, steady);
	#endif
	
	if (steady >= 0)
	{
		/* Value reaches stready state somewhere ... */
		if (t < s->t)
			return linear(s->v, mid, t, s->t);	/* initial transition */
		else
		{
			t -= s->t;
			if (t <= steady)
				return mid;                 /* steady state */
			else
				return linear(mid, e->v, (int) (t - steady), e->t);
				/* final transition */
		}
	}
	else
	{
		float f = (float) 1.0f - ((float) t / (float) d);
		float sp = linear(s->v, mid, t, s->t);
		float ep = linear(e->v, mid, d - t, e->t);
		return f * sp + ((float) 1.0f - f) * ep;
	}
}


unsigned holmes(unsigned nelm, unsigned char *elm, unsigned nsamp, short *samp_base)
{
	filter_t flt[nEparm];
	klatt_frame_t pars;
	short *samp = samp_base;
	Elm_ptr le = &Elements[0];
	unsigned i = 0;
	unsigned tstress = 0;
	unsigned ntstress = 0;
	slope_t stress_s;
	slope_t stress_e;
	float top = 1.1 * def_pars.F0hz10;
	int j;
	pars = def_pars;
	pars.FNPhz = le->p[fn].stdy;
	pars.B1phz = pars.B1hz = 60;
	pars.B2phz = pars.B2hz = 90;
	pars.B3phz = pars.B3hz = 150;
	#if 0
	pars.F4hz = 3500;
	#endif
	pars.B4phz = def_pars.B4phz;

	/* flag new utterance */
	parwave_init(&klatt_global);

	/* Set stress attack/decay slope */
	stress_s.t = 40;
	stress_e.t = 40;
	stress_e.v = 0.0f;

	for (j = 0; j < nEparm; j++)
	{
		flt[j].v = le->p[j].stdy;
		flt[j].a = frac;
		flt[j].b = (float) 1.0f - (float) frac;
	}
	while (i < nelm)
	{
		Elm_ptr ce = &Elements[elm[i++]];
		unsigned dur = elm[i++];
		i++; /* skip stress */
		/* Skip zero length elements which are only there to affect
		boundary values of adjacent elements
		*/
		if (dur > 0)
		{
			Elm_ptr ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
						slope_t startyy[nEparm];
						slope_t end[nEparm];
			unsigned t;

			if (ce->rk > le->rk)
			{
			  				set_trans(startyy, ce, le, 0, 's');
				/* we dominate last */
			}
			else
			{
			  				set_trans(startyy, le, ce, 1, 's');
				/* last dominates us */
			}

			if (ne->rk > ce->rk)
			{
			  				set_trans(end, ne, ce, 1, 'e');
				/* next dominates us */
			}
			else
			{
			  				set_trans(end, ce, ne, 0, 'e');
				/* we dominate next */
			}


			for (t = 0; t < dur; t++, tstress++)
			{
				float base = top * 0.8f /* 3 * top / 5 */;
				float tp[nEparm];
				int j;

				if (tstress == ntstress)
				{
					unsigned j = i;
					stress_s = stress_e;
					tstress = 0;
					ntstress = dur;
					#ifdef DEBUG_STRESS
					printf("Stress %g -> ", stress_s.v);
					#endif
					while (j <= nelm)
					{
						Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
						unsigned du = (j < nelm) ? elm[j++] : 0;
						unsigned s  = (j < nelm) ? elm[j++] : 3;
						if (s || e->feat & vwl)
						{
							unsigned d = 0;
							if (s)
								stress_e.v = (float) s / 3.0f;
							else
								stress_e.v = (float) 0.1f;
							do
							{
								d += du;
								#ifdef DEBUG_STRESS
								printf("%s", (e && e->dict) ? e->dict : "");
								#endif
								e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
								du = elm[j++];
							}
							while ((e->feat & vwl) && elm[j++] == s);
							ntstress += d / 2;
							break;
						}
						ntstress += du;
					}
					#ifdef DEBUG_STRESS
					printf(" %g @ %d\n", stress_e.v, ntstress);
					#endif
				}

				for (j = 0; j < nEparm; j++)
				  tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

				/* Now call the synth for each frame */

				pars.F0hz10 = base + (top - base) *
				interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

				pars.AVdb = pars.AVpdb = tp[av];
				pars.AF = tp[af];
				pars.FNZhz = tp[fn];
				pars.ASP = tp[asp];
				pars.Aturb = tp[avc];
				pars.B1phz = pars.B1hz = tp[b1];
				pars.B2phz = pars.B2hz = tp[b2];
				pars.B3phz = pars.B3hz = tp[b3];
				pars.F1hz = tp[f1];
				pars.F2hz = tp[f2];
				pars.F3hz = tp[f3];
				/* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
				Needs to be fixed properly in tables
				*/
				/*
				pars.ANP  = AMP_ADJ + tp[an];
				*/
				pars.AB = AMP_ADJ + tp[ab];
				pars.A5 = AMP_ADJ + tp[a5];
				pars.A6 = AMP_ADJ + tp[a6];
				pars.A1 = AMP_ADJ + tp[a1];
				pars.A2 = AMP_ADJ + tp[a2];
				pars.A3 = AMP_ADJ + tp[a3];
				pars.A4 = AMP_ADJ + tp[a4];

				parwave(&klatt_global, &pars, samp);

				samp += klatt_global.nspfr;
				/* Declination of f0 envelope 0.25Hz / cS */
				top -= 0.5;
}
}
		le = ce;
} 
	return (samp - samp_base);
}

static filter_t flt[nEparm];
static Elm_ptr le;// = &Elements[0];
static Elm_ptr ne;
static unsigned i = 31;
static unsigned tstress = 0;
static unsigned ntstress = 0;
static slope_t stress_s;
static slope_t stress_e;
static float top;// = 1.1 * def_pars.F0hz10;
static unsigned char t;

static u8 nextelement=1;
static klatt_frame_t pars;

static u8 elm_single[6]={28, 10, 0, 47, 6, 0};

void klatt_newsay_single(){ /// elm_single is our list selx is pitch // sely is len // selz is next phoneme
i=0; 
le = &Elements[0];// but what do we do with le?
top = 1.1 * def_pars.F0hz10;

u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
top*=logpitch[val];

u8 len=_sely*130.0f;
MAXED(len,127);
len=127-len;
len=logpitch[len]*10.0f;

// list of two phonemes in elm_single - newsay shifts last phoneme out and adds new one
// selz selects phoneme (set standard lengthy for each phoneme) - there are 64 phonemes also:
// test_elm[xaxis*3]=phoneme_prob_remap[val]; // 64 phonemes

val=_selz*65.0f;
MAXED(val,63);
val=63-val;
val=phoneme_prob_remap[val];

// deal with old phoneme 

elm_single[0]=elm_single[3];
elm_single[1]=elm_single[4];
elm_single[2]=elm_single[5];

// new phoneme

elm_single[3]=val;
elm_single[4]=len; // TEST - just fixed for now
elm_single[5]=0; // always


    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);
    /* Set stress attack/decay slope */
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0f;

    for (u8 j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0f - (float) frac;
      }
    nextelement=1;
}

int16_t klatt_get_sample_single(){
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  int16_t sample;
  unsigned nelm=6; // only 2 phonemes=6 elements
  unsigned char *elm=elm_single; // is our list of phonemes in order phon_number, duration, stress - we cycle through it
  u8 j; 
  static u8 dur,first=0;
  static slope_t startyy[nEparm];
  static slope_t end[nEparm];
  if (i>2 && nextelement==1){   // we just have one element
    klatt_newsay_single();
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elm[i++]];
    dur = elm[i++];
    i++; /* skip stress */
    /* Skip zero length elements which are only there to affect
       boundary values of adjacent elements
    */

    if (dur == 0) { // do what? NOTHING
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	    /* we dominate last */
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	    /* last dominates us */
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	    /* next dominates us */
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	    /* we dominate next */
	  }
	// next set of frames what do we need to init?
	t=0;
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
	newframe=1;
      } // if dur==0
  } // is nextelement==1
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
                  float base = top * 0.8f /* 3 * top / 5 */;
      //      float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3.0f;
		  else
		    stress_e.v = (float) 0.1f;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
	 Needs to be fixed properly in tables
      */
      /*
	pars.ANP  = AMP_ADJ + tp[an];
      */
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    } // if t<dur
    else { // hit end of DUR number of frames...
      nextelement=1; 
      le = ce; // where we can put this?????? TODO!!!
      klatt_get_sample_single();
    }
}
//  if (nextelement==0){
    // always run through samples till we hit next frame
    //    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); 
    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 
    
    ///x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>=klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; 
    }
//  }
  return sample;
}

void klatt_newsay(){ /// elm is our list
i=0; 
le = &Elements[0];
top = 1.1 * def_pars.F0hz10;
//        top= 200+ adc_buffer[SELX];
u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
top*=logpitch[val];

    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);
    /* Set stress attack/decay slope */
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0f;

    for (u8 j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0f - (float) frac;
      }
    nextelement=1;
}

int16_t klatt_get_sample(){
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  int16_t sample;
  unsigned nelm=ELM_LEN; // 10 phonemes = how many frames approx ???? - in test case we have 87 frames - now 16 phonemes *3 = 48
  unsigned char *elm=test_elm; // is our list of phonemes in order phon_number, duration, stress - we cycle through it
  u8 j; 
  static u8 dur,first=0;
  static slope_t startyy[nEparm];
  static slope_t end[nEparm];
  if (i>nelm && nextelement==1){   // NEW utterance which means we hit nelm=0 in our cycling:
    klatt_newsay();
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elm[i++]];
    dur = elm[i++];
    i++; /* skip stress */
    /* Skip zero length elements which are only there to affect
       boundary values of adjacent elements
    */

    if (dur == 0) { // do what? NOTHING
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	    /* we dominate last */
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	    /* last dominates us */
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	    /* next dominates us */
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	    /* we dominate next */
	  }
	// next set of frames what do we need to init?
	t=0;
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
	newframe=1;
      } // if dur==0
  } // is nextelement==1
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
                  float base = top * 0.8f /* 3 * top / 5 */;
      //      float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3.0f;
		  else
		    stress_e.v = (float) 0.1f;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
	 Needs to be fixed properly in tables
      */
      /*
	pars.ANP  = AMP_ADJ + tp[an];
      */
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    } // if t<dur
    else { // hit end of DUR number of frames...
      nextelement=1; 
      le = ce; // where we can put this?????? TODO!!!
      klatt_get_sample();
    }
}
//  if (nextelement==0){
    // always run through samples till we hit next frame
    //    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); 
    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 
    
    ///x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>=klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; 
    }
//  }
  return sample;
}

static u8 elm_plen;
static u8 const *vocab_elm;

void klatt_newsay_vocab(){ /// elm is our list
i=0; 
le = &Elements[0];
top = 1.1 * def_pars.F0hz10;
#ifndef LAP
 u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
top*=logpitch[val];
#endif
 
// select vocab but for now

 elm_plen=vocab_help.length; // TODO: sely bend on length
 vocab_elm=vocab_help.elements;
 
    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);
    /* Set stress attack/decay slope */
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0f;

    for (u8 j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0f - (float) frac;
      }
    nextelement=1;
}

int16_t klatt_get_sample_vocab(){
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  int16_t sample;
  unsigned nelm=elm_plen; 
  unsigned char const *elm=vocab_elm; // is our list of phonemes in order phon_number, duration, stress - we cycle through it
  u8 j; 
  static u8 dur,first=0;
  static slope_t startyy[nEparm];
  static slope_t end[nEparm];
  if (i>nelm && nextelement==1){   // NEW utterance which means we hit nelm=0 in our cycling:
    klatt_newsay_vocab();
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elm[i++]];
    dur = elm[i++];
    i++; /* skip stress */
    /* Skip zero length elements which are only there to affect
       boundary values of adjacent elements
    */

    if (dur == 0) { // do what? NOTHING
      // try next element
      klatt_get_sample_vocab();
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	    /* we dominate last */
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	    /* last dominates us */
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	    /* next dominates us */
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	    /* we dominate next */
	  }
	// next set of frames what do we need to init?
	t=0;
	//	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0]; // ????
	newframe=1;
      } // if dur==0
  } // is nextelement==1
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
                  float base = top * 0.8f /* 3 * top / 5 */;
      //      float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3.0f;
		  else
		    stress_e.v = (float) 0.1f;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
	 Needs to be fixed properly in tables
      */
      /*
	pars.ANP  = AMP_ADJ + tp[an];
      */
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    } // if t<dur
    else { // hit end of DUR number of frames...
      nextelement=1; 
      le = ce; // where we can put this?????? TODO!!!
      klatt_get_sample_vocab();
    }
}
//  if (nextelement==0){
    // always run through samples till we hit next frame
    //    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); 
    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 
    
    ///x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>=klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; 
    }
//  }
  return sample;
}


static unsigned char *elmer;


void klatt_newsayTTS(){
//static signed char testphone[]="sEHEHvEHEHntIYIY sEHEHvAXAXn THTHAWAWzAEAEnd sEHEHvAXAXn hAHAHndrEHEHd sEHEHvEHEHntIYIY sEHEHvAXAXn";
darray_free(&wav_elm);

u8 lenny=text2speechforklatt(32,TTSinarray,TTSoutarray); // restrict TTS to just ascii
if (lenny>32) lenny=32;
PhonemeToWaveData(TTSoutarray, lenny); // only works out if restricted  to say 32 - NOT 64 _ STRESS TESTING - still crashes on 32
//PhonemeToWaveData(testphone, 16);

elmer=(unsigned char *) darray_find(&wav_elm, 0); // is our list of phonemes in order phon_number, duration, stress - we cycle through it
i=0; 
le = &Elements[0];
top = 1.1f * def_pars.F0hz10;
u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
top*=logpitch[val];

    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);
    /* Set stress attack/decay slope */
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0f;

    for (u8 j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0f - (float) frac;
      }
    nextelement=1;
}

int16_t klatt_get_sampleTTS(){
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  int16_t sample=0;
  unsigned nelm=wav_elm.items; // 10 phonemes = how many frames approx ???? - in test case we have 87 frames - now 16 phonemes
  u8 j; 
  unsigned char *elm=elmer;

  static u8 dur,first=0;
  static slope_t startyy[nEparm];
  static slope_t end[nEparm];
  if (i>nelm && nextelement==1){   // NEW utterance which means we hit nelm=0 in our cycling:
    klatt_newsayTTS();
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elmer[i++]];
    dur = elm[i++];
    i++; /* skip stress */
    /* Skip zero length elements which are only there to affect
       boundary values of adjacent elements
    */

    if (dur == 0) { // do what? NOTHING
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	    /* we dominate last */
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	    /* last dominates us */
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	    /* next dominates us */
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	    /* we dominate next */
	  }
	// next set of frames what do we need to init?
	t=0;
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
	newframe=1;
      } // if dur==0
  }
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
                  float base = top * 0.8f /* 3 * top / 5 */;
      //      float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3.0f;
		  else
		    stress_e.v = (float) 0.1f;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
	 Needs to be fixed properly in tables
      */
      /*
	pars.ANP  = AMP_ADJ + tp[an];
      */
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    } // if t<dur
    else { // hit end of DUR number of frames...
      nextelement=1;
      le = ce; // where we can put this?????? TODO!!!
      klatt_get_sampleTTS();
    }
  }
//  if (nextelement==0){// causes clicks
    // always run through samples till we hit next frame
    //    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); 
    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 
    
    ///x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>=klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; // where we can put this?
//    }
  }
  return sample;
}

#ifdef LAP
void main(){
  klatt_init();
    klatt_newsay_vocab();

    //    while(1){
    uint16_t samp=klatt_get_sample_vocab()+32768;
    printf("%c", samp>>8);
    //    }

    uint16_t samples[32000];
  vocab_elm=vocab_help.elements;
  unsigned char *elm=vocab_elm; // is our list of phonemes in order phon_number, duration, stress - we cycle through it

      while(1){
    holmes(vocab_help.length, elm, 16000, samples);
  for (int16_t x=0;x<16000;x++){
    uint16_t samp=samples[x]+32768;
    printf("%c", samp>>8);
    }
      }
      }
#endif

