// Thnaks to: perry cook srconvrt.c:

#include "audio.h"

#define WIDTH 16              
#define DELAY_SIZE 6 // was 40 --- 3*width=16 = 3*16=48-5=43 - use 7 for simplea

#define SAMPLES_PER_ZERO_CROSSING 8   /* this defines how finely the sinc function 
					   is sampled for storage in the table  */

//float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING] = { 0.0 }; 
//float sinc_table[1] = { 0.0 };

extern __IO uint16_t adc_buffer[10];
extern float _selx, _sely, _selz;
extern float exy[240];
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
  smoothed_adc_value[4] += 0.01f * (value - smoothed_adc_value[4]); // 
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
}

static int16_t delay_buffer[2] = { 0 }; // was 48 but it doesn't need to be so big

static void new_data(int16_t data)
{
  //    for (u8 ii=0;ii<DELAY_SIZE-5;ii++)	
  delay_buffer[0] = delay_buffer[1];
    delay_buffer[1] = data;
}


void samplerate_simple(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio){
  float alpha;
  static float time_now=0.0f;
  long last_time;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;

for (u8 ii=0;ii<size;ii++){

  if (time_now>32768){
    int_time=0; // preserve???
    time_now-=32768.0f;
  }

  // deal also with trigger
    if (in[ii]>THRESH && !triggered) {
      doadc();
      newsay();
      triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;


  //  out[ii]=getsample();
    alpha = time_now - (float)int_time;

    //    if (factor>1.0){
    //       out[ii]=InterpolateHermite4pt3oX(delay_buffer[DELAY_SIZE-8], delay_buffer[DELAY_SIZE-7], delay_buffer[DELAY_SIZE-6], delay_buffer[DELAY_SIZE-5], alpha);
      //      out[ii]=0;
    //    }
     out[ii] = ((float)delay_buffer[DELAY_SIZE-5] * alpha) + ((float)delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
    //out[ii] = delay_buffer[DELAY_SIZE-5];


  time_now += factor;
  last_time = int_time;
  int_time = time_now;
  while(last_time<int_time)      {
    doadc();
    int16_t val=getsample();
    new_data(val);
    last_time += 1;
  }
 }
}

void samplerate_simple_exy(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), float sampleratio, u8 extent){
  static u8 parammode=1;
  float alpha;
  static float time_now=0.0f;
  long last_time;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;

for (u8 ii=0;ii<size;ii++){

  if (time_now>32768){
    int_time=0; // preserve???
    time_now-=32768.0f;
  }

  // deal also with trigger
    if (in[ii]>THRESH && !triggered) {
    parammode^=1;
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;
    alpha = time_now - (float)int_time;
     out[ii] = ((float)delay_buffer[DELAY_SIZE-5] * alpha) + ((float)delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));

  time_now += factor;
  last_time = int_time;
  int_time = time_now;

 if (parammode==0){
   doadc();
   u8 xaxis=_selx*((float)extent+4.0f); 
   MAXED(xaxis,extent);
   xaxis=extent-xaxis;
   //   exy[xaxis]=1.0f-_sely; // invert or?
   exy[xaxis]=_sely;
 }

  while(last_time<int_time)      {
    doadc();
    int16_t val=getsample();
    new_data(val);
    last_time += 1;
  }
 }
}

void samplerate_simple_exy_trigger(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio, u8 extent){
  float alpha;
  static float time_now=0.0f;
  long last_time;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;

for (u8 ii=0;ii<size;ii++){

  if (time_now>32768){
    int_time=0; // preserve???
    time_now-=32768.0f;
  }

  // deal also with trigger
    if (in[ii]>THRESH && !triggered) {
      doadc();
      newsay();
      triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;
    alpha = time_now - (float)int_time;
     out[ii] = ((float)delay_buffer[DELAY_SIZE-5] * alpha) + ((float)delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));

  time_now += factor;
  last_time = int_time;
  int_time = time_now;

   doadc();
   u8 xaxis=_selx*((float)extent+4.0f); 
   MAXED(xaxis,extent);
   xaxis=extent-xaxis;
   //   exy[xaxis]=1.0f-_sely; // invert or?
   exy[xaxis]=_sely; 

  while(last_time<int_time)      {
    doadc();
    int16_t val=getsample();
    new_data(val);
    last_time += 1;
  }
 }
}


