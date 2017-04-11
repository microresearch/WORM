// perry cook srconvert.c:

#include "audio.h"
#include "nvp.h"


#define TRUE 1
#define WIDTH 16                /* this controls the number of neighboring samples
				   which are used to interpolate the new samples.  The
				   processing time is linearly related to this width */
#define DELAY_SIZE 40

#define USE_TABLE 1          /* this controls whether a linearly interpolated lookup
				   table is used for sinc function calculation, or the
				   sinc is calculated by floating point trig function calls.  */

#define SAMPLES_PER_ZERO_CROSSING 8   /* this defines how finely the sinc function 
					   is sampled for storage in the table  */

//float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING] = { 0.0 }; // TODO: if we use table then store as const
//float sinc_table[1] = { 0.0 };

extern __IO uint16_t adc_buffer[10];
extern float _selx, _sely, _selz;
extern float exy[64];
extern float smoothed_adc_value[5];

static inline void doadc(){
  float value;
  
  value =(float)adc_buffer[SELX]/65536.0f; 
  smoothed_adc_value[2] += 0.1f * (value - smoothed_adc_value[2]);
  _selx=smoothed_adc_value[2];
  CONSTRAIN(_selx,0.0f,1.0f);

  value =(float)adc_buffer[SELY]/65536.0f; 
  smoothed_adc_value[3] += 0.1f * (value - smoothed_adc_value[3]);
  _sely=smoothed_adc_value[3];
  CONSTRAIN(_sely,0.0f,1.0f);

  value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // smoothing!!!
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
}

// this is for width 16, crossing 8

const float sinc_table[]={0.974349, 0.899774, 0.783151, 0.635087, 0.468759, 0.298481, 0.138189, -0.000000, -0.106962, -0.177365, -0.210003, -0.207638, -0.176405, -0.124857, -0.062790, 0.000000, 0.054864, 0.095233, 0.117213, 0.119807, 0.104753, 0.076025, 0.039083, -0.000000, -0.035425, -0.062441, -0.077914, -0.080624, -0.071277, -0.052247, -0.027101, 0.000000, 0.024946, 0.044263, 0.055562, 0.057805, 0.051352, 0.037806, 0.019687, 0.000000, -0.018242, -0.032456, -0.040840, -0.042578, -0.037893, -0.027940, -0.014568, 0.000000, 0.013523, 0.024072, 0.030300, 0.031593, 0.028114, 0.020724, 0.010800, 0.000000, -0.010010, -0.017800, -0.022378, -0.023301, -0.020703, -0.015234, -0.007924, 0.000000, 0.007312, 0.012972, 0.016265, 0.016889, 0.014961, 0.010974, 0.005689, -0.000000, -0.005212, -0.009210, -0.011501, -0.011890, -0.010485, -0.007654, -0.003948, -0.000000, 0.003578, 0.006285, 0.007800, 0.008012, 0.007018, 0.005087, 0.002604, 0.000000, -0.002322, -0.004044, -0.004974, -0.005060, -0.004387, -0.003146, -0.001592, 0.000000, 0.001385, 0.002380, 0.002885, 0.002890, 0.002465, 0.001737, 0.000863, -0.000000, -0.000720, -0.001208, -0.001428, -0.001392, -0.001152, -0.000786, -0.000377, -0.000000, 0.000289, 0.000462, 0.000516, 0.000473, 0.000364, 0.000229, 0.000099, -0.000000, -0.000059, -0.000080, -0.000072, -0.000049, -0.000025, -0.000009, -0.000001};

int16_t delay_buffer[3*WIDTH] = { 0 };

int16_t gimme_data(int16_t j)
{
     return delay_buffer[(int) j + WIDTH];
}

void new_data(int16_t data)
{
    u8 ii;
    for (ii=0;ii<DELAY_SIZE-5;ii++)	delay_buffer[ii] = delay_buffer[ii+1];
    delay_buffer[DELAY_SIZE-5] = data;
}

float linear_interp(float first_number,float second_number,float fraction)
{
    return (first_number + ((second_number - first_number) * fraction));
}

float sinc(float x)
{
    int low;
    float temp,delta;
    if (fabsf(x)>=WIDTH-1)
	return 0.0f;
    else {
	temp = fabsf(x) * (float) SAMPLES_PER_ZERO_CROSSING;
	low = temp;          /* these are interpolation steps */
	delta = temp - low;  /* and can be ommited if desired */
	return linear_interp(sinc_table[low],sinc_table[low + 1],delta);
	//return sinc_table[low];
    }
}

void samplerate(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), u8 trigger, float sampleratio){
  float one_over_factor;
  float alpha;
  float temp1=0.0f;
  static float time_now=0.0f;
  long j;
  long left_limit,right_limit,last_time=0;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;
 
  if (trigger==1) newsay();   // first trigger from mode-change

for (u8 ii=0;ii<size;ii++){
  temp1 = 0.0f;
  // factor=1.0f;

  if (time_now>327680.0){
    int_time-=time_now; // preserve???
    time_now=0.0f;
  }

  // deal also with trigger
  if (in[ii]>THRESH && !triggered) {
    newsay();
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;

left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
 out[ii] = (int) temp1;
}
 else    {
   one_over_factor = 1.0f / factor;
   for (j=left_limit;j<right_limit;j++)
     temp1 += gimme_data(j-int_time) * one_over_factor *
       sinc(one_over_factor * (time_now - (float) j));
   out[ii] = (int) temp1;
}

time_now += factor;
last_time = int_time;
int_time = time_now;
 doadc();
while(last_time<int_time)      {
  int16_t val=getsample();
  new_data(val);
last_time += 1;
}
 }
}




void samplerate_exy(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), u8 trigger, float sampleratio, u8 extent){
  static u8 parammode=0;
  float one_over_factor;
  float alpha;
  float temp1=0.0f;
  static float time_now=0.0f;
  long j;
  long left_limit,right_limit,last_time=0;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;
 
  if (trigger==1) newsay();   // first trigger from mode-change

for (u8 ii=0;ii<size;ii++){
  temp1 = 0.0f;
  // factor=1.0f;

  if (time_now>327680.0){
    int_time-=time_now; // preserve???
    time_now=0.0f;
  }

  // deal also with trigger
  if (in[ii]>THRESH && !triggered) {
    parammode^=1;
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;

left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
 out[ii] = (int) temp1;
}
 else    {
   one_over_factor = 1.0f / factor;
   for (j=left_limit;j<right_limit;j++)
     temp1 += gimme_data(j-int_time) * one_over_factor *
       sinc(one_over_factor * (time_now - (float) j));
   out[ii] = (int) temp1;
}

time_now += factor;
last_time = int_time;
int_time = time_now;
 doadc();

 if (parammode==0){
   u8 xaxis=_selx*((float)extent+4.0f); 
   MAXED(xaxis,extent);
   xaxis=extent-xaxis;
   exy[xaxis]=1.0f-_sely; // invert or?
 }

while(last_time<int_time)      {
  int16_t val=getsample();
  new_data(val);
last_time += 1;
}
 }
}


