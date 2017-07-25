// license:BSD-3-Clause
// copyright-holders:Joseph Zbiciak,Tim Lindner
// modified for WORM by Martin Howse

/**********************************************************************

    SP0256 Narrator Speech Processor emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 28  OSC 2
                _RESET   2 |             | 27  OSC 1
           ROM DISABLE   3 |             | 26  ROM CLOCK
                    C1   4 |             | 25  _SBY RESET
                    C2   5 |             | 24  DIGITAL OUT
                    C3   6 |             | 23  Vdi
                   Vdd   7 |    SP0256   | 22  TEST
                   SBY   8 |             | 21  SER IN
                  _LRQ   9 |             | 20  _ALD
                    A8  10 |             | 19  SE
                    A7  11 |             | 18  A1
               SER OUT  12 |             | 17  A2
                    A6  13 |             | 16  A3
                    A5  14 |_____________| 15  A4

**********************************************************************/

/*
   GI SP0256 Narrator Speech Processor

   By Joe Zbiciak. Ported to MESS by tim lindner.

*/

#include "sp0256.h"
#include "audio.h"
#include "english2phoneme/TTS.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "resources.h"

extern const unsigned char m_rom12[] __attribute__ ((section (".flash")));
extern const unsigned char m_rom19[] __attribute__ ((section (".flash")));
extern const unsigned char m_rom003[] __attribute__ ((section (".flash")));
extern const unsigned char m_rom004[] __attribute__ ((section (".flash")));
extern float exy[240];
extern float _selx, _sely, _selz;

static const unsigned char *m_romm;
typedef unsigned char UINT8;
typedef signed char INT8;
typedef u16 UINT16;
typedef int16_t INT16;
typedef uint32_t UINT32;
typedef int32_t INT32;

struct lpc12_t
{
	INT16     rpt, cnt;       /* Repeat counter, Period down-counter.         */
  UINT32  per, perorig,rng;       /* Period, Amplitude, Random Number Generator   */
  INT16     amp, amporig;
  INT16   f_coef[6],f_coeforig[6];      /* F0 through F5.                               */
  INT16   b_coef[6], b_coeforig[6];      /* B0 through B5.                               */
	INT16   z_data[6][2];   /* Time-delay data for the filter stages.       */
	UINT8   r[16];          /* The encoded register set.                    */
	INT16     interp;
};

static u8           m_silent;          /* Flag: SP0256 is silent.                      */


static struct lpc12_t m_filt;            /* 12-pole filter                               */
static uint16_t            m_lrq;             /* Load ReQuest.  == 0 if we can accept a load  */
static int32_t            m_ald;             /* Address LoaD.  < 0 if no command pending.    */
static int32_t            m_pc;              /* Microcontroller's PC value.                  */
static uint32_t            m_stack;           /* Microcontroller's PC stack.                  */
static u8            m_fifo_sel;        /* True when executing from FIFO.               */
static u8            m_halted;          /* True when CPU is halted.                     */
static UINT32         m_mode;            /* Mode register.                               */
static UINT32         m_page;            /* Page set by SETPAGE                          */

static UINT32         m_fifo_head;       /* FIFO head pointer (where new data goes).     */
static UINT32         m_fifo_tail;       /* FIFO tail pointer (where data comes from).   */
static UINT32         m_fifo_bitp;       /* FIFO bit-pointer (for partial decles).       */
static UINT16         m_fifo[64];        /* The 64-decle FIFO.                           */


#define CLOCK_DIVIDER (7*6*8)
#define HIGH_QUALITY

#define PER_PAUSE    (64)               /* Equiv timing period for pauses.  */
#define PER_NOISE    (64)               /* Equiv timing period for noise.   */

#define FIFO_ADDR    (0x1800 << 3)      /* SP0256 address of SPB260 speech FIFO. = 49152 decimal  */


/* ======================================================================== */
/*  qtbl  -- Coefficient Quantization Table.  This comes from a             */
/*              SP0250 data sheet, and should be correct for SP0256.        */
/* ======================================================================== */

extern const INT16 qtbl[128];

static void sp0256_iinit()
{

	/* -------------------------------------------------------------------- */
	/*  Configure our internal variables.                                   */
	/* -------------------------------------------------------------------- */
	m_filt.rng = 1;

	/* -------------------------------------------------------------------- */
	/*  Set up the microsequencer's initial state.                          */
	/* -------------------------------------------------------------------- */

	m_halted   = 1; // was 1
	m_filt.rpt = -1;
	m_lrq      = 0x8000;
 	m_page     = 0x1000 << 3; //32768 =0x8000
	m_silent   = 1;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

static void reset()
{
	// reset FIFO and SP0256
	m_fifo_head = m_fifo_tail = m_fifo_bitp = 0;

	memset(&m_filt, 0, sizeof(m_filt));
	m_halted   = 1;
	m_filt.rpt = -1;
	m_filt.rng = 1;
	m_lrq      = 0x8000;
	m_ald      = 0x0000;
	m_pc       = 0x0000;
	m_stack    = 0x0000;
	m_fifo_sel = 0;
	m_mode     = 0;
	m_page     = 0x1000 << 3;
	m_silent   = 1;
	//	m_sby_line = 0;

	  m_lrq = 0;
}

/* ======================================================================== */
/*  LIMIT            -- Limiter function for digital sample output.         */
/* ======================================================================== */
static inline INT16 limit(INT16 s){
#ifdef HIGH_QUALITY /* Higher quality than the original, but who cares? */
	if (s >  8191) return  8191;
	if (s < -8192) return -8192;
#else
	if (s >  127) return  127;
	if (s < -128) return -128;
#endif
	return s;
}

/* ======================================================================== */
/*  LPC12_UPDATE     -- Update the 12-pole filter, outputting samples.      */
/* ======================================================================== */
static inline void lpc12_update(struct lpc12_t *f, INT16* out)
{
	u8 j;
	INT16 samp, val;
	u8 do_int;

	/* -------------------------------------------------------------------- */
	/*  Iterate up to the desired number of samples.  We actually may       */
	/*  break out early if our repeat count expires.                        */
	/* -------------------------------------------------------------------- */
		/* ---------------------------------------------------------------- */
		/*  Generate a series of periodic impulses, or random noise.        */
		/* ---------------------------------------------------------------- */
		do_int = 0;
		samp   = 0;
		val=exy[1]*1028.0f;
		MAXED(val,1023);
		f->amp=f->amporig*logspeed[1023-val];
		if (f->perorig)
		{
		  val=exy[0]*1028.0f;
		  MAXED(val,1023);
		  f->per=f->perorig*logspeed[val];

			if (f->cnt <= 0)
			{
				f->cnt += f->per;
				samp    = f->amp;
				f->rpt--;
				//				do_int  = f->interp;//????

				for (j = 0; j < 6; j++)
					f->z_data[j][1] = f->z_data[j][0] = 0;

			} else
			{
				samp = 0;
				f->cnt--;
			}

		} else
		{
			u8 bit;

			if (--f->cnt <= 0)
			{
			  //				do_int = f->interp;
				f->cnt = PER_NOISE;
				f->rpt--;
				for (j = 0; j < 6; j++)
					f->z_data[j][0] = f->z_data[j][1] = 0;
			}

			bit = f->rng & 1;
			f->rng = (f->rng >> 1) ^ (bit ? 0x4001 : 0);
			//			bit=rand()%2;

			if (bit) { samp =  f->amp; }
			else     { samp = -f->amp; }
		}

		/* ---------------------------------------------------------------- */
		/*  If we need to, process the interpolation registers.             */
		/* ---------------------------------------------------------------- */
		/*		if (do_int)
		{
			f->r[0] += f->r[14];
			f->r[1] += f->r[15];

			f->amp   = (f->r[0] & 0x1F) << (((f->r[0] & 0xE0) >> 5) + 0); 
			f->per   = f->r[1];

			do_int   = 0;
			}*/

		/* ---------------------------------------------------------------- */
		/*  Stop if we expire our repeat counter and return the actual      */
		/*  number of samples we did.                                       */
		/* ---------------------------------------------------------------- */
		//		if (f->rpt <= 0) return 0; 

		/* ---------------------------------------------------------------- */
		/*  Each 2nd order stage looks like one of these.  The App. Manual  */
		/*  gives the first form, the patent gives the second form.         */
		/*  They're equivalent except for time delay.  I implement the      */
		/*  first form.   (Note: 1/Z == 1 unit of time delay.)              */
		/*                                                                  */
		/*          ---->(+)-------->(+)----------+------->                 */
		/*                ^           ^           |                         */
		/*                |           |           |                         */
		/*                |           |           |                         */
		/*               [B]        [2*F]         |                         */
		/*                ^           ^           |                         */
		/*                |           |           |                         */
		/*                |           |           |                         */
		/*                +---[1/Z]<--+---[1/Z]<--+                         */
		/*                                                                  */
		/*                                                                  */
		/*                +---[2*F]<---+                                    */
		/*                |            |                                    */
		/*                |            |                                    */
		/*                v            |                                    */
		/*          ---->(+)-->[1/Z]-->+-->[1/Z]---+------>                 */
		/*                ^                        |                        */
		/*                |                        |                        */
		/*                |                        |                        */
		/*                +-----------[B]<---------+                        */
		/*                                                                  */
		/* ---------------------------------------------------------------- */
		for (j = 0; j < 6; j++)
		{
		  // intersperse
		  //		  f->b_coef[j]=f->b_coeforig[j]+ (256-(int)((exy[2 + 2*j]*512.0)));
		  //	  val=exy[2 + (2*j)]*1028.0f;
		  //		  MAXED(val,1023);
		  //		  f->b_coef[j]=f->b_coeforig[j]*logspeed[1023-val];
		  f->b_coef[j]=f->b_coeforig[j]+(64-(exy[2 + (2*j)]*128.0f));
		  //		  val=exy[3 + (2*j)]*1028.0f;
		  //		  MAXED(val,1023);
		  //		  val=411;
		  //f->f_coef[j]=f->f_coeforig[j]*logspeed[1023-val];
		  f->f_coef[j]=f->f_coeforig[j]+(64-(exy[3 + (2*j)]*128.0f));
		  //		  f->f_coef[j]=f->f_coeforig[j]+ (256-(int)((exy[3 + 2*j]*512.0)));
		  samp += (((int32_t)f->b_coef[j] * (int32_t)f->z_data[j][1]) >> 9);
		  samp += (((int32_t)f->f_coef[j] * (int32_t)f->z_data[j][0]) >> 8);

			f->z_data[j][1] = f->z_data[j][0];
			f->z_data[j][0] = samp;
		}
		*out= limit(samp);
		//		return 1;
}

static u8 stage_map[6] = { 0, 1, 2, 3, 4, 5 };

/* ======================================================================== */
/*  LPC12_REGDEC -- Decode the register set in the filter bank.             */
/* ======================================================================== */
static inline void lpc12_regdec(struct lpc12_t *f)
{
	u8 i;

	/*	for (i = 0; i < 16; i++) // the bends
	{
	  //	  f->r[i]=f->rorig[i]+(exy[i]*256.0f); // scheme to keep original!
	  f->r[i]=f->rorig[i];//+(exy[i]*256.0f); // no bend
	  }*/
	
	/* -------------------------------------------------------------------- */
	/*  Decode the Amplitude and Period registers.  Force the 'cnt' to 0    */
	/*  to get an initial impulse.  We compensate elsewhere by setting      */
	/*  the repeat count to "repeat + 1".                                   */
	/* -------------------------------------------------------------------- */
	f->amporig = (f->r[0] & 0x1F) << (((f->r[0] & 0xE0) >> 5) + 0);
	//	f->cnt = 0;
	f->perorig = f->r[1];

	/* -------------------------------------------------------------------- */
	/*  Decode the filter coefficients from the quant table.                */
	/* -------------------------------------------------------------------- */
	for (i = 0; i < 6; i++)
	{
#define IQ(x) (((x) & 0x80) ? qtbl[0x7F & -(x)] : -qtbl[(x)])
		f->b_coeforig[stage_map[i]] = IQ(f->r[2 + 2*i]);
		f->f_coeforig[stage_map[i]] = IQ(f->r[3 + 2*i]);
	}

	/* -------------------------------------------------------------------- */
	/*  Set the Interp flag based on whether we have interpolation parms    */
	/* -------------------------------------------------------------------- */
	//	f->interp = f->r[14] || f->r[15];

	return;
}

/* ======================================================================== */
/*  SP0256_DATAFMT   -- Data format table for the SP0256's microsequencer   */
/*                                                                          */
/*  len     4 bits      Length of field to extract                          */
/*  lshift  4 bits      Left-shift amount on field                          */
/*  param   4 bits      Parameter number being updated                      */
/*  delta   1 bit       This is a delta-update.  (Implies sign-extend)      */
/*  field   1 bit       This is a field replace.                            */
/*  clr5    1 bit       Clear F5, B5.                                       */
/*  clrall  1 bit       Clear all before doing this update                  */
/* ======================================================================== */

#define CR(l,s,p,d,f,c5,ca)         \
		(                           \
			(((l)  & 15) <<  0) |   \
			(((s)  & 15) <<  4) |   \
			(((p)  & 15) <<  8) |   \
			(((d)  &  1) << 12) |   \
			(((f)  &  1) << 13) |   \
			(((c5) &  1) << 14) |   \
			(((ca) &  1) << 15)     \
		)

#define CR_DELTA  CR(0,0,0,1,0,0,0)
#define CR_FIELD  CR(0,0,0,0,1,0,0)
#define CR_CLR5   CR(0,0,0,0,0,1,0)
#define CR_CLRA   CR(0,0,0,0,0,0,1)
#define CR_LEN(x) ((x) & 15)
#define CR_SHF(x) (((x) >> 4) & 15)
#define CR_PRM(x) (((x) >> 8) & 15)

enum { AM = 0, PR, B0, F0, B1, F1, B2, F2, B3, F3, B4, F4, B5, F5, IA, IP };

extern const UINT16 sp0256_datafmt[]   __attribute__ ((section (".flash")));
extern const INT16 sp0256_df_idx[16 * 8]   __attribute__ ((section (".flash")));

/* ======================================================================== */
/*  BITREV32       -- Bit-reverse a 32-bit number.                            */
/* ======================================================================== */
static inline UINT32 bitrev32(UINT32 val)
{
	val = ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16);
	val = ((val & 0xFF00FF00) >>  8) | ((val & 0x00FF00FF) <<  8);
	val = ((val & 0xF0F0F0F0) >>  4) | ((val & 0x0F0F0F0F) <<  4);
	val = ((val & 0xCCCCCCCC) >>  2) | ((val & 0x33333333) <<  2);
	val = ((val & 0xAAAAAAAA) >>  1) | ((val & 0x55555555) <<  1);

	return val;
}

/* ======================================================================== */
/*  BITREV8       -- Bit-reverse a 8-bit number.                            */
/* ======================================================================== */
static inline UINT8 bitrev8(UINT8 val)
{
	val = ((val & 0xF0) >>  4) | ((val & 0x0F) <<  4);
	val = ((val & 0xCC) >>  2) | ((val & 0x33) <<  2);
	val = ((val & 0xAA) >>  1) | ((val & 0x55) <<  1);

	return val;
}


/* ======================================================================== */
/*  SP0256_GETB  -- Get up to 8 bits at the current PC.                     */
/* ======================================================================== */
static UINT32 getb( int len )
{
  UINT32 data, minus;
	u16 d0, d1;

	//	fprintf(stderr,"m_pc %d\n",m_pc>>3);

	/* -------------------------------------------------------------------- */
	/*  Fetch data from the FIFO or from the MASK                           */
	/* -------------------------------------------------------------------- */
	if (m_fifo_sel)
	{
		d0 = m_fifo[(m_fifo_tail    ) & 63];
		d1 = m_fifo[(m_fifo_tail + 1) & 63];

		data = ((d1 << 10) | d0) >> m_fifo_bitp;


		/* ---------------------------------------------------------------- */
		/*  Note the PC doesn't advance when we execute from FIFO.          */
		/*  Just the FIFO's bit-pointer advances.   (That's not REALLY      */
		/*  what happens, but that's roughly how it behaves.)               */
		/* ---------------------------------------------------------------- */
		m_fifo_bitp += len;
		if (m_fifo_bitp >= 10)
		{
			m_fifo_tail++;
			m_fifo_bitp -= 10;
		}
	} else
	{
		/* ---------------------------------------------------------------- */
		/*  Figure out which ROMs are being fetched into, and grab two      */
		/*  adjacent bytes.  The byte we're interested in is extracted      */
		/*  from the appropriate bit-boundary between them.                 */
		/* ---------------------------------------------------------------- */
	  int32_t idx0 = (m_pc    ) >> 3, d0; //???
	  int32_t idx1 = (m_pc + 8) >> 3, d1;

	  data=0;

	  if (idx0<0x1800 && idx0>=0x1000) {
	  data=1; //
	  minus=0x1000; // default
	}

	  //	  else if (idx0>=0x1800 && idx0<0x4000){// we don't need this
	    //  fprintf(stderr, "fifo?????? 0x%X\n", idx0);
	  //	    data=0;
	  //	  }
	  else if (idx0>=0x4000 && idx0<0x8000) {
		    m_romm=m_rom003; // 003 has phonemes as AL2 and some phrases but not so many WHY?
		    minus=0x4000;
		    data=1;
	  }
	  else if (idx0>=0x8000 && idx0<0xC000) {
	    m_romm=m_rom004; 
	    minus=0x8000;
	    data=1;
	    }


	  
	  if (data!=0){
	  int32_t firstadd=(idx0 & 0xffff)-minus;
	  int32_t secondadd=(idx1 & 0xffff)-minus;

		d0 = m_romm[firstadd];
		d1 = m_romm[secondadd]; // was 0xffff

		data = ((d1 << 8) | d0) >> (m_pc & 7);
		//				}

		m_pc += len;
		  

	/* -------------------------------------------------------------------- */
	/*  Mask data to the requested length.                                  */
	/* -------------------------------------------------------------------- */
	data &= ((1 << len) - 1);
	  }
	}

	return data;
}

/* ======================================================================== */
/*  SP0256_MICRO -- Emulate the microsequencer in the SP0256.  Executes     */
/*                  instructions either until the repeat count != 0 or      */
/*                  the sequencer gets halted by a RTS to 0.                */
/* ======================================================================== */
static void micro()
{
	UINT8  immed4;
	UINT8  opcode;
	UINT16 cr;
	u8 ctrl_xfer;
	int8_t repeat;
	u8 i, idx0, idx1;

	/* -------------------------------------------------------------------- */
	/*  Only execute instructions while the filter is not busy.             */
	/* -------------------------------------------------------------------- */
	while (m_filt.rpt <= 0)
	{
		/* ---------------------------------------------------------------- */
		/*  If the CPU is halted, see if we have a new command pending      */
		/*  in the Address LoaD buffer.                                     */
		/* ---------------------------------------------------------------- */
		if (m_halted && !m_lrq)
		{
		 	  m_pc       = m_ald | (0x1000 << 3); // OR with 0x8000 this adds 0x1000 which we subtract later when shifts back
			  // m_pc = m_ald;
			m_fifo_sel = 0;
			m_halted   = 0;
			m_lrq      = 0x8000;
			m_ald      = 0;
			for (i = 0; i < 16; i++)
				m_filt.r[i] = 0;
			//			m_drq_cb(1);
		}

		/* ---------------------------------------------------------------- */
		/*  If we're still halted, do nothing.                              */
		/* ---------------------------------------------------------------- */
		if (m_halted)
		{
			m_filt.rpt = 1;
			m_lrq      = 0x8000;
			m_ald      = 0;
			for (i = 0; i < 16; i++)
				m_filt.r[i] = 0;

			//			SET_SBY(1)

			return;
		}

		/* ---------------------------------------------------------------- */
		/*  Fetch the first 8 bits of the opcode, which are always in the   */
		/*  same approximate format -- immed4 followed by opcode.           */
		/* ---------------------------------------------------------------- */
		immed4 = getb(4);
		opcode = getb(4);
		repeat = 0;
		ctrl_xfer = 0;

		//		printf("$%.4X.%.1X: OPCODE %d%d%d%d.%d%d\n",
		//		(m_pc >> 3) - 1, m_pc & 7,
		//		!!(opcode & 1), !!(opcode & 2),
		//		!!(opcode & 4), !!(opcode & 8),
		//		!!(m_mode&4), !!(m_mode&2));

		/* ---------------------------------------------------------------- */
		/*  Handle the special cases for specific opcodes.                  */
		/* ---------------------------------------------------------------- */
		switch (opcode)
		{
			/* ------------------------------------------------------------ */
			/*  OPCODE 0000:  RTS / SETPAGE                                 */
			/* ------------------------------------------------------------ */
			case 0x0:
			{
				/* -------------------------------------------------------- */
				/*  If immed4 != 0, then this is a SETPAGE instruction.     */
				/* -------------------------------------------------------- */
				if (immed4)     /* SETPAGE */
				{
					m_page = bitrev32(immed4) >> 13;
				} else
				/* -------------------------------------------------------- */
				/*  Otherwise, this is an RTS / HLT.                        */
				/* -------------------------------------------------------- */
				{
					UINT32 btrg;

					/* ---------------------------------------------------- */
					/*  Figure out our branch target.                       */
					/* ---------------------------------------------------- */
					btrg = m_stack;

					m_stack = 0;

					/* ---------------------------------------------------- */
					/*  If the branch target is zero, this is a HLT.        */
					/*  Otherwise, it's an RTS, so set the PC.              */
					/* ---------------------------------------------------- */
					if (!btrg)
					{
						m_halted   = 1;
						m_pc       = 0;
						ctrl_xfer  = 1;
					} else
					{
						m_pc      = btrg;
						ctrl_xfer = 1;
					}
				}

				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 0111:  JMP          Jump to 12-bit/16-bit Abs Addr   */
			/*  OPCODE 1011:  JSR          Jump to Subroutine               */
			/* ------------------------------------------------------------ */
			case 0xE:
			case 0xD:
			{
				uint32_t btrg;

				/* -------------------------------------------------------- */
				/*  Figure out our branch target.                           */
				/* -------------------------------------------------------- */
				btrg = m_page                     |
						(bitrev32(immed4)  >> 17) |
						(bitrev32(getb(8)) >> 21);
				ctrl_xfer = 1;

				/* -------------------------------------------------------- */
				/*  If this is a JSR, push our return address on the        */
				/*  stack.  Make sure it's byte aligned.                    */
				/* -------------------------------------------------------- */
				if (opcode == 0xD)
					m_stack = (m_pc + 7) & ~7;

				/* -------------------------------------------------------- */
				/*  Jump to the new location!                               */
				/* -------------------------------------------------------- */
				m_pc = btrg;
				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 1000:  SETMODE      Set the Mode and Repeat MSBs     */
			/* ------------------------------------------------------------ */
			case 0x1:
			{
				m_mode = ((immed4 & 8) >> 2) | (immed4 & 4) | ((immed4 & 3) << 4);
				break;
			}

			/* ------------------------------------------------------------ */
			/*  OPCODE 0001:  LOADALL      Load All Parameters              */
			/*  OPCODE 0010:  LOAD_2       Load Per, Ampl, Coefs, Interp.   */
			/*  OPCODE 0011:  SETMSB_3     Load Pitch, Ampl, MSBs, & Intrp  */
			/*  OPCODE 0100:  LOAD_4       Load Pitch, Ampl, Coeffs         */
			/*  OPCODE 0101:  SETMSB_5     Load Pitch, Ampl, and Coeff MSBs */
			/*  OPCODE 0110:  SETMSB_6     Load Ampl, and Coeff MSBs.       */
			/*  OPCODE 1001:  DELTA_9      Delta update Ampl, Pitch, Coeffs */
			/*  OPCODE 1010:  SETMSB_A     Load Ampl and MSBs of 3 Coeffs   */
			/*  OPCODE 1100:  LOAD_C       Load Pitch, Ampl, Coeffs         */
			/*  OPCODE 1101:  DELTA_D      Delta update Ampl, Pitch, Coeffs */
			/*  OPCODE 1110:  LOAD_E       Load Pitch, Amplitude            */
			/*  OPCODE 1111:  PAUSE        Silent pause                     */
			/* ------------------------------------------------------------ */
			default:
			{
				repeat = immed4 | (m_mode & 0x30);
				int val = (exy[14]*131.0f);
				MAXED(val,127);
				repeat=((float)(repeat)*logpitch[val]);


				if (repeat<1) repeat=1;
				break;
			}
		}
		if (opcode != 1) m_mode &= 0xF;

		/* ---------------------------------------------------------------- */
		/*  If this was a control transfer, handle setting "fifo_sel"       */
		/*  and all that ugliness.                                          */
		/* ---------------------------------------------------------------- */
		if (ctrl_xfer)
		{

			/* ------------------------------------------------------------ */
			/*  Set our "FIFO Selected" flag based on whether we're going   */
			/*  to the FIFO's address.                                      */
			/* ------------------------------------------------------------ */
			m_fifo_sel = m_pc == FIFO_ADDR;


			/* ------------------------------------------------------------ */
			/*  Control transfers to the FIFO cause it to discard the       */
			/*  partial decle that's at the front of the FIFO.              */
			/* ------------------------------------------------------------ */
			if (m_fifo_sel && m_fifo_bitp)
			{

				/* Discard partially-read decle. */
				if (m_fifo_tail < m_fifo_head) m_fifo_tail++;
				m_fifo_bitp = 0;
			}


			continue;
		}

		/* ---------------------------------------------------------------- */
		/*  Otherwise, if we have a repeat count, then go grab the data     */
		/*  block and feed it to the filter.                                */
		/* ---------------------------------------------------------------- */
		if (!repeat) continue;

		m_filt.rpt = repeat + 1;

		i = (opcode << 3) | (m_mode & 6);
		idx0 = sp0256_df_idx[i++];
		idx1 = sp0256_df_idx[i  ];

		//		assert(idx0 >= 0 && idx1 >= 0 && idx1 >= idx0);

		/* ---------------------------------------------------------------- */
		/*  Step through control words in the description for data block.   */
		/* ---------------------------------------------------------------- */
		for (i = idx0; i <= idx1; i++)
		{
			int16_t len, shf, delta, field, prm, clra, clr5;
			INT8 value;

			/* ------------------------------------------------------------ */
			/*  Get the control word and pull out some important fields.    */
			/* ------------------------------------------------------------ */
			cr = sp0256_datafmt[i];

			len   = CR_LEN(cr);
			shf   = CR_SHF(cr);
			prm   = CR_PRM(cr);
			clra  = cr & CR_CLRA;
			clr5  = cr & CR_CLR5;
			delta = cr & CR_DELTA;
			field = cr & CR_FIELD;
			value = 0;

			/* ------------------------------------------------------------ */
			/*  Clear any registers that were requested to be cleared.      */
			/* ------------------------------------------------------------ */
			if (clra)
			{
				for (u8 j = 0; j < 16; j++)
					m_filt.r[j] = 0;

				m_silent = 1;
			}

			if (clr5)
				m_filt.r[B5] = m_filt.r[F5] = 0;

			/* ------------------------------------------------------------ */
			/*  If this entry has a bitfield with it, grab the bitfield.    */
			/* ------------------------------------------------------------ */
			if (len)
			{
				value = getb(len);
			}
			else
			{
				continue;
			}

			/* ------------------------------------------------------------ */
			/*  Sign extend if this is a delta update.                      */
			/* ------------------------------------------------------------ */
			if (delta)  /* Sign extend */
			{
				if (value & (1 << (len - 1))) value |= -1 << len;
			}

			/* ------------------------------------------------------------ */
			/*  Shift the value to the appropriate precision.               */
			/* ------------------------------------------------------------ */
			if (shf)
				value <<= shf;


			m_silent = 0;

			/* ------------------------------------------------------------ */
			/*  If this is a field-replace, insert the field.               */
			/* ------------------------------------------------------------ */
			if (field)
			{

				m_filt.r[prm] &= ~(~0 << shf); /* Clear the old bits.     */
				m_filt.r[prm] |= value;        /* Merge in the new bits.  */


				continue;
			}

			/* ------------------------------------------------------------ */
			/*  If this is a delta update, add to the appropriate field.    */
			/* ------------------------------------------------------------ */
			if (delta)
			{

				m_filt.r[prm] += value;

				continue;
			}

			/* ------------------------------------------------------------ */
			/*  Otherwise, just write the new value.                        */
			/* ------------------------------------------------------------ */
			m_filt.r[prm] = value;
		}

		/* ---------------------------------------------------------------- */
		/*  Special case:  Set PAUSE's equivalent period.                   */
		/* ---------------------------------------------------------------- */
		if (opcode == 0xF)
		{
			m_silent = 1;
			m_filt.r[1] = PER_PAUSE;
		}

		/* ---------------------------------------------------------------- */
		/*  Now that we've updated the registers, go decode them.           */
		/* ---------------------------------------------------------------- */
		lpc12_regdec(&m_filt);

		/* ---------------------------------------------------------------- */
		/*  Break out since we now have a repeat count.                     */
		/* ---------------------------------------------------------------- */
		break;
	}
}

void sp0256_initbend(void){
   reset();
 }


int16_t sp0256_get_samplebend(void){
  static int16_t output; 

      micro();
      lpc12_update(&m_filt, &output);
   
   if (m_halted==1 && m_filt.rpt <= 0)     {
     sp0256_newsaybend();
   }
      //          }
   return output;
}

extern const unsigned char remap19[];

void sp0256_newsaybend(void){
  u8 dada, indexy;
  m_lrq=0; m_halted=1; m_filt.rpt=0;

  u8 selector=_selz*88.0f; // total is 36+49=85
  MAXED(selector,84);
  selector=84-selector;
  if (selector<37) { // so top is 36
    m_page=0x1000<<3;
   m_romm=m_rom12;
   dada=6+selector; // they are 6->42
    }
    else {    
      m_romm=m_rom19;
      indexy=selector-37;
      dada=remap19[indexy];
      if (indexy>19) m_page=0x8000<<3;
      else m_page=0x1000<<3;
    }
      m_ald = ((dada) << 4);
      m_lrq = 0; //from 8 bit write
    }
