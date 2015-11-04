/*
  Copyright 2008-2011 David Robillard <http://drobilla.net>
  Copyright 1999-2000 Paul Kellett (Maxim Digital Audio)

  This is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this software. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mdavocoder.h"

void mdavocal_init(mdavocal* unit) 
{
  unit->param[0] = 0.9f;  //Tracking Off / On / Quant
  unit->param[1] = 0.90f;  //Pitch
  unit->param[2] = 0.90f;  //Breath Noise
  unit->param[3] = 0.90f;  //Voiced/Unvoiced Thresh
  unit->param[4] = 0.95f;  //Max Freq

  unit->track = 0;
  unit->pstep = unit->pmult = unit->sawbuf = unit->noise = unit->lenv = unit->henv = 0.0f;
  unit->lbuf0 = unit->lbuf1 = unit->lbuf2 = unit->lbuf3 = unit->lfreq = unit->vuv = unit->maxp = unit->minp = 0.0f;
  unit->root = 0.0;

  float fs, ifs;
  fs = 48000.0f;
  ifs = 1.0f / fs;

  unit->track = (int32_t)(2.99f * unit->param[0]);
  unit->pmult = (float)powf(1.0594631f, floor(48.0f * unit->param[1] - 24.0f));
  if(unit->track==0) unit->pstep = 110.0f * unit->pmult * ifs;

  unit->noise = 6.0f * unit->param[2];
  unit->lfreq = 660.0f * ifs;
  unit->minp = (float)powf(16.0f, 0.5f - unit->param[4]) * fs / 440.0f;
  unit->maxp = 0.03f * fs;
  unit->root = log10(8.1757989f * ifs);
  unit->vuv = unit->param[3] * unit->param[3];
}


void mdavocal_process(mdavocal *unit, float *input1, float *input2, float *output, int sampleFrames)
{
  /*  float *in1 = inputs[0];
  float *in2 = inputs[1];
  float *out1 = outputs[0];
  float *out2 = outputs[1];
  */
  float a, b;
  float ds=unit->pstep, s=unit->sawbuf, n=unit->noise;
  float l0=unit->lbuf0, l1=unit->lbuf1, l2=unit->lbuf2, l3=unit->lbuf3;
  float le=unit->lenv, he=unit->henv, et=unit->lfreq*0.1f, lf=unit->lfreq, v=unit->vuv, mn=unit->minp, mx=unit->maxp;
  float rootm=39.863137f;
  int32_t  tr=unit->track;

  --input1;
  --input2;
  --output;

  while(--sampleFrames >= 0)
  {
    a = *++input1;
    b = *++input2;

    l0 -= lf * (l1 + a);       //fundamental filter (peaking 2nd-order 100Hz lpf)
    l1 -= lf * (l1 - l0);

    b = l0; if(b<0.0f) b = -b;
    le -= et * (le - b);       //fundamental level

    b = (a + 0.03f) * v;
    if(b<0.0f) b = -b;
    he -= et * (he - b);       //overall level (+ constant so >f0 when quiet)

    l3 += 1.0f;
    if(tr>0)                   //pitch tracking
    {
      if(l1>0.0f && l2<=0.0f)  //found +ve zero crossing
      {
        if(l3>mn && l3<mx)     //...in allowed range
        {
          mn = 0.6f * l3;       //new max pitch to discourage octave jumps!
          l2 = l1 / (l1 - l2);   //fractional period...
          ds = unit->pmult / (l3 - l2); //new period

          if(tr==2)            //quantize pitch
          {
            ds = rootm * (float)(log10(ds) - unit->root);
            ds = (float)powf(1.0594631, floor(ds + 0.5) + rootm * unit->root);
          }
        }
        l3 = l2;               //restart period measurement
      }
      l2=l1;                   //remember previous sample
    }

    b = 0.00001f * (float)((rand() & 32767) - 16384);  //sibilance
    if(le>he) b *= s * n;                    //...or modulated breath noise
    b += s; s += ds; if(s>0.5f) s-=1.0f;     //badly aliased sawtooth!

    *++output = b;
    //    *++out2 = b;
  }
  unit->sawbuf=s;

  if(fabsf(he)>1.0e-10) unit->henv = he; else unit->henv=0.0f; //catch denormals
  if(fabsf(l1)>1.0e-10) { unit->lbuf0=l0; unit->lbuf1=l1; unit->lenv=le; } else { unit->lbuf0 = unit->lbuf1= unit->lenv = 0.0f; }

  unit->lbuf2=l2, unit->lbuf3=l3;
  if(tr) unit->pstep=ds;
}

