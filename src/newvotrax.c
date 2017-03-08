// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// modified for WORM by Martin Howse

/***************************************************************************

    votrax.c

    Votrax SC01A simulation

***************************************************************************/

// gcc newvotrax.c -lm -ovot -std=c99 -DLAP

/*
  tp3 stb i1 i2 tp2
  1   1   o  o  white noise
  1   0   -  1  phone timing clock
  1   0   1  0  closure tick
  1   0   0  0  sram write pulse
  0   -   -  -  sram write pulse

i1.o = glottal impulse
i2.o = white noise

tp1 = phi clock (tied to f2q rom access)
*/
#ifdef LAP
#include "votrax.h"
#include <math.h>
#include <stdio.h>
#include <memory.h>
#include <linux/types.h>
#define M_PI 3.141592f
float _selx, _sely, _selz;
float exy[64];
u8 TTS=1;
FILE* fo;
#else
#include "audio.h"
#include "english2phoneme/TTS.h"
#include <stdio.h>
#include <ctype.h>
#include "votrax.h"
#include "resources.h"
extern float exy[64];
extern float _selx, _sely, _selz;
extern u8 TTS;
#endif
#include "vocab_votrax.h"

extern char TTSinarray[17];
static u8 TTSoutarray[256];
static u8 TTSindex=0;
static u8 TTSlength=0;

//this is from sc01a.bin:

const u8 m_rom[512]          __attribute__ ((section (".flash")))  ={0xA4, 0x50, 0xA0, 0xF0, 0xE0, 0x0, 0x0, 0x3, 0xA4, 0x50, 0xA0, 0x0, 0x23, 0xA, 0x0, 0x3E, 0xA4, 0x58, 0xA0, 0x30, 0xF0, 0x0, 0x0, 0x3F, 0xA3, 0x80, 0x69, 0xB0, 0xC1, 0xC, 0x0, 0x3D, 0x26, 0xD3, 0x49, 0x90, 0xA1, 0x9, 0x0, 0x3C, 0x27, 0x81, 0x68, 0x94, 0x21, 0xA, 0x0, 0x3B, 0x82, 0xC3, 0x48, 0x24, 0xA1, 0x8, 0x0, 0x3A, 0xA4, 0x0, 0x38, 0x18, 0x68, 0x1, 0x0, 0x39, 0x20, 0x52, 0xE1, 0x88, 0x63, 0xA, 0x0, 0x38, 0x22, 0xC1, 0xE8, 0x90, 0x61, 0x4, 0x0, 0x37, 0xA2, 0x83, 0x60, 0x10, 0x66, 0x3, 0x0, 0x36, 0xA2, 0xC1, 0xE8, 0x80, 0xA1, 0x9, 0x0, 0x35, 0xA2, 0xC1, 0xE8, 0x34, 0x61, 0xA, 0x0, 0x34, 0xA3, 0x81, 0x89, 0xB4, 0x21, 0xA, 0x0, 0x33, 0xA3, 0x81, 0x89, 0xE4, 0xA1, 0x7, 0x0, 0x32, 0xA3, 0x81, 0x89, 0x54, 0x63, 0x1, 0x0, 0x31, 0xA3, 0x80, 0x69, 0x60, 0x61, 0x4, 0x0, 0x30, 0xA7, 0x80, 0xE8, 0x74, 0xA0, 0x7, 0x0, 0x2F, 0xA7, 0x80, 0xE8, 0x74, 0x20, 0xA, 0x0, 0x2E, 0x22, 0xC1, 0x60, 0x14, 0x66, 0xA, 0x0, 0x2D, 0x26, 0xD3, 0x49, 0x70, 0x20, 0xA, 0x0, 0x2C, 0x82, 0x43, 0x8, 0x54, 0x63, 0x4, 0x0, 0x2B, 0xE0, 0x32, 0x11, 0xE8, 0x72, 0x1, 0x0, 0x2A, 0x26, 0x53, 0x1, 0x64, 0xA1, 0x7, 0x0, 0x29, 0x22, 0xC1, 0xE8, 0x80, 0x21, 0xA, 0x0, 0x28, 0xA6, 0x91, 0x61, 0x80, 0x21, 0xA, 0x0, 0x27, 0xA2, 0xC1, 0xE8, 0x84, 0x21, 0xA, 0x0, 0x26, 0xA8, 0x24, 0x13, 0x63, 0xB2, 0x7, 0x0, 0x25, 0xA3, 0x40, 0xE9, 0x84, 0xC1, 0xC, 0x0, 0x24, 0xA3, 0x81, 0x89, 0x54, 0xE3, 0x0, 0x0, 0x23, 0x26, 0x12, 0xA0, 0x64, 0x61, 0xA, 0x0, 0x22, 0x26, 0xD3, 0x69, 0x70, 0x61, 0x5, 0x0, 0x21, 0xA6, 0xC1, 0xC9, 0x84, 0x21, 0xA, 0x0, 0x20, 0xE0, 0x32, 0x91, 0x48, 0x68, 0x4, 0x0, 0x1F, 0x26, 0x91, 0xE8, 0x0, 0x7C, 0xB, 0x0, 0x1E, 0xA8, 0x2C, 0x83, 0x65, 0xA2, 0x7, 0x0, 0x1D, 0x26, 0xC1, 0x41, 0xE0, 0x73, 0x1, 0x0, 0x1C, 0xAC, 0x4, 0x22, 0xFD, 0x62, 0x1, 0x0, 0x1B, 0x2C, 0x34, 0x7B, 0xDB, 0xE8, 0x0, 0x0, 0x1A, 0x2C, 0x64, 0x23, 0x11, 0x72, 0xA, 0x0, 0x19, 0xA2, 0xD0, 0x9, 0xF4, 0xA1, 0x7, 0x0, 0x18, 0x23, 0x81, 0x49, 0x20, 0x21, 0xA, 0x0, 0x17, 0x23, 0x81, 0x49, 0x30, 0xA1, 0x7, 0x0, 0x16, 0xA3, 0x40, 0xE9, 0x84, 0xA1, 0x8, 0x0, 0x15, 0x36, 0x4B, 0x8, 0xD4, 0xA0, 0x9, 0x0, 0x14, 0xA3, 0x80, 0x69, 0x70, 0xA0, 0x8, 0x0, 0x13, 0x60, 0x58, 0xD1, 0x9C, 0x63, 0x1, 0x0, 0x12, 0x6C, 0x54, 0x8B, 0xFB, 0xA2, 0x9, 0x0, 0x11, 0x6C, 0x54, 0x8B, 0xFB, 0x63, 0x1, 0x0, 0x10, 0x28, 0x64, 0xD3, 0xF7, 0x63, 0x1, 0x0, 0xF, 0x22, 0x91, 0xE1, 0x90, 0x73, 0x1, 0x0, 0xE, 0x36, 0x19, 0x24, 0xE6, 0x61, 0xA, 0x0, 0xD, 0x32, 0x88, 0xA5, 0x66, 0xA3, 0x7, 0x0, 0xC, 0xA6, 0x91, 0x61, 0x90, 0xA1, 0x9, 0x0, 0xB, 0xA6, 0x91, 0x61, 0x90, 0x61, 0xA, 0x0, 0xA, 0xA6, 0x91, 0x61, 0x80, 0x61, 0xB, 0x0, 0x9, 0xA3, 0x40, 0xE9, 0xC4, 0x61, 0x1, 0x0, 0x8, 0x6C, 0x54, 0xCB, 0xF3, 0x63, 0x4, 0x0, 0x7, 0xA6, 0xC1, 0xC9, 0x34, 0xA1, 0x7, 0x0, 0x6, 0xA6, 0xC1, 0xC9, 0x64, 0x61, 0x1, 0x0, 0x5, 0xE8, 0x16, 0x3, 0x61, 0xFB, 0x0, 0x0, 0x4, 0x27, 0x81, 0x68, 0xC4, 0xA1, 0x9, 0x0, 0x2, 0x27, 0x81, 0x68, 0xD4, 0x61, 0x1, 0x0, 0x1, 0x27, 0x81, 0x68, 0x74, 0x61, 0x3, 0x0, 0x0};


// This waveform is built using a series of transistors as a resistor
// ladder.  There is first a transistor to ground, then a series of
// seven transistors one quarter the size of the first one, then it
// finishes by an active resistor to +9V.
//
// The terminal of the transistor to ground is used as a middle value.
// Index 0 is at that value. Index 1 is at 0V.  Index 2 to 8 start at
// just after the resistor down the latter.  Indices 9+ are the middle
// value again.
//
// For simplicity, we rescale the values to get the middle at 0 and
// the top at 1.  The final wave is very similar to the patent
// drawing.

const float s_glottal_wave[9] =
{
	0.0f,
	-4.0f/7.0f,
	7.0f/7.0f,
	6.0f/7.0f,
	5.0f/7.0f,
	4.0f/7.0f,
	3.0f/7.0f,
	2.0f/7.0f,
	1.0f/7.0f
};


/*votrax_sc01_device::votrax_sc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VOTRAX_SC01, "Votrax SC-01", tag, owner, clock, "votrax", __FILE__),
	  device_sound_interface(mconfig, *this),
	  m_stream(nullptr),
	  m_rom(*this, "internal"),
	  m_ar_cb(*this)
{
}
*/

void writer(int data)
{
	u8 prev = m_phone;

	// only 6 bits matter
	m_phone = data & 0x3f;
	m_ar_state = 0; // was CLEAR_LINE

	// Schedule a commit/ar reset at roughly 0.1ms in the future (one
	// phi1 transition followed by the rom extra state in practice),
	// but only if there isn't already one on the fly.  It will
	// override an end-of-phone timeout if there's one pending, but
	// that's not a problem since stb does that anyway.
	//	if(m_timer->expire().is_never() || m_timer->param() != T_COMMIT_PHONE)
	//	m_timer->adjust(attotime::from_ticks(72, m_mainclock), T_COMMIT_PHONE);
}


//-------------------------------------------------
//  inflection_w - handle a write to the
//  inflection bits
//-------------------------------------------------

void inflection_w(int data)
{
	// only 2 bits matter
	data &= 3;
	if(m_inflection == data)
		return;

	//	m_stream->update();
	m_inflection = data;
}


//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_start - handle device startup
//-------------------------------------------------

void device_start()
{
	// initialize internal state
  m_mainclock = 720000; // TODO as we need m_mainclock - 	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000)

  m_sclock = m_mainclock / 18.0f; // so 40000
  m_cclock = m_mainclock / 36.0f; // so 20000
}

//-------------------------------------------------
//  device_reset - handle device reset
//-------------------------------------------------

void device_reset()
{
	// Technically, there's no reset in this chip, and initial state
	// is random.  Still, it's a good idea to start it with something
	// sane.

	m_phone = 0x3f;
	m_inflection = 0;
	//	m_ar_state = 1; //ASSERT_LINE
	//	m_ar_cb(m_ar_state);

	m_sample_count = 0;

	// Initialize the m_rom* values
	phone_commit();

	// Clear the interpolation sram
	m_cur_fa = m_cur_fc = m_cur_va = m_cur_fa_orig = m_cur_fc_orig = m_cur_va_orig = 0;
	m_cur_f1 = m_cur_f2 = m_cur_f2q = m_cur_f3 = m_cur_f1_orig = m_cur_f2_orig = m_cur_f2q_orig = m_cur_f3_orig = 0;

	// Initialize the m_filt* values and the filter coefficients
	filters_commit(true);

	// Clear the rest of the internal digital state
	m_pitch = 0;
	m_closure = 0;
	m_update_counter = 0;
	m_cur_closure = true;
	m_noise = 0;
	m_cur_noise = false;

	// Clear the analog level histories
	memset(m_voice_1, 0, sizeof(m_voice_1));
	memset(m_voice_2, 0, sizeof(m_voice_2));
	memset(m_voice_3, 0, sizeof(m_voice_3));

	memset(m_noise_1, 0, sizeof(m_noise_1));
	memset(m_noise_2, 0, sizeof(m_noise_2));
	memset(m_noise_3, 0, sizeof(m_noise_3));
	memset(m_noise_4, 0, sizeof(m_noise_4));

	memset(m_vn_1, 0, sizeof(m_vn_1));
	memset(m_vn_2, 0, sizeof(m_vn_2));
	memset(m_vn_3, 0, sizeof(m_vn_3));
	memset(m_vn_4, 0, sizeof(m_vn_4));
	memset(m_vn_5, 0, sizeof(m_vn_5));



	memset(m_vn_6, 0, sizeof(m_vn_6));
}

void phone_commit()
{
	// Only these two counters are reset on phone change, the rest is
	// free-running.
	m_phonetick = 0;
	m_ticks = 0;

	// In the real chip, the rom is re-read all the time.  Since it's
	// internal and immutable, no point in not caching it though.
	for(int i=0; i<512; i+=8) {// added +8 
	  //	  u64 val = *(u64 *)(m_rom+i);// was reinterpet_cast
	  u64 val = *(u64 *)(m_rom+i);// was reinterpet_cast
	  if(m_phone == ((val >> 56) & 0x3f)) {// matches but????
			m_rom_f1  = BITSWAP4(val,  0,  7, 14, 21);
#ifdef LAP
			printf("ROMF1%d\n",m_rom_f1); // this works
#endif
			m_rom_va  = BITSWAP4(val,  1,  8, 15, 22);
			m_rom_f2  = BITSWAP4(val,  2,  9, 16, 23);
			m_rom_fc  = BITSWAP4(val,  3, 10, 17, 24);
			m_rom_f2q = BITSWAP4(val,  4, 11, 18, 25);
			m_rom_f3  = BITSWAP4(val,  5, 12, 19, 26);
			m_rom_fa  = BITSWAP4(val,  6, 13, 20, 27);
			// These two values have their bit orders inverted
			// compared to everything else due to a bug in the
			// prototype (miswiring of the comparator with the ticks
			// count) they compensated in the rom.

			m_rom_cld = BITSWAP4(val, 34, 32, 30, 28);
			m_rom_vd  = BITSWAP4(val, 35, 33, 31, 29);

			m_rom_closure  = BITSWAP1(val, 36);
			m_rom_duration = BITSWAP7(~val, 37, 38, 39, 40, 41, 42, 43);

			// Hard-wired on the die, not an actual part of the rom.
			m_rom_pause = (m_phone == 0x03) || (m_phone == 0x3e);
#ifdef LAP
			//			printf("ROMF1%d\n",m_rom_f1); // this works
	printf("commit fa=%x va=%x fc=%x f1=%x f2=%x f2q=%x f3=%x dur=%02x cld=%x vd=%d cl=%d pause=%d\n", m_rom_fa, m_rom_va, m_rom_fc, m_rom_f1, m_rom_f2, m_rom_f2q, m_rom_f3, m_rom_duration, m_rom_cld, m_rom_vd, m_rom_closure, m_rom_pause);
#endif
			// That does not happen in the sc01(a) rom, but let's
			// cover our behind.
			if(m_rom_cld == 0)
				m_cur_closure = m_rom_closure;

			return;
		}
	}
}


u8 interpolate(u8 reg, u8 target) // does nothing as it was
{
	// One step of interpolation, adds one eight of the distance
	// between the current value and the target.
	reg = reg - (reg >> 3) + (target << 1);
	return reg;
}

#ifndef LAP
void chip_update_bend()
{
	// Phone tick counter update.  Stopped when ticks reach 16.
	// Technically the counter keeps updating, but the comparator is
	// disabled.
	if(m_ticks != 0x10) {
	  //	  printf("MTICKS: %d %d %d\n",m_ticks, m_phonetick, m_rom_cld);

	  m_phonetick++;
		// Comparator is with duration << 2, but there's a one-tick
		// delay in the path.
	  if(m_phonetick == ((m_rom_duration << 2) | 1)) {
			m_phonetick = 0;
			m_ticks++;
			if(m_ticks == m_rom_cld)
				m_cur_closure = m_rom_closure;
				}
		}

	// The two update timing counters.  One divides by 16, the other
	// by 48, and they're phased so that the 208Hz counter ticks
	// exactly between two 625Hz ticks.
	m_update_counter++;
	if(m_update_counter == 0x30)
		m_update_counter = 0;

	bool tick_625 = !(m_update_counter & 0xf);
	bool tick_208 = m_update_counter == 0x28;

	// Formant update.  Die bug there: fc should be updated, not va.
	// The formants are frozen on a pause phone unless both voice and
	// noise volumes are zero.
	if(tick_208 && (!m_rom_pause || !(m_filt_fa || m_filt_va))) {
		//      interpolate(m_cur_va,  m_rom_va);
	  // what ranges these should be?
	  // and which should be inverted (no inversion in audio.c)
	  m_cur_fc_orig=interpolate(m_cur_fc_orig,  m_rom_fc);
	  m_cur_f1_orig=interpolate(m_cur_f1_orig,  m_rom_f1);
	  m_cur_f2_orig=interpolate(m_cur_f2_orig,  m_rom_f2);
	  m_cur_f2q_orig=interpolate(m_cur_f2q_orig, m_rom_f2q);
	  m_cur_f3_orig=interpolate(m_cur_f3_orig,  m_rom_f3);
	  m_cur_f1=m_cur_f1_orig+(128-(exy[2]*256.0f)); // TESTING!
	  m_cur_f2=m_cur_f2_orig+(64-(exy[3]*128.0f)); // TESTING!
	  m_cur_f3=m_cur_f3_orig+(64-(exy[4]*128.0f)); // TESTING!
	  m_cur_f2q=m_cur_f2q_orig+(64-(exy[5]*128.0f)); // TESTING!
	  m_cur_fc=m_cur_fc_orig+(64-(exy[6]*128.0f)); // TESTING!
	}

	// Non-formant update. Same bug there, va should be updated, not fc.
	if(tick_625) {
	  if(m_ticks >= m_rom_vd){
		  m_cur_fa_orig=interpolate(m_cur_fa_orig, m_rom_fa);
		  m_cur_fa=m_cur_fa_orig+(64-(exy[7]*128.0f)); // TESTING!
	  }
		if(m_ticks >= m_rom_cld){
			//          interpolate(m_cur_fc, m_rom_fc);
				  
		  m_cur_va_orig=interpolate(m_cur_va_orig, m_rom_va);
		  m_cur_va=m_cur_va_orig+(64-(exy[1]*128.0f)); // TESTING!

		}
	}

	// Closure counter, reset every other tick in theory when not
	// active (on the extra rom cycle).
	//
	// The closure level is immediatly used in the analog path,
	// there's no pitch synchronization.

	if(!m_cur_closure && (m_filt_fa || m_filt_va))
		m_closure = 0;
	else if(m_closure != 7 << 2)
		m_closure ++;

	// Pitch counter.  Equality comparison, so it's possible to make
	// it miss by manipulating the inflection inputs, but it'll wrap.
	// There's a delay, hence the +1.
	m_pitch = (m_pitch + 1) & 0x7f;
	//	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ (m_filt_f1+((int)((1.0f-exy[0])*64.0f)-8)) + 1)) m_pitch = 0;
	u8 val=exy[0]*130.0f;
	MAXED(val,127);
	val=127-val;

	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ ((int)(m_filt_f1*logpitch[val])))) m_pitch = 0;

	// Filters are updated in index 1 of the pitch wave, which does
	// indeed mean four times in a row.
	if((m_pitch >> 2) == 1){
		filters_commit(false);
	}
	// Noise shift register.  15 bits, with a nxor on the last two
	// bits for the loop.
	bool inp = (1||m_filt_fa) && m_cur_noise && (m_noise != 0x7fff);
	m_noise = ((m_noise << 1) & 0x7ffe) | inp;
	m_cur_noise = !(((m_noise >> 14) ^ (m_noise >> 13)) & 1);
}

void chip_update_raw()
{
  m_cur_f1=exy[1]*255.0f;
  m_cur_f2=exy[2]*255.0f;
  m_cur_f3=exy[3]*255.0f;
  m_cur_f2q=exy[4]*255.0f;
  m_cur_fc=exy[5]*255.0f;
  m_cur_fa=exy[6]*255.0f;
  m_cur_va=exy[0]*255.0f;

  m_pitch = (m_pitch + 1) & 0x7f;
  //  if (m_pitch == (0x7f ^  ((int)((1.0f-_selz)*64.0f)) + 1)) m_pitch = 0;

	u8 val=_selz*130.0f;
	MAXED(val,127);
	val=127-val;
	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ ((int)(m_filt_f1*logpitch[val])))) m_pitch = 0; // maintain as ==
		


	// Filters are updated in index 1 of the pitch wave, which does
	// indeed mean four times in a row.
	if((m_pitch >> 2) == 1){
		filters_commit(false);
	}
	// Noise shift register.  15 bits, with a nxor on the last two
	// bits for the loop.
	bool inp = (1||m_filt_fa) && m_cur_noise && (m_noise != 0x7fff);
	m_noise = ((m_noise << 1) & 0x7ffe) | inp;
	m_cur_noise = !(((m_noise >> 14) ^ (m_noise >> 13)) & 1);
}
#endif

void chip_update()
{
	// Phone tick counter update.  Stopped when ticks reach 16.
	// Technically the counter keeps updating, but the comparator is
	// disabled.
	if(m_ticks != 0x10) {
	  //	  printf("MTICKS: %d %d %d\n",m_ticks, m_phonetick, m_rom_cld);

	  m_phonetick++;
		// Comparator is with duration << 2, but there's a one-tick
		// delay in the path.
	  if(m_phonetick == ((m_rom_duration << 2) | 1)) {
			m_phonetick = 0;
			m_ticks++;
			if(m_ticks == m_rom_cld)
				m_cur_closure = m_rom_closure;
				}
		}

	// The two update timing counters.  One divides by 16, the other
	// by 48, and they're phased so that the 208Hz counter ticks
	// exactly between two 625Hz ticks.
	m_update_counter++;
	if(m_update_counter == 0x30)
		m_update_counter = 0;

	bool tick_625 = !(m_update_counter & 0xf);
	bool tick_208 = m_update_counter == 0x28;

	// Formant update.  Die bug there: fc should be updated, not va.
	// The formants are frozen on a pause phone unless both voice and
	// noise volumes are zero.
	if(tick_208 && (!m_rom_pause || !(m_filt_fa || m_filt_va))) {
		//      interpolate(m_cur_va,  m_rom_va);
	  m_cur_fc=interpolate(m_cur_fc,  m_rom_fc);
	  m_cur_f1=interpolate(m_cur_f1,  m_rom_f1);
	  m_cur_f2=interpolate(m_cur_f2,  m_rom_f2);
	  m_cur_f2q=interpolate(m_cur_f2q, m_rom_f2q);
	  m_cur_f3=interpolate(m_cur_f3,  m_rom_f3);
	}

	// Non-formant update. Same bug there, va should be updated, not fc.
	if(tick_625) {
		if(m_ticks >= m_rom_vd)
			m_cur_fa=interpolate(m_cur_fa, m_rom_fa);
		if(m_ticks >= m_rom_cld){
			//          interpolate(m_cur_fc, m_rom_fc);
				  
		  m_cur_va=interpolate(m_cur_va, m_rom_va);
		}
	}

	// Closure counter, reset every other tick in theory when not
	// active (on the extra rom cycle).
	//
	// The closure level is immediatly used in the analog path,
	// there's no pitch synchronization.

	if(!m_cur_closure && (m_filt_fa || m_filt_va))
		m_closure = 0;
	else if(m_closure != 7 << 2)
		m_closure ++;

	// Pitch counter.  Equality comparison, so it's possible to make
	// it miss by manipulating the inflection inputs, but it'll wrap.
	// There's a delay, hence the +1.
	m_pitch = (m_pitch + 1) & 0x7f;

	// tuning this DONE - TODO make exponential vot_pitch[]
	//	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ (m_filt_f1+((int)((1.0f-_selx)*64.0f)-8)) + 1)) m_pitch = 0; // maintain as ==
#ifdef LAP
	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ (m_filt_f1))) m_pitch = 0; // maintain as ==
#else
	u8 val=_selx*130.0f;
	MAXED(val,127);
	val=127-val;
		if(m_pitch == (0x7f ^ (m_inflection << 4) ^ ((int)(m_filt_f1*logpitch[val])))) m_pitch = 0; // maintain as ==
	//	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ (m_filt_f1))) m_pitch = 0; // maintain as ==
#endif
	// Filters are updated in index 1 of the pitch wave, which does
	// indeed mean four times in a row.
	if((m_pitch >> 2) == 1){
	  //	  printf("update");
		filters_commit(false);
	}
	// Noise shift register.  15 bits, with a nxor on the last two
	// bits for the loop.
	bool inp = (1||m_filt_fa) && m_cur_noise && (m_noise != 0x7fff);
	m_noise = ((m_noise << 1) & 0x7ffe) | inp;
	m_cur_noise = !(((m_noise >> 14) ^ (m_noise >> 13)) & 1);

}

#ifndef LAP
void chip_updateTTS()
{
	// Phone tick counter update.  Stopped when ticks reach 16.
	// Technically the counter keeps updating, but the comparator is
	// disabled.
	if(m_ticks != 0x10) {
	  //	  printf("MTICKS: %d %d %d\n",m_ticks, m_phonetick, m_rom_cld);

	  m_phonetick++;
		// Comparator is with duration << 2, but there's a one-tick
		// delay in the path.
	  if(m_phonetick == ((m_rom_duration << 2) | 1)) {
			m_phonetick = 0;
			m_ticks++;
			if(m_ticks == m_rom_cld)
				m_cur_closure = m_rom_closure;
				}
		}

	// The two update timing counters.  One divides by 16, the other
	// by 48, and they're phased so that the 208Hz counter ticks
	// exactly between two 625Hz ticks.
	m_update_counter++;
	if(m_update_counter == 0x30)
		m_update_counter = 0;

	bool tick_625 = !(m_update_counter & 0xf);
	bool tick_208 = m_update_counter == 0x28;

	// Formant update.  Die bug there: fc should be updated, not va.
	// The formants are frozen on a pause phone unless both voice and
	// noise volumes are zero.
	if(tick_208 && (!m_rom_pause || !(m_filt_fa || m_filt_va))) {
		//      interpolate(m_cur_va,  m_rom_va);
	  m_cur_fc=interpolate(m_cur_fc,  m_rom_fc);
	  m_cur_f1=interpolate(m_cur_f1,  m_rom_f1);
	  m_cur_f2=interpolate(m_cur_f2,  m_rom_f2);
	  m_cur_f2q=interpolate(m_cur_f2q, m_rom_f2q);
	  m_cur_f3=interpolate(m_cur_f3,  m_rom_f3);
	}

	// Non-formant update. Same bug there, va should be updated, not fc.
	if(tick_625) {
		if(m_ticks >= m_rom_vd)
			m_cur_fa=interpolate(m_cur_fa, m_rom_fa);
		if(m_ticks >= m_rom_cld){
			//          interpolate(m_cur_fc, m_rom_fc);
				  
		  m_cur_va=interpolate(m_cur_va, m_rom_va);
		}
	}

	// Closure counter, reset every other tick in theory when not
	// active (on the extra rom cycle).
	//
	// The closure level is immediatly used in the analog path,
	// there's no pitch synchronization.

	if(!m_cur_closure && (m_filt_fa || m_filt_va))
		m_closure = 0;
	else if(m_closure != 7 << 2)
		m_closure ++;

	// Pitch counter.  Equality comparison, so it's possible to make
	// it miss by manipulating the inflection inputs, but it'll wrap.
	// There's a delay, hence the +1.
	m_pitch = (m_pitch + 1) & 0x7f;
	//	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ (m_filt_f1+((int)((1.0f-_selx)*64.0f)-8)) + 1)) m_pitch = 0;
	u8 val=_selx*130.0f;
	MAXED(val,127);
	val=127-val;
	if(m_pitch == (0x7f ^ (m_inflection << 4) ^ ((int)(m_filt_f1*logpitch[val])))) m_pitch = 0; // maintain as ==


	// Filters are updated in index 1 of the pitch wave, which does
	// indeed mean four times in a row.
	if((m_pitch >> 2) == 1){
		filters_commit(false);
	}
	// Noise shift register.  15 bits, with a nxor on the last two
	// bits for the loop.
	bool inp = (1||m_filt_fa) && m_cur_noise && (m_noise != 0x7fff);
	m_noise = ((m_noise << 1) & 0x7ffe) | inp;
	m_cur_noise = !(((m_noise >> 14) ^ (m_noise >> 13)) & 1);
}
#endif


void filters_commit(bool force)
{
  u32 capsf1[4]={ 2546, 4973, 9861, 19724 };
  u32 capsf2q[4]={ 1390, 2965, 5875, 11297 };
  u32 capsf2[5]={ 833, 1663, 3164, 6327, 12654 };
  u32 capsf3[4]={ 2226, 4485, 9056, 18111 };
  //  printf("commit %d\n",m_cur_va);  
	m_filt_fa = m_cur_fa >> 4;
	m_filt_fc = m_cur_fc >> 4;
	m_filt_va = m_cur_va >> 4;

	if(force || m_filt_f1 != m_cur_f1 >> 4) {
	  //	  printf("filtercommit\n");
		m_filt_f1 = m_cur_f1 >> 4;

		build_standard_filter(m_f1_a, m_f1_b,
							  11247,
							  11797,
							  949,
							  52067,
				      2280 + bits_to_caps(m_filt_f1, capsf1, 4),
							  166272);
	}

	if(force || m_filt_f2 != m_cur_f2 >> 3 || m_filt_f2q != m_cur_f2q >> 4) {
		m_filt_f2 = m_cur_f2 >> 3;
		m_filt_f2q = m_cur_f2q >> 4;
		
		build_standard_filter(m_f2v_a, m_f2v_b,
							  24840,
							  29154,
				      829 + bits_to_caps(m_filt_f2q, capsf2q,4),
							  38180,
				      2352+ bits_to_caps(m_filt_f2, capsf2,5),
							  34270);

		build_injection_filter(m_f2n_a, m_f2n_b,
							   29154,
				       829+ bits_to_caps(m_filt_f2q, capsf2q,4),
							   38180,
				       2352+ bits_to_caps(m_filt_f2, capsf2,5),
							   34270);
	}

	if(force || m_filt_f3 != m_cur_f3 >> 4) {
		m_filt_f3 = m_cur_f3 >> 4;
		build_standard_filter(m_f3_a, m_f3_b,
							  0,
							  17594,
							  868,
							  18828,
				      8480+ bits_to_caps(m_filt_f3, capsf3, 4 ),
							  50019);
	}

	if(force) {
		build_standard_filter(m_f4_a, m_f4_b,
							  0,
							  28810,
							  1165,
							  21457,
							  8558,
							  7289);

		build_lowpass_filter(m_fx_a, m_fx_b,
							 1122,
							 23131);

		build_noise_shaper_filter(m_fn_a, m_fn_b,
								  15500,
								  14854,
								  8450,
								  9523,
								  14083);
	}
}

u32 analog_calc()
{
	// Voice-only path.
	// 1. Pick up the pitch wave

	float v = m_pitch >= (9 << 2) ? 0 : s_glottal_wave[m_pitch >> 2];

	// 2. Multiply by the initial amplifier.  It's linear on the die,
	// even if it's not in the patent.
	v = v * m_filt_va / 15.0f;
	shift_hist(v, m_voice_1, 4);

	// 3. Apply the f1 filter
	v = apply_filter(m_voice_1, m_voice_2, m_f1_a, m_f1_b, 4,4);
	shift_hist(v, m_voice_2, 4);

	// 4. Apply the f2 filter, voice half
	v = apply_filter(m_voice_2, m_voice_3, m_f2v_a, m_f2v_b,4,4);
	shift_hist(v, m_voice_3, 4);

	// Noise-only path
	// 5. Pick up the noise pitch.  Amplitude is linear.  Base
	// intensity should be checked w.r.t the voice.
	float n = 10000.0f * ((m_pitch & 0x40 ? m_cur_noise : false) ? 1 : -1);
	n = n * m_filt_fa / 15.0f;
	shift_hist(n, m_noise_1, 3);

	// 6. Apply the noise shaper
	n = apply_filter(m_noise_1, m_noise_2, m_fn_a, m_fn_b, 3,3);
	shift_hist(n, m_noise_2, 3);

	// 7. Scale with the f2 noise input
	float n2 = n * m_filt_fc / 15.0f;
	shift_hist(n2, m_noise_3, 2);

	// 8. Apply the f2 filter, noise half,
	n2 = apply_filter(m_noise_3, m_noise_4, m_f2n_a, m_f2n_b, 2,2);
	shift_hist(n2, m_noise_4, 2);

	// Mixed path
	// 9. Add the f2 voice and f2 noise outputs
	float vn = v + n2;
	shift_hist(vn, m_vn_1, 4);

	// 10. Apply the f3 filter
	vn = apply_filter(m_vn_1, m_vn_2, m_f3_a, m_f3_b,4,4);
	shift_hist(vn, m_vn_2, 4);

	// 11. Second noise insertion
	vn += n * (5.0f + (15^m_filt_fc))/20.0f;
	shift_hist(vn, m_vn_3, 4);

	// 12. Apply the f4 filter
	vn = apply_filter(m_vn_3, m_vn_4, m_f4_a, m_f4_b,4,4);
	shift_hist(vn, m_vn_4, 4);

	// 13. Apply the glottal closure amplitude, also linear
	vn = vn * (7 ^ (m_cur_closure >> 2)) / 7.0f;
	shift_hist(vn, m_vn_5, 2);

	// 13. Apply the final fixed filter
	vn = apply_filter(m_vn_5, m_vn_6, m_fx_a, m_fx_b,1,2); // fx_a is array of 1
	shift_hist(vn, m_vn_6, 2);
	//	printf("%d\n",vn);
	//	return vn*50000;
	return vn*50000.0f;
}

/*
  Playing with analog filters, or where all the magic filter formulas are coming from.

  First you start with an analog circuit, for instance this one:

  |                     +--[R2]--+
  |                     |        |
  |                     +--|C2|--+<V1     +--|C3|--+
  |                     |        |        |        |
  |  Vi   +--[R1]--+    |  |\    |        |  |\    |
  |  -----+        +----+--+-\   |        +--+-\   |
  |       +--|C1|--+       |  >--+--[Rx]--+  |  >--+----- Vo
  |                |     0-++/             0-++/   |
  |                |       |/    +--[R0]--+  |/    |
  |                |             |        |        |
  |                |             |    /|  |        |
  |                |             |   /-+--+--[R0]--+
  |                +--[R4]-------+--<  |
  |                            V2^   \++-0
  |                                   \|

  It happens to be what most of the filters in the sc01a look like.

  You need to determine the transfer function H(s) of the circuit, which is
  defined as the ratio Vo/Vi.  To do that, you use some properties:

  - The intensity through an element is equal to the voltage
    difference through the element divided by the impedence

  - The impedence of a resistance is equal to its resistance

  - The impedence of a capacitor is 1/(s*C) where C is its capacitance

  - The impedence of elements in series is the sum of the impedences

  - The impedence of elements in parallel is the inverse of the sum of
    the inverses

  - The sum of all intensities flowing into a node is 0 (there's no
    charge accumulation in a wire)

  - An operational amplifier in looped mode is an interesting beast:
    the intensity at its two inputs is always 0, and the voltage is
    forced identical between the inputs.  In our case, since the '+'
    inputs are all tied to ground, that means that the '-' inputs are at
    voltage 0, intensity 0.

  From here we can build some equations.  Noting:
  X1 = 1/(1/R1 + s*C1)
  X2 = 1/(1/R2 + s*C2)
  X3 = 1/(s*C3)

  Then computing the intensity flow at each '-' input we have:
  Vi/X1 + V2/R4 + V1/X2 = 0
  V2/R0 + Vo/R0 = 0
  V1/Rx + Vo/X3 = 0

  Wrangling the equations, one eventually gets:
  |                            1 + s * C1*R1
  | Vo/Vi = H(s) = (R4/R1) * -------------------------------------------
  |                            1 + s * C3*Rx*R4/R2 + s^2 * C2*C3*Rx*R4

  To check the mathematics between the 's' stuff, check "Laplace
  transform".  In short, it's a nice way of manipulating derivatives
  and integrals without having to manipulate derivatives and
  integrals.

  With that transfer function, we first can compute what happens to
  every frequency in the input signal.  You just compute H(2i*pi*f)
  where f is the frequency, which will give you a complex number
  representing the amplitude and phase effect.  To get the usual dB
  curves, compute 20*log10(abs(v))).

  Now, once you have an analog transfer function, you can build a
  digital filter from it using what is called the bilinear transform.

  In our case, we have an analog filter with the transfer function:
  |                 1 + k[0]*s
  |        H(s) = -------------------------
  |                 1 + k[1]*s + k[2]*s^2

  We can always reintroduce the global multipler later, and it's 1 in
  most of our cases anyway.

  The we pose:
  |                    z-1
  |        s(z) = zc * ---
  |                    z+1

  where zc = 2*pi*fr/tan(pi*fr/fs)
  with fs = sampling frequency
  and fr = most interesting frequency

  Then we rewrite H in function of negative integer powers of z.

  Noting m0 = zc*k[0], m1 = zc*k[1], m2=zc*zc*k[2],

  a little equation wrangling then gives:

  |                 (1+m0)    + (3+m0)   *z^-1 + (3-m0)   *z^-2 +    (1-m0)*z^-3
  |        H(z) = ----------------------------------------------------------------
  |                 (1+m1+m2) + (3+m1-m2)*z^-1 + (3-m1-m2)*z^-2 + (1-m1+m2)*z^-3

  That beast in the digital transfer function, of which you can
  extract response curves by posing z = exp(2*i*pi*f/fs).

  Note that the bilinear transform is an approximation, and H(z(f)) =
  H(s(f)) only at frequency fr.  And the shape of the filter will be
  better respected around fr.  If you look at the curves of the
  filters we're interested in, the frequency:
  fr = sqrt(abs(k[0]*k[1]-k[2]))/(2*pi*k[2])

  which is a (good) approximation of the filter peak position is a
  good choice.

  Note that terminology wise, the "standard" bilinear transform is
  with fr = fs/2, and using a different fr is called "pre-warping".

  So now we have a digital transfer function of the generic form:

  |                 a[0] + a[1]*z^-1 + a[2]*z^-2 + a[3]*z^-3
  |        H(z) = --------------------------------------------
  |                 b[0] + b[1]*z^-1 + b[2]*z^-2 + b[3]*z^-3

  The magic then is that the powers of z represent time in samples.
  Noting x the input stream and y the output stream, you have:
  H(z) = y(z)/x(z)

  or in other words:
  y*b[0]*z^0 + y*b[1]*z^-1 + y*b[2]*z^-2 + y*b[3]*z^-3 = x*a[0]*z^0 + x*a[1]*z^-1 + x*a[2]*z^-2 + x*a[3]*z^-3

  i.e.

  y*z^0 = (x*a[0]*z^0 + x*a[1]*z^-1 + x*a[2]*z^-2 + x*a[3]*z^-3 - y*b[1]*z^-1 - y*b[2]*z^-2 - y*b[3]*z^-3) / b[0]

  and powers of z being time in samples,

  y[0] = (x[0]*a[0] + x[-1]*a[1] + x[-2]*a[2] + x[-3]*a[3] - y[-1]*b[1] - y[-2]*b[2] - y[-3]*b[3]) / b[0]

  So you have a filter you can apply.  Note that this is why you want
  negative powers of z.  Positive powers would mean looking into the
  future (which is possible in some cases, in particular with x, and
  has some very interesting properties, but is not very useful in
  analog circuit simulation).

  Note that if you have multiple inputs, all this stuff is linear.
  Or, in other words, you just have to split it in multiple circuits
  with only one input connected each time and sum the results.  It
  will be correct.

  Also, since we're in practice in a dynamic system, for an amplifying
  filter (i.e. where things like r4/r1 is not 1), it's better to
  proceed in two steps:

  - amplify the input by the current value of the coefficient, and
    historize it
  - apply the now non-amplifying filter to the historized amplified
    input

  That way reduces the probability of the output bouncing all over the
  place.

  Except, we're not done yet.  Doing resistors precisely in an IC is
  very hard and/or expensive (you may have heard of "laser cut
  resistors" in DACs of the time).  Doing capacitors is easier, and
  their value is proportional to their surface.  So there are no
  resistors on the sc01 die (which is a lie, there are three, but not
  in the filter path.  They are used to scale the voltage in the pitch
  wave and to generate +5V from the +9V), but a magic thing called a
  switched capacitor.  Lookup patent 4,433,210 for details.  Using
  high frequency switching a capacitor can be turned into a resistor
  of value 1/(C*f) where f is the switching frequency (20Khz,
  main/36).  And the circuit is such that the absolute value of the
  capacitors is irrelevant, only their ratio is useful, which factors
  out the intrinsic capacity-per-surface-area of the IC which may be
  hard to keep stable from one die to another.  As a result all the
  capacitor values we use are actually surfaces in square micrometers.

  For the curious, it looks like the actual capacitance was around 25
  femtofarad per square micrometer.

*/

void build_standard_filter(float *a, float *b,
			   float c1t, // Unswitched cap, input, top
			   float c1b, // Switched cap, input, bottom
			   float c2t, // Unswitched cap, over first amp-op, top
			   float c2b, // Switched cap, over first amp-op, bottom
			   float c3,  // Cap between the two op-amps
			   float c4)  // Cap over second op-amp
{
	// First compute the three coefficients of H(s).  One can note
	// that there is as many capacitor values on both sides of the
	// division, which confirms that the capacity-per-surface-area
	// is not needed.
	float k0 = c1t / (m_cclock * c1b);
	float k1 = c4 * c2t / (m_cclock * c1b * c3);
	float k2 = c4 * c2b / (m_cclock * m_cclock * c1b * c3);

	// Estimate the filter cutoff frequency
	float fpeak = sqrtf(fabsf(k0*k1 - k2))/(2.0f*M_PI*k2);

	// Turn that into a warp multiplier
	float zc = 2.0f*M_PI*fpeak/tanf(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	float m0 = zc*k0;
	float m1 = zc*k1;
	float m2 = zc*zc*k2;


	
	a[0] = 1.0f+m0;
	a[1] = 3.0f+m0;
	a[2] = 3.0f-m0;
	a[3] = 1.0f-m0;
	b[0] = 1.0f+m1+m2;
	b[1] = 3.0f+m1-m2;
	b[2] = 3.0f-m1-m2;
	b[3] = 1.0f-m1+m2;

#ifdef LAP
	printf("BUILD  %f clock %f\n",a[0], m_cclock); // this works
#endif


}

/*
  Second filter type used once at the end, much simpler:

  |           +--[R1]--+
  |           |        |
  |           +--|C1|--+
  |           |        |
  |  Vi       |  |\    |
  |  ---[R0]--+--+-\   |
  |              |  >--+------ Vo
  |            0-++/
  |              |/


  Vi/R0 = Vo / (1/(1/R1 + s.C1)) = Vo (1/R1 + s.C1)
  H(s) = Vo/Vi = (R1/R0) * (1 / (1 + s.R1.C1))
*/

void build_lowpass_filter(float *a, float *b,
											  float c1t, // Unswitched cap, over amp-op, top
											  float c1b) // Switched cap, over amp-op, bottom
{
	// Compute the only coefficient we care about
	float k = c1b / (m_cclock * c1t);

	// Compute the filter cutoff frequency
	float fpeak = 1/(2*M_PI*k);

	// Turn that into a warp multiplier
	float zc = 2*M_PI*fpeak/tanf(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	float m = zc*k;

	a[0] = 1.0f;
	b[0] = 1.0f+m;
	b[1] = 1.0f-m;
}

/*
  Used to shape the white noise

         +-------------------------------------------------------------------+
         |                                                                   |
         +--|C1|--+---------|C3|----------+--|C4|--+                         |
         |        |      +        +       |        |                         |
   Vi    |  |\    |     (1)      (1)      |        |       +        +        |
   -|R0|-+--+-\   |      |        |       |  |\    |      (1)      (1)       |
            |  >--+--(2)-+--|C2|--+---(2)-+--+-\   |       |        |        |
          0-++/          |                   |  >--+--(2)--+--|C5|--+---(2)--+
            |/          Vo                 0-++/
                                             |/
   Equivalent:

         +------------------|R5|-------------------+
         |                                         |
         +--|C1|--+---------|C3|----------+--|C4|--+
         |        |                       |        |
   Vi    |  |\    |                       |        |
   -|R0|-+--+-\   |                       |  |\    |
            |  >--+---------|R2|----------+--+-\   |
          0-++/   |                          |  >--+
            |/   Vo                        0-++/
                                             |/

  We assume r0 = r2
*/

void build_noise_shaper_filter(float *a, float *b,
												   float c1,  // Cap over first amp-op
												   float c2t, // Unswitched cap between amp-ops, input, top
												   float c2b, // Switched cap between amp-ops, input, bottom
												   float c3,  // Cap over second amp-op
												   float c4)  // Switched cap after second amp-op
{
	// Coefficients of H(s) = k1*s / (1 + k2*s + k3*s^2)
	float k0 = c2t*c3*c2b/c4;
	float k1 = c2t*(m_cclock * c2b);
	float k2 = c1*c2t*c3/(m_cclock * c4);

	// Estimate the filter cutoff frequency
	float fpeak = sqrtf(1.0f/k2)/(2.0f*M_PI);

	// Turn that into a warp multiplier
	float zc = 2.0f*M_PI*fpeak/tanf(M_PI*fpeak / m_sclock);

	// Finally compute the result of the z-transform
	float m0 = zc*k0;
	float m1 = zc*k1;
	float m2 = zc*zc*k2;

	a[0] = m0;
	a[1] = 0.0f;
	a[2] = -m0;
	b[0] = 1.0f+m1+m2;
	b[1] = 2.0f-2.0f*m2;
	b[2] = 1.0f-m1+m2;
}

/*
  Noise injection in f2

  |                     +--[R2]--+        +--[R1]-------- Vi
  |                     |        |        |
  |                     +--|C2|--+<V1     +--|C3|--+
  |                     |        |        |        |
  |                     |  |\    |        |  |\    |
  |                +----+--+-\   |        +--+-\   |
  |                |       |  >--+--[Rx]--+  |  >--+----- Vo
  |                |     0-++/             0-++/   |
  |                |       |/    +--[R0]--+  |/    |
  |                |             |        |        |
  |                |             |    /|  |        |
  |                |             |   /-+--+--[R0]--+
  |                +--[R4]-------+--<  |
  |                            V2^   \++-0
  |                                   \|

  We drop r0/r1 out of the equation (it factorizes), and we rescale so
  that H(infinity)=1.
*/

void build_injection_filter(float *a, float *b,
												float c1b, // Switched cap, input, bottom
												float c2t, // Unswitched cap, over first amp-op, top
												float c2b, // Switched cap, over first amp-op, bottom
												float c3,  // Cap between the two op-amps
												float c4)  // Cap over second op-amp
{
	// First compute the three coefficients of H(s) = (k0 + k2*s)/(k1 - k2*s)
	float k0 = m_cclock * c2t;
	float k1 = m_cclock * (c1b * c3 / c2t - c2t);
	float k2 = c2b;

	// Don't pre-warp
	float zc = 2.0f*m_sclock;

	// Finally compute the result of the z-transform
	float m = zc*k2;

	a[0] = k0 + m;
	a[1] = k0 - m;
	b[0] = k1 - m;
	b[1] = k1 + m;

	// That ends up in a numerically unstable filter.  Neutralize it for now.
	a[0] = 1.0f;
	a[1] = 0.0f;
	b[0] = 1.0f;
	b[1] = 0.0f;
}


void generate_votrax_samples(int samples)
{
	for(int i=0; i<samples; i++) {
		m_sample_count++;
		if(m_sample_count & 1)
			chip_update();
#ifdef LAP	      
//				printf("%c",analog_calc()>>8);
		//		printf("%d\n",analog_calc());
		int s16 = analog_calc();
		//		printf("%d\n",s16);
		
		
		unsigned char c = (unsigned)s16 & 255;
		fwrite(&c, 1, 1, fo);
		c = ((unsigned)s16 / 256) & 255;
		fwrite(&c, 1, 1, fo);
#endif
	}
}

static unsigned int lenny;

void votrax_init(){
  device_start();
  device_reset();
  lenny=1000;
}


#ifndef LAP
////[[[[[[[[[[[[[[[[[[[ audio functions:

static int16_t sample_count=0;
//static float intervals[32]={1.0f, 2.0f, 4.0f, 8.0f, 16.0f, 32.0f, 64.0f, 128.0f, 128.0f}; // TODO: fix these

void votrax_newsay(){
  u8 sel=_selz*65.0f; 
  MAXED(sel,64);
  sel=64-sel;
  writer(sel); // what are we writing - is ROM index
  phone_commit();
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 
}

int16_t votrax_get_sample(){ 
  uint16_t sample; u8 x;
  m_sample_count++;
  if(m_sample_count & 1)
    chip_update();
  sample=analog_calc();
  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsay();
  }
  return sample;
}

int16_t votrax_get_sample_rawparam(){ 
  uint16_t sample; u8 x;
  m_sample_count++;
  if(m_sample_count & 1)
    chip_update_raw();
  sample=analog_calc();
  return sample;
}

/////

void votrax_newsay_bend(u8 reset){
  signed char tmp;
  u8 it;
  // do bend now for GORF:
  static u8 vocabindex=0, whichone=0;
  it=*(vocablist_gorf[whichone]+vocabindex);
  vocabindex++;
  if (it==255  || reset==1){
    vocabindex=0;
    whichone=_selz*117.0f; 
    MAXED(whichone,114);
    whichone=114-whichone;
    it=*(vocablist_gorf[whichone]+vocabindex);
  }   

  writer(it); 
  phone_commit();
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 
}

int16_t votrax_get_sample_bend(){ 
  uint16_t sample; u8 x;
  
  m_sample_count++;
  if(m_sample_count & 1){
    chip_update_bend();        
  }

  sample=analog_calc();
  // hit end and then newsay
  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsay_bend(0);
  }
  return sample;
}

/////

void votrax_newsaygorf(u8 reset){
     static u8 vocabindex=0, whichone=0;
     u8 it;
     it=*(vocablist_gorf[whichone]+vocabindex);
   vocabindex++;
   if (it==255 || reset==1){
     vocabindex=0;
     whichone=_selz*116.0f; 
     MAXED(whichone,114);
     whichone=114-whichone;
     it=*(vocablist_gorf[whichone]);
   }   

   writer(it); 
  phone_commit();
  inflection_w(it>>6); 
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 
  }
  
void votrax_newsaywow(u8 reset){
     static u8 vocabindex=0, whichone=0;
     u8 it;
     it=*(vocablist_wow[whichone]+vocabindex);
   vocabindex++;
   if (it==255 || reset==1){
     vocabindex=0;
     whichone=_selz*80.0f; 
     MAXED(whichone,78);
     whichone=78-whichone;
     it=*(vocablist_wow[whichone]);
   }   
   writer(it); 
  phone_commit();
  inflection_w(it>>6); // how many bits?
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 

  //m_timer->adjust(attotime::from_ticks(16*(m_rom_duration*4+1)*4*9+2, m_mainclock), T_END_OF_PHONE);
  //  m_sclock = m_mainclock / 18.0f * logpitch[val]; // so 40000 - doesn't do anything if we have both in sync - crashes filter
  //  m_cclock = m_mainclock / 36.0f * logpitch[val]; // so 20000 

}

void votrax_newsaywow_bendfilter(u8 reset){
     static u8 vocabindex=0, whichone=0;
     u8 it;
     it=*(vocablist_wow[whichone]+vocabindex);
   vocabindex++;
   if (it==255 || reset==1){
     vocabindex=0;
     whichone=_selz*80.0f; 
     MAXED(whichone,78);
     whichone=78-whichone;
     it=*(vocablist_wow[whichone]);
   }   
   writer(it); 
  phone_commit();
  inflection_w(it>>6); // how many bits?
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
p
  lenny=(16*(m_rom_duration*4+1)*4*9+2); 
  m_cclock = m_mainclock / 36.0f * logpitch[val]; 

}



int16_t votrax_get_samplegorf(){ 
  int16_t sample; u8 x;

  m_sample_count++;
  if(m_sample_count & 1)
    chip_update();
  sample=analog_calc();
  // hit end and then newsay
  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsaygorf(0);
  }
  return sample;
}

int16_t votrax_get_samplewow(){ 
  int16_t sample; u8 x;

  m_sample_count++;
  if(m_sample_count & 1)
    chip_update();
  sample=analog_calc();
  // hit end and then newsay
  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsaywow(0);
  }
  return sample;
}

void votrax_newsayTTS(){

  writer(TTSoutarray[TTSindex]); 
  phone_commit();
  inflection_w(TTSoutarray[TTSindex]>>6); // how many bits?
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 

  TTSindex++;
   if (TTSindex>=TTSlength) {
     TTSindex=0;
     TTSlength= text2speechforvotrax(16,TTSinarray,TTSoutarray);
   }
}

int16_t votrax_get_sampleTTS(){ 
  int16_t sample; u8 x;
  m_sample_count++;
  if(m_sample_count & 1)
    chip_updateTTS();
  sample=analog_calc();
  // hit end and then newsay
  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsayTTS();
  }
  return sample;
}

void votrax_retriggerTTS(){
  TTSlength= text2speechforvotrax(16,TTSinarray,TTSoutarray);
  writer(TTSoutarray[TTSindex]); 
  phone_commit();
  inflection_w(TTSoutarray[0]>>6); // how many bits?
  u8 val=_sely*130.0f;
  MAXED(val,127);
  val=127-val;
  lenny=(2*(m_rom_duration*4+1)*4*9+2)*logpitch[val]; 
  TTSindex=1;
}

#endif

#ifdef LAP
void main(void){

  
  fo = fopen("testnewvotrax.pcm", "wb");
  int x;
  
  // set up
  device_start();
  device_reset();
;
//  const unsigned char TTStest[]  = {24, 27, 0, 24, 38, 40, 62, 57, 0, 43, 62, 27, 36, 61, 38, 40, 62, 36, 43, 62, 41, 40, 40, 0, 62};

 const unsigned char TTStest[]  = {7, 12, 36, 43, 42, 39, 13, 62}; // MARTIN
 
  // try and say
  for (x=1;x<TTStest[0]+1;x++){ // for vocab [0] is length
    writer(TTStest[x]);
  phone_commit();
//	m_votrax->inflection_w(space, 0, data >> 6);
//  inflection_w(TTStest[x]>>6);
    // how to get duration:
  //m_timer->adjust(attotime::from_ticks(16*(m_rom_duration*4+1)*4*9+2, m_mainclock), T_END_OF_PHONE);

    ///    int lenny=PhonemeLengths[welcome[x]]*100;
    //    int lenny=m_rom_duration*144;
    int lenny=((16*(m_rom_duration*4+1)*4*9+2)/30); // what of sample-rate?
// this is not precise - say 1200 above = 16*41*4*9+2=24000 odd
    // sample rate? say 24000 as about right?
  generate_votrax_samples(lenny);
  }
  //  writer(1);
  //  phone_commit();
  //  generate_votrax_samples(lenny);
  //  }
  //  printf("rom_durxxxxxxxxxxzzzzzzzzzz %d\n",m_rom_duration);

}
#endif
