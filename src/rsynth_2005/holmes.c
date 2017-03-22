/*
    Copyright (c) 1994,2001-2003 Nick Ing-Simmons. All rights reserved.
 
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA

*/
#include "config.h"
/* $Id: //depot/rsynth/holmes.c#39 $
 */
char *holmes_id = "$Id: //depot/rsynth/holmes.c#39 $";
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "useconfig.h"
#include <math.h>
#include "rsynth.h"

typedef struct {
    float v;			/* boundary value */
    long t;			/* transition time */
} slope_t;



typedef struct {
    float a;
    float b;
    float v;
} filter_t, *filter_ptr;

static float
filter(filter_ptr p, float v)
{
    return p->v = (p->a * v + p->b * p->v);
}

/* 'a' is dominant element, 'b' is dominated
   ext is flag to say to use external times from 'a' rather
   than internal i.e. ext != 0 if 'a' is NOT current element.

 */

static void
set_trans(slope_t * t, int i, Elm_ptr a, Elm_ptr b, int ext, int e,
	  float speed)
{
    t[i].t = (long) (((ext) ? a->p[i].ed : a->p[i].id) * speed);
    if (t[i].t) {
	float afrac = a->p[i].prop * 0.01F;
	t[i].v = a->p[i].stdy * (1.0F - afrac) + (afrac * b->p[i].stdy);
    }
    else
	t[i].v = b->p[i].stdy;
}


/*
   ______________ b
   /
   /
   /
   a____________/
   0   d
   ---------------t---------------
 */

float
linear(float a, float b, long t, long d)
{
    if (t <= 0)
	return a;
    else if (t >= d)
	return b;
    else {
	float f = (float) t / (float) d;
	return a + (b - a) * f;
    }
}

float
interpolate(char *w, char *p, slope_t * s, slope_t * e, float mid, long t,
	    long d)
{
    float steady = d - (s->t + e->t);
    if (steady >= 0) {
	/* Value reaches stready state somewhere ... */
	if (t < s->t)
	    return linear(s->v, mid, t, s->t);	/* initial transition */
	else {
	    t -= s->t;
	    if (t <= steady)
		return mid;	/* steady state */
	    else
		return linear(mid, e->v, (int) (t - steady), e->t);
	    /* final transition */
	}
    }
    else {
	float f = (float) 1.0 - ((float) t / (float) d);
	float sp = linear(s->v, mid, t, s->t);
	float ep = linear(e->v, mid, d - t, e->t);
	return f * sp + ((float) 1.0 - f) * ep;
    }
}

// TODO: break down and convert to newsay/get sample

// rsynth and rest as fixed:

extern rsynth_t rsynthi;
static rsynth_t * rsynth=&rsynthi; // where do we fix this
static float *f0; // >>>???
static unsigned char nf0; // >>>???
static filter_t flt[nEparm];

void rsynth_newsay()
{
  unsigned char nelm; // this will be from vocab
  unsigned char *elm; // this will be from vocab
  Elm_ptr le = &Elementz[0];
  unsigned i = 0;
  float contour[3];
  float f0s = rsynth->speaker->F0Hz;
  float f0e = f0s;
  int j;
  float F0Hz;

	i=0;

    if (nf0 < 3 || !f0 || (rsynth->flags & RSYNTH_MONOTONE)) {
	unsigned i;
	f0 = contour;
	nf0 = 3;
	for (i = 0; i < nelm; i += 2) {
	    f0[1] += elm[i + 1];
	}
	if ((rsynth->flags & RSYNTH_MONOTONE)) {
	    f0[0] = rsynth->speaker->F0Hz;	/* top */
	    f0[2] = f0[0];	/* bottom */
	}
	else {
	    f0[0] = 1.1 * rsynth->speaker->F0Hz;	/* top */
	    f0[2] = 0.6 * f0[0];	/* bottom */
	}
    }

    f0e = f0s = *f0++;


    /* flag new utterance */


    /* Experimental feature:
       frac is the proporion of the new value that is fed into lowpass 
       filter. This is added to (1-frac) of old value.
       So a value of 1.0 means no smoothing. A value of 0.5 seems ok
       and tracks look smoother. Smoothing is insurance against 
       interpollation bugs and mis-features changing parameters too fast.

       Revisit this with some theory...
     */
    for (j = 0; j < nEparm; j++) {
	flt[j].v = le->p[j].stdy;
	flt[j].a = rsynth->smooth;
	flt[j].b = 1.0F - rsynth->smooth;
    }
}

static unsigned char nextelement=1;


int16_t rsynth_get_sample(){
  unsigned char nelm; // this will be from vocab
  unsigned char *elm; // this will be from vocab

  static short samplenumber=0;
  static unsigned char newframe=0, dur;
  int16_t sample;
  float ep[nEparm];
  Elm_ptr le = &Elementz[0];
  static unsigned char i = 0;
  unsigned tf0 = 0;
  unsigned ntf0 = 0;
  float f0s = rsynth->speaker->F0Hz;
  float f0e = f0s;
  float speed = rsynth->speed;
  float F0Hz;
  Elm_ptr ce;
  slope_t start[nEparm];
  slope_t end[nEparm];
  static unsigned t;
  
  if (i>nelm && nextelement==1){   // NEW utterance which means we hit nelm=0 in our cycling:
    rsynth_newsay();
  }

  if (nextelement==1){ // we have a new element

    ce = &Elementz[elm[i++]];
    dur = elm[i++];
	/* Skip zero length elements which are only there to affect
	   boundary values of adjacent elements.
	   Note this mainly refers to "QQ" element in fricatives
	   as stops have a non-zero length central element.
	 */
	if (dur > 0) {
	    Elm_ptr ne = (i < nelm) ? &Elementz[elm[i]] : &Elementz[0];

	    int i;
	    for (i = 0; i < nEparm; i++) {


		if (ce->p[i].rk > le->p[i].rk) {
		    set_trans(start, i, ce, le, 0, 's', speed);
		    /* we dominate last */
		}
		else {
		    if (rsynth->parm_file)
 		    set_trans(start, i, le, ce, 1, 's', speed);
		    /* last dominates us */
		}

		if (ne->p[i].rk > ce->p[i].rk) {
		    if (rsynth->parm_file)
		    set_trans(end, i, ne, ce, 1, 'e', speed);
		    /* next dominates us */
		}
		else {
		    if (rsynth->parm_file)
		    set_trans(end, i, ce, ne, 0, 'e', speed);
		    /* we dominate next */
		}

	    }
	    t=0;
	    newframe=1;
	}
	else {
	  // dur==0???
	}
  }	    
	//	    for (t = 0; t < dur; t++, tf0++) {
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
      int j;
		float peak = 0.25;

		for (j = 0; j < nEparm; j++) {
		    ep[j] =
			filter(flt + j,
			       interpolate(ce->name, Ep_namez[j], &start[j],
					   &end[j], (float) ce->p[j].stdy,
					   t, dur));
		}

		while (tf0 == ntf0) {
		    tf0 = 0;
		    f0s = f0e;
		    ntf0 = (unsigned) *f0++;
		    f0e = *f0++;
		}

		/* interpolate the f0 value */
		F0Hz = linear(f0s, f0e, tf0, ntf0);
		nextelement=0;
		t++;
    }

    else { // hit end of DUR number of frames...
      nextelement=1; 
      le = ce; // where we can put this?????? TODO!!!
      rsynth_get_sample();
    }
  }


		/* Now call the synth for each frame */
		//		CHECK_SPACING(ep[f2], ep[f1]);
		//		CHECK_SPACING(ep[f3], ep[f2]);
		//		CHECK_SPACING(rsynth->speaker->F4hz, ep[f3]);

  //samp += rsynth_frame(rsynth, F0Hz, ep, ce->name);
  sample=rsynth_frame_single(rsynth, F0Hz, ep);//TODO

      samplenumber++;
      if (samplenumber>=rsynth->samples_frame) { // how many in a frame??? 256 for 32000 samplerate
      // end of frame so...????
      newframe=1;
      samplenumber=0;
    }
//  }
  return sample;
}




unsigned
rsynth_interpolate(rsynth_t * rsynth, 
    	    	   unsigned char *elm, unsigned nelm, 
		   float *f0, unsigned nf0)
{
    filter_t flt[nEparm];
    float ep[nEparm];
    Elm_ptr le = &Elementz[0];
    unsigned i = 0;
    unsigned tf0 = 0;
    unsigned ntf0 = 0;
    unsigned samp = 0;
    float contour[3];
    float f0s = rsynth->speaker->F0Hz;
    float f0e = f0s;
    int j;
    float speed = rsynth->speed;
    float F0Hz;


	i=0;

    if (nf0 < 3 || !f0 || (rsynth->flags & RSYNTH_MONOTONE)) {
	unsigned i;
	f0 = contour;
	nf0 = 3;
	for (i = 0; i < nelm; i += 2) {
	    f0[1] += elm[i + 1];
	}
	if ((rsynth->flags & RSYNTH_MONOTONE)) {
	    f0[0] = rsynth->speaker->F0Hz;	/* top */
	    f0[2] = f0[0];	/* bottom */
	}
	else {
	    f0[0] = 1.1 * rsynth->speaker->F0Hz;	/* top */
	    f0[2] = 0.6 * f0[0];	/* bottom */
	}
    }

    f0e = f0s = *f0++;


    /* flag new utterance */


    /* Experimental feature:
       frac is the proporion of the new value that is fed into lowpass 
       filter. This is added to (1-frac) of old value.
       So a value of 1.0 means no smoothing. A value of 0.5 seems ok
       and tracks look smoother. Smoothing is insurance against 
       interpollation bugs and mis-features changing parameters too fast.

       Revisit this with some theory...
     */
    for (j = 0; j < nEparm; j++) {
	flt[j].v = le->p[j].stdy;
	flt[j].a = rsynth->smooth;
	flt[j].b = 1.0F - rsynth->smooth;
    }
}
