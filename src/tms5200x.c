/// stripped out of mame: LICENSE! for testing so far... use parse frame from other

// TABLES and defines

#include "audio.h"
#include "stdio.h"
#include "math.h"
#include "time.h"

extern float _selx, _sely, _selz;

typedef u8 UINT8;
//typedef char INT8;
typedef uint16_t UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32;
typedef int32_t INT32;

#include "tms5110r.inc"

// include all ROMS re-grouped TODO

#include "LPC/roms/vocab_2304.h"
#include "LPC/roms/vocab_2303.h"

// struct ROM_wordlist pointer, length of rom, extent, chipset pointer=   m_coeff = &T0280B_0281A_coeff; // this is for 5100! // TODO - sett/swap here

// list of which_coeff:

//** 5100-speak and spell
//** 5200- early echo II, disks of TRON????, TI99/4 
//** 5220- later echo II, BBC MICRO

// 5100: &T0280B_0281A_coeff
// 5200: &T0285_2501E_coeff
// 5220: &tms5220_coeff

typedef struct TMS_vocab__ {
  // pointer to const
  const uint8_t **wordlist;
  const struct tms5100_coeffs *m_coeff;
  uint16_t extent;
  float extentplus;
} TMS_vocab;



static const uint8_t* ptrAddr; static uint8_t ptrBit;
extern uint8_t byte_rev[256];

#define PERFECT_INTERPOLATION_HACK 0

static INT16 clip_analog(INT16 cliptemp);

/* *****optional defines***** */

/* Hacky improvements which don't match patent: */
/* Interpolation shift logic:
 * One of the following two lines should be used, and the other commented
 * The second line is more accurate mathematically but not accurate to the patent
 */
#define INTERP_SHIFT >> m_coeff->interp_coeff[m_IP]
//define INTERP_SHIFT / (1<<m_coeff->interp_coeff[m_IP])

/* Other hacks */
/* HACK?: if defined, outputs the low 4 bits of the lattice filter to the i/o
 * or clip logic, even though the real hardware doesn't do this, partially verified by decap */
#undef ALLOW_4_LSB

//#define ALLOW_4_LSB 1

/* *****configuration of chip connection stuff***** */
/* must be defined; if 0, output the waveform as if it was tapped on the speaker pin as usual, if 1, output the waveform as if it was tapped on the i/o pin (volume is much lower in the latter case) */
#define FORCE_DIGITAL 0

/* must be defined; if 1, normal speech (one A cycle, one B cycle per interpolation step); if 0; speak as if SPKSLOW was used (two A cycles, one B cycle per interpolation step) */
#define FORCE_SUBC_RELOAD 1


/* *****debugging defines***** */
#undef VERBOSE
// above is general, somewhat obsolete, catch all for debugs which don't fit elsewhere
#define DEBUG_DUMP_INPUT_DATA 1
// above dumps the data input to the tms52xx to stdout, useful for making logged data dumps for real hardware tests
#undef DEBUG_FIFO
// above debugs fifo stuff: writes, reads and flag updates
#undef DEBUG_PARSE_FRAME_DUMP
// above dumps each frame to stderr: be sure to select one of the options below if you define it!
#undef DEBUG_PARSE_FRAME_DUMP_BIN
// dumps each speech frame as binary
#undef DEBUG_PARSE_FRAME_DUMP_HEX
// dumps each speech frame as hex
#undef DEBUG_FRAME_ERRORS
// above dumps info if a frame ran out of data
#undef DEBUG_COMMAND_DUMP
// above dumps all non-speech-data command writes
#undef DEBUG_PIN_READS
// above spams the errorlog with i/o ready messages whenever the ready or irq pin is read
#undef DEBUG_GENERATION
// above dumps debug information related to the sample generation loop, i.e. whether interpolation is inhibited or not, and what the current and target values for each frame are.
#undef DEBUG_GENERATION_VERBOSE
// above dumps MUCH MORE debug information related to the sample generation loop, namely the excitation, energy, pitch, k*, and output values for EVERY SINGLE SAMPLE during a frame.
#undef DEBUG_LATTICE
// above dumps the lattice filter state data each sample.
#undef DEBUG_CLIP
// above dumps info to stderr whenever the analog clip hardware is (or would be) clipping the signal.
#undef DEBUG_IO_READY
// above debugs the io ready callback
#undef DEBUG_RS_WS
// above debugs the tms5220_data_r and data_w access methods which actually respect rs and ws

#define MAX_SAMPLE_CHUNK    512

/* Variants */

#define TMS5220_IS_5220C    (4)
#define TMS5220_IS_5200     (5)
#define TMS5220_IS_5220     (6)
#define TMS5220_IS_CD2501ECD (7)

#define TMS5220_IS_CD2501E  TMS5220_IS_5200

#define TMS5220_HAS_RATE_CONTROL ((m_variant == TMS5220_IS_5220C) || (m_variant == TMS5220_IS_CD2501ECD))
#define TMS5220_IS_52xx ((m_variant == TMS5220_IS_5220C) || (m_variant == TMS5220_IS_5200) || (m_variant == TMS5220_IS_5220) || (m_variant == TMS5220_IS_CD2501ECD))


static const UINT8 reload_table[4] = { 0, 2, 4, 6 }; //sample count reload for 5220c and cd2501ecd only; 5200 and 5220 always reload with 0; keep in mind this is loaded on IP=0 PC=12 subcycle=1 so it immediately will increment after one sample, effectively being 1,3,5,7 as in the comments above.

// vars

	// internal state

	/* coefficient tables */
//int m_variant=TMS5220_IS_5200;                /* Variant of the 5xxx - see tms5110r.h */

/// for 5100= ????

	int m_variant=0;

	/* coefficient tables */
const struct tms5100_coeffs *m_coeff; // this is fine with MAX_K (max number of coeffs)

	/* these contain global status bits */
	UINT8 m_speaking_now;     /* True only if actual speech is being generated right now. Is set when a speak vsm command happens OR when speak external happens and buffer low becomes nontrue; Is cleared when speech halts after the last stop frame or the last frame after talk status is otherwise cleared.*/
	UINT8 m_speak_external;   /* If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO. */
UINT8 m_talk_status;      /* If 1, TS status bit is 1, i.e. speak or speak external is in progress and we have not encountered a stop frame yet; */

	/* these contain global status bits */
	UINT8 m_previous_TALK_STATUS;      /* this is the OLD value of TALK_STATUS (i.e. previous value of m_SPEN|m_TALKD), needed for generating interrupts on a falling TALK_STATUS edge */
	UINT8 m_SPEN;             /* set on speak(or speak external and BL falling edge) command, cleared on stop command, reset command, or buffer out */
	UINT8 m_DDIS;             /* If 1, DDIS is 1, i.e. Speak External command in progress, writes go to FIFO. */
	UINT8 m_TALK;             /* set on SPEN & RESETL4(pc12->pc0 transition), cleared on stop command or reset command */
#define TALK_STATUS (m_SPEN|m_TALKD)
	UINT8 m_TALKD;            /* TALK(TCON) value, latched every RESETL4 */
	UINT8 m_buffer_low;       /* If 1, FIFO has less than 8 bytes in it */
	UINT8 m_buffer_empty;     /* If 1, FIFO is empty */
	UINT8 m_irq_pin;          /* state of the IRQ pin (output) */
	UINT8 m_ready_pin;        /* state of the READY pin (output) */

	/* these contain data describing the current and previous voice frames */
#define OLD_FRAME_SILENCE_FLAG m_OLDE // 1 if E=0, 0 otherwise.
#define OLD_FRAME_UNVOICED_FLAG m_OLDP // 1 if P=0 (unvoiced), 0 if voiced
	UINT8 m_OLDE;
	UINT8 m_OLDP;

#define NEW_FRAME_STOP_FLAG (m_new_frame_energy_idx == 0xF) // 1 if this is a stop (Energy = 0xF) frame
#define NEW_FRAME_SILENCE_FLAG (m_new_frame_energy_idx == 0) // ditto as above
#define NEW_FRAME_UNVOICED_FLAG (m_new_frame_pitch_idx == 0) // ditto as above
	UINT8 m_new_frame_energy_idx;
	UINT8 m_new_frame_pitch_idx;
	UINT8 m_new_frame_k_idx[10];

	INT16 m_target_energy;
	INT16 m_target_pitch;
	INT16 m_target_k[10];


	/* these are all used to contain the current state of the sound generation */
#ifndef PERFECT_INTERPOLATION_HACK
	INT16 m_current_energy;
	INT16 m_current_pitch;
	INT16 m_current_k[10];
#else
	UINT8 m_old_frame_energy_idx;
	UINT8 m_old_frame_pitch_idx;
	UINT8 m_old_frame_k_idx[10];
	UINT8 m_old_zpar;
	UINT8 m_old_uv_zpar;

	INT32 m_current_energy;
	INT32 m_current_pitch;
	INT32 m_current_k[10];
#endif

	UINT16 m_previous_energy; /* needed for lattice filter to match patent */

	UINT8 m_subcycle;         /* contains the current subcycle for a given PC: 0 is A' (only used on SPKSLOW mode on 51xx), 1 is A, 2 is B */
	UINT8 m_subc_reload;      /* contains 1 for normal speech, 0 when SPKSLOW is active */
	UINT8 m_PC;               /* current parameter counter (what param is being interpolated), ranges from 0 to 12 */
	/* NOTE: the interpolation period counts 1,2,3,4,5,6,7,0 for divide by 8,8,8,4,4,2,2,1 */
	UINT8 m_IP;               /* the current interpolation period */
	UINT8 m_inhibit;          /* If 1, interpolation is inhibited until the DIV1 period */
	UINT8 m_uv_zpar;          /* If 1, zero k5 thru k10 coefficients */
	UINT8 m_zpar;             /* If 1, zero ALL parameters. */
	UINT8 m_pitch_zero;       /* circuit 412; pitch is forced to zero under certain circumstances */
	UINT8 m_c_variant_rate;    /* only relevant for tms5220C's multi frame rate feature; is the actual 4 bit value written on a 0x2* or 0x0* command */
	UINT16 m_pitch_count;     /* pitch counter; provides chirp rom address */

	INT32 m_u[11];
	INT32 m_x[10];

	UINT16 m_RNG;             /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 TODO: no it isn't */
	INT16 m_excitation_data;

	/* The TMS52xx has two different ways of providing output data: the
	   analog speaker pin (which was usually used) and the Digital I/O pin.
	   The internal DAC used to feed the analog pin is only 8 bits, and has the
	   funny clipping/clamping logic, while the digital pin gives full 10 bit
	   resolution of the output data.
	   TODO: add a way to set/reset this other than the FORCE_DIGITAL define
	 */
	UINT8 m_digital_select;

	/* io_ready: page 3 of the datasheet specifies that READY will be asserted until
	 * data is available or processed by the system.
	 */
	UINT8 m_io_ready;

	/* flag for "true" timing involving rs/ws */
	UINT8 m_true_timing;


// helpers

int extract_bits(int count) // extract from rom image/array
{

  uint8_t value;
  UINT16 data;
  INT8 num_bits=count;

  	data = byte_rev[*ptrAddr]<<8;
	//	data = (*ptrAddr)<<8;
	if (ptrBit+num_bits > 8)
	{
	    data |= byte_rev[*(ptrAddr+1)];
	    //  	  data |= *(ptrAddr+1);
	}
	data <<= ptrBit;
	value = data >> (16-num_bits);
	ptrBit += num_bits;
	//	didntjump=1;
	if (ptrBit >= 8)
	{
	  //	  fprintf(stderr,"%x, ",*ptrAddr);
		ptrBit -= 8;
		ptrAddr++;
		//		didntjump=2;
		if (ptrBit==0) {
		  //		  didntjump=0;
		}
	}
//	tobits(value,num_bits);
	return value;


}


/**********************************************************************************************

     clip_analog -- clips the 14 bit return value from the lattice filter to its final 10 bit value (-512 to 511), and upshifts/range extends this to 16 bits

***********************************************************************************************/

static INT16 clip_analog(INT16 cliptemp)
{
	/* clipping, just like the patent shows:
	 * the top 10 bits of this result are visible on the digital output IO pin.
	 * next, if the top 3 bits of the 14 bit result are all the same, the lowest of those 3 bits plus the next 7 bits are the signed analog output, otherwise the low bits are all forced to match the inverse of the topmost bit, i.e.:
	 * 1x xxxx xxxx xxxx -> 0b10000000
	 * 11 1bcd efgh xxxx -> 0b1bcdefgh
	 * 00 0bcd efgh xxxx -> 0b0bcdefgh
	 * 0x xxxx xxxx xxxx -> 0b01111111
	 */
#ifdef DEBUG_CLIP
	if ((cliptemp > 2047) || (cliptemp < -2048)) fprintf(stderr,"clipping cliptemp to range; was %d\n", cliptemp);
#endif
	if (cliptemp > 2047) cliptemp = 2047;
	else if (cliptemp < -2048) cliptemp = -2048;
	/* at this point the analog output is tapped */
#ifdef ALLOW_4_LSB
	// input:  ssss snnn nnnn nnnn
	// N taps:       ^^^ ^         = 0x0780
	// output: snnn nnnn nnnn NNNN
	return (cliptemp << 4)|((cliptemp&0x780)>>7); // upshift and range adjust
#else
	cliptemp &= ~0xF;
	// input:  ssss snnn nnnn 0000
	// N taps:       ^^^ ^^^^      = 0x07F0
	// P taps:       ^             = 0x0400
	// output: snnn nnnn NNNN NNNP
	return (cliptemp << 4)|((cliptemp&0x7F0)>>3)|((cliptemp&0x400)>>10); // upshift and range adjust
#endif
}


/**********************************************************************************************

     matrix_multiply -- does the proper multiply and shift
     a is the k coefficient and is clamped to 10 bits (9 bits plus a sign)
     b is the running result and is clamped to 14 bits.
     output is 14 bits, but note the result LSB bit is always 1.
     Because the low 4 bits of the result are trimmed off before
     output, this makes almost no difference in the computation.

**********************************************************************************************/
static INT32 matrix_multiply(INT32 a, INT32 b)
{
	INT32 result;
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	result = ((a*b)>>9)|1;//&(~1);
#ifdef VERBOSE
	if (result>16383) fprintf(stderr,"matrix multiplier overflowed! a: %x, b: %x, result: %x", a, b, result);
	if (result<-16384) fprintf(stderr,"matrix multiplier underflowed! a: %x, b: %x, result: %x", a, b, result);
#endif
	return result;
}

/**********************************************************************************************

     lattice_filter -- executes one 'full run' of the lattice filter on a specific byte of
     excitation data, and specific values of all the current k constants,  and returns the
     resulting sample.

***********************************************************************************************/

INT32 lattice_filter()
{
	// Lattice filter here
	// Aug/05/07: redone as unrolled loop, for clarity - LN
	/* Originally Copied verbatim from table I in US patent 4,209,804, now updated to be in same order as the actual chip does it, not that it matters.
	  notation equivalencies from table:
	  Yn(i) == m_u[n-1]
	  Kn = m_current_k[n-1]
	  bn = m_x[n-1]
	 */
	/*
	    int ep = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
	     m_u[10] = ep;
	    for (int i = 0; i < 10; i++)
	    {
	        int ii = 10-i; // for m = 10, this would be 11 - i, and since i is from 1 to 10, then ii ranges from 10 to 1
	        //int jj = ii+1; // this variable, even on the fortran version, is never used. it probably was intended to be used on the two lines below the next one to save some redundant additions on each.
	        ep = ep - (((m_current_k[ii-1] * m_x[ii-1])>>9)|1); // subtract reflection from lower stage 'top of lattice'
	         m_u[ii-1] = ep;
	        m_x[ii] = m_x[ii-1] + (((m_current_k[ii-1] * ep)>>9)|1); // add reflection from upper stage 'bottom of lattice'
	    }
	m_x[0] = ep; // feed the last section of the top of the lattice directly to the bottom of the lattice
	*/
		m_u[10] = matrix_multiply(m_previous_energy, (m_excitation_data<<6));  //Y(11)
		m_u[9] = m_u[10] - matrix_multiply(m_current_k[9], m_x[9]);
		m_u[8] = m_u[9] - matrix_multiply(m_current_k[8], m_x[8]);
		m_u[7] = m_u[8] - matrix_multiply(m_current_k[7], m_x[7]);
		m_u[6] = m_u[7] - matrix_multiply(m_current_k[6], m_x[6]);
		m_u[5] = m_u[6] - matrix_multiply(m_current_k[5], m_x[5]);
		m_u[4] = m_u[5] - matrix_multiply(m_current_k[4], m_x[4]);
		m_u[3] = m_u[4] - matrix_multiply(m_current_k[3], m_x[3]);
		m_u[2] = m_u[3] - matrix_multiply(m_current_k[2], m_x[2]);
		m_u[1] = m_u[2] - matrix_multiply(m_current_k[1], m_x[1]);
		m_u[0] = m_u[1] - matrix_multiply(m_current_k[0], m_x[0]);
		m_x[9] = m_x[8] + matrix_multiply(m_current_k[8], m_u[8]);
		m_x[8] = m_x[7] + matrix_multiply(m_current_k[7], m_u[7]);
		m_x[7] = m_x[6] + matrix_multiply(m_current_k[6], m_u[6]);
		m_x[6] = m_x[5] + matrix_multiply(m_current_k[5], m_u[5]);
		m_x[5] = m_x[4] + matrix_multiply(m_current_k[4], m_u[4]);
		m_x[4] = m_x[3] + matrix_multiply(m_current_k[3], m_u[3]);
		m_x[3] = m_x[2] + matrix_multiply(m_current_k[2], m_u[2]);
		m_x[2] = m_x[1] + matrix_multiply(m_current_k[1], m_u[1]);
		m_x[1] = m_x[0] + matrix_multiply(m_current_k[0], m_u[0]);
		m_x[0] = m_u[0];
		m_previous_energy = m_current_energy;
#ifdef DEBUG_LATTICE
		int i;
		fprintf(stderr,"V:%04d ", m_u[10]);
		for (i = 9; i >= 0; i--)
		{
			fprintf(stderr,"Y%d:%04d ", i+1, m_u[i]);
			fprintf(stderr,"b%d:%04d ", i+1, m_x[i]);
			if ((i % 5) == 0) fprintf(stderr,"\n");
		}
#endif
		return m_u[0];
}


// parse frame

void parse_frame()
{
	int indx, i, rep_flag;

	// We actually don't care how many bits are left in the fifo here; the frame subpart will be processed normally, and any bits extracted 'past the end' of the fifo will be read as zeroes; the fifo being emptied will set the /BE latch which will halt speech exactly as if a stop frame had been encountered (instead of whatever partial frame was read); the same exact circuitry is used for both on the real chip, see us patent 4335277 sheet 16, gates 232a (decode stop frame) and 232b (decode /BE plus DDIS (decode disable) which is active during speak external).

	/* if the chip is a tms5220C, and the rate mode is set to that each frame (0x04 bit set)
	has a 2 bit rate preceding it, grab two bits here and store them as the rate; */
	if ((TMS5220_HAS_RATE_CONTROL) && (m_c_variant_rate & 0x04))
	{
		indx = extract_bits(2);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(indx,2);
		fprintf(stderr," ");
#endif
		m_IP = reload_table[indx];
	}
	else // non-5220C and 5220C in fixed rate mode
	m_IP = reload_table[m_c_variant_rate&0x3];

	//	update_fifo_status_and_ints();
	if (!m_talk_status) goto ranout;

	// attempt to extract the energy index
	m_new_frame_energy_idx = extract_bits(m_coeff->energy_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_energy_idx,m_coeff->energy_bits);
	fprintf(stderr," ");
#endif
	//	update_fifo_status_and_ints();
	if (!m_talk_status) goto ranout;
	// if the energy index is 0 or 15, we're done
	if ((m_new_frame_energy_idx == 0) || (m_new_frame_energy_idx == 15))
		return;


	// attempt to extract the repeat flag
	rep_flag = extract_bits(1);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(rep_flag, 1);
	fprintf(stderr," ");
#endif

	// attempt to extract the pitch
	m_new_frame_pitch_idx = extract_bits(m_coeff->pitch_bits);
#ifdef DEBUG_PARSE_FRAME_DUMP
	printbits(m_new_frame_pitch_idx,m_coeff->pitch_bits);
	fprintf(stderr," ");
#endif
	//	update_fifo_status_and_ints();
	if (!m_talk_status) goto ranout;
	// if this is a repeat frame, just do nothing, it will reuse the old coefficients
	if (rep_flag)
		return;

	// extract first 4 K coefficients
	for (i = 0; i < 4; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i],m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		//		update_fifo_status_and_ints();
		if (!m_talk_status) goto ranout;
	}

	// if the pitch index was zero, we only need 4 K's...
	if (m_new_frame_pitch_idx == 0)
	{
		/* and the rest of the coefficients are zeroed, but that's done in the generator code */
		return;
	}

	// If we got here, we need the remaining 6 K's
	for (i = 4; i < m_coeff->num_k; i++)
	{
		m_new_frame_k_idx[i] = extract_bits(m_coeff->kbits[i]);
#ifdef DEBUG_PARSE_FRAME_DUMP
		printbits(m_new_frame_k_idx[i],m_coeff->kbits[i]);
		fprintf(stderr," ");
#endif
		//		update_fifo_status_and_ints();
		if (!m_talk_status) goto ranout;
	}
#ifdef VERBOSE
	if (m_speak_external)
		logerror("Parsed a frame successfully in FIFO - %d bits remaining\n", (m_fifo_count*8)-(m_fifo_bits_taken));
	else
		logerror("Parsed a frame successfully in ROM\n");
#endif
	return;

	ranout:
#ifdef DEBUG_FRAME_ERRORS
	logerror("Ran out of bits on a parse!\n");
#endif
	return;
}


// main process loop

int16_t process(u8 *ending)
{
	int i, bitout, zpar;
	INT32 this_sample;
	int16_t sample;
	/* loop until the buffer is full or we've stopped speaking */
	//	if (m_speaking_now)
	//	{
		/* if it is the appropriate time to update the old energy/pitch idxes,
		 * i.e. when IP=7, PC=12, T=17, subcycle=2, do so. Since IP=7 PC=12 T=17
		 * is JUST BEFORE the transition to IP=0 PC=0 T=0 sybcycle=(0 or 1),
		 * which happens 4 T-cycles later), we change on the latter.*/
		if ((m_IP == 0) && (m_PC == 0) && (m_subcycle < 2))
		{
			m_OLDE = (m_new_frame_energy_idx == 0);
			m_OLDP = (m_new_frame_pitch_idx == 0);
		}

		/* if we're ready for a new frame to be applied, i.e. when IP=0, PC=12, Sub=1
		 * (In reality, the frame was really loaded incrementally during the entire IP=0
		 * PC=x time period, but it doesn't affect anything until IP=0 PC=12 happens)
		 */
		if ((m_IP == 0) && (m_PC == 12) && (m_subcycle == 1))
		{
			// HACK for regression testing, be sure to comment out before release!
			//m_RNG = 0x1234;
			// end HACK

			/* appropriately override the interp count if needed; this will be incremented after the frame parse! */
			m_IP = reload_table[m_c_variant_rate&0x3];

#ifdef PERFECT_INTERPOLATION_HACK
			/* remember previous frame energy, pitch, and coefficients */
			m_old_frame_energy_idx = m_new_frame_energy_idx;
			m_old_frame_pitch_idx = m_new_frame_pitch_idx;
			for (i = 0; i < m_coeff->num_k; i++)
				m_old_frame_k_idx[i] = m_new_frame_k_idx[i];
#endif

			/* if the talk status was clear last frame, halt speech now. */
			if (m_talk_status == 0)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"tms5220_process: processing frame: talk status = 0 caused by stop frame or buffer empty, halting speech.\n");
#endif
				//				m_speaking_now = 1; // finally halt speech
				*ending=1;
				// keep speaking - RESET TOD!
				//				goto empty;
				return 0;
			}


			/* Parse a new frame into the new_target_energy, new_target_pitch and new_target_k[] */
			parse_frame(); //TODO!
#ifdef DEBUG_PARSE_FRAME_DUMP
			fprintf(stderr,"\n");
#endif

			/* if the new frame is a stop frame, set an interrupt and set talk status to 0 */
			if (NEW_FRAME_STOP_FLAG == 1)
				{
					m_talk_status = m_speak_external = 0;
					//					set_interrupt_state(1);
					//					update_fifo_status_and_ints();
				}

			/* in all cases where interpolation would be inhibited, set the inhibit flag; otherwise clear it.
			   Interpolation inhibit cases:
			 * Old frame was voiced, new is unvoiced
			 * Old frame was silence/zero energy, new has nonzero energy
			 * Old frame was unvoiced, new is voiced (note this is the case on the patent but may not be correct on the real final chip)
			 */
			if ( ((OLD_FRAME_UNVOICED_FLAG == 0) && (NEW_FRAME_UNVOICED_FLAG == 1))
				|| ((OLD_FRAME_UNVOICED_FLAG == 1) && (NEW_FRAME_UNVOICED_FLAG == 0)) /* this line needs further investigation, starwars tie fighters may sound better without it */
				|| ((OLD_FRAME_SILENCE_FLAG == 1) && (NEW_FRAME_SILENCE_FLAG == 0)) )
				m_inhibit = 1;
			else // normal frame, normal interpolation
				m_inhibit = 0;

			/* load new frame targets from tables, using parsed indices */
			m_target_energy = m_coeff->energytable[m_new_frame_energy_idx];
			m_target_pitch = m_coeff->pitchtable[m_new_frame_pitch_idx]+(_sely*255.0f); // TODO: very rough but makes very croaky!
			zpar = NEW_FRAME_UNVOICED_FLAG; // find out if parameters k5-k10 should be zeroed
			for (i = 0; i < 4; i++)
				m_target_k[i] = m_coeff->ktable[i][m_new_frame_k_idx[i]];
			for (i = 4; i < m_coeff->num_k; i++)
				m_target_k[i] = (m_coeff->ktable[i][m_new_frame_k_idx[i]] * (1-zpar));

#ifdef DEBUG_GENERATION
			/* Debug info for current parsed frame */
			fprintf(stderr, "OLDE: %d; OLDP: %d; ", m_OLDE, m_OLDP);
			fprintf(stderr,"Processing frame: ");
			if (m_inhibit == 0)
				fprintf(stderr, "Normal Frame\n");
			else
				fprintf(stderr,"Interpolation Inhibited\n");
			fprintf(stderr,"*** current Energy, Pitch and Ks =      %04d,   %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",m_current_energy, m_current_pitch, m_current_k[0], m_current_k[1], m_current_k[2], m_current_k[3], m_current_k[4], m_current_k[5], m_current_k[6], m_current_k[7], m_current_k[8], m_current_k[9]);
			fprintf(stderr,"*** target Energy(idx), Pitch, and Ks = %04d(%x),%04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d, %04d\n",m_target_energy, m_new_frame_energy_idx, m_target_pitch, m_target_k[0], m_target_k[1], m_target_k[2], m_target_k[3], m_target_k[4], m_target_k[5], m_target_k[6], m_target_k[7], m_target_k[8], m_target_k[9]);
#endif

			/* if TS is now 0, ramp the energy down to 0. Is this really correct to hardware? */
			if (m_talk_status == 0)
			{
#ifdef DEBUG_GENERATION
				fprintf(stderr,"Talk status is 0, forcing target energy to 0\n");
#endif
				m_target_energy = 0;
			}
		}
		else // Not a new frame, just interpolate the existing frame.
		{
			int inhibit_state = ((m_inhibit==1)&&(m_IP != 0)); // disable inhibit when reaching the last interp period, but don't overwrite the m_inhibit value
#ifdef PERFECT_INTERPOLATION_HACK
			int samples_per_frame = m_subc_reload?175:266; // either (13 A cycles + 12 B cycles) * 7 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 7 interps for SPKSLOW
			//int samples_per_frame = m_subc_reload?200:304; // either (13 A cycles + 12 B cycles) * 8 interps for normal SPEAK/SPKEXT, or (13*2 A cycles + 12 B cycles) * 8 interps for SPKSLOW
			int current_sample = (m_subcycle - m_subc_reload)+(m_PC*(3-m_subc_reload))+((m_subc_reload?25:38)*((m_IP-1)&7));

			zpar = OLD_FRAME_UNVOICED_FLAG;
			//fprintf(stderr, "CS: %03d", current_sample);
			// reset the current energy, pitch, etc to what it was at frame start
			m_current_energy = m_coeff->energytable[m_old_frame_energy_idx];
			m_current_pitch = m_coeff->pitchtable[m_old_frame_pitch_idx];
			for (i = 0; i < 4; i++)
				m_current_k[i] = m_coeff->ktable[i][m_old_frame_k_idx[i]];
			for (i = 4; i < m_coeff->num_k; i++)
				m_current_k[i] = (m_coeff->ktable[i][m_old_frame_k_idx[i]] * (1-zpar));
			// now adjust each value to be exactly correct for each of the samples per frame
			if (m_IP != 0) // if we're still interpolating...
			{
				m_current_energy += (((m_target_energy - m_current_energy)*(1-inhibit_state))*current_sample)/samples_per_frame;
				m_current_pitch += (((m_target_pitch - m_current_pitch)*(1-inhibit_state))*current_sample)/samples_per_frame;
				for (i = 0; i < m_coeff->num_k; i++)
					m_current_k[i] += (((m_target_k[i] - m_current_k[i])*(1-inhibit_state))*current_sample)/samples_per_frame;
			}
			else // we're done, play this frame for 1/8 frame.
			{
				m_current_energy = m_target_energy;
				m_current_pitch = m_target_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_current_k[i] = m_target_k[i];
			}
#else
			//Updates to parameters only happen on subcycle '2' (B cycle) of PCs.
			if (m_subcycle == 2)
			{
				switch(m_PC)
				{
					case 0: /* PC = 0, B cycle, write updated energy */
					m_current_energy += (((m_target_energy - m_current_energy)*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 1: /* PC = 1, B cycle, write updated pitch */
					m_current_pitch += (((m_target_pitch - m_current_pitch)*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
					/* PC = 2 through 11, B cycle, write updated K1 through K10 */
					m_current_k[m_PC-2] += (((m_target_k[m_PC-2] - m_current_k[m_PC-2])*(1-inhibit_state)) INTERP_SHIFT);
					break;
					case 12: /* PC = 12, do nothing */
					break;
				}
			}
#endif
		}

		// calculate the output
		if (OLD_FRAME_UNVOICED_FLAG == 1)
		{
			// generate unvoiced samples here
			if (m_RNG & 1)
				m_excitation_data = ~0x3F; /* according to the patent it is (either + or -) half of the maximum value in the chirp table, so either 01000000(0x40) or 11000000(0xC0)*/
			else
				m_excitation_data = 0x40;
		}
		else /* (OLD_FRAME_UNVOICED_FLAG == 0) */
		{
			// generate voiced samples here
			/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			 * function has a chirp/peak and then a long chain of zeroes.
			 * The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
			 * and if the address reaches that point the ADDRESS incrementer is
			 * disabled, forcing all samples beyond 51d to be == 51d
			 */
			if (m_pitch_count >= 51)
				m_excitation_data = (INT8)m_coeff->chirptable[51];
			else /*m_pitch_count < 51*/
				m_excitation_data = (INT8)m_coeff->chirptable[m_pitch_count];
		}

		// Update LFSR *20* times every sample (once per T cycle), like patent shows
	for (i=0; i<20; i++)
	{
		bitout = ((m_RNG >> 12) & 1) ^
				((m_RNG >>  3) & 1) ^
				((m_RNG >>  2) & 1) ^
				((m_RNG >>  0) & 1);
		m_RNG <<= 1;
		m_RNG |= bitout;
	}
		this_sample = lattice_filter(); /* execute lattice filter */
#ifdef DEBUG_GENERATION_VERBOSE
		//fprintf(stderr,"C:%01d; ",m_subcycle);
		fprintf(stderr,"IP:%01d PC:%02d X:%04d E:%03d P:%03d Pc:%03d ",m_IP, m_PC, m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
		//fprintf(stderr,"X:%04d E:%03d P:%03d Pc:%03d ", m_excitation_data, m_current_energy, m_current_pitch, m_pitch_count);
		for (i=0; i<10; i++)
			fprintf(stderr,"K%d:%04d ", i+1, m_current_k[i]);
		fprintf(stderr,"Out:%06d", this_sample);
		fprintf(stderr,"\n");
#endif
		/* next, force result to 14 bits (since its possible that the addition at the final (k1) stage of the lattice overflowed) */
		while (this_sample > 16383) this_sample -= 32768;
		while (this_sample < -16384) this_sample += 32768;
		if (m_digital_select == 0) // analog SPK pin output is only 8 bits, with clipping
		  //			buffer[buf_count] = clip_analog(this_sample);
		  sample= clip_analog(this_sample);
		else // digital I/O pin output is 12 bits
		{
#ifdef ALLOW_4_LSB
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn nnnn
			// N taps:                       ^                 = 0x2000;
			// output: ssss ssss ssss ssss snnn nnnn nnnn nnnN
		  //			buffer[buf_count] = (this_sample<<1)|((this_sample&0x2000)>>13);
		  sample=(this_sample<<1)|((this_sample&0x2000)>>13);
#else
			this_sample &= ~0xF;
			// input:  ssss ssss ssss ssss ssnn nnnn nnnn 0000
			// N taps:                       ^^ ^^^            = 0x3E00;
			// output: ssss ssss ssss ssss snnn nnnn nnnN NNNN
			//			buffer[buf_count] = (this_sample<<1)|((this_sample&0x3E00)>>9);
			sample=(this_sample<<1)|((this_sample&0x3E00)>>9);
#endif
		}
		// Update all counts

		m_subcycle++;
		if ((m_subcycle == 2) && (m_PC == 12))
		{
			/* Circuit 412 in the patent acts a reset, resetting the pitch counter to 0
			 * if INHIBIT was true during the most recent frame transition.
			 * The exact time this occurs is betwen IP=7, PC=12 sub=0, T=t12
			 * and m_IP = 0, PC=0 sub=0, T=t12, a period of exactly 20 cycles,
			 * which overlaps the time OLDE and OLDP are updated at IP=7 PC=12 T17
			 * (and hence INHIBIT itself 2 t-cycles later). We do it here because it is
			 * convenient and should make no difference in output.
			 */
			if ((m_IP == 7)&&(m_inhibit==1)) m_pitch_count = 0;
			m_subcycle = m_subc_reload;
			m_PC = 0;
			m_IP++;
			m_IP&=0x7;
		}
		else if (m_subcycle == 3)
		{
			m_subcycle = m_subc_reload;
			m_PC++;
		}
		m_pitch_count++;
		if (m_pitch_count >= m_current_pitch) m_pitch_count = 0;
		m_pitch_count &= 0x1FF;
		//		buf_count++;
		//		size--;
		//	}
	return sample;
}

const uint8_t sp_parNICEe[] __attribute__ ((section (".flash"))) = {0x46, 0xE3, 0xB2, 0x27, 0x24, 0x14, 0x37, 0xCD, 0x47, 0xF, 0x5F, 0x95, 0x73, 0xB4, 0x18, 0x3B, 0x3C, 0xDA, 0xCE, 0xD5, 0xAE, 0x76, 0xD4, 0x14, 0x33, 0x2C, 0x5B, 0xF1, 0x51, 0x4A, 0x76, 0xD7, 0x2D, 0xD9, 0x4B, 0x2E, 0xC1, 0xDC, 0xD6, 0xA5, 0xE, 0xA9, 0x3A, 0x9, 0x7F, 0x57, 0xD2, 0x84, 0x62, 0x38, 0xA2, 0x82, 0x56, 0xE2, 0xB3, 0x65, 0x93, 0x4F, 0x39, 0x8E, 0x4F, 0xC2, 0x38, 0x6B, 0x9D, 0x30, 0xE0, 0xEB, 0xC, 0xA, 0x50, 0x80, 0x2, 0x4, 0x98, 0xCC, 0x12, 0x1, 0x86, 0x57, 0x21, 0x20, 0x30, 0x69, 0x5, 0xBC, 0x31, 0xE1, 0x1, 0xF, 0x44, 0x20, 0x3, 0x1, 0xB8, 0xBB, 0xA9, 0xF0, 0xCD, 0xB0, 0xBA, 0x4D, 0x93, 0x21, 0xF6, 0x64, 0xCA, 0x9A, 0x55, 0x86, 0xD8, 0xB2, 0xB, 0x6B, 0x54, 0x6E, 0x62, 0x4B, 0x26, 0x6C, 0x31, 0x3B, 0x48, 0xD3, 0x89, 0xB2, 0x65, 0x23, 0x3, 0xC, 0x19, 0x61, 0x80, 0xAF, 0x8C, 0x19, 0x40, 0x53, 0x4D, 0x31, 0x72, 0x50, 0x8B, 0x5A, 0xDA, 0xC9, 0xCE, 0x41, 0xA2, 0x22, 0xE9, 0x44, 0xDB, 0xDB, 0xC6, 0x8E, 0x5A, 0x6B, 0x54, 0xD5, 0x18, 0x3B, 0x8, 0xAF, 0x14, 0xB5, 0xA3, 0xAD, 0x7};

const uint8_t sp_spk0352n8[]         __attribute__ ((section (".flash")))  ={0x2B, 0x26, 0xC9, 0x5A, 0x35, 0x55, 0x6E, 0x38, 0x21, 0x2A, 0xD5, 0xC8, 0xBE, 0x78, 0xC3, 0xD, 0x37, 0xEC, 0xB8, 0xE3, 0x88, 0x41, 0xB1, 0x95, 0xD4, 0xC3, 0x11, 0x83, 0x14, 0xB, 0x8D, 0x97, 0x2D, 0x46, 0x29, 0x9A, 0x6A, 0x3F, 0x47, 0x2C, 0xCC, 0xD4, 0x45, 0x7B, 0xB5, 0x38, 0xA4, 0xA9, 0x29, 0xE7, 0xE9, 0x51, 0x81, 0xDF, 0xC2, 0x2C, 0xD0, 0x40, 0x81, 0x97, 0xA2, 0x2D, 0x78};

const uint8_t sp_spk0352n9[]         __attribute__ ((section (".flash")))  ={0xCB, 0x87, 0x26, 0x15, 0x93, 0x8A, 0xAE, 0x3F, 0x36, 0x64, 0x49, 0x9F, 0xB2, 0x7A, 0xCD, 0x55, 0x47, 0x1A, 0xEB, 0x95, 0x71, 0x52, 0xAE, 0x38, 0xC2, 0xD8, 0x70, 0x8F, 0x33, 0x7B, 0xC5, 0x10, 0x5A, 0x7B, 0x98, 0x5F, 0xE8, 0x2E, 0x8B, 0x9E, 0x14, 0xB3, 0x2C, 0x6A, 0x19, 0x70, 0xA, 0xB1, 0x3, 0x3F, 0xB2, 0x2D, 0xE7, 0xB5, 0x5A, 0xC4, 0x54, 0xBA, 0xFE, 0xC4, 0xE0, 0xC4, 0x3C, 0x37, 0xF2, 0x49, 0xD1, 0x9B, 0xBB, 0x6F, 0x98, 0x9B, 0x6F, 0xBD, 0xF5, 0xB4, 0xA0, 0x4A, 0x2C, 0x32, 0xF2, 0x6D, 0xB3, 0x8D, 0x66, 0xBD, 0x86, 0x58, 0x64, 0x9E, 0x75, 0xE6, 0x99, 0x6A, 0x2A, 0xD1, 0x26, 0xA7, 0x4C, 0x93, 0xFC}; // ends okay


// break out into generator...

void tms5200_init()
{

  INT16 i,j;
  // do we need to set each time we change coeffs TODO!
  
  /*
		case TMS5110_IS_TMC0281:
			m_coeff = &T0280B_0281A_coeff;
			break;
		case TMS5110_IS_TMC0281D:
			m_coeff = &T0280D_0281D_coeff;
			break;
		case TMS5110_IS_CD2801:
			m_coeff = &T0280F_2801A_coeff;
			break;
		case TMS5110_IS_M58817:
			m_coeff = &M58817_coeff;
			break;
		case TMS5110_IS_CD2802:
			m_coeff = &T0280F_2802_coeff;
			break;
		case TMS5110_IS_TMS5110A:
			m_coeff = &tms5110a_coeff;
   */

  //  m_coeff=&T0285_2501E_coeff; // this is for 5200! //		m_coeff = &tms5220_coeff;
  //   m_coeff = &T0280B_0281A_coeff; // this is for 5100! // TODO - sett/swap here
	
  //  m_coeff=tms5220_coeff;
  for(i=0;i<256;i++)
    {
      j = (i>>4) | (i<<4); // Swap in groups of 4
      j = ((j & 0xcc)>>2) | ((j & 0x33)<<2); // Swap in groups of 2
      byte_rev[i] = ((j & 0xaa)>>1) | ((j & 0x55)<<1); // Swap bit pairs
    }

  // testing

  TMS_vocab vocab_2110={wordlist_spell2303, &T0280B_0281A_coeff, 102, 104.0f}; // tested for wordlist access on lap!
  m_coeff=vocab_2110.m_coeff;
  
  // what needs to be set to start speaking?

  m_new_frame_energy_idx = 0;
  m_new_frame_pitch_idx = 0;
  for (i = 0; i < 4; i++)
    m_new_frame_k_idx[i] = 0;
  for (i = 4; i < 7; i++)
    m_new_frame_k_idx[i] = 0xF;
  for (i = 7; i < m_coeff->num_k; i++)    m_new_frame_k_idx[i] = 0x7;
  m_talk_status = 1;
};

//tms_newssay_
//tms_reset_ called on mode change trigger ->   for (i = 7; i < m_coeff->num_k; i++)    m_new_frame_k_idx[i] = 0x7;
//tms_get_sample

void tms5200_newsay(){
  INT16 i;
  m_new_frame_energy_idx = 0;
  m_new_frame_pitch_idx = 0;
  for (i = 0; i < 4; i++)
    m_new_frame_k_idx[i] = 0;
  for (i = 4; i < 7; i++)
    m_new_frame_k_idx[i] = 0xF;
  for (i = 7; i < m_coeff->num_k; i++)
    m_new_frame_k_idx[i] = 0x7;
  m_talk_status = 1;

  //  ptrAddr = sp_parNICEe;
  //  ptrAddr = sp_spk0352n9; // TODO ptr to const
  INT16 sel=_selx*132.0f; // TODO - length of bank
  ptrAddr=wordlist_spell2304[sel]; // bank TODO but test now extent
  //  ptrAddr = sp_spk2304nn131;
  ptrBit = 0;

};

int16_t tms5200_get_sample(){
  int16_t sample; u8 ending;
    sample=  process(&ending);
    //    printf("%c",(sample+32768)>>8);
    if (ending==1){
      ending=0; tms5200_newsay();
    }
    return sample;

}

/*
void main(){
  INT16 i, sample; u8 ending=0;
  // buffer?
  tms5200_init();
  tms5200_newsay();
  // ptraddr and speech data array
  //  m_IP=0;

  while(1){
    sample=  process(&ending);
    printf("%c",(sample+32768)>>8);
    if (ending==1){
      ending=0; tms5200_newsay();
    }

  //  for (i=0;i<32768;i++) printf("%c",(speechbuffer[i]+32768)>>8);
  }
}
*/
