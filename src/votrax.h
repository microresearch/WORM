/***************************************************************************

    votrax.h

    Simple VOTRAX SC-01 simulator based on sample fragments.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __VOTRAX_H__
#define __VOTRAX_H__

//#include "sound/samples.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VOTRAX_SC01_ADD(_tag, _clock, _interface) \
	MCFG_DEVICE_ADD(_tag, VOTRAX_SC01, _clock) \
	votrax_sc01_device::static_set_interface(*device, _interface);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> votrax_sc01_interface

/*struct votrax_sc01_interface
{
	devcb_write_line m_request_cb;      // callback for request
};
*/

// ======================> votrax_sc01_device

//votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, unsigned int clock);

	// static configuration helpers
//static void static_set_interface(device_t &device, const votrax_sc01_interface &interface);

	// writers
//DECLARE_WRITE8_MEMBER( write );
//DECLARE_WRITE8_MEMBER( inflection_w );
//DECLARE_READ_LINE_MEMBER( request ) { return m_request_state; }

	// device-level overrides
//const rom_entry *device_rom_region() const;
void device_start();
void device_reset();
void device_clock_changed();
//void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_sound_interface overrides
//void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal helpers
void update_subphoneme_clock_period();
static double bits_to_caps(unsigned int value, int caps_count, const double *caps_values);
static void shift_hist(double val, double *hist_array, int hist_size);
static void filter_s_to_z(const double *k, double fs, double *a, double *b);
static double apply_filter(const double *x, const double *y, const double *a, const double *b);

	// internal state
//sound_stream *              m_stream;               // output stream
//emu_timer *                 m_phoneme_timer;        // phoneme timer
//const unsigned char *               m_rom;                  // pointer to our ROM

	// inputs
unsigned char                       m_inflection;           // 2-bit inflection value
unsigned char                       m_phoneme;              // 6-bit phoneme value

	// outputs
//devcb_resolved_write_line   m_request_func;         // request callback
unsigned char                       m_request_state;        // request as seen to the outside world
unsigned char                       m_internal_request;     // request managed by stream timing

	// timing circuit
unsigned int                      m_master_clock_freq;    // frequency of the master clock
unsigned char                       m_master_clock;         // master clock
unsigned int                      m_counter_34;           // ripple counter @ 34
unsigned char                       m_latch_70;             // 4-bit latch @ 70
unsigned char                       m_latch_72;             // 4-bit latch @ 72
unsigned char                       m_beta1;                // beta1 clock state
unsigned char                       m_p2;                   // P2 clock state
unsigned char                       m_p1;                   // P1 clock state
unsigned char                       m_phi2;                 // phi2 clock state
unsigned char                       m_phi1;                 // phi1 clock state
unsigned char                       m_phi2_20;              // alternate phi2 clock state (20kHz)
unsigned char                       m_phi1_20;              // alternate phi1 clock state (20kHz)
unsigned int                      m_subphoneme_period;    // period of the subphoneme timer
unsigned int                      m_subphoneme_count;     // number of ticks executed already
unsigned char                       m_clock_88;             // subphoneme clock output @ 88
unsigned char                       m_latch_42;             // D flip-flop @ 42
unsigned char                       m_counter_84;           // 4-bit phoneme counter @ 84
unsigned char                       m_latch_92;             // 2-bit latch @ 92

// low parameter clocking
unsigned char                       m_srff_132;             // S/R flip-flop @ 132
unsigned char                       m_srff_114;             // S/R flip-flop @ 114
unsigned char                       m_srff_112;             // S/R flip-flop @ 112
unsigned char                       m_srff_142;             // S/R flip-flop @ 142
unsigned char                       m_latch_80;             // phoneme timing latch @ 80

// glottal circuit
unsigned char                       m_counter_220;          // 4-bit counter @ 220
unsigned char                       m_counter_222;          // 4-bit counter @ 222
unsigned char                       m_counter_224;          // 4-bit counter @ 224
unsigned char                       m_counter_234;          // 4-bit counter @ 234
unsigned char                       m_counter_236;          // 4-bit counter @ 236
unsigned char                       m_fgate;                // FGATE signal
unsigned char                       m_glottal_sync;         // Glottal Sync signal

// transition circuit
unsigned char                       m_0625_clock;           // state of 0.625kHz clock
unsigned char                       m_counter_46;           // 4-bit counter in block @ 46
unsigned char                       m_latch_46;             // 4-bit latch in block @ 46
unsigned char                       m_ram[8];               // RAM to hold parameters
unsigned char                       m_latch_168;            // 4-bit latch @ 168
unsigned char                       m_latch_170;            // 4-bit latch @ 170
unsigned char                       m_f1;                   // latched 4-bit F1 value
unsigned char                       m_f2;                   // latched 5-bit F2 value
unsigned char                       m_fc;                   // latched 4-bit FC value
unsigned char                       m_f3;                   // latched 4-bit F3 value
unsigned char                       m_f2q;                  // latched 4-bit F2Q value
unsigned char                       m_va;                   // latched 4-bit VA value
unsigned char                       m_fa;                   // latched 4-bit FA value

// noise generator circuit
unsigned char                       m_noise_clock;          // clock input to noise generator
unsigned int                      m_shift_252;            // shift register @ 252
unsigned char                       m_counter_250;          // 4-bit counter @ 250

// stages outputs history
double                      m_ni_hist[4];
double                      m_no_hist[4];
double                      m_va_hist[4];
double                      m_s1_hist[4];
double                      m_s2g_hist[4];
double                      m_s2ni_hist[4];
double                      m_s2n_hist[4];
double                      m_s2_hist[4];
double                      m_s3_hist[4];
double                      m_s4i_hist[4];
double                      m_s4_hist[4];

// static tables
//static const char *const s_phoneme_table[64];
//static const double s_glottal_wave[16];



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
//extern const device_type VOTRAX_SC01;


#endif /* __VOTRAX_H__ */
