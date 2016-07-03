// from tms5110r.hxx

struct tms5100_coeffs
{
	int             subtype;
	int             num_k;
	int             energy_bits;
	int             pitch_bits;
	int             kbits[MAX_K];
	unsigned short  energytable[MAX_SCALE];
	unsigned short  pitchtable[MAX_SCALE];
	int             ktable[MAX_K][MAX_SCALE];
	INT16           chirptable[MAX_CHIRP_SIZE];
	INT8            interp_coeff[8];
};

static const struct tms5100_coeffs tms5110a_coeff =
{
	/* subtype */
	SUBTYPE_5110,
	10,
	4,
	5,
	{ 5, 5, 4, 4, 4, 4, 4, 3, 3, 3 }, // kbits
	TI_028X_LATER_ENERGY
	TI_5110_PITCH
	{
	TI_5110_5220_LPC
	},
	TI_LATER_CHIRP
	TI_INTERP
};


#define TI_028X_LATER_ENERGY \
		/* E  */\
		{   0,  1,  2,  3,  4,  6,  8, 11, \
			16, 23, 33, 47, 63, 85,114, 0 },

#define TI_5110_PITCH \
	/* P */\
	{   0,  15,  16,  17,  19,  21,  22,  25,  \
		26,  29,  32,  36,  40,  42,  46,  50,  \
		55,  60,  64,  68,  72,  76,  80,  84,  \
		86,  93, 101, 110, 120, 132, 144, 159},

#define TI_LATER_CHIRP \
	/* Chirp table */\
	{   0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50,\
		0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13,\
		0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00 },

/* Interpolation Table */
#define TI_INTERP \
	/* interpolation shift coefficients */\
	{ 0, 3, 3, 3, 2, 2, 1, 1 }

// TODO: check against talkie?

#define TI_5110_5220_LPC \
		/* K1  */\
		{ -501, -498, -497, -495, -493, -491, -488, -482,\
			-478, -474, -469, -464, -459, -452, -445, -437,\
			-412, -380, -339, -288, -227, -158,  -81,   -1,\
			80,  157,  226,  287,  337,  379,  411,  436 },\
		/* K2  */\
		{ -328, -303, -274, -244, -211, -175, -138,  -99,\
			-59,  -18,   24,   64,  105,  143,  180,  215,\
			248,  278,  306,  331,  354,  374,  392,  408,\
			422,  435,  445,  455,  463,  470,  476,  506 },\
		/* K3  */\
		{ -441, -387, -333, -279, -225, -171, -117,  -63,\
			-9,   45,   98,  152,  206,  260,  314,  368  },\
		/* K4  */\
		{ -328, -273, -217, -161, -106,  -50,    5,   61,\
			116,  172,  228,  283,  339,  394,  450,  506  },\
		/* K5  */\
		{ -328, -282, -235, -189, -142,  -96,  -50,   -3,\
			43,   90,  136,  182,  229,  275,  322,  368  },\
		/* K6  */\
		{ -256, -212, -168, -123,  -79,  -35,   10,   54,\
			98,  143,  187,  232,  276,  320,  365,  409  },\
		/* K7  */\
		{ -308, -260, -212, -164, -117,  -69,  -21,   27,\
			75,  122,  170,  218,  266,  314,  361,  409  },\
		/* K8  */\
		{ -256, -161,  -66,   29,  124,  219,  314,  409  },\
		/* K9  */\
		{ -256, -176,  -96,  -15,   65,  146,  226,  307  },\
		/* K10 */\
		{ -205, -132,  -59,   14,   87,  160,  234,  307  },

// from 5110.c see also 5110.h

void tms5110_device::process(INT16 *buffer, unsigned int size)
{
	int buf_count=0;
	int i, interp_period, bitout;
	INT16 Y11, cliptemp;

	/* if we're not speaking, fill with nothingness */
	if (!m_speaking_now)
		goto empty;

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
	while ((size > 0) && m_speaking_now)
	{
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
				goto empty;
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
			buffer[buf_count] = 127*256;
		else if (cliptemp < -128)
			buffer[buf_count] = -128*256;
		else
			buffer[buf_count] = cliptemp *256;

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

		buf_count++;
		size--;
	}

empty:

	while (size > 0)
	{
		m_sample_count = (m_sample_count + 1) % 200;
		m_interp_count = (m_interp_count + 1) % 25;

		buffer[buf_count] = 0x00;
		buf_count++;
		size--;
	}
}


