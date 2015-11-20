/*
 * lpc.h
 * 03-26-2015 E. Brombaugh
 * borrows heavily from Peter Knight's Talkie library for Arduino
 * This code is released under GPLv2 license
 */

#ifndef _lpc_
#define _lpc_

#include "audio.h"
//#include "stm32f0xx.h"

void lpc_init(void);
void lpc_newsay(void);

void lpc_say(uint8_t* address);
u16 lpc_get_sample(void);
u8 lpc_busy(void);
void lpc_running();

#endif
