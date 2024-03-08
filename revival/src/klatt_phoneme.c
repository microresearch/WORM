/* what of accents and so on. schedule for huge pwavbuffer fill?
 */

#include "audio.h"
#include "klatt_phoneme.h"
#include "say.h"
#include "nsynth.h"

extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); 

pair klatt_phoneme(u16 writepos, u8 phoneme){
  u16 x;u16 wpos=writepos;

  PhonemeToWaveData(phoneme,1, 0);

  for (x=0;x<wav_len;x++){
    audio_buffer[wpos]=pWavBuffer[x];
    wpos++;
    if (wpos>=AUDIO_BUFSZ) wpos=0; // or ENDING as in audio.c?
  }

  //  FreePhonemeToWaveData();
  pair r={wav_len,wpos};
  return r;
}


