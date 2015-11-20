/******************************************************************************
 * $Id: vocode.h,v 1.2 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1996-1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include "config.h"

//extern size_t vocode_window_length, vocode_window_overlap;
//extern unsigned char vocode_band_count;
//extern VREAL vocode_volume;
//extern VBOOL vocode_normalize;

//extern VINT vocode_modulator_rate;

//void vocode(void);
void vocoder(int16_t *modulator_sample_buffer,int16_t *carrier_sample_buffer, int16_t *output_sample_buffer1); // wihtout overlap
