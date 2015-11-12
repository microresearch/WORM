/* what of accents and so on. schedule for huge pwavbuffer fill?
 */

#include "audio.h"
#include "klatt_phoneme.h"
#include "say.h"
#include "nsynth.h"

extern int16_t audio_buffer[AUDIO_BUFSZ] __attribute__ ((section (".data"))); // TESTY!

// THIS is not list of phonemes! but what is matched in ELM!

static const char phonemmm[69][4] __attribute__ ((section (".flash"))) =
{"END\0","Q\0","P\0","PY\0","PZ\0","T\0","TY\0","TZ\0","K\0","KY\0","KZ\0","B\0","BY\0","BZ\0","D\0","DY\0","DZ\0","G\0","GY\0","GZ\0","M\0","N\0","NG\0","F\0","TH\0","S\0","SH\0","X\0","H\0","V\0","QQ\0","DH\0","DI\0","Z\0","ZZ\0","ZH\0","CH\0","CI\0","J\0","JY\0","L\0","LL\0","RX\0","R\0","W\0","Y\0","I\0","E\0","AA\0","U\0","O\0","OO\0","A\0","EE\0","ER\0","AR\0","AW\0","UU\0","AI\0","IE\0","OI\0","OU\0","OV\0","OA\0","IA\0","IB\0","AIR\0","OOR\0","OR\0"}; // these come from elements.def - do we need to do all phtoelm?

u16 klatt_phoneme(u16* writepos, u8 phoneme){
  char phonOUT[3]; u16 x;u16 wpos=*writepos;


    strcpy(phonOUT,phonemmm[phoneme]);
  //    strcpy(phonOUT,"he");
  PhonemeToWaveData(phonOUT,1, 0);

  for (x=0;x<wav_len;x++){
    audio_buffer[wpos]=pWavBuffer[x];
    wpos++;
    if (wpos>=AUDIO_BUFSZ) wpos=0; // or ENDING as in audio.c?
  }

  FreePhonemeToWaveData();
  *writepos=wpos;
  return wav_len;
}


