#ifndef SAM_H
#define SAM_H

#include "audio.h"

void SetInput(char *_input);
void SetSpeed(unsigned char _speed);
void SetPitch(unsigned char _pitch);
void SetMouth(unsigned char _mouth);
void SetThroat(unsigned char _throat);
void EnableSingmode();
void EnableDebug();

void sam_init();
void sam_newsay_banks0();
void sam_newsay_TTS();
void sam_newsay_phon();
void sam_newsay_xy();
void sam_newsay_param();

u8 sam_get_sample_banks0(int16_t *newsample);
u8 sam_get_sample_banks1(int16_t *newsample);
u8 sam_get_sample_TTS(int16_t *newsample);
u8 sam_get_sample_TTSs(int16_t *newsample);
u8 sam_get_sample_phon(int16_t *newsample);
u8 sam_get_sample_phons(int16_t *newsample);
u8 sam_get_sample_phonsing(int16_t *newsample);
u8 sam_get_sample_xy(int16_t *newsample);
u8 sam_get_sample_param(int16_t *newsample);
u8 sam_get_sample_bend(int16_t *newsample);

#endif

