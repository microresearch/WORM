/* say.h
*/

#include "darray.h"



//extern short *pWavBuffer;
extern int16_t pWavBuffer[3840];
extern darray_t wav_elm;
extern unsigned short wav_len;

void PhonemeToWaveData(char *phone, int len, int verbose);
void FreePhonemeToWaveData(void);
