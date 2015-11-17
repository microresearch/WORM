#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "stdlib.h"
#include "stdint.h"
#include "arm_const_structs.h"

typedef struct {
	u8* txt;
	u8* phoneme;
} VOCAB_T;

typedef struct {
	u8 *txt;
	u8 *phoneme;
	u8 attenuate;
} PHONEMET_T;

typedef struct{
	u8 SoundNumber;
	signed char byte1;
	u8 byte2;
} SOUND_INDEX_T;


void init(void);

#include "english.h"

//#define min(a,b) (a < b) ? a : b
#endif
