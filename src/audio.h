/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdint.h"
#include "arm_const_structs.h"

#define BUFF_LEN 128 // was 128
#define AUDIO_BUFSZ 32768 // 59392 // was 32768- now 58k samples (x2)

#define MODE 3 // for pcb=2
#define SELX 0 //3
#define SPEED 2 //0
#define END 4 //4
#define SELY 1 //1


void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz);


#endif

