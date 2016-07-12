//can't get working (only very briefly)

#include "mdavocoder.h"

void mdaVocoder_init(mdavocoder* unit) 
{
  float param[7];
  
  param[0] = 0.33f;  //input select
  param[1] = 0.99f;  //output dB
  param[2] = 0.40f;  //hi unit->thru was 0.40
  param[3] = 0.40f;  //hi band was 0.40
  param[4] = 0.16f;  //envelope was 0.16
  param[5] = 0.55f;  //filter q was 0.55
  param[6] = 0.6667f;//freq range was 0.6667
  
  //  param[7] = 0.66f;  //num bands      was 0.66 
  
  const float tpofs = 6.2831853f/32000.0f;
  float rr, th, re;
  float sh;
  u8 i,ii;

  for (i=0;i<16;i++){
      for (ii=0;ii<13;ii++){
	unit->f[i][ii]=0.0f;
      }
      //            unit->offset[i]=rand()%16;
               unit->offset[i]=0;
      }

  unit->kval=0;
  unit->swap = 1; if(param[0]>0.5f) unit->swap = 0;
  unit->gain = powf(10.0f, 2.0f * param[1] - 3.0f * param[5] - 2.0f);
  unit->thru = powf(10.0f, 0.5f + 2.0f * param[1]);
  unit->high =  param[3] * param[3] * param[3] * unit->thru;
  unit->thru *= param[2] * param[2] * param[2];
  
  /*  if(param[7]<0.5f) 
  {
    unit->nbnd=8;
    re=0.003f;
    unit->f[1][2] = 3000.0f;
    unit->f[2][2] = 2200.0f;
    unit->f[3][2] = 1500.0f;
    unit->f[4][2] = 1080.0f;
    unit->f[5][2] = 700.0f;
    unit->f[6][2] = 390.0f;
    unit->f[7][2] = 190.0f;
  }
  else 
  {*/
    unit->nbnd=16;
    re=0.0015f;
    unit->f[ 1][2] = 5000.0f; //+1000
    unit->f[ 2][2] = 4000.0f; //+750
    unit->f[ 3][2] = 3250.0f; //+500
    unit->f[ 4][2] = 2750.0f; //+450
    unit->f[ 5][2] = 2300.0f; //+300
    unit->f[ 6][2] = 2000.0f; //+250
    unit->f[ 7][2] = 1750.0f; //+250
    unit->f[ 8][2] = 1500.0f; //+250
    unit->f[ 9][2] = 1250.0f; //+250
    unit->f[10][2] = 1000.0f; //+250
    unit->f[11][2] =  750.0f; //+210
    unit->f[12][2] =  540.0f; //+190
    unit->f[13][2] =  350.0f; //+155
    unit->f[14][2] =  195.0f; //+100
    unit->f[15][2] =   95.0f;
    //  }

  if(param[4]<0.05f) //freeze
  {
    for(i=0;i<unit->nbnd;i++) unit->f[i][12]=0.0f;
  }
  else
  {
    unit->f[0][12] = powf(10.0f, -1.7f - 2.7f * param[4]); //envelope speed
    rr = 0.022f / (float)unit->nbnd; //minimum proportional to frequency to stop distortion0.022f/

    for(i=1;i<unit->nbnd;i++) 
    {                   
      unit->f[i][12] = (float)(0.025f - rr * (float)i);// was 0.025
      if(unit->f[0][12] < unit->f[i][12]) unit->f[i][12] = unit->f[0][12];
    }
    unit->f[0][12] = 0.5f * unit->f[0][12]; //only top band is at full rate
  }

  rr = 1.0 - powf(10.0f, -1.0f - 1.2f * param[5]);///
  sh = (float)powf(2.0f, 3.0f * param[6] - 1.0f); //filter bank range shift 
  //  rr=1.0f;
  //  sh=1.0f;

  for(i=1;i<unit->nbnd;i++)
  {
    unit->f[i][2] *= sh;
    th = acosf((2.0f * rr * cosf(tpofs * unit->f[i][2])) / (1.0f + rr * rr));
    unit->f[i][0] = (float)(2.0f * rr * cosf(th)); //a0
    unit->f[i][1] = (float)(-rr * rr);           //a1
                //was .98
    unit->f[i][2] *= 0.90f; //shift 2nd stage slightly to stop unit->high resonance peaks
    th = acosf((2.0f * rr * cosf(tpofs * unit->f[i][2])) / (1.0f + rr * rr));
    unit->f[i][2] = (float)(2.0f * rr * cosf(th));
  }
}

void mdaVocodersuspend(mdavocoder* unit) ///clear any buffers...
{
  u8 i, j;
  
  for(i=0; i<unit->nbnd; i++) for(j=3; j<12; j++) unit->f[i][j] = 0.0f; //zero band filters and envelopes
  unit->kout = 0.0f;
  unit->kval = 0;
}


void mdaVocoderprocess(mdavocoder* unit,float *input1, float *input2, float *output, int sampleFrames)
{
  float a, b, o=0.0f, aa, bb, oo=unit->kout, g=unit->gain, ht=unit->thru, hh=unit->high, tmp;
  u8 i, sw=unit->swap, nb=unit->nbnd;
  u8 k=unit->kval;
  --input1;
  --input2;
  --output;

  while(--sampleFrames >= 0)
  {
    a = *++input1; //speech  
    b = *++input2; //synth
    //    if(sw==0) { tmp=a; a=b; b=tmp; } //swap channels
 
    tmp = a - unit->f[0][7]; //integrate modulator for HF band and filter bank pre-emphasis
    unit->f[0][7] = a;
    a = tmp;
    
    if(tmp<0.0f) tmp = -tmp;
    unit->f[0][11] -= unit->f[0][12] * (unit->f[0][11] - tmp);      //high band envelope
    o = unit->f[0][11] * (ht * a + hh * (b - unit->f[0][3])); //high band + high thru
    
    unit->f[0][3] = b; //integrate carrier for HF band

    if(++k & 1) //this block runs at half sample rate
      {
	oo = 0.0f;
      aa = a + unit->f[0][9] - unit->f[0][8] - unit->f[0][8];  //apply zeros here instead of in each reson
      unit->f[0][9] = unit->f[0][8];  unit->f[0][8] = a;
     
	       bb = b + unit->f[0][5] - unit->f[0][4] - unit->f[0][4]; 
               unit->f[0][5] = unit->f[0][4];  unit->f[0][4] = b; 

	       for(i=1; i<nb; i++) //filter bank: 4th-order band pass
      {
        tmp = unit->f[i][0] * unit->f[i][3] + unit->f[i][1] * unit->f[i][4] + bb; // carrier
        unit->f[i][4] = unit->f[i][3];
        unit->f[i][3] = tmp;
        tmp += unit->f[i][2] * unit->f[i][5] + unit->f[i][1] * unit->f[i][6];
        unit->f[i][6] = unit->f[i][5];
        unit->f[i][5] = tmp;
      }

	       for(i=1; i<nb; i++) //filter bank: 4th-order band pass
      {
        tmp = unit->f[i][0] * unit->f[i][7] + unit->f[i][1] * unit->f[i][8] + aa; // speech
        unit->f[i][8] = unit->f[i][7];
        unit->f[i][7] = tmp;
        tmp += unit->f[i][2] * unit->f[i][9] + unit->f[i][1] * unit->f[i][10];
        unit->f[i][10] = unit->f[i][9];
        unit->f[i][9] = tmp;
        
	if(tmp<0.0f) tmp = -tmp;
        unit->f[i][11] -= unit->f[i][12] * (unit->f[i][11] - tmp);  // envelope of speech
	oo += unit->f[((i+unit->offset[i])%16)][5] * unit->f[i][11]; // f]i][5] is carrier f[i][11] is speech envelope

	//       oo += unit->f[i][5] * unit->f[i][11]; // f[i][5] is carrier f[i][11] is speech envelope
	// patch envelope x to carrier y (offset one or other but need precalc...)
      }
      }
        o += oo * g; //effect of interpolating back up to Fs would be minimal (aliasing >16kHz)

    *++output = o;
  }

  unit->kout = oo;  
  unit->kval = k & 1;
    if(fabsf(unit->f[0][11])<1.0e-10) unit->f[0][11] = 0.0f; //catch HF envelope denormal

      for(i=1;i<nb;i++) 
    if(fabsf(unit->f[i][3])<1.0e-10 || fabsf(unit->f[i][7])<1.0e-10) 
      for(k=3; k<12; k++) unit->f[i][k] = 0.0f; //catch reson & envelope denormals
    
      //      if(fabs(o)>10.0f) mdaVocodersuspend(unit); //catch 
}
