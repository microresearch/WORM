/*
 * audio.c - justttt the callback 

LINEIN/OUTR-main IO
LINEIN/OUTL-filter

*/

#define STEREO_BUFSZ (BUFF_LEN/2) // 64
#define MONO_BUFSZ (STEREO_BUFSZ/2) // 32

#include "audio.h"
#include "effect.h"
extern __IO uint16_t adc_buffer[10];
extern int16_t* buf16;
extern u8* datagenbuffer;
int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!
int16_t	left_buffer[MONO_BUFSZ], mono_buffer[MONO_BUFSZ];
#define float float32_t

void Audio_Init(void)
{
	uint32_t i;
	int16_t *audio_ptr;
	
	/* clear the buffer */
	//	audio_ptr = audio_buffer;
	i = AUDIO_BUFSZ;
	while(i-- > 0)
		*audio_ptr++ = 0;
}

void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *(src++);
		sz--;
		*(rdst++) = *(src++);
		//		*(rdst++) = 0;
		sz--;
	}
}

inline void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}


void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz)
{
  register int32_t lp,samplepos;
  register int32_t tmp,tmpl,tmp16,tmp32d;
  u16 x;

  static float samplep=0.0f;
	for (x=0;x<sz/2;x++){
	  samplepos=samplep;
	  //	  mono_buffer[x]=audio_buffer[samplepos&32767];//-32768;
	  mono_buffer[x]=buf16[samplepos&32767]-32768;//-32768;
	  //	  samplep+=8.0f/(float)((adc_buffer[SECOND]>>6)+1);
	  samplep+=1.0f;
	  //	  if (samplepos>=(adc_buffer[THIRD]<<3)) samplep=0.0f;
	  if (samplepos>=32768.0f) samplep=0.0f;
	}
	audio_comb_stereo(sz, dst, left_buffer, mono_buffer);

	  }
