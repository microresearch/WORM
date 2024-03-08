/*
 * i2s.h - I2S feeder routines
 */

#ifndef __i2s__
#define __i2s__

#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_dma.h"
#include "codec.h"
#include "audio.h"

void I2S_Block_Init(void);
void I2S_Block_PlayRec(uint32_t txAddr, uint32_t rxAddr, uint32_t Size);

#endif

