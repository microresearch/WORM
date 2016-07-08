//#include "audio.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"
#include "forlap.h"

#include "tms5110r.inc"

#define DEBUG_5110  0

//extern uint8_t* ptrAddr, ptrBit;
//extern uint8_t byte_rev[256];

uint8_t* ptrAddr, ptrBit;
uint8_t byte_rev[256];

const uint8_t sp_spk0352n9[]         __attribute__ ((section (".flash")))  ={0xCB, 0x87, 0x26, 0x15, 0x93, 0x8A, 0xAE, 0x3F, 0x36, 0x64, 0x49, 0x9F, 0xB2, 0x7A, 0xCD, 0x55, 0x47, 0x1A, 0xEB, 0x95, 0x71, 0x52, 0xAE, 0x38, 0xC2, 0xD8, 0x70, 0x8F, 0x33, 0x7B, 0xC5, 0x10, 0x5A, 0x7B, 0x98, 0x5F, 0xE8, 0x2E, 0x8B, 0x9E, 0x14, 0xB3, 0x2C, 0x6A, 0x19, 0x70, 0xA, 0xB1, 0x3, 0x3F, 0xB2, 0x2D, 0xE7, 0xB5, 0x5A, 0xC4, 0x54, 0xBA, 0xFE, 0xC4, 0xE0, 0xC4, 0x3C, 0x37, 0xF2, 0x49, 0xD1, 0x9B, 0xBB, 0x6F, 0x98, 0x9B, 0x6F, 0xBD, 0xF5, 0xB4, 0xA0, 0x4A, 0x2C, 0x32, 0xF2, 0x6D, 0xB3, 0x8D, 0x66, 0xBD, 0x86, 0x58, 0x64, 0x9E, 0x75, 0xE6, 0x99, 0x6A, 0x2A, 0xD1, 0x26, 0xA7, 0x4C, 0x93, 0xFC}; // ends okay


/// vars - all from tms5110.c in mame2

	UINT8 m_talk_status;
	UINT8 m_speaking_now;
	UINT8 m_state;

	/* Rom interface */
	UINT32 m_address;
	UINT8  m_next_is_address;
	UINT8  m_schedule_dummy_read;
	UINT8  m_addr_bit;
	/* read byte */
	UINT8  m_CTL_buffer;

	/* these contain data describing the current and previous voice frames */
	UINT16 m_old_energy;
	UINT16 m_old_pitch;
	INT32 m_old_k[10];

	UINT16 m_new_energy;
	UINT16 m_new_pitch;
	INT32 m_new_k[10];


	/* these are all used to contain the current state of the sound generation */
	UINT16 m_current_energy;
	UINT16 m_current_pitch;
	INT32 m_current_k[10];

	UINT16 m_target_energy;
	UINT16 m_target_pitch;
	INT32 m_target_k[10];

	UINT8 m_interp_count;       /* number of interp periods (0-7) */
	UINT8 m_sample_count;       /* sample number within interp (0-24) */
	INT32 m_pitch_count;

	INT32 m_x[11];

	INT32 m_RNG;  /* the random noise generator configuration is: 1 + x + x^3 + x^4 + x^13 */

	INT32 m_speech_rom_bitnum;

	/* coefficient tables */
	const struct tms5100_coeffs *m_coeff;
	const UINT8 *m_table;


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


void parse_frame()
{
	int bits, indx, i, rep_flag;
#if (DEBUG_5110)
	int ene;
#endif

	/* count the total number of bits available */ /// ?????
	//	bits = m_fifo_count;


	/* attempt to extract the energy index */
	bits -= m_coeff->energy_bits;
	/*	if (bits < 0)
	{
		request_bits( -bits ); // toggle M0 to receive needed bits
		bits = 0;
	}*/
	indx = extract_bits(m_coeff->energy_bits);
	m_new_energy = m_coeff->energytable[indx];
#if (DEBUG_5110)
	ene = indx;
#endif

	/* if the energy index is 0 or 15, we're done */

	if ((indx == 0) || (indx == 15))
	{
		if (DEBUG_5110) logerror("  (4-bit energy=%d frame)\n",m_new_energy);

	/* clear the k's */
		if (indx == 0)
		{
			for (i = 0; i < m_coeff->num_k; i++)
				m_new_k[i] = 0;
		}

		/* clear fifo if stop frame encountered */
/*		if (indx == 15)
		{
			if (DEBUG_5110) logerror("  (4-bit energy=%d STOP frame)\n",m_new_energy);
			m_fifo_head = m_fifo_tail = m_fifo_count = 0;
		}
		return;*/
	}


	/* attempt to extract the repeat flag */
/*	bits -= 1;
	if (bits < 0)
	{
		request_bits( -bits ); // toggle M0 to receive needed bits 
		bits = 0;
	}*/
	rep_flag = extract_bits(1);

	/* attempt to extract the pitch */
/*	bits -= m_coeff->pitch_bits;
	if (bits < 0)
	{
		request_bits( -bits ); // toggle M0 to receive needed bits 
		bits = 0;
	}*/
	indx = extract_bits(m_coeff->pitch_bits);
	m_new_pitch = m_coeff->pitchtable[indx];

	/* if this is a repeat frame, just copy the k's */
	if (rep_flag)
	{
	//actually, we do nothing because the k's were already loaded (on parsing the previous frame)

		if (DEBUG_5110) logerror("  (10-bit energy=%d pitch=%d rep=%d frame)\n", m_new_energy, m_new_pitch, rep_flag);
		return;
	}


	/* if the pitch index was zero, we need 4 k's */
	if (indx == 0)
	{
		/* attempt to extract 4 K's */
	  /*		bits -= 18;
		if (bits < 0)
		{
		request_bits( -bits ); / toggle M0 to receive needed bits
		bits = 0;
		}*/
		for (i = 0; i < 4; i++)
			m_new_k[i] = m_coeff->ktable[i][extract_bits(m_coeff->kbits[i])];

	/* and clear the rest of the new_k[] */
		for (i = 4; i < m_coeff->num_k; i++)
			m_new_k[i] = 0;

		if (DEBUG_5110) logerror("  (28-bit energy=%d pitch=%d rep=%d 4K frame)\n", m_new_energy, m_new_pitch, rep_flag);
		return;
	}

	/* else we need 10 K's */
/*	bits -= 39;
	if (bits < 0)
	{
			request_bits( -bits ); // toggle M0 to receive needed bits 
		bits = 0;
	}*/
#if (DEBUG_5110)
	printf("FrameDump %02d ", ene);
	for (i = 0; i < m_coeff->num_k; i++)
	{
		int x;
		x = extract_bits( m_coeff->kbits[i]);
		m_new_k[i] = m_coeff->ktable[i][x];
		printf("%02d ", x);
	}
	printf("\n");
#else
	for (i = 0; i < m_coeff->num_k; i++)
	{
		int x;
		x = extract_bits( m_coeff->kbits[i]);
		m_new_k[i] = m_coeff->ktable[i][x];
	}
#endif
	if (DEBUG_5110) logerror("  (49-bit energy=%d pitch=%d rep=%d 10K frame)\n", m_new_energy, m_new_pitch, rep_flag);

}



//////////////////

int16_t fiveoneprocess(u8* ending)
{
	int i, interp_period, bitout;
	INT16 Y11, cliptemp, sample;

	/* if we're to speak, but haven't started */
	if (!m_talk_status)
	{
	/* a "dummy read" is mentioned in the tms5200 datasheet */
	/* The Bagman speech roms data are organized in such a way that
	** the bit at address 0 is NOT a speech data. The bit at address 1
	** is the speech data. It seems that the tms5110 performs a dummy read
	** just before it executes a SPEAK command.
	** This has been moved to command logic ...
	**  perform_dummy_read();
	*/

		/* clear out the new frame parameters (it will become old frame just before the first call to parse_frame() ) */
		m_new_energy = 0;
		m_new_pitch = 0;
		for (i = 0; i < m_coeff->num_k; i++)
			m_new_k[i] = 0;

		m_talk_status = 1;
	}


	/* loop until the buffer is full or we've stopped speaking */
	//	while ((size > 0) && m_speaking_now)
	//	{
		int current_val;

		/* if we're ready for a new frame */
		if ((m_interp_count == 0) && (m_sample_count == 0))
		{
			/* remember previous frame */
			m_old_energy = m_new_energy;
			m_old_pitch = m_new_pitch;
			for (i = 0; i < m_coeff->num_k; i++)
				m_old_k[i] = m_new_k[i];


			/* if the old frame was a stop frame, exit and do not process any more frames */
			if (m_old_energy == COEFF_ENERGY_SENTINEL)
			{
				if (DEBUG_5110) logerror("processing frame: stop frame\n");
				m_target_energy = m_current_energy = 0;
				m_speaking_now = m_talk_status = 0;
				m_interp_count = m_sample_count = m_pitch_count = 0;
				//				goto empty; // ENDING
				*ending=1;
				return 0;
			}


			/* Parse a new frame into the new_energy, new_pitch and new_k[] */
			parse_frame();


			/* Set old target as new start of frame */
			m_current_energy = m_old_energy;
			m_current_pitch = m_old_pitch;

			for (i = 0; i < m_coeff->num_k; i++)
				m_current_k[i] = m_old_k[i];


			/* is this the stop (ramp down) frame? */
			if (m_new_energy == COEFF_ENERGY_SENTINEL)
			{
				/*logerror("processing frame: ramp down\n");*/
				m_target_energy = 0;
				m_target_pitch = m_old_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_old_k[i];
			}
			else if ((m_old_energy == 0) && (m_new_energy != 0)) /* was the old frame a zero-energy frame? */
			{
				/* if so, and if the new frame is non-zero energy frame then the new parameters
				   should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/

				/*logerror("processing non-zero energy frame after zero-energy frame\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else if ((m_old_pitch == 0) && (m_new_pitch != 0))    /* is this a change from unvoiced to voiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: UNVOICED->VOICED frame change\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else if ((m_old_pitch != 0) && (m_new_pitch == 0))    /* is this a change from voiced to unvoiced frame ? */
			{
				/* if so, then the new parameters should become our current and target parameters immediately,
				   i.e. we should NOT interpolate them slowly in.
				*/
				/*if (DEBUG_5110) logerror("processing frame: VOICED->UNVOICED frame change\n");*/
				m_target_energy = m_new_energy;
				m_target_pitch = m_current_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_current_k[i] = m_new_k[i];
			}
			else
			{
				/*logerror("processing frame: Normal\n");*/
				/*logerror("*** Energy = %d\n",current_energy);*/
				/*logerror("proc: %d %d\n",last_fbuf_head,fbuf_head);*/

				m_target_energy = m_new_energy;
				m_target_pitch = m_new_pitch;
				for (i = 0; i < m_coeff->num_k; i++)
					m_target_k[i] = m_new_k[i];
			}
		}
		else
		{
			interp_period = m_sample_count / 25;
			switch(m_interp_count)
			{
				/*         PC=X  X cycle, rendering change (change for next cycle which chip is actually doing) */
				case 0: /* PC=0, A cycle, nothing happens (calc energy) */
				break;
				case 1: /* PC=0, B cycle, nothing happens (update energy) */
				break;
				case 2: /* PC=1, A cycle, update energy (calc pitch) */
				m_current_energy += ((m_target_energy - m_current_energy) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 3: /* PC=1, B cycle, nothing happens (update pitch) */
				break;
				case 4: /* PC=2, A cycle, update pitch (calc K1) */
				m_current_pitch += ((m_target_pitch - m_current_pitch) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 5: /* PC=2, B cycle, nothing happens (update K1) */
				break;
				case 6: /* PC=3, A cycle, update K1 (calc K2) */
				m_current_k[0] += ((m_target_k[0] - m_current_k[0]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 7: /* PC=3, B cycle, nothing happens (update K2) */
				break;
				case 8: /* PC=4, A cycle, update K2 (calc K3) */
				m_current_k[1] += ((m_target_k[1] - m_current_k[1]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 9: /* PC=4, B cycle, nothing happens (update K3) */
				break;
				case 10: /* PC=5, A cycle, update K3 (calc K4) */
				m_current_k[2] += ((m_target_k[2] - m_current_k[2]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 11: /* PC=5, B cycle, nothing happens (update K4) */
				break;
				case 12: /* PC=6, A cycle, update K4 (calc K5) */
				m_current_k[3] += ((m_target_k[3] - m_current_k[3]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 13: /* PC=6, B cycle, nothing happens (update K5) */
				break;
				case 14: /* PC=7, A cycle, update K5 (calc K6) */
				m_current_k[4] += ((m_target_k[4] - m_current_k[4]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 15: /* PC=7, B cycle, nothing happens (update K6) */
				break;
				case 16: /* PC=8, A cycle, update K6 (calc K7) */
				m_current_k[5] += ((m_target_k[5] - m_current_k[5]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 17: /* PC=8, B cycle, nothing happens (update K7) */
				break;
				case 18: /* PC=9, A cycle, update K7 (calc K8) */
				m_current_k[6] += ((m_target_k[6] - m_current_k[6]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 19: /* PC=9, B cycle, nothing happens (update K8) */
				break;
				case 20: /* PC=10, A cycle, update K8 (calc K9) */
				m_current_k[7] += ((m_target_k[7] - m_current_k[7]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 21: /* PC=10, B cycle, nothing happens (update K9) */
				break;
				case 22: /* PC=11, A cycle, update K9 (calc K10) */
				m_current_k[8] += ((m_target_k[8] - m_current_k[8]) >> m_coeff->interp_coeff[interp_period]);
				break;
				case 23: /* PC=11, B cycle, nothing happens (update K10) */
				break;
				case 24: /* PC=12, A cycle, update K10 (do nothing) */
				m_current_k[9] += ((m_target_k[9] - m_current_k[9]) >> m_coeff->interp_coeff[interp_period]);
				break;
			}
		}


		/* calculate the output */

		if (m_current_energy == 0)
		{
			/* generate silent samples here */
			current_val = 0x00;
		}
		else if (m_old_pitch == 0)
		{
			/* generate unvoiced samples here */
			if (m_RNG&1)
				current_val = -64; /* according to the patent it is (either + or -) half of the maximum value in the chirp table */
			else
				current_val = 64;

		}
		else
		{
			// generate voiced samples here
			/* US patent 4331836 Figure 14B shows, and logic would hold, that a pitch based chirp
			 * function has a chirp/peak and then a long chain of zeroes.
			 * The last entry of the chirp rom is at address 0b110011 (51d), the 52nd sample,
			 * and if the address reaches that point the ADDRESS incrementer is
			 * disabled, forcing all samples beyond 51d to be == 51d
			 */
			if (m_pitch_count >= 51)
				current_val = (INT8)m_coeff->chirptable[51];
			else /*m_pitch_count < 51*/
				current_val = (INT8)m_coeff->chirptable[m_pitch_count];
		}

		/* Update LFSR *20* times every sample, like patent shows */
		for (i=0; i<20; i++)
		{
			bitout = ((m_RNG>>12)&1) ^
					((m_RNG>>10)&1) ^
					((m_RNG>> 9)&1) ^
					((m_RNG>> 0)&1);
			m_RNG >>= 1;
			m_RNG |= (bitout<<12);
		}

		/* Lattice filter here */

		Y11 = (current_val * 64 * m_current_energy) / 512;

		for (i = m_coeff->num_k - 1; i >= 0; i--)
		{
			Y11 = Y11 - ((m_current_k[i] * m_x[i]) / 512);
			m_x[i+1] = m_x[i] + ((m_current_k[i] * Y11) / 512);
		}

		m_x[0] = Y11;


		/* clipping & wrapping, just like the patent */

		/* YL10 - YL4 ==> DA6 - DA0 */
		cliptemp = Y11 / 16;

		/* M58817 seems to be different */
		if (m_coeff->subtype & (SUBTYPE_M58817))
			cliptemp = cliptemp / 2;

		if (cliptemp > 511) cliptemp = -512 + (cliptemp-511);
		else if (cliptemp < -512) cliptemp = 511 - (cliptemp+512);

		if (cliptemp > 127)
		  //			buffer[buf_count] = 127*256;
		  sample=32767;
		else if (cliptemp < -128)
		  //	buffer[buf_count] = -128*256;
		  sample=-32767;
		  
		else
		  //	buffer[buf_count] = cliptemp *256;
		  sample=cliptemp<<8;

		/* Update all counts */

		m_sample_count = (m_sample_count + 1) % 200;

		if (m_current_pitch != 0)
		{
			m_pitch_count++;
			if (m_pitch_count >= m_current_pitch)
				m_pitch_count = 0;
		}
		else
			m_pitch_count = 0;

		m_interp_count = (m_interp_count + 1) % 25;

		return sample;
		//		buf_count++;
		//		size--;
		//	}
}


void tms5100_init(){
   m_coeff = &T0280B_0281A_coeff; // this is for 5100!
   INT16 i,j;
  //  m_coeff=tms5220_coeff;
  // set up byte_rev
  for(i=0;i<256;i++)
    {
      j = (i>>4) | (i<<4); // Swap in groups of 4
      j = ((j & 0xcc)>>2) | ((j & 0x33)<<2); // Swap in groups of 2
      byte_rev[i] = ((j & 0xaa)>>1) | ((j & 0x55)<<1); // Swap bit pairs
    }

	m_speaking_now = m_talk_status = 1;
	m_RNG = 0x1fff;
	m_CTL_buffer = 0;
	//	m_PDC = 0;

	/* initialize the energy/pitch/k states */
	m_old_energy = m_new_energy = m_current_energy = m_target_energy = 0;
	m_old_pitch = m_new_pitch = m_current_pitch = m_target_pitch = 0;
	memset(m_old_k, 0, sizeof(m_old_k));
	memset(m_new_k, 0, sizeof(m_new_k));
	memset(m_current_k, 0, sizeof(m_current_k));
	memset(m_target_k, 0, sizeof(m_target_k));

	/* initialize the sample generators */
	m_interp_count = m_sample_count = m_pitch_count = 0;
	memset(m_x, 0, sizeof(m_x));
	m_next_is_address = FALSE;
	m_address = 0;
	if (m_table != NULL)
	{
		/* legacy interface */
		m_schedule_dummy_read = TRUE;
	}
	else
	{
		/* no dummy read! This makes bagman and ad2083 speech fail
		 * with the new cycle and transition exact interfaces
		 */
		m_schedule_dummy_read = FALSE;
	}
	m_addr_bit = 0;



}

void tms5100_newsay(){

  ptrAddr = sp_spk0352n9;
  ptrBit = 0;

}

//  tms5100_get_sample();


void main(){
  INT16 i, sample; u8 ending=0;
  // buffer?
  tms5100_init();
  tms5100_newsay();
  // ptraddr and speech data array
  //  m_IP=0;

  while(1){
    sample=  fiveoneprocess(&ending);
    printf("%c",(sample+32768)>>8);
    if (ending==1){
      ending=0; tms5100_newsay();
    }

  //  for (i=0;i<32768;i++) printf("%c",(speechbuffer[i]+32768)>>8);
  }
}
