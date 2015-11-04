/*
 * audio_delay.c - Audio processing routines for simple fixed delay
 */

#include "audio.h"

/* Raw ADC results from knobs */ 
extern __IO uint16_t adc_buffer[2];

/* Stereo buffers */
#define STEREO_BUFSZ (BUFF_LEN/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)
int16_t	left_buffer[MONO_BUFSZ], right_buffer[MONO_BUFSZ],
		mono_buffer[MONO_BUFSZ];

/* Audio Delay buffer */
#define AUDIO_BUFSZ 32768
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".ccmdata")));;
int16_t *audio_ptr;

/**
  * @brief  This function sets up audio processing. 
  * @param  none
  * @retval none
  */
void Audio_Init(void)
{
	uint32_t i;
	
	/* clear the buffer */
	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
	
	/* init the pointer */
	audio_ptr = audio_buffer;
}

/**
  * @brief  Split interleaved stereo into two separate buffers
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  src - pointer to source buffer
  * @param  ldst - pointer to left dest buffer (even samples)
  * @param  rdst - pointer to right dest buffer (odd samples)
  * @retval none
  */
void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src++;
		sz--;
	}
}

/**
  * @brief  combine two separate buffers into interleaved stereo
  * @param  sz -  samples per output buffer (divisible by 2)
  * @param  dst - pointer to source buffer
  * @param  lsrc - pointer to left dest buffer (even samples)
  * @param  rsrc - pointer to right dest buffer (odd samples)
  * @retval none
  */
void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = *rsrc++;
		sz--;
	}
}

/**
  * @brief put data in circular buffer and update pointer
  * @param  in - data going in
  * @retval none
  */
void buffer_put(int16_t in)
{
	/* put data in */
	*audio_ptr++ = in;
	
	/* wrap pointer */
	if(audio_ptr-audio_buffer == AUDIO_BUFSZ)
		audio_ptr = audio_buffer;
}

/**
  * @brief  delay with feedback
  * @param  sz -  samples per buffer
  * @param  dst - pointer to dest buffer
  * @param  src - pointer to source buffer
  * @param  fbk - float feedback gain
  * @retval none
  */
void audio_delay_fbk(int16_t sz, int16_t *dst, int16_t *src, float32_t fbk)
{
	float32_t f_sum;
	int32_t sum;
	
	while(sz--)
	{
		/* copy delayed value to destination */
		*dst++ = *audio_ptr;
		
		/* mix input and delayed with saturation */
		f_sum = ((float32_t)*src++) + ((float32_t)*audio_ptr * fbk);
		sum = f_sum;
#if 0
		sum = sum > 32767 ? 32767 : sum;
		sum = sum < -32768 ? -32768 : sum;
#else
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (sum) : [src] "r" (sum));
#endif
		
		/* stuff feedback mixed result in delay buffer */
		buffer_put(sum);
	}
}

/**
  * @brief  morph a to b into destination
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  asrc - pointer to dest buffer
  * @param  bsrc - pointer to dest buffer
  * @param  morph - float morph coeff. 0 = a, 1 = b
  * @retval none
  */
void audio_morph(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
				float32_t morph)
{
	float32_t morph_inv = 1.0 - morph, f_sum;
	int32_t sum;
	
	while(sz--)
	{
		f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;
		sum = f_sum;
#if 0
		sum = sum > 32767 ? 32767 : sum;
		sum = sum < -32768 ? -32768 : sum;
#else
		asm("ssat %[dst], #16, %[src]" : [dst] "=r" (sum) : [src] "r" (sum));
#endif
		
		/* save to destination */
		*dst++ = sum;
	}
}

/**
  * @brief  This function handles I2S RX buffer processing. 
  * @param  src - pointer to source buffer
  * @param  dst - pointer to dest buffer
  * @param  sz -  samples per buffer
  * @retval none
  */
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz, uint16_t ht)
{
	float32_t f_p0, f_p1, tb_l, tb_h, f_i, m;
	int32_t i;
	
	/* Setup pot variables */
	f_p0 = (float32_t)adc_buffer[0]/4096.0;
	f_p1 = (float32_t)adc_buffer[1]/4096.0;
	
	/* Split Stereo */
	audio_split_stereo(sz, src, left_buffer, right_buffer);
	
	/* delay left channel into mono buffer */
	audio_delay_fbk(sz/2, mono_buffer, left_buffer, f_p0);
	
	/* wet/dry mix mono buffer & left buffer into left buffer */
	audio_morph(sz/2, left_buffer, left_buffer, mono_buffer, f_p1);

	/* Combine stereo */
	audio_comb_stereo(sz, dst, left_buffer, right_buffer);
}
