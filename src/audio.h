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
//#define AUDIO_BUFSZ 58870 // 59392 // was 32768- now 58k samples (x2) // adapted to log_gen.py
#define AUDIO_BUFSZ 32768 // 59392 // was 32768- now 58k samples (x2) // adapted to log_gen.py

#ifdef TEST
#define MODE 3 // for pcb=2
#define SELX 0 //3
#define SPEED 2 //0
#define SELY 4 //4
#define SELZ 1 //1
#else
#define MODE 3 // for pcb=2 - speed is at TOP
#define SELX 4 //3 - left
#define SPEED 2 // - right
#define SELY 0 // down left
#define SELZ 1 // down right
#endif

void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz);

typedef struct adc_transformer {
  u8 whichone; // of MODE,X,Y,Z,SPEED
  u8 flip;
  float filter_coeff;
  float multiplier;
} adc_transform;


#endif

