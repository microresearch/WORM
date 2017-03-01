// perry cook srconvert.c:

#include "audio.h"
#include "nvp.h"


#define TRUE 1
#define WIDTH 8                /* this controls the number of neighboring samples
				   which are used to interpolate the new samples.  The
				   processing time is linearly related to this width */
#define DELAY_SIZE 32

#define USE_TABLE 1          /* this controls whether a linearly interpolated lookup
				   table is used for sinc function calculation, or the
				   sinc is calculated by floating point trig function calls.  */

#define SAMPLES_PER_ZERO_CROSSING 8    /* this defines how finely the sinc function 
					   is sampled for storage in the table  */

float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING] = { 0.0 };

int delay_buffer[3*WIDTH] = { 0 };

int gimme_data(int16_t j)
{
     return delay_buffer[(int) j + WIDTH];
}

void new_data(int16_t data)
{
    int i;
    for (i=0;i<DELAY_SIZE-5;i++)
	delay_buffer[i] = delay_buffer[i+1];
    delay_buffer[DELAY_SIZE-5] = data;
}


#define SINCMODE 0

void make_sinc()
{
    int i;
    float temp,win_freq,win;
    win_freq = PI / WIDTH / SAMPLES_PER_ZERO_CROSSING;
    sinc_table[0] = 1.0;
    for (i=1;i<WIDTH * SAMPLES_PER_ZERO_CROSSING;i++)   {
	temp = (float) i * PI / SAMPLES_PER_ZERO_CROSSING;
	sinc_table[i] = sinf(temp) / temp;
	win = 0.5 + 0.5 * cosf(win_freq * i);
	sinc_table[i] *= win;
    }
}

float t_sinc(float x)
{
    int low;
    float temp,delta;
    if (fabsf(x)>=WIDTH-1)
	return 0.0;
    else {
	temp = fabsf(x) * (float) SAMPLES_PER_ZERO_CROSSING;
	low = temp;          /* these are interpolation steps */
return sinc_table[low];
    }
}

float sinc(float x)
{
    float temp;
    if(USE_TABLE) return t_sinc(x);
    else        {
	if (x==0.0) return 1.0;
	else {
	    temp = PI * x;
	    return sinf(temp) / (temp);
	}
    }      
}


void sample_rate_init(){
    if(USE_TABLE) make_sinc();
//    factor = initial_factor;
//    delta_factor = (final_factor -initial_factor) / (float) num_samples;
}


float linear_interp(float first_number,float second_number,float fraction)
{
    return (first_number + ((second_number - first_number) * fraction));
}


void dosamplerate(int16_t* in, int16_t* out, float factor, u8 size){

float one_over_factor,delta_factor,final_factor,initial_factor;
float alpha;
//    u8 mode;
 int16_t data_in;
float temp1=0.0f,temp3;
static float time_now=0.0f;
static int32_t total_written=0,j;
int32_t left_limit,right_limit;
static int32_t int_time=0,last_time=0;
 int16_t x=0;
for (u8 ii=0;ii<size;ii++){
temp1 = 0.0f;

if (SINCMODE)       {

left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;
//if (right_limit>size) right_limit = size;

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
 }// not SINCMODE but interpol
 else {
alpha = time_now - (float) int_time;
out[ii] = (delay_buffer[DELAY_SIZE-5] * alpha)
  + (delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
 out[ii] = data_in;

}


//            fwrite(&data_out,2,1,sound_out);
total_written++;
time_now += factor;
last_time = int_time;
int_time = time_now;
while(last_time<int_time)      {
new_data(in[x%size]);
 data_in=in[x%size];
 x++;
last_time += 1;
}

}
//            factor  = initial_factor + (time_now * delta_factor);
}

