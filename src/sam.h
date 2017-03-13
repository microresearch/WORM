#ifndef SAM_H
#define SAM_H

#include "audio.h"

/*
typedef unsigned char uint8_t;
typedef unsigned char u8; 
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef unsigned short u16;
typedef unsigned short u16;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
*/

void SetInput(char *_input);
void SetSpeed(unsigned char _speed);
void SetPitch(unsigned char _pitch);
void SetMouth(unsigned char _mouth);
void SetThroat(unsigned char _throat);
void EnableSingmode();
void EnableDebug();

void sam_init();
void sam_newsay_banks0();
u8 sam_get_sample_banks0(int16_t *newsample);

#endif

