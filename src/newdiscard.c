unsigned int new_parwave(klatt_global_ptr globals, klatt_frame_ptr frame, short *jwave, unsigned int klatthead)
{
	short ns;
	float out = 0.0;
	/* Output of cascade branch, also final output  */

	/* Initialize synthesizer and get specification for current speech
    frame from host microcomputer */

	frame_init(globals, frame);

	if (globals->f0_flutter != 0)
	{
		time_count++;                  /* used for f0 flutter */
		flutter(globals, frame);       /* add f0 flutter */
	}

	/* MAIN LOOP, for each output sample of current frame: */

	for (ns = 0; ns < globals->nspfr; ns++)
	{
		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75 * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++)
		{
				voice = natural_source(nper);

			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;
		out = resonator(&rout, out);//*8.0f; - why so quiet tho?
		//	*(jwave+klatthead) = clip(globals, out); /* Convert back to integer */
		//		*(jwave+klatthead) = rand()%32768;
				jwave[klatthead] = clip(globals, out); /* Convert back to integer */
		klatthead++;
		//		if (klatthead>=AUDIO_BUFSZ) klatthead=0;

	}
	return klatthead;
}

/*
   function PARWAV

   CONVERT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
   Synthesize globals->nspfr samples of waveform and store in jwave[].
 */

void parwave(klatt_global_ptr globals, klatt_frame_ptr frame, short *jwave)
{
	long ns;
	float out = 0.0f;
	/* Output of cascade branch, also final output  */

	/* Initialize synthesizer and get specification for current speech
    frame from host microcomputer */

	frame_init(globals, frame);

	if (globals->f0_flutter != 0)
	{
		time_count++;                  /* used for f0 flutter */
		flutter(globals, frame);       /* add f0 flutter */
	}

	/* MAIN LOOP, for each output sample of current frame: */

	for (ns = 0; ns < globals->nspfr; ns++)
	{
		static unsigned long seed = 5; /* Fixed staring value */
		float noise;
		int n4;
		float sourc;                   /* Sound source if all-parallel config used  */
		float glotout;                 /* Output of glottal sound source  */
		float par_glotout;             /* Output of parallelglottal sound sourc  */
		float voice;                   /* Current sample of voicing waveform  */
		float frics;                   /* Frication sound source  */
		float aspiration;              /* Aspiration sound source  */
		long nrand;                    /* Varible used by random number generator  */

		/* Our own code like rand(), but portable
		whole upper 31 bits of seed random 
		assumes 32-bit unsigned arithmetic
		with untested code to handle larger.
		*/
		seed = seed * 1664525 + 1;
		if (8 * sizeof(unsigned long) > 32)
			seed &= 0xFFFFFFFF;

		/* Shift top bits of seed up to top of long then back down to LS 14 bits */
		/* Assumes 8 bits per sizeof unit i.e. a "byte" */
		nrand = (((long) seed) << (8 * sizeof(long) - 32)) >> (8 * sizeof(long) - 14);

		/* Tilt down noise spectrum by soft low-pass filter having
		*    a pole near the origin in the z-plane, i.e.
		*    output = input + (0.75 * lastoutput) */

		noise = nrand + (0.75f * nlast);	/* Function of samp_rate ? */
		nlast = noise;

		/* Amplitude modulate noise (reduce noise amplitude during
		second half of glottal period) if voicing simultaneously present
		*/

		if (nper > nmod)
		{
			noise *= 0.5f;
		}

		/* Compute frication noise */
		sourc = frics = amp_frica * noise;

		/* Compute voicing waveform : (run glottal source simulation at
		4 times normal sample rate to minimize quantization noise in 
		period of female voice)
		*/

		for (n4 = 0; n4 < 4; n4++) // TODO ALL SOURCES as below
		{
		  //				voice = impulsive_source(nper);
				voice = natural_source(nper);
		

/*            Modify F1 and BW1 pitch synchrounously - from parwv.c */
/*
                if (nper == nopen) {
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz,B1hz);
                    }
                    F1hzmod = 0;                // Glottis closes 
                    B1hzmod = 0;
                }
                if (nper == T0) {
                    F1hzmod = dF1hz;            // opens
                    B1hzmod = dB1hz;
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz+F1hzmod,B1hz+B1hzmod);
                    }
                }
            }
*/

			/* Reset period when counter 'nper' reaches T0 */
			if (nper >= T0)
			{
				nper = 0;
				pitch_synch_par_reset(globals, frame, ns);
			}

			/* Low-pass filter voicing waveform before downsampling from 4*globals->samrate */
			/* to globals->samrate samples/sec.  Resonator f=.09*globals->samrate, bw=.06*globals->samrate  */

			voice = resonator(&rlp, voice);	/* in=voice, out=voice */

			/* Increment counter that keeps track of 4*globals->samrate samples/sec */
			nper++;
		}

		/* Tilt spectrum of voicing source down by soft low-pass filtering, amount
		of tilt determined by TLTdb
		*/
		voice = (voice * onemd) + (vlast * decay);
		vlast = voice;

		/* Add breathiness during glottal open phase */
		if (nper < nopen)
		{
			/* Amount of breathiness determined by parameter Aturb */
			/* Use nrand rather than noise because noise is low-passed */
			voice += amp_breth * nrand;
		}

		/* Set amplitude of voicing */
		glotout = amp_voice * voice;

		/* Compute aspiration amplitude and add to voicing source */
		aspiration = amp_aspir * noise;
		glotout += aspiration;

		par_glotout = glotout;

		if (globals->synthesis_model != ALL_PARALLEL)
		{
			/* Cascade vocal tract, excited by laryngeal sources.
			Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1
			*/
			float rnzout = antiresonator(&rnz, glotout);	/* Output of cascade nazal zero resonator  */
			float casc_next_in = resonator(&rnpc, rnzout);	/* in=rnzout, out=rnpc.p1 */

			/* Recoded from sequence of if's to use C's fall through switch
			semantics. May allow compiler to optimize
			*/
			switch (globals->nfcascade)
			{
				case 8:
					casc_next_in = resonator(&r8c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 7:
					casc_next_in = resonator(&r7c, casc_next_in);	/* Do not use unless samrat = 16000 */
				case 6:
					casc_next_in = resonator(&r6c, casc_next_in);	/* Do not use unless long vocal tract or samrat increased */
				case 5:
					casc_next_in = resonator(&r5c, casc_next_in);
				case 4:
					casc_next_in = resonator(&r4c, casc_next_in);
				case 3:
					casc_next_in = resonator(&r3c, casc_next_in);
				case 2:
					casc_next_in = resonator(&r2c, casc_next_in);
				case 1:
					out = resonator(&r1c, casc_next_in);
					break;
				default:
					out = 0.0f;
			}
			#if 0
			/* Excite parallel F1 and FNP by voicing waveform */
			/* Source is voicing plus aspiration */
			/* Add in phase, boost lows for nasalized */
			out += (resonator(&rnpp, par_glotout) + resonator(&r1p, par_glotout));
			#endif
		}
		else
		{
			/* Is ALL_PARALLEL */
			/* NIS - rsynth "hack"
			As Holmes' scheme is weak at nasals and (physically) nasal cavity
			is "back near glottis" feed glottal source through nasal resonators
			Don't think this is quite right, but improves things a bit
			*/
			par_glotout = antiresonator(&rnz, par_glotout);
			par_glotout = resonator(&rnpc, par_glotout);
			/* And just use r1p NOT rnpp */
			out = resonator(&r1p, par_glotout);
			/* Sound sourc for other parallel resonators is frication
			plus first difference of voicing waveform.
			*/
			sourc += (par_glotout - glotlast);
			glotlast = par_glotout;
		}

		/* Standard parallel vocal tract
		Formants F6,F5,F4,F3,F2, outputs added with alternating sign
		*/
		out = resonator(&r6p, sourc) - out;
		out = resonator(&r5p, sourc) - out;
		out = resonator(&r4p, sourc) - out;
		out = resonator(&r3p, sourc) - out;
		out = resonator(&r2p, sourc) - out;

		out = amp_bypas * sourc - out;

		out = resonator(&rout, out);
		*jwave++ = clip(globals, out); /* Convert back to integer */
	}
}


// other sources - eventually all excitations but just test here simple wavetable

// from klatt in docs

/*static float sampled_source(long nper)
{
  int itemp;
  float ftemp;
  float result;
  float diff_value;
  int current_value;
  int next_value;
  float temp_diff;

  if(T0!=0)
  {
    ftemp = (float) nper;
    ftemp = ftemp / T0;
    ftemp = ftemp * 100.0f;
    itemp = (int) ftemp;

    temp_diff = ftemp - (float) itemp;
  
    current_value = natural_samples[itemp];
    next_value = natural_samples[itemp+1];

    diff_value = (float) next_value - (float) current_value;
    diff_value = diff_value * temp_diff;

    result = natural_samples[itemp] + diff_value;
    result = result * 0.00005f;
    //    printf("xxx %f",result);
  }
  else
  {
    result = 0.0f;
  }
  return(result);
}
*



//extern Wavetable wavtable;
//extern __IO uint16_t adc_buffer[10];


static float wave_source(long nper) {
  float res;
  //  res=dosinglewavetable(&wavtable, (adc_buffer[SPEED]>>6)+(F0hz10/16.0)); // TODO FIX is we use
  return res*2048.0f;
}

static float triangular_source(long nper) {

/*    See if glottis open */
        if (nper < nopen) {
            if (nper < nfirsthalf) {
                vwave += slopet1;
                if (vwave > maxt1)    return(maxt1);
            }
            else {
                vwave += slopet2;
                if (vwave < maxt2)    return(maxt2);
            }
            return(vwave);
        }

/*    Glottis closed */
        else {
            return(0.);
        }
}




//static unsigned char rsynth_vocab_help[]={37, 10, 66, 4, 38, 8, 2, 8, 3, 1, 4, 2, 1, 6}; // TEST generated by   ~/rsync2016/projects/ERD_modules/worm/docs/rsynth-2005-12-16 ./say "help"

//static const vocab_t_ rsynth_test_comp={{146.300003, 108.000000, 133.339996}, {14, 8, 15, 1, 16, 4, 82, 6, 21, 8, 2, 8, 3, 1, 4, 2, 42, 7, 62, 9, 8, 6, 9, 1, 10, 2, 72, 4, 26, 10, 1, 6, 38, 8, 77, 6, 28, 5, 1, 6}, 40}; // last is length 

//static const vocab_t_ rsynth_test_worm={{146.300003, 137.000000, 129.860001}, {1, 8, 75, 16, 21, 8, 1, 6, 31, 12, 2, 8, 3, 1, 4, 2, 57, 7, 14, 8, 15, 1, 16, 4, 31, 12, 1, 6, 41, 8, 75, 16, 21, 8, 1, 6}, 36};

//static const vocab_t_ rsynth_test_help={{146.300003, 96.000000, 134.779999}, {37, 10, 66, 4, 38, 8, 2, 8, 3, 1, 4, 2, 1, 6, 21, 8, 57, 7, 1, 6, 2, 8, 3, 1, 4, 2, 38, 8, 57, 7, 32, 4, 1, 6}, 34}; 

//static const vocab_t_  *rsynth_vocab[]={&rsynth_test_comp, &rsynth_test_worm, &rsynth_test_help};


void digitalker_step_mode_0d()
{
	INT8 dac = 0;
	int i, k, l;
	UINT8 wpos = 0;
	UINT8 h = m_rom[m_apos];
	UINT16 bits = 0x80;
	UINT8 vol = h >> 5;
	UINT8 pitch_id = m_cur_segment ? digitalker_pitch_next(h, m_prev_pitch, m_cur_repeat) : h & 0x1f;
	m_pitch = pitch_vals[pitch_id];

	for(i=0; i<32; i++)
		m_dac[wpos++] = 0;

	for(k=1; k != 9; k++) {
		bits |= m_rom[m_apos+k] << 8;
		for(l=0; l<4; l++) {
		  #ifdef LAP
		  dac += delta1[(bits >> (6+2*l)) & 15];
		#else	
		  	u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
			MAXED(val,127);
			dac += delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
			#endif
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=0; l--) {
#ifndef LAP
		  u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
		  MAXED(val,127);
		  dac -= delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
		  #else
		    dac -= delta1[(bits >> (6+2*l)) & 15];
		  #endif
			digitalker_write(&wpos, vol, dac);
		}
	}

	for(i=0; i<31; i++)
		m_dac[wpos++] = 0;

	m_cur_repeat++;
	if(m_cur_repeat == m_repeats) {
		m_apos += 9;
		m_prev_pitch = pitch_id;
		m_cur_repeat = 0;
		m_cur_segment++;
	}
}

void digitalker_step_mode_3d()
{
	UINT8 h = m_rom[m_apos];
	UINT8 vol = h >> 5;
	UINT16 bits;
	UINT8 dac, apos, wpos;
	int k, l;

	m_pitch = pitch_vals[h & 0x1f];

	if(m_cur_segment == 0 && m_cur_repeat == 0) {
		m_cur_bits = 0x40;
		m_cur_dac = 0;
	}
	bits = m_cur_bits;
	dac = 0;

	apos = m_apos + 1 + 32*m_cur_segment;
	wpos = 0;
	for(k=0; k != 32; k++) {
		bits |= m_rom[apos++] << 8;
		for(l=0; l<4; l++) {
		  #ifdef LAP
		  dac += delta2[(bits >> (6+2*l)) & 15];
		  #else
		  u8 val=exy[16+((bits >> (6+2*l)) & 15)]*131.0f;
		  MAXED(val,127);
		  dac += delta2[(bits >> (6+2*l)) & 15]*logpitch[val];
		  #endif
		  digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	m_cur_bits = bits;
	m_cur_dac = dac;

	m_cur_segment++;
	if(m_cur_segment == m_segments) {
		m_cur_segment = 0;
		m_cur_repeat++;
	}
}



void digitalker_step_mode_2d()
{
	INT8 dac = 0;
	int k, l;
	UINT8 wpos=0;
	UINT8 h = m_rom[m_apos];
	UINT16 bits = 0x80;
	UINT8 vol = h >> 5;
	UINT8 pitch_id = m_cur_segment ? digitalker_pitch_next(h, m_prev_pitch, m_cur_repeat) : h & 0x1f;

	m_pitch = pitch_vals[pitch_id];
	for(k=1; k != 9; k++) {
		bits |= m_rom[m_apos+k] << 8;
		for(l=0; l<4; l++) {

#ifdef LAP
			dac += delta1[(bits >> (6+2*l)) & 15];
#else
			u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
			MAXED(val,127);
			dac += delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
			#endif
			digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
#ifdef LAP
		  dac -= delta1[(bits >> (6+2*l)) & 15];
		  
		  #else
		  u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
		  MAXED(val,127);
		  dac -= delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
		  #endif
		  digitalker_write(&wpos, vol, dac);
		}
	}

	digitalker_write(&wpos, vol, dac);

	for(k=1; k != 9; k++) {
		int start = k == 1 ? 1 : 0;
		bits |= m_rom[m_apos+k] << 8;
		for(l=start; l<4; l++) {
		  #ifdef LAP
		  dac += delta1[(bits >> (6+2*l)) & 15];
		  #else
		  u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
		  MAXED(val,127);
		  dac += delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
		  #endif
		  digitalker_write(&wpos, vol, dac);
		}
		bits >>= 8;
	}

	digitalker_write(&wpos, vol, dac);

	for(k=7; k >= 0; k--) {
		int limit = k ? 0 : 1;
		bits = (bits << 8) | (k ? m_rom[m_apos+k] : 0x80);
		for(l=3; l>=limit; l--) {
		  #ifdef LAP
		  dac -= delta1[(bits >> (6+2*l)) & 15];
		  #else
		  u8 val=exy[(bits >> (6+2*l)) & 15]*131.0f;
		  MAXED(val,127);
		  dac -= delta1[(bits >> (6+2*l)) & 15]*logpitch[val];
		  #endif
		  digitalker_write(&wpos, vol, dac);
		}
	}

	m_cur_repeat++;
	if(m_cur_repeat == m_repeats) {
		m_apos += 9;
		m_prev_pitch = pitch_id;
		m_cur_repeat = 0;
		m_cur_segment++;
	}
}


void digitalker_step_bendd()
{
	if(m_cur_segment == m_segments || m_cur_repeat == m_repeats) {
		if(m_stop_after == 0 && m_bpos == 0xffff)
			return;
		if(m_stop_after == 0) {
			UINT8 v1 = m_rom[m_bpos++];
			UINT8 v2 = m_rom[m_bpos++];
			UINT8 v3 = m_rom[m_bpos++];
			m_apos = v2 | ((v3 << 8) & 0x3f00);
			m_segments = (v1 & 15) + 1;
			m_repeats = ((v1 >> 4) & 7) + 1 ;
			m_mode = (v3 >> 6) & 3;
			m_stop_after = (v1 & 0x80) != 0;

			m_cur_segment = 0;
			m_cur_repeat = 0;

			if(!m_apos) {
				m_zero_count = 40*128*m_segments*m_repeats;
				m_segments = 0;
				m_repeats = 0;
				return;
			}
		} else if(m_stop_after == 1) {
			m_bpos = 0xffff;
			m_zero_count = 81920;
			m_stop_after = 2;
			m_cur_segment = 0;
			m_cur_repeat = 0;
			m_segments = 0;
			m_repeats = 0;
		} else {
			m_stop_after = 0;
			//			digitalker_set_intr(1);
		}
	}

	switch(m_mode) {
	case 0: digitalker_step_mode_0d(); break;
	case 1: digitalker_step_mode_1(); break;
	case 2: digitalker_step_mode_2d(); break;
	case 3: digitalker_step_mode_3d(); break;
	}
	if(!m_zero_count)
		m_dac_index = 0;
}


int16_t digitalk_get_sample_benddelta(){ 
  modus=1; // no changes
  int16_t sample; static int pp;

  for (u8 xx=0;xx<32;xx++){

	if(m_zero_count == 0 && m_dac_index == 128)
	  digitalker_step_bendd();

	if(m_zero_count) {
	  sample = 0;
	  m_zero_count -= 1;
	}
	else if(m_dac_index != 128) {
	    short v = m_dac[m_dac_index];
	    if (pp==m_pitch)
	      {
		pp=0;
		m_dac_index++;
	      }
	    else {
	      sample=v;
	      pp++;
	      //	      return sample;
	    }
	}
	else {
	  digitalk_newsay();
	  sample=0;
	}
  }
	return sample;
	}


unsigned holmes(unsigned nelm, unsigned char *elm, unsigned nsamp, short *samp_base)
{
	filter_t flt[nEparm];
	klatt_frame_t pars;
	short *samp = samp_base;
	Elm_ptr le = &Elements[0];
	unsigned i = 0;
	unsigned tstress = 0;
	unsigned ntstress = 0;
	slope_t stress_s;
	slope_t stress_e;
	float top = 1.1 * def_pars.F0hz10;
	int j;
	pars = def_pars;
	pars.FNPhz = le->p[fn].stdy;
	pars.B1phz = pars.B1hz = 60;
	pars.B2phz = pars.B2hz = 90;
	pars.B3phz = pars.B3hz = 150;
	#if 0
	pars.F4hz = 3500;
	#endif
	pars.B4phz = def_pars.B4phz;

	/* flag new utterance */
	parwave_init(&klatt_global);

	/* Set stress attack/decay slope */
	stress_s.t = 40;
	stress_e.t = 40;
	stress_e.v = 0.0f;

	for (j = 0; j < nEparm; j++)
	{
		flt[j].v = le->p[j].stdy;
		flt[j].a = frac;
		flt[j].b = (float) 1.0f - (float) frac;
	}
	while (i < nelm)
	{
		Elm_ptr ce = &Elements[elm[i++]];
		unsigned dur = elm[i++];
		i++; /* skip stress */
		/* Skip zero length elements which are only there to affect
		boundary values of adjacent elements
		*/
		if (dur > 0)
		{
			Elm_ptr ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
						slope_t startyy[nEparm];
						slope_t end[nEparm];
			unsigned t;

			if (ce->rk > le->rk)
			{
			  				set_trans(startyy, ce, le, 0, 's');
				/* we dominate last */
			}
			else
			{
			  				set_trans(startyy, le, ce, 1, 's');
				/* last dominates us */
			}

			if (ne->rk > ce->rk)
			{
			  				set_trans(end, ne, ce, 1, 'e');
				/* next dominates us */
			}
			else
			{
			  				set_trans(end, ce, ne, 0, 'e');
				/* we dominate next */
			}


			for (t = 0; t < dur; t++, tstress++)
			{
				float base = top * 0.8f /* 3 * top / 5 */;
				float tp[nEparm];
				int j;

				if (tstress == ntstress)
				{
					unsigned j = i;
					stress_s = stress_e;
					tstress = 0;
					ntstress = dur;
					#ifdef DEBUG_STRESS
					printf("Stress %g -> ", stress_s.v);
					#endif
					while (j <= nelm)
					{
						Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
						unsigned du = (j < nelm) ? elm[j++] : 0;
						unsigned s  = (j < nelm) ? elm[j++] : 3;
						if (s || e->feat & vwl)
						{
							unsigned d = 0;
							if (s)
								stress_e.v = (float) s / 3.0f;
							else
								stress_e.v = (float) 0.1f;
							do
							{
								d += du;
								#ifdef DEBUG_STRESS
								printf("%s", (e && e->dict) ? e->dict : "");
								#endif
								e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
								du = elm[j++];
							}
							while ((e->feat & vwl) && elm[j++] == s);
							ntstress += d / 2;
							break;
						}
						ntstress += du;
					}
					#ifdef DEBUG_STRESS
					printf(" %g @ %d\n", stress_e.v, ntstress);
					#endif
				}

				for (j = 0; j < nEparm; j++)
				  tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

				/* Now call the synth for each frame */

				pars.F0hz10 = base + (top - base) *
				interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

				pars.AVdb = pars.AVpdb = tp[av];
				pars.AF = tp[af];
				pars.FNZhz = tp[fn];
				pars.ASP = tp[asp];
				pars.Aturb = tp[avc];
				pars.B1phz = pars.B1hz = tp[b1];
				pars.B2phz = pars.B2hz = tp[b2];
				pars.B3phz = pars.B3hz = tp[b3];
				pars.F1hz = tp[f1];
				pars.F2hz = tp[f2];
				pars.F3hz = tp[f3];
				/* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
				Needs to be fixed properly in tables
				*/
				/*
				pars.ANP  = AMP_ADJ + tp[an];
				*/
				pars.AB = AMP_ADJ + tp[ab];
				pars.A5 = AMP_ADJ + tp[a5];
				pars.A6 = AMP_ADJ + tp[a6];
				pars.A1 = AMP_ADJ + tp[a1];
				pars.A2 = AMP_ADJ + tp[a2];
				pars.A3 = AMP_ADJ + tp[a3];
				pars.A4 = AMP_ADJ + tp[a4];

				parwave(&klatt_global, &pars, samp);

				samp += klatt_global.nspfr;
				/* Declination of f0 envelope 0.25Hz / cS */
				top -= 0.5;
}
}
		le = ce;
} 
	return (samp - samp_base);
}

// removed: &tuber, &tubsinger, &tubbender, &tubrawer, &tubxyer, 

// this is the shorter list of 43 modes 0-42 with composter as 42

//static const wormer *wormlist[]={&sp0256er, &sp0256TTSer, &sp0256vocaboneer, &sp0256vocabtwoer, &sp02561219er, &sp0256bender, &votraxer, &votraxTTSer, &votraxgorfer, &votraxwower, &votraxwowfilterbender, &votraxbender, &votraxparamer, &sambanks0er, &sambanks1er, &samTTSer, &samphoner, &samphonser,&samxyer, &samparamer, &sambender, &tmser, &tmsphoner, &tmsttser, &tmsbendlengther, &tmsraw5100er, &tmsraw5200er, &tmsraw5220er, &tmsbend5100er, &tmsbend5200er, &tms5100kandpitchtablebender, &tms5200kandpitchtablebender, &digitalker, &digitalker_bendpitchvals, &rsynthy, &rsynthelm, &rsynthsingle, &klatter, &klattsingle, &klattvocab, &nvper, &nvpvocaber, &composter};



//static const wormer tuber={0, 1.0f, tube_get_sample, tube_newsay, 0, 0}; // //60tubes=tube.c=tube_get_sample
//static const wormer tubsinger={0, 1.0f, tube_get_sample_sing, tube_newsay_sing, 0, 0};
//static const wormer tubbender={19, 1.0f, tube_get_sample_bend, tube_newsay_bend, 1, 0}; // now we add extra parameters 
//static const wormer tubrawer={19, 1.0f, tube_get_sample_raw, tube_newsay_raw, 1, 0};
//static const wormer tubxyer={4, 1.0f, tube_get_sample_xy, tube_newsay_xy, 1, 0};


/*int16_t compost_get_sample_frozen(){ // do we have a function for Z? - redo with toggle of freeze/unfreeze
 static u8 oldcompost=255, compostmode=255;
 doadc();
  u16 startx=(1.0f-_selx)*32767.0f;
  u16 endy=(1.0f-_sely)*32767.0f;
  signed char dir=1;

  // resets at start or end - newsay will reset in full
  if (startx>endy){
    dir=-1;
    if (comp_counter<=endy) comp_counter=startx; // swopped round
  }
  else {
    dir=1;
    if (comp_counter>=endy) comp_counter=startx;
  }

  int16_t pos=comp_counter; // restricted to 32768
  int16_t sample=audio_buffer[pos%32768];
  comp_counter+=dir;
  return sample;
  }*/


/// from samplerate

void samplerate(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio){
  float one_over_factor;
  float temp1=0.0f;
  static float time_now=0.0f;
  long j;
  long left_limit,right_limit,last_time=0;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;
  
  //  factor=32.0f;

for (u8 ii=0;ii<size;ii++){
  temp1 = 0.0f;

    if (time_now>32768){
        int_time=0; // preserve???
      time_now-=32768.0f;
   }

  // deal also with trigger
    if (in[ii]>THRESH && !triggered) {
    newsay();
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;
    
left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
 out[ii] = (int) temp1;
}
 else    {
   one_over_factor = 1.0f / factor;
   for (j=left_limit;j<right_limit;j++)
     temp1 += gimme_data(j-int_time) * one_over_factor *
       sinc(one_over_factor * (time_now - (float) j));
   out[ii] = (int) temp1;
}

time_now += factor;
last_time = int_time;
int_time = time_now;
while(last_time<int_time)      {
 doadc();
  int16_t val=getsample();
  new_data(val);
last_time += 1;
}
 }
}

void samplerate_exy(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio, u8 extent){
  static u8 parammode=0;
  float one_over_factor;
  float temp1=0.0f;
  static float time_now=0.0f;
  long j;
  long left_limit,right_limit,last_time=0;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;
 
for (u8 ii=0;ii<size;ii++){
  temp1 = 0.0f;
  // factor=1.0f;

  /* if (time_now>327680.0){
    int_time-=time_now; // preserve???
    time_now=0.0f;
    }*/

  // deal also with trigger
  if (in[ii]>THRESH && !triggered) {
    parammode^=1;
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;

left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
 out[ii] = (int) temp1;
}
 else    {
   one_over_factor = 1.0f / factor;
   for (j=left_limit;j<right_limit;j++)
     temp1 += gimme_data(j-int_time) * one_over_factor *
       sinc(one_over_factor * (time_now - (float) j));
   out[ii] = (int) temp1;
}

time_now += factor;
last_time = int_time;
int_time = time_now;
 doadc();

 if (parammode==0){
   u8 xaxis=_selx*((float)extent+4.0f); 
   MAXED(xaxis,extent);
   xaxis=extent-xaxis;
   //   exy[xaxis]=1.0f-_sely; // invert or?
   exy[xaxis]=_sely;
 }

while(last_time<int_time)      {
 doadc();
  int16_t val=getsample();
  new_data(val);
last_time += 1;
}
 }
}




float linear_interp(float first_number,float second_number,float fraction) // not used
{
    return (first_number + ((second_number - first_number) * fraction));
}

static inline float sinc(float x)
{
    int low;
    float temp;//,delta;
    if (fabsf(x)>=WIDTH-1)
	return 0.0f;
    else {
	temp = fabsf(x) * (float) SAMPLES_PER_ZERO_CROSSING;
	//	low = temp;          /* these are interpolation steps */
	//	delta = temp - low;  /* and can be ommited if desired */
	//	return linear_interp(sinc_table[low],sinc_table[low + 1],delta);
	//		return sinc_table[(int)temp];
    }
}

static float InterpolateHermite4pt3oX(int16_t x0, int16_t x1, int16_t x2, int16_t x3, float t)
{
    float c0 = x1;
    float c1 = .5F * (x2 - x0);
    float c2 = x0 - (2.5F * x1) + (2 * x2) - (.5F * x3);
    float c3 = (.5F * (x3 - x0)) + (1.5F * (x1 - x2));
    return (((((c3 * t) + c2) * t) + c1) * t) + c0;
}



/*void klatt_newsayTTS(){
//static signed char testphone[]="sEHEHvEHEHntIYIY sEHEHvAXAXn THTHAWAWzAEAEnd sEHEHvAXAXn hAHAHndrEHEHd sEHEHvEHEHntIYIY sEHEHvAXAXn";
darray_free(&wav_elm);

u8 lenny=text2speechforklatt(32,TTSinarray,TTSoutarray); // restrict TTS to just ascii
if (lenny>32) lenny=32;
PhonemeToWaveData(TTSoutarray, lenny); // only works out if restricted  to say 32 - NOT 64 _ STRESS TESTING - still crashes on 32
//PhonemeToWaveData(testphone, 16);

elmer=(unsigned char *) darray_find(&wav_elm, 0); // is our list of phonemes in order phon_number, duration, stress - we cycle through it
i=0; 
le = &Elements[0];
top = 1.1f * def_pars.F0hz10;
u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
top*=logpitch[val];

    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0f;

    for (u8 j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0f - (float) frac;
      }
    nextelement=1;
}
*/

/*int16_t klatt_get_sampleTTS(){
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  int16_t sample=0;
  unsigned nelm=wav_elm.items; // 10 phonemes = how many frames approx ???? - in test case we have 87 frames - now 16 phonemes
  u8 j; 
  unsigned char *elm=elmer;

  static u8 dur,first=0;
  static slope_t startyy[nEparm];
  static slope_t end[nEparm];
  if (i>nelm && nextelement==1){   // NEW utterance which means we hit nelm=0 in our cycling:
    klatt_newsayTTS();
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elmer[i++]];
    dur = elm[i++];
    i++; 
    if (dur == 0) { // do what? NOTHING
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	  }
	// next set of frames what do we need to init?
	t=0;
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
	newframe=1;
      } // if dur==0
  }
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<=dur){ //
                  float base = top * 0.8f /* 3 * top / 5 */;
      //      float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3.0f;
		  else
		    stress_e.v = (float) 0.1f;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    } // if t<dur
    else { // hit end of DUR number of frames...
      nextelement=1;
      le = ce; // where we can put this?????? TODO!!!
      klatt_get_sampleTTS();
    }
  }
//  if (nextelement==0){// causes clicks
    // always run through samples till we hit next frame
    //    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); 
    sample=parwavesinglesample(&klatt_global, &pars, samplenumber); 
    
    ///x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>=klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; // where we can put this?
//    }
  }
  return sample;
}
*/


// this is for width 16, crossing 8 = length 128

static const float sinc_table[]={1.0f, 0.974349, 0.899774, 0.783151, 0.635087, 0.468759, 0.298481, 0.138189, -0.000000, -0.106962, -0.177365, -0.210003, -0.207638, -0.176405, -0.124857, -0.062790, 0.000000, 0.054864, 0.095233, 0.117213, 0.119807, 0.104753, 0.076025, 0.039083, -0.000000, -0.035425, -0.062441, -0.077914, -0.080624, -0.071277, -0.052247, -0.027101, 0.000000, 0.024946, 0.044263, 0.055562, 0.057805, 0.051352, 0.037806, 0.019687, 0.000000, -0.018242, -0.032456, -0.040840, -0.042578, -0.037893, -0.027940, -0.014568, 0.000000, 0.013523, 0.024072, 0.030300, 0.031593, 0.028114, 0.020724, 0.010800, 0.000000, -0.010010, -0.017800, -0.022378, -0.023301, -0.020703, -0.015234, -0.007924, 0.000000, 0.007312, 0.012972, 0.016265, 0.016889, 0.014961, 0.010974, 0.005689, -0.000000, -0.005212, -0.009210, -0.011501, -0.011890, -0.010485, -0.007654, -0.003948, -0.000000, 0.003578, 0.006285, 0.007800, 0.008012, 0.007018, 0.005087, 0.002604, 0.000000, -0.002322, -0.004044, -0.004974, -0.005060, -0.004387, -0.003146, -0.001592, 0.000000, 0.001385, 0.002380, 0.002885, 0.002890, 0.002465, 0.001737, 0.000863, -0.000000, -0.000720, -0.001208, -0.001428, -0.001392, -0.001152, -0.000786, -0.000377, -0.000000, 0.000289, 0.000462, 0.000516, 0.000473, 0.000364, 0.000229, 0.000099, -0.000000, -0.000059, -0.000080, -0.000072, -0.000049, -0.000025, -0.000009, -0.000001};

// width 15, crossing 32

//static const float sinc_table[]={1.0f, 0.998385, 0.993549, 0.985522, 0.974349, 0.960096, 0.942846, 0.922700, 0.899774, 0.874202, 0.846131, 0.815723, 0.783151, 0.748601, 0.712270, 0.674361, 0.635087, 0.594666, 0.553320, 0.511275, 0.468759, 0.425997, 0.383216, 0.340638, 0.298481, 0.256958, 0.216272, 0.176620, 0.138189, 0.101154, 0.065680, 0.031916, 0.000000, -0.029945, -0.057813, -0.083510, -0.106962, -0.128108, -0.146906, -0.163328, -0.177365, -0.189022, -0.198320, -0.205296, -0.210003, -0.212505, -0.212881, -0.211225, -0.207638, -0.202235, -0.195139, -0.186483, -0.176405, -0.165052, -0.152573, -0.139123, -0.124857, -0.109935, -0.094512, -0.078746, -0.062790, -0.046794, -0.030905, -0.015263, -0.000000, 0.014757, 0.028891, 0.042293, 0.054864, 0.066516, 0.077171, 0.086762, 0.095233, 0.102540, 0.108651, 0.113545, 0.117212, 0.119655, 0.120884, 0.120924, 0.119807, 0.117576, 0.114282, 0.109986, 0.104753, 0.098659, 0.091782, 0.084208, 0.076025, 0.067325, 0.058204, 0.048758, 0.039083, 0.029275, 0.019430, 0.009642, 0.000000, -0.009408, -0.018499, -0.027195, -0.035425, -0.043120, -0.050222, -0.056678, -0.062441, -0.067473, -0.071743, -0.075228, -0.077914, -0.079792, -0.080863, -0.081136, -0.080624, -0.079350, -0.077344, -0.074639, -0.071277, -0.067304, -0.062771, -0.057732, -0.052247, -0.046376, -0.040185, -0.033737, -0.027101, -0.020343, -0.013530, -0.006727, -0.000000, 0.006589, 0.012981, 0.019117, 0.024946, 0.030418, 0.035487, 0.040114, 0.044263, 0.047904, 0.051013, 0.053570, 0.055562, 0.056981, 0.057825, 0.058096, 0.057805, 0.056964, 0.055591, 0.053712, 0.051352, 0.048545, 0.045325, 0.041732, 0.037806, 0.033592, 0.029136, 0.024485, 0.019687, 0.014791, 0.009846, 0.004900, 0.000000, -0.004807, -0.009478, -0.013969, -0.018242, -0.022259, -0.025987, -0.029395, -0.032456, -0.035149, -0.037453, -0.039353, -0.040840, -0.041906, -0.042550, -0.042772, -0.042578, -0.041979, -0.040986, -0.039618, -0.037893, -0.035836, -0.033473, -0.030830, -0.027940, -0.024835, -0.021547, -0.018113, -0.014568, -0.010948, -0.007290, -0.003629, -0.000000, 0.003562, 0.007023, 0.010353, 0.013523, 0.016503, 0.019269, 0.021799, 0.024072, 0.026072, 0.027783, 0.029195, 0.030300, 0.031093, 0.031571, 0.031737, 0.031593, 0.031148, 0.030411, 0.029395, 0.028114, 0.026587, 0.024831, 0.022869, 0.020724, 0.018418, 0.015978, 0.013430, 0.010800, 0.008115, 0.005403, 0.002689, 0.000000, -0.002638, -0.005201, -0.007666, -0.010010, -0.012213, -0.014256, -0.016124, -0.017800, -0.019273, -0.020532, -0.021569, -0.022378, -0.022956, -0.023301, -0.023415, -0.023301, -0.022964, -0.022412, -0.021654, -0.020703, -0.019569, -0.018270, -0.016819, -0.015234, -0.013533, -0.011735, -0.009858, -0.007924, -0.005951, -0.003960, -0.001970, -0.000000, 0.001930, 0.003804, 0.005603, 0.007312, 0.008916, 0.010402, 0.011757, 0.012972, 0.014036, 0.014943, 0.015688, 0.016265, 0.016674, 0.016913, 0.016984, 0.016889, 0.016633, 0.016221, 0.015661, 0.014961, 0.014132, 0.013183, 0.012126, 0.010974, 0.009741, 0.008440, 0.007084, 0.005689, 0.004269, 0.002838, 0.001411, 0.000000, -0.001380, -0.002717, -0.003998, -0.005212, -0.006350, -0.007400, -0.008356, -0.009210, -0.009956, -0.010589, -0.011104, -0.011501, -0.011777, -0.011933, -0.011970, -0.011890, -0.011696, -0.011394, -0.010988, -0.010485, -0.009892, -0.009216, -0.008467, -0.007654, -0.006785, -0.005871, -0.004922, -0.003948, -0.002959, -0.001964, -0.000975, -0.000000, 0.000951, 0.001870, 0.002748, 0.003578, 0.004352, 0.005065, 0.005711, 0.006285, 0.006784, 0.007204, 0.007543, 0.007800, 0.007975, 0.008068, 0.008080, 0.008012, 0.007869, 0.007652, 0.007367, 0.007018, 0.006609, 0.006147, 0.005638, 0.005087, 0.004501, 0.003888, 0.003253, 0.002604, 0.001948, 0.001291, 0.000639, 0.000000, -0.000621, -0.001219, -0.001787, -0.002322, -0.002819, -0.003274, -0.003683, -0.004044, -0.004356, -0.004615, -0.004821, -0.004974, -0.005073, -0.005120, -0.005115, -0.005060, -0.004957, -0.004808, -0.004617, -0.004387, -0.004121, -0.003822, -0.003496, -0.003146, -0.002776, -0.002391, -0.001995, -0.001592, -0.001187, -0.000785, -0.000387, -0.000000, 0.000374, 0.000732, 0.001070, 0.001385, 0.001676, 0.001940, 0.002175, 0.002380, 0.002554, 0.002696, 0.002807, 0.002885, 0.002932, 0.002947, 0.002933, 0.002890, 0.002820, 0.002724, 0.002605, 0.002465, 0.002306, 0.002129, 0.001939, 0.001737, 0.001526, 0.001308, 0.001086, 0.000863, 0.000640, 0.000421, 0.000207, 0.000000, -0.000198, -0.000385, -0.000559, -0.000720, -0.000866, -0.000997, -0.001111, -0.001208, -0.001289, -0.001352, -0.001399, -0.001428, -0.001442, -0.001440, -0.001423, -0.001392, -0.001348, -0.001293, -0.001227, -0.001152, -0.001069, -0.000980, -0.000885, -0.000786, -0.000684, -0.000581, -0.000479, -0.000377, -0.000277, -0.000180, -0.000088, -0.000000, 0.000082, 0.000158, 0.000227, 0.000289, 0.000344, 0.000391, 0.000430, 0.000462, 0.000486, 0.000503, 0.000513, 0.000516, 0.000513, 0.000505, 0.000491, 0.000473, 0.000450, 0.000424, 0.000395, 0.000364, 0.000331, 0.000298, 0.000263, 0.000229, 0.000195, 0.000162, 0.000130, 0.000099, 0.000071, 0.000045, 0.000021, 0.000000, -0.000019, -0.000035, -0.000048, -0.000059, -0.000068, -0.000074, -0.000078, -0.000080, -0.000080, -0.000079, -0.000076, -0.000072, -0.000067, -0.000062, -0.000056, -0.000049, -0.000043, -0.000037, -0.000031, -0.000025, -0.000020, -0.000016, -0.000012, -0.000009, -0.000006, -0.000004, -0.000002, -0.000001, -0.000000, -0.000000, -0.000000};

//static const float sinc_table[]={1.0f, 0.998394, 0.993585, 0.985600, 0.974486, 0.960307, 0.943145, 0.923099, 0.900282, 0.874827, 0.846878, 0.816594, 0.784147, 0.749719, 0.713503, 0.675702, 0.636524, 0.596185, 0.554905, 0.512908, 0.470417, 0.427659, 0.384858, 0.342233, 0.300004, 0.258380, 0.217567, 0.177761, 0.139149, 0.101909, 0.066204, 0.032188, 0.000000, -0.030235, -0.058407, -0.084420, -0.108195, -0.129669, -0.148794, -0.165541, -0.179894, -0.191855, -0.201440, -0.208684, -0.213633, -0.216349, -0.216908, -0.215398, -0.211919, -0.206583, -0.199510, -0.190832, -0.180685, -0.169214, -0.156570, -0.142906, -0.128379, -0.113150, -0.097376, -0.081217, -0.064829, -0.048366, -0.031979, -0.015811, -0.000000, 0.015322, 0.030032, 0.044015, 0.057167, 0.069394, 0.080610, 0.090743, 0.099730, 0.107523, 0.114081, 0.119380, 0.123403, 0.126147, 0.127622, 0.127844, 0.126845, 0.124664, 0.121350, 0.116962, 0.111566, 0.105236, 0.098052, 0.090101, 0.081475, 0.072268, 0.062579, 0.052508, 0.042159, 0.031632, 0.021030, 0.010454, 0.000000, -0.010236, -0.020163, -0.029695, -0.038751, -0.047256, -0.055141, -0.062346, -0.068815, -0.074503, -0.079372, -0.083391, -0.086538, -0.088801, -0.090175, -0.090663, -0.090276, -0.089035, -0.086965, -0.084102, -0.080485, -0.076163, -0.071187, -0.065617, -0.059514, -0.052945, -0.045980, -0.038691, -0.031152, -0.023438, -0.015624, -0.007787, -0.000000, 0.007664, 0.015134, 0.022344, 0.029229, 0.035728, 0.041787, 0.047354, 0.052386, 0.056841, 0.060687, 0.063895, 0.066446, 0.068325, 0.069522, 0.070038, 0.069876, 0.069048, 0.067572, 0.065469, 0.062769, 0.059506, 0.055718, 0.051448, 0.046744, 0.041656, 0.036236, 0.030542, 0.024631, 0.018561, 0.012393, 0.006186, 0.000000, -0.006107, -0.012078, -0.017858, -0.023394, -0.028637, -0.033541, -0.038063, -0.042164, -0.045812, -0.048977, -0.051635, -0.053766, -0.055357, -0.056398, -0.056888, -0.056826, -0.056221, -0.055085, -0.053434, -0.051291, -0.048680, -0.045634, -0.042184, -0.038370, -0.034231, -0.029810, -0.025152, -0.020306, -0.015318, -0.010238, -0.005116, -0.000000, 0.005060, 0.010018, 0.014826, 0.019442, 0.023821, 0.027927, 0.031721, 0.035172, 0.038250, 0.040929, 0.043188, 0.045010, 0.046382, 0.047295, 0.047746, 0.047735, 0.047266, 0.046349, 0.044996, 0.043226, 0.041059, 0.038519, 0.035636, 0.032438, 0.028961, 0.025240, 0.021312, 0.017218, 0.012999, 0.008694, 0.004347, 0.000000, -0.004306, -0.008531, -0.012635, -0.016579, -0.020327, -0.023847, -0.027105, -0.030073, -0.032725, -0.035040, -0.036998, -0.038583, -0.039784, -0.040592, -0.041004, -0.041019, -0.040641, -0.039876, -0.038735, -0.037233, -0.035386, -0.033217, -0.030747, -0.028004, -0.025016, -0.021814, -0.018430, -0.014897, -0.011252, -0.007530, -0.003767, -0.000000, 0.003736, 0.007405, 0.010972, 0.014404, 0.017670, 0.020739, 0.023584, 0.026179, 0.028502, 0.030532, 0.032253, 0.033651, 0.034715, 0.035436, 0.035813, 0.035842, 0.035527, 0.034874, 0.033891, 0.032591, 0.030988, 0.029101, 0.026949, 0.024555, 0.021944, 0.019143, 0.016180, 0.013084, 0.009887, 0.006619, 0.003313, 0.000000, -0.003288, -0.006519, -0.009663, -0.012691, -0.015574, -0.018286, -0.020802, -0.023100, -0.025159, -0.026961, -0.028491, -0.029737, -0.030688, -0.031337, -0.031680, -0.031717, -0.031450, -0.030882, -0.030022, -0.028880, -0.027469, -0.025805, -0.023904, -0.021788, -0.019478, -0.016997, -0.014371, -0.011625, -0.008787, -0.005885, -0.002946, -0.000000, 0.002926, 0.005803, 0.008604, 0.011303, 0.013875, 0.016296, 0.018544, 0.020598, 0.022441, 0.024055, 0.025428, 0.026546, 0.027403, 0.027990, 0.028305, 0.028346, 0.028114, 0.027614, 0.026853, 0.025838, 0.024582, 0.023099, 0.021403, 0.019514, 0.017449, 0.015230, 0.012880, 0.010422, 0.007880, 0.005278, 0.002643, 0.000000, -0.002626, -0.005210, -0.007727, -0.010153, -0.012466, -0.014645, -0.016669, -0.018520, -0.020181, -0.021638, -0.022878, -0.023890, -0.024666, -0.025201, -0.025490, -0.025532, -0.025329, -0.024884, -0.024203, -0.023293, -0.022166, -0.020832, -0.019307, -0.017606, -0.015747, -0.013747, -0.011628, -0.009411, -0.007117, -0.004768, -0.002388, -0.000000, 0.002374, 0.004710, 0.006987, 0.009183, 0.011277, 0.013250, 0.015084, 0.016762, 0.018269, 0.019592, 0.020718, 0.021638, 0.022345, 0.022834, 0.023100, 0.023142, 0.022962, 0.022563, 0.021949, 0.021128, 0.020108, 0.018902, 0.017521, 0.015980, 0.014295, 0.012482, 0.010560, 0.008547, 0.006465, 0.004332, 0.002170, 0.000000, -0.002158, -0.004282, -0.006353, -0.008351, -0.010257, -0.012053, -0.013724, -0.015253, -0.016627, -0.017833, -0.018861, -0.019702, -0.020349, -0.020796, -0.021042, -0.021083, -0.020922, -0.020561, -0.020004, -0.019259, -0.018332, -0.017235, -0.015978, -0.014575, -0.013039, -0.011387, -0.009635, -0.007800, -0.005900, -0.003954, -0.001981, -0.000000, 0.001970, 0.003910, 0.005802, 0.007628, 0.009371, 0.011013, 0.012541, 0.013940, 0.015198, 0.016302, 0.017244, 0.018015, 0.018609, 0.019020, 0.019247, 0.019287, 0.019142, 0.018814, 0.018307, 0.017627, 0.016781, 0.015778, 0.014629, 0.013346, 0.011941, 0.010429, 0.008825, 0.007145, 0.005405, 0.003623, 0.001815, 0.000000, -0.001806, -0.003585, -0.005319, -0.006994, -0.008592, -0.010100, -0.011502, -0.012786, -0.013941, -0.014956, -0.015821, -0.016530, -0.017077, -0.017456, -0.017666, -0.017705, -0.017573, -0.017273, -0.016809, -0.016186, -0.015411, -0.014491, -0.013437, -0.012260, -0.010970, -0.009582, -0.008109, -0.006566, -0.004968, -0.003330, -0.001669, -0.000000, 0.001660, 0.003296, 0.004891, 0.006431, 0.007902, 0.009289, 0.010580, 0.011762, 0.012825, 0.013760, 0.014558, 0.015211, 0.015715, 0.016066, 0.016260, 0.016297, 0.016177, 0.015902, 0.015476, 0.014904, 0.014191, 0.013345, 0.012375, 0.011292, 0.010105, 0.008827, 0.007471, 0.006049, 0.004577, 0.003068, 0.001538, 0.000000, -0.001530, -0.003038, -0.004509, -0.005929, -0.007285, -0.008564, -0.009755, -0.010846, -0.011827, -0.012690, -0.013426, -0.014030, -0.014495, -0.014820, -0.015000, -0.015035, -0.014925, -0.014673, -0.014281, -0.013753, -0.013096, -0.012316, -0.011422, -0.010422, -0.009327, -0.008148, -0.006897, -0.005585, -0.004226, -0.002833, -0.001420, -0.000000, 0.001413, 0.002805, 0.004164, 0.005476, 0.006729, 0.007911, 0.009011, 0.010020, 0.010927, 0.011724, 0.012406, 0.012964, 0.013395, 0.013695, 0.013863, 0.013896, 0.013795, 0.013563, 0.013201, 0.012714, 0.012107, 0.011386, 0.010560, 0.009636, 0.008625, 0.007535, 0.006378, 0.005165, 0.003908, 0.002620, 0.001313, 0.000000, -0.001307, -0.002595, -0.003852, -0.005066, -0.006225, -0.007319, -0.008338, -0.009271, -0.010111, -0.010849, -0.011480, -0.011997, -0.012397, -0.012675, -0.012830, -0.012861, -0.012769, -0.012554, -0.012219, -0.011769, -0.011208, -0.010541, -0.009777, -0.008922, -0.007985, -0.006976, -0.005905, -0.004783, -0.003619, -0.002427, -0.001216, -0.000000, 0.001210, 0.002404, 0.003568, 0.004692, 0.005766, 0.006780, 0.007723, 0.008588, 0.009366, 0.010051, 0.010635, 0.011115, 0.011485, 0.011744, 0.011888, 0.011917, 0.011832, 0.011633, 0.011323, 0.010906, 0.010386, 0.009769, 0.009061, 0.008269, 0.007401, 0.006466, 0.005474, 0.004433, 0.003355, 0.002249, 0.001127, 0.000000, -0.001122, -0.002228, -0.003308, -0.004350, -0.005346, -0.006286, -0.007161, -0.007963, -0.008684, -0.009319, -0.009861, -0.010306, -0.010650, -0.010890, -0.011024, -0.011051, -0.010972, -0.010788, -0.010501, -0.010115, -0.009633, -0.009060, -0.008404, -0.007669, -0.006864, -0.005997, -0.005077, -0.004112, -0.003112, -0.002086, -0.001046, -0.000000, 0.001041, 0.002067, 0.003068, 0.004036, 0.004959, 0.005831, 0.006643, 0.007387, 0.008057, 0.008646, 0.009149, 0.009562, 0.009881, 0.010104, 0.010228, 0.010254, 0.010181, 0.010010, 0.009744, 0.009385, 0.008938, 0.008407, 0.007798, 0.007117, 0.006370, 0.005565, 0.004711, 0.003816, 0.002888, 0.001936, 0.000971, 0.000000, -0.000966, -0.001918, -0.002848, -0.003745, -0.004603, -0.005412, -0.006165, -0.006856, -0.007478, -0.008024, -0.008492, -0.008875, -0.009171, -0.009378, -0.009493, -0.009517, -0.009449, -0.009291, -0.009044, -0.008711, -0.008296, -0.007803, -0.007238, -0.006605, -0.005912, -0.005166, -0.004373, -0.003542, -0.002680, -0.001797, -0.000901, -0.000000, 0.000897, 0.001781, 0.002643, 0.003476, 0.004272, 0.005023, 0.005723, 0.006364, 0.006941, 0.007449, 0.007882, 0.008238, 0.008513, 0.008705, 0.008812, 0.008834, 0.008771, 0.008624, 0.008395, 0.008086, 0.007701, 0.007243, 0.006718, 0.006131, 0.005488, 0.004795, 0.004059, 0.003287, 0.002488, 0.001668, 0.000836, 0.000000, -0.000832, -0.001653, -0.002453, -0.003227, -0.003966, -0.004663, -0.005312, -0.005907, -0.006442, -0.006913, -0.007316, -0.007646, -0.007901, -0.008079, -0.008179, -0.008199, -0.008141, -0.008004, -0.007791, -0.007505, -0.007147, -0.006723, -0.006235, -0.005690, -0.005093, -0.004450, -0.003767, -0.003051, -0.002309, -0.001548, -0.000776, -0.000000, 0.000772, 0.001534, 0.002277, 0.002994, 0.003680, 0.004327, 0.004929, 0.005481, 0.005978, 0.006415, 0.006789, 0.007095, 0.007332, 0.007497, 0.007589, 0.007608, 0.007553, 0.007427, 0.007229, 0.006963, 0.006631, 0.006237, 0.005785, 0.005279, 0.004725, 0.004128, 0.003495, 0.002830, 0.002142, 0.001436, 0.000720, 0.000000, -0.000716, -0.001423, -0.002112, -0.002778, -0.003413, -0.004013, -0.004572, -0.005084, -0.005545, -0.005950, -0.006296, -0.006580, -0.006800, -0.006953, -0.007038, -0.007055, -0.007005, -0.006887, -0.006704, -0.006457, -0.006149, -0.005784, -0.005364, -0.004895, -0.004382, -0.003828, -0.003240, -0.002624, -0.001986, -0.001332, -0.000667, -0.000000, 0.000664, 0.001319, 0.001958, 0.002575, 0.003164, 0.003721, 0.004238, 0.004713, 0.005140, 0.005515, 0.005836, 0.006099, 0.006302, 0.006444, 0.006523, 0.006539, 0.006492, 0.006383, 0.006213, 0.005984, 0.005698, 0.005360, 0.004971, 0.004536, 0.004060, 0.003547, 0.003002, 0.002432, 0.001840, 0.001234, 0.000618, 0.000000, -0.000615, -0.001222, -0.001814, -0.002385, -0.002931, -0.003446, -0.003926, -0.004365, -0.004761, -0.005108, -0.005405, -0.005649, -0.005837, -0.005968, -0.006041, -0.006055, -0.006012, -0.005911, -0.005753, -0.005541, -0.005276, -0.004962, -0.004602, -0.004200, -0.003759, -0.003284, -0.002780, -0.002251, -0.001703, -0.001142, -0.000572, -0.000000, 0.000570, 0.001131, 0.001679, 0.002208, 0.002713, 0.003189, 0.003633, 0.004040, 0.004405, 0.004727, 0.005001, 0.005227, 0.005400, 0.005521, 0.005589, 0.005602, 0.005561, 0.005468, 0.005322, 0.005125, 0.004880, 0.004590, 0.004257, 0.003884, 0.003476, 0.003037, 0.002570, 0.002082, 0.001575, 0.001056, 0.000529, 0.000000, -0.000527, -0.001046, -0.001552, -0.002041, -0.002508, -0.002948, -0.003358, -0.003734, -0.004072, -0.004369, -0.004622, -0.004830, -0.004991, -0.005102, -0.005164, -0.005177, -0.005139, -0.005052, -0.004917, -0.004735, -0.004509, -0.004240, -0.003932, -0.003588, -0.003211, -0.002805, -0.002374, -0.001923, -0.001455, -0.000975, -0.000489, -0.000000, 0.000486, 0.000965, 0.001433, 0.001884, 0.002315, 0.002722, 0.003100, 0.003447, 0.003759, 0.004033, 0.004266, 0.004458, 0.004606, 0.004709, 0.004766, 0.004777, 0.004742, 0.004661, 0.004537, 0.004369, 0.004160, 0.003912, 0.003628, 0.003310, 0.002962, 0.002587, 0.002190, 0.001773, 0.001342, 0.000899, 0.000451, 0.000000, -0.000448, -0.000890, -0.001321, -0.001737, -0.002134, -0.002509, -0.002858, -0.003177, -0.003464, -0.003717, -0.003932, -0.004109, -0.004245, -0.004339, -0.004392, -0.004401, -0.004369, -0.004295, -0.004179, -0.004024, -0.003832, -0.003603, -0.003341, -0.003048, -0.002728, -0.002383, -0.002016, -0.001633, -0.001235, -0.000828, -0.000415, -0.000000, 0.000413, 0.000819, 0.001216, 0.001599, 0.001964, 0.002309, 0.002630, 0.002924, 0.003188, 0.003420, 0.003618, 0.003780, 0.003905, 0.003992, 0.004040, 0.004048, 0.004018, 0.003950, 0.003844, 0.003701, 0.003524, 0.003313, 0.003072, 0.002803, 0.002508, 0.002190, 0.001854, 0.001501, 0.001135, 0.000761, 0.000381, 0.000000, -0.000379, -0.000753, -0.001117, -0.001469, -0.001805, -0.002121, -0.002416, -0.002685, -0.002928, -0.003141, -0.003322, -0.003471, -0.003586, -0.003665, -0.003709, -0.003717, -0.003689, -0.003626, -0.003528, -0.003397, -0.003234, -0.003041, -0.002819, -0.002572, -0.002301, -0.002010, -0.001701, -0.001377, -0.001041, -0.000698, -0.000350, -0.000000, 0.000348, 0.000690, 0.001025, 0.001347, 0.001655, 0.001945, 0.002215, 0.002462, 0.002684, 0.002879, 0.003045, 0.003181, 0.003286, 0.003358, 0.003398, 0.003405, 0.003379, 0.003321, 0.003231, 0.003111, 0.002962, 0.002784, 0.002581, 0.002355, 0.002107, 0.001840, 0.001557, 0.001260, 0.000953, 0.000639, 0.000320, 0.000000, -0.000318, -0.000632, -0.000937, -0.001232, -0.001514, -0.001779, -0.002025, -0.002251, -0.002454, -0.002632, -0.002784, -0.002908, -0.003004, -0.003070, -0.003106, -0.003112, -0.003088, -0.003035, -0.002953, -0.002843, -0.002706, -0.002544, -0.002358, -0.002151, -0.001924, -0.001680, -0.001422, -0.001151, -0.000870, -0.000583, -0.000292, -0.000000, 0.000291, 0.000577, 0.000855, 0.001124, 0.001381, 0.001623, 0.001848, 0.002054, 0.002239, 0.002401, 0.002539, 0.002652, 0.002739, 0.002799, 0.002832, 0.002837, 0.002815, 0.002766, 0.002691, 0.002591, 0.002466, 0.002318, 0.002148, 0.001959, 0.001753, 0.001530, 0.001295, 0.001048, 0.000793, 0.000531, 0.000266, 0.000000, -0.000264, -0.000525, -0.000779, -0.001023, -0.001257, -0.001477, -0.001681, -0.001868, -0.002036, -0.002184, -0.002309, -0.002412, -0.002491, -0.002545, -0.002574, -0.002579, -0.002559, -0.002514, -0.002446, -0.002354, -0.002240, -0.002106, -0.001952, -0.001780, -0.001592, -0.001390, -0.001176, -0.000952, -0.000720, -0.000482, -0.000241, -0.000000, 0.000240, 0.000476, 0.000706, 0.000928, 0.001140, 0.001340, 0.001525, 0.001694, 0.001847, 0.001980, 0.002094, 0.002186, 0.002258, 0.002307, 0.002333, 0.002337, 0.002319, 0.002278, 0.002216, 0.002132, 0.002029, 0.001907, 0.001767, 0.001612, 0.001441, 0.001258, 0.001064, 0.000861, 0.000651, 0.000436, 0.000219, 0.000000, -0.000217, -0.000431, -0.000639, -0.000840, -0.001031, -0.001211, -0.001379, -0.001532, -0.001669, -0.001789, -0.001892, -0.001975, -0.002040, -0.002084, -0.002107, -0.002111, -0.002094, -0.002057, -0.002000, -0.001925, -0.001832, -0.001721, -0.001595, -0.001454, -0.001300, -0.001135, -0.000960, -0.000777, -0.000587, -0.000393, -0.000197, -0.000000, 0.000196, 0.000388, 0.000576, 0.000756, 0.000929, 0.001091, 0.001242, 0.001379, 0.001503, 0.001611, 0.001703, 0.001778, 0.001836, 0.001875, 0.001896, 0.001899, 0.001884, 0.001850, 0.001799, 0.001731, 0.001647, 0.001547, 0.001434, 0.001307, 0.001169, 0.001020, 0.000862, 0.000698, 0.000527, 0.000353, 0.000177, 0.000000, -0.000176, -0.000348, -0.000517, -0.000679, -0.000833, -0.000979, -0.001114, -0.001237, -0.001348, -0.001445, -0.001527, -0.001594, -0.001645, -0.001680, -0.001699, -0.001701, -0.001687, -0.001657, -0.001611, -0.001550, -0.001474, -0.001385, -0.001283, -0.001170, -0.001046, -0.000912, -0.000771, -0.000624, -0.000472, -0.000316, -0.000158, -0.000000, 0.000157, 0.000311, 0.000462, 0.000606, 0.000744, 0.000874, 0.000994, 0.001104, 0.001203, 0.001289, 0.001363, 0.001422, 0.001468, 0.001499, 0.001515, 0.001517, 0.001504, 0.001477, 0.001436, 0.001381, 0.001314, 0.001234, 0.001143, 0.001042, 0.000931, 0.000812, 0.000687, 0.000555, 0.000420, 0.000281, 0.000141, 0.000000, -0.000140, -0.000277, -0.000410, -0.000539, -0.000662, -0.000777, -0.000884, -0.000981, -0.001069, -0.001145, -0.001210, -0.001263, -0.001303, -0.001330, -0.001345, -0.001346, -0.001334, -0.001310, -0.001273, -0.001224, -0.001164, -0.001094, -0.001013, -0.000923, -0.000825, -0.000719, -0.000608, -0.000492, -0.000372, -0.000249, -0.000124, -0.000000, 0.000124, 0.000245, 0.000363, 0.000477, 0.000585, 0.000686, 0.000781, 0.000867, 0.000944, 0.001011, 0.001068, 0.001114, 0.001150, 0.001174, 0.001186, 0.001187, 0.001177, 0.001155, 0.001122, 0.001079, 0.001026, 0.000963, 0.000892, 0.000813, 0.000726, 0.000633, 0.000535, 0.000433, 0.000327, 0.000219, 0.000109, 0.000000, -0.000109, -0.000215, -0.000319, -0.000419, -0.000514, -0.000603, -0.000685, -0.000761, -0.000828, -0.000887, -0.000937, -0.000977, -0.001008, -0.001029, -0.001040, -0.001040, -0.001031, -0.001012, -0.000983, -0.000945, -0.000898, -0.000843, -0.000780, -0.000711, -0.000635, -0.000554, -0.000468, -0.000378, -0.000286, -0.000191, -0.000096, -0.000000, 0.000095, 0.000188, 0.000278, 0.000365, 0.000448, 0.000526, 0.000597, 0.000663, 0.000722, 0.000773, 0.000816, 0.000851, 0.000878, 0.000895, 0.000905, 0.000905, 0.000897, 0.000880, 0.000854, 0.000821, 0.000780, 0.000732, 0.000678, 0.000617, 0.000551, 0.000481, 0.000406, 0.000328, 0.000248, 0.000166, 0.000083, 0.000000, -0.000082, -0.000163, -0.000241, -0.000316, -0.000388, -0.000455, -0.000517, -0.000573, -0.000624, -0.000668, -0.000705, -0.000735, -0.000758, -0.000773, -0.000781, -0.000781, -0.000773, -0.000758, -0.000736, -0.000708, -0.000672, -0.000631, -0.000584, -0.000531, -0.000474, -0.000413, -0.000349, -0.000282, -0.000213, -0.000142, -0.000071, -0.000000, 0.000070, 0.000140, 0.000207, 0.000271, 0.000332, 0.000390, 0.000443, 0.000491, 0.000534, 0.000572, 0.000603, 0.000629, 0.000648, 0.000661, 0.000667, 0.000667, 0.000660, 0.000648, 0.000629, 0.000604, 0.000573, 0.000538, 0.000497, 0.000453, 0.000404, 0.000352, 0.000297, 0.000240, 0.000181, 0.000121, 0.000061, 0.000000, -0.000060, -0.000119, -0.000175, -0.000230, -0.000282, -0.000331, -0.000375, -0.000416, -0.000453, -0.000484, -0.000511, -0.000532, -0.000548, -0.000559, -0.000564, -0.000564, -0.000558, -0.000547, -0.000531, -0.000510, -0.000484, -0.000454, -0.000419, -0.000381, -0.000340, -0.000296, -0.000250, -0.000202, -0.000152, -0.000102, -0.000051, -0.000000, 0.000050, 0.000099, 0.000147, 0.000193, 0.000236, 0.000277, 0.000314, 0.000348, 0.000379, 0.000405, 0.000427, 0.000445, 0.000458, 0.000467, 0.000471, 0.000470, 0.000465, 0.000456, 0.000442, 0.000424, 0.000403, 0.000377, 0.000349, 0.000317, 0.000283, 0.000246, 0.000208, 0.000168, 0.000126, 0.000084, 0.000042, 0.000000, -0.000042, -0.000082, -0.000122, -0.000160, -0.000195, -0.000229, -0.000260, -0.000287, -0.000312, -0.000334, -0.000352, -0.000366, -0.000377, -0.000384, -0.000387, -0.000386, -0.000382, -0.000374, -0.000363, -0.000348, -0.000330, -0.000309, -0.000286, -0.000259, -0.000231, -0.000201, -0.000170, -0.000137, -0.000103, -0.000069, -0.000034, -0.000000, 0.000034, 0.000067, 0.000099, 0.000130, 0.000159, 0.000186, 0.000211, 0.000233, 0.000253, 0.000270, 0.000285, 0.000296, 0.000305, 0.000310, 0.000312, 0.000312, 0.000308, 0.000302, 0.000292, 0.000280, 0.000265, 0.000248, 0.000229, 0.000208, 0.000186, 0.000161, 0.000136, 0.000109, 0.000082, 0.000055, 0.000027, 0.000000, -0.000027, -0.000053, -0.000079, -0.000103, -0.000126, -0.000148, -0.000167, -0.000185, -0.000201, -0.000214, -0.000226, -0.000234, -0.000241, -0.000245, -0.000247, -0.000246, -0.000243, -0.000238, -0.000230, -0.000220, -0.000209, -0.000195, -0.000180, -0.000163, -0.000145, -0.000126, -0.000106, -0.000086, -0.000064, -0.000043, -0.000021, -0.000000, 0.000021, 0.000042, 0.000061, 0.000080, 0.000098, 0.000114, 0.000129, 0.000143, 0.000155, 0.000165, 0.000174, 0.000181, 0.000185, 0.000188, 0.000189, 0.000189, 0.000186, 0.000182, 0.000176, 0.000168, 0.000159, 0.000149, 0.000137, 0.000124, 0.000110, 0.000096, 0.000081, 0.000065, 0.000049, 0.000032, 0.000016, 0.000000, -0.000016, -0.000031, -0.000046, -0.000060, -0.000073, -0.000086, -0.000097, -0.000107, -0.000116, -0.000123, -0.000129, -0.000134, -0.000138, -0.000140, -0.000140, -0.000140, -0.000138, -0.000134, -0.000130, -0.000124, -0.000117, -0.000109, -0.000101, -0.000091, -0.000081, -0.000070, -0.000059, -0.000047, -0.000035, -0.000024, -0.000012, -0.000000, 0.000011, 0.000023, 0.000033, 0.000043, 0.000053, 0.000061, 0.000069, 0.000076, 0.000083, 0.000088, 0.000092, 0.000095, 0.000098, 0.000099, 0.000099, 0.000099, 0.000097, 0.000094, 0.000091, 0.000087, 0.000082, 0.000076, 0.000070, 0.000063, 0.000056, 0.000048, 0.000041, 0.000033, 0.000024, 0.000016, 0.000008, 0.000000, -0.000008, -0.000015, -0.000023, -0.000029, -0.000036, -0.000041, -0.000047, -0.000051, -0.000055, -0.000059, -0.000062, -0.000064, -0.000065, -0.000066, -0.000066, -0.000065, -0.000064, -0.000062, -0.000060, -0.000057, -0.000053, -0.000049, -0.000045, -0.000041, -0.000036, -0.000031, -0.000026, -0.000021, -0.000016, -0.000010, -0.000005, -0.000000, 0.000005, 0.000010, 0.000014, 0.000018, 0.000022, 0.000026, 0.000029, 0.000032, 0.000034, 0.000036, 0.000037, 0.000038, 0.000039, 0.000039, 0.000039, 0.000039, 0.000038, 0.000037, 0.000035, 0.000033, 0.000031, 0.000029, 0.000026, 0.000024, 0.000021, 0.000018, 0.000015, 0.000012, 0.000009, 0.000006, 0.000003, 0.000000, -0.000003, -0.000005, -0.000008, -0.000010, -0.000012, -0.000014, -0.000015, -0.000017, -0.000018, -0.000019, -0.000019, -0.000020, -0.000020, -0.000020, -0.000020, -0.000019, -0.000019, -0.000018, -0.000017, -0.000016, -0.000015, -0.000014, -0.000012, -0.000011, -0.000010, -0.000008, -0.000007, -0.000005, -0.000004, -0.000003, -0.000001, -0.000000, 0.000001, 0.000002, 0.000003, 0.000004, 0.000005, 0.000006, 0.000006, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000006, 0.000006, 0.000005, 0.000005, 0.000004, 0.000004, 0.000003, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001, 0.000001, 0.000000, 0.000000, -0.000000, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000};


extern int16_t* buf16;



const u8 phoneme_prob_remap[64] __attribute__ ((section (".flash")))={1, 46, 30, 5, 7, 6, 21, 15, 14, 16, 25, 40, 43, 53, 47, 29, 52, 48, 20, 34, 33, 59, 32, 31, 28, 62, 44, 9, 8, 10, 54, 11, 13, 12, 3, 2, 4, 50, 23, 49, 56, 58, 57, 63, 24, 22, 17, 19, 18, 61, 39, 26, 45, 37, 36, 51, 38, 60, 65, 64, 35, 68, 61, 62}; // this is for klatt - where do we use it?

arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate


extern u8 test_elm[51];
extern u8 test_elm_rsynthy[106]; // as is just phon code and length... with lat stop


// this is generated by test.c using srconvrt.c with width 64 and zero cross as 32

//const float sinc_table[]={0.998394, 0.993585, 0.985600, 0.974486, 0.960307, 0.943145, 0.923099, 0.900282, 0.874827, 0.846878, 0.816594, 0.784147, 0.749719, 0.713503, 0.675702, 0.636524, 0.596185, 0.554905, 0.512908, 0.470417, 0.427659, 0.384858, 0.342233, 0.300004, 0.258380, 0.217567, 0.177761, 0.139149, 0.101909, 0.066204, 0.032188, 0.000000, -0.030235, -0.058407, -0.084420, -0.108195, -0.129669, -0.148794, -0.165541, -0.179894, -0.191855, -0.201440, -0.208684, -0.213633, -0.216349, -0.216908, -0.215398, -0.211919, -0.206583, -0.199510, -0.190832, -0.180685, -0.169214, -0.156570, -0.142906, -0.128379, -0.113150, -0.097376, -0.081217, -0.064829, -0.048366, -0.031979, -0.015811, -0.000000, 0.015322, 0.030032, 0.044015, 0.057167, 0.069394, 0.080610, 0.090743, 0.099730, 0.107523, 0.114081, 0.119380, 0.123403, 0.126147, 0.127622, 0.127844, 0.126845, 0.124664, 0.121350, 0.116962, 0.111566, 0.105236, 0.098052, 0.090101, 0.081475, 0.072268, 0.062579, 0.052508, 0.042159, 0.031632, 0.021030, 0.010454, 0.000000, -0.010236, -0.020163, -0.029695, -0.038751, -0.047256, -0.055141, -0.062346, -0.068815, -0.074503, -0.079372, -0.083391, -0.086538, -0.088801, -0.090175, -0.090663, -0.090276, -0.089035, -0.086965, -0.084102, -0.080485, -0.076163, -0.071187, -0.065617, -0.059514, -0.052945, -0.045980, -0.038691, -0.031152, -0.023438, -0.015624, -0.007787, -0.000000, 0.007664, 0.015134, 0.022344, 0.029229, 0.035728, 0.041787, 0.047354, 0.052386, 0.056841, 0.060687, 0.063895, 0.066446, 0.068325, 0.069522, 0.070038, 0.069876, 0.069048, 0.067572, 0.065469, 0.062769, 0.059506, 0.055718, 0.051448, 0.046744, 0.041656, 0.036236, 0.030542, 0.024631, 0.018561, 0.012393, 0.006186, 0.000000, -0.006107, -0.012078, -0.017858, -0.023394, -0.028637, -0.033541, -0.038063, -0.042164, -0.045812, -0.048977, -0.051635, -0.053766, -0.055357, -0.056398, -0.056888, -0.056826, -0.056221, -0.055085, -0.053434, -0.051291, -0.048680, -0.045634, -0.042184, -0.038370, -0.034231, -0.029810, -0.025152, -0.020306, -0.015318, -0.010238, -0.005116, -0.000000, 0.005060, 0.010018, 0.014826, 0.019442, 0.023821, 0.027927, 0.031721, 0.035172, 0.038250, 0.040929, 0.043188, 0.045010, 0.046382, 0.047295, 0.047746, 0.047735, 0.047266, 0.046349, 0.044996, 0.043226, 0.041059, 0.038519, 0.035636, 0.032438, 0.028961, 0.025240, 0.021312, 0.017218, 0.012999, 0.008694, 0.004347, 0.000000, -0.004306, -0.008531, -0.012635, -0.016579, -0.020327, -0.023847, -0.027105, -0.030073, -0.032725, -0.035040, -0.036998, -0.038583, -0.039784, -0.040592, -0.041004, -0.041019, -0.040641, -0.039876, -0.038735, -0.037233, -0.035386, -0.033217, -0.030747, -0.028004, -0.025016, -0.021814, -0.018430, -0.014897, -0.011252, -0.007530, -0.003767, -0.000000, 0.003736, 0.007405, 0.010972, 0.014404, 0.017670, 0.020739, 0.023584, 0.026179, 0.028502, 0.030532, 0.032253, 0.033651, 0.034715, 0.035436, 0.035813, 0.035842, 0.035527, 0.034874, 0.033891, 0.032591, 0.030988, 0.029101, 0.026949, 0.024555, 0.021944, 0.019143, 0.016180, 0.013084, 0.009887, 0.006619, 0.003313, 0.000000, -0.003288, -0.006519, -0.009663, -0.012691, -0.015574, -0.018286, -0.020802, -0.023100, -0.025159, -0.026961, -0.028491, -0.029737, -0.030688, -0.031337, -0.031680, -0.031717, -0.031450, -0.030882, -0.030022, -0.028880, -0.027469, -0.025805, -0.023904, -0.021788, -0.019478, -0.016997, -0.014371, -0.011625, -0.008787, -0.005885, -0.002946, -0.000000, 0.002926, 0.005803, 0.008604, 0.011303, 0.013875, 0.016296, 0.018544, 0.020598, 0.022441, 0.024055, 0.025428, 0.026546, 0.027403, 0.027990, 0.028305, 0.028346, 0.028114, 0.027614, 0.026853, 0.025838, 0.024582, 0.023099, 0.021403, 0.019514, 0.017449, 0.015230, 0.012880, 0.010422, 0.007880, 0.005278, 0.002643, 0.000000, -0.002626, -0.005210, -0.007727, -0.010153, -0.012466, -0.014645, -0.016669, -0.018520, -0.020181, -0.021638, -0.022878, -0.023890, -0.024666, -0.025201, -0.025490, -0.025532, -0.025329, -0.024884, -0.024203, -0.023293, -0.022166, -0.020832, -0.019307, -0.017606, -0.015747, -0.013747, -0.011628, -0.009411, -0.007117, -0.004768, -0.002388, -0.000000, 0.002374, 0.004710, 0.006987, 0.009183, 0.011277, 0.013250, 0.015084, 0.016762, 0.018269, 0.019592, 0.020718, 0.021638, 0.022345, 0.022834, 0.023100, 0.023142, 0.022962, 0.022563, 0.021949, 0.021128, 0.020108, 0.018902, 0.017521, 0.015980, 0.014295, 0.012482, 0.010560, 0.008547, 0.006465, 0.004332, 0.002170, 0.000000, -0.002158, -0.004282, -0.006353, -0.008351, -0.010257, -0.012053, -0.013724, -0.015253, -0.016627, -0.017833, -0.018861, -0.019702, -0.020349, -0.020796, -0.021042, -0.021083, -0.020922, -0.020561, -0.020004, -0.019259, -0.018332, -0.017235, -0.015978, -0.014575, -0.013039, -0.011387, -0.009635, -0.007800, -0.005900, -0.003954, -0.001981, -0.000000, 0.001970, 0.003910, 0.005802, 0.007628, 0.009371, 0.011013, 0.012541, 0.013940, 0.015198, 0.016302, 0.017244, 0.018015, 0.018609, 0.019020, 0.019247, 0.019287, 0.019142, 0.018814, 0.018307, 0.017627, 0.016781, 0.015778, 0.014629, 0.013346, 0.011941, 0.010429, 0.008825, 0.007145, 0.005405, 0.003623, 0.001815, 0.000000, -0.001806, -0.003585, -0.005319, -0.006994, -0.008592, -0.010100, -0.011502, -0.012786, -0.013941, -0.014956, -0.015821, -0.016530, -0.017077, -0.017456, -0.017666, -0.017705, -0.017573, -0.017273, -0.016809, -0.016186, -0.015411, -0.014491, -0.013437, -0.012260, -0.010970, -0.009582, -0.008109, -0.006566, -0.004968, -0.003330, -0.001669, -0.000000, 0.001660, 0.003296, 0.004891, 0.006431, 0.007902, 0.009289, 0.010580, 0.011762, 0.012825, 0.013760, 0.014558, 0.015211, 0.015715, 0.016066, 0.016260, 0.016297, 0.016177, 0.015902, 0.015476, 0.014904, 0.014191, 0.013345, 0.012375, 0.011292, 0.010105, 0.008827, 0.007471, 0.006049, 0.004577, 0.003068, 0.001538, 0.000000, -0.001530, -0.003038, -0.004509, -0.005929, -0.007285, -0.008564, -0.009755, -0.010846, -0.011827, -0.012690, -0.013426, -0.014030, -0.014495, -0.014820, -0.015000, -0.015035, -0.014925, -0.014673, -0.014281, -0.013753, -0.013096, -0.012316, -0.011422, -0.010422, -0.009327, -0.008148, -0.006897, -0.005585, -0.004226, -0.002833, -0.001420, -0.000000, 0.001413, 0.002805, 0.004164, 0.005476, 0.006729, 0.007911, 0.009011, 0.010020, 0.010927, 0.011724, 0.012406, 0.012964, 0.013395, 0.013695, 0.013863, 0.013896, 0.013795, 0.013563, 0.013201, 0.012714, 0.012107, 0.011386, 0.010560, 0.009636, 0.008625, 0.007535, 0.006378, 0.005165, 0.003908, 0.002620, 0.001313, 0.000000, -0.001307, -0.002595, -0.003852, -0.005066, -0.006225, -0.007319, -0.008338, -0.009271, -0.010111, -0.010849, -0.011480, -0.011997, -0.012397, -0.012675, -0.012830, -0.012861, -0.012769, -0.012554, -0.012219, -0.011769, -0.011208, -0.010541, -0.009777, -0.008922, -0.007985, -0.006976, -0.005905, -0.004783, -0.003619, -0.002427, -0.001216, -0.000000, 0.001210, 0.002404, 0.003568, 0.004692, 0.005766, 0.006780, 0.007723, 0.008588, 0.009366, 0.010051, 0.010635, 0.011115, 0.011485, 0.011744, 0.011888, 0.011917, 0.011832, 0.011633, 0.011323, 0.010906, 0.010386, 0.009769, 0.009061, 0.008269, 0.007401, 0.006466, 0.005474, 0.004433, 0.003355, 0.002249, 0.001127, 0.000000, -0.001122, -0.002228, -0.003308, -0.004350, -0.005346, -0.006286, -0.007161, -0.007963, -0.008684, -0.009319, -0.009861, -0.010306, -0.010650, -0.010890, -0.011024, -0.011051, -0.010972, -0.010788, -0.010501, -0.010115, -0.009633, -0.009060, -0.008404, -0.007669, -0.006864, -0.005997, -0.005077, -0.004112, -0.003112, -0.002086, -0.001046, -0.000000, 0.001041, 0.002067, 0.003068, 0.004036, 0.004959, 0.005831, 0.006643, 0.007387, 0.008057, 0.008646, 0.009149, 0.009562, 0.009881, 0.010104, 0.010228, 0.010254, 0.010181, 0.010010, 0.009744, 0.009385, 0.008938, 0.008407, 0.007798, 0.007117, 0.006370, 0.005565, 0.004711, 0.003816, 0.002888, 0.001936, 0.000971, 0.000000, -0.000966, -0.001918, -0.002848, -0.003745, -0.004603, -0.005412, -0.006165, -0.006856, -0.007478, -0.008024, -0.008492, -0.008875, -0.009171, -0.009378, -0.009493, -0.009517, -0.009449, -0.009291, -0.009044, -0.008711, -0.008296, -0.007803, -0.007238, -0.006605, -0.005912, -0.005166, -0.004373, -0.003542, -0.002680, -0.001797, -0.000901, -0.000000, 0.000897, 0.001781, 0.002643, 0.003476, 0.004272, 0.005023, 0.005723, 0.006364, 0.006941, 0.007449, 0.007882, 0.008238, 0.008513, 0.008705, 0.008812, 0.008834, 0.008771, 0.008624, 0.008395, 0.008086, 0.007701, 0.007243, 0.006718, 0.006131, 0.005488, 0.004795, 0.004059, 0.003287, 0.002488, 0.001668, 0.000836, 0.000000, -0.000832, -0.001653, -0.002453, -0.003227, -0.003966, -0.004663, -0.005312, -0.005907, -0.006442, -0.006913, -0.007316, -0.007646, -0.007901, -0.008079, -0.008179, -0.008199, -0.008141, -0.008004, -0.007791, -0.007505, -0.007147, -0.006723, -0.006235, -0.005690, -0.005093, -0.004450, -0.003767, -0.003051, -0.002309, -0.001548, -0.000776, -0.000000, 0.000772, 0.001534, 0.002277, 0.002994, 0.003680, 0.004327, 0.004929, 0.005481, 0.005978, 0.006415, 0.006789, 0.007095, 0.007332, 0.007497, 0.007589, 0.007608, 0.007553, 0.007427, 0.007229, 0.006963, 0.006631, 0.006237, 0.005785, 0.005279, 0.004725, 0.004128, 0.003495, 0.002830, 0.002142, 0.001436, 0.000720, 0.000000, -0.000716, -0.001423, -0.002112, -0.002778, -0.003413, -0.004013, -0.004572, -0.005084, -0.005545, -0.005950, -0.006296, -0.006580, -0.006800, -0.006953, -0.007038, -0.007055, -0.007005, -0.006887, -0.006704, -0.006457, -0.006149, -0.005784, -0.005364, -0.004895, -0.004382, -0.003828, -0.003240, -0.002624, -0.001986, -0.001332, -0.000667, -0.000000, 0.000664, 0.001319, 0.001958, 0.002575, 0.003164, 0.003721, 0.004238, 0.004713, 0.005140, 0.005515, 0.005836, 0.006099, 0.006302, 0.006444, 0.006523, 0.006539, 0.006492, 0.006383, 0.006213, 0.005984, 0.005698, 0.005360, 0.004971, 0.004536, 0.004060, 0.003547, 0.003002, 0.002432, 0.001840, 0.001234, 0.000618, 0.000000, -0.000615, -0.001222, -0.001814, -0.002385, -0.002931, -0.003446, -0.003926, -0.004365, -0.004761, -0.005108, -0.005405, -0.005649, -0.005837, -0.005968, -0.006041, -0.006055, -0.006012, -0.005911, -0.005753, -0.005541, -0.005276, -0.004962, -0.004602, -0.004200, -0.003759, -0.003284, -0.002780, -0.002251, -0.001703, -0.001142, -0.000572, -0.000000, 0.000570, 0.001131, 0.001679, 0.002208, 0.002713, 0.003189, 0.003633, 0.004040, 0.004405, 0.004727, 0.005001, 0.005227, 0.005400, 0.005521, 0.005589, 0.005602, 0.005561, 0.005468, 0.005322, 0.005125, 0.004880, 0.004590, 0.004257, 0.003884, 0.003476, 0.003037, 0.002570, 0.002082, 0.001575, 0.001056, 0.000529, 0.000000, -0.000527, -0.001046, -0.001552, -0.002041, -0.002508, -0.002948, -0.003358, -0.003734, -0.004072, -0.004369, -0.004622, -0.004830, -0.004991, -0.005102, -0.005164, -0.005177, -0.005139, -0.005052, -0.004917, -0.004735, -0.004509, -0.004240, -0.003932, -0.003588, -0.003211, -0.002805, -0.002374, -0.001923, -0.001455, -0.000975, -0.000489, -0.000000, 0.000486, 0.000965, 0.001433, 0.001884, 0.002315, 0.002722, 0.003100, 0.003447, 0.003759, 0.004033, 0.004266, 0.004458, 0.004606, 0.004709, 0.004766, 0.004777, 0.004742, 0.004661, 0.004537, 0.004369, 0.004160, 0.003912, 0.003628, 0.003310, 0.002962, 0.002587, 0.002190, 0.001773, 0.001342, 0.000899, 0.000451, 0.000000, -0.000448, -0.000890, -0.001321, -0.001737, -0.002134, -0.002509, -0.002858, -0.003177, -0.003464, -0.003717, -0.003932, -0.004109, -0.004245, -0.004339, -0.004392, -0.004401, -0.004369, -0.004295, -0.004179, -0.004024, -0.003832, -0.003603, -0.003341, -0.003048, -0.002728, -0.002383, -0.002016, -0.001633, -0.001235, -0.000828, -0.000415, -0.000000, 0.000413, 0.000819, 0.001216, 0.001599, 0.001964, 0.002309, 0.002630, 0.002924, 0.003188, 0.003420, 0.003618, 0.003780, 0.003905, 0.003992, 0.004040, 0.004048, 0.004018, 0.003950, 0.003844, 0.003701, 0.003524, 0.003313, 0.003072, 0.002803, 0.002508, 0.002190, 0.001854, 0.001501, 0.001135, 0.000761, 0.000381, 0.000000, -0.000379, -0.000753, -0.001117, -0.001469, -0.001805, -0.002121, -0.002416, -0.002685, -0.002928, -0.003141, -0.003322, -0.003471, -0.003586, -0.003665, -0.003709, -0.003717, -0.003689, -0.003626, -0.003528, -0.003397, -0.003234, -0.003041, -0.002819, -0.002572, -0.002301, -0.002010, -0.001701, -0.001377, -0.001041, -0.000698, -0.000350, -0.000000, 0.000348, 0.000690, 0.001025, 0.001347, 0.001655, 0.001945, 0.002215, 0.002462, 0.002684, 0.002879, 0.003045, 0.003181, 0.003286, 0.003358, 0.003398, 0.003405, 0.003379, 0.003321, 0.003231, 0.003111, 0.002962, 0.002784, 0.002581, 0.002355, 0.002107, 0.001840, 0.001557, 0.001260, 0.000953, 0.000639, 0.000320, 0.000000, -0.000318, -0.000632, -0.000937, -0.001232, -0.001514, -0.001779, -0.002025, -0.002251, -0.002454, -0.002632, -0.002784, -0.002908, -0.003004, -0.003070, -0.003106, -0.003112, -0.003088, -0.003035, -0.002953, -0.002843, -0.002706, -0.002544, -0.002358, -0.002151, -0.001924, -0.001680, -0.001422, -0.001151, -0.000870, -0.000583, -0.000292, -0.000000, 0.000291, 0.000577, 0.000855, 0.001124, 0.001381, 0.001623, 0.001848, 0.002054, 0.002239, 0.002401, 0.002539, 0.002652, 0.002739, 0.002799, 0.002832, 0.002837, 0.002815, 0.002766, 0.002691, 0.002591, 0.002466, 0.002318, 0.002148, 0.001959, 0.001753, 0.001530, 0.001295, 0.001048, 0.000793, 0.000531, 0.000266, 0.000000, -0.000264, -0.000525, -0.000779, -0.001023, -0.001257, -0.001477, -0.001681, -0.001868, -0.002036, -0.002184, -0.002309, -0.002412, -0.002491, -0.002545, -0.002574, -0.002579, -0.002559, -0.002514, -0.002446, -0.002354, -0.002240, -0.002106, -0.001952, -0.001780, -0.001592, -0.001390, -0.001176, -0.000952, -0.000720, -0.000482, -0.000241, -0.000000, 0.000240, 0.000476, 0.000706, 0.000928, 0.001140, 0.001340, 0.001525, 0.001694, 0.001847, 0.001980, 0.002094, 0.002186, 0.002258, 0.002307, 0.002333, 0.002337, 0.002319, 0.002278, 0.002216, 0.002132, 0.002029, 0.001907, 0.001767, 0.001612, 0.001441, 0.001258, 0.001064, 0.000861, 0.000651, 0.000436, 0.000219, 0.000000, -0.000217, -0.000431, -0.000639, -0.000840, -0.001031, -0.001211, -0.001379, -0.001532, -0.001669, -0.001789, -0.001892, -0.001975, -0.002040, -0.002084, -0.002107, -0.002111, -0.002094, -0.002057, -0.002000, -0.001925, -0.001832, -0.001721, -0.001595, -0.001454, -0.001300, -0.001135, -0.000960, -0.000777, -0.000587, -0.000393, -0.000197, -0.000000, 0.000196, 0.000388, 0.000576, 0.000756, 0.000929, 0.001091, 0.001242, 0.001379, 0.001503, 0.001611, 0.001703, 0.001778, 0.001836, 0.001875, 0.001896, 0.001899, 0.001884, 0.001850, 0.001799, 0.001731, 0.001647, 0.001547, 0.001434, 0.001307, 0.001169, 0.001020, 0.000862, 0.000698, 0.000527, 0.000353, 0.000177, 0.000000, -0.000176, -0.000348, -0.000517, -0.000679, -0.000833, -0.000979, -0.001114, -0.001237, -0.001348, -0.001445, -0.001527, -0.001594, -0.001645, -0.001680, -0.001699, -0.001701, -0.001687, -0.001657, -0.001611, -0.001550, -0.001474, -0.001385, -0.001283, -0.001170, -0.001046, -0.000912, -0.000771, -0.000624, -0.000472, -0.000316, -0.000158, -0.000000, 0.000157, 0.000311, 0.000462, 0.000606, 0.000744, 0.000874, 0.000994, 0.001104, 0.001203, 0.001289, 0.001363, 0.001422, 0.001468, 0.001499, 0.001515, 0.001517, 0.001504, 0.001477, 0.001436, 0.001381, 0.001314, 0.001234, 0.001143, 0.001042, 0.000931, 0.000812, 0.000687, 0.000555, 0.000420, 0.000281, 0.000141, 0.000000, -0.000140, -0.000277, -0.000410, -0.000539, -0.000662, -0.000777, -0.000884, -0.000981, -0.001069, -0.001145, -0.001210, -0.001263, -0.001303, -0.001330, -0.001345, -0.001346, -0.001334, -0.001310, -0.001273, -0.001224, -0.001164, -0.001094, -0.001013, -0.000923, -0.000825, -0.000719, -0.000608, -0.000492, -0.000372, -0.000249, -0.000124, -0.000000, 0.000124, 0.000245, 0.000363, 0.000477, 0.000585, 0.000686, 0.000781, 0.000867, 0.000944, 0.001011, 0.001068, 0.001114, 0.001150, 0.001174, 0.001186, 0.001187, 0.001177, 0.001155, 0.001122, 0.001079, 0.001026, 0.000963, 0.000892, 0.000813, 0.000726, 0.000633, 0.000535, 0.000433, 0.000327, 0.000219, 0.000109, 0.000000, -0.000109, -0.000215, -0.000319, -0.000419, -0.000514, -0.000603, -0.000685, -0.000761, -0.000828, -0.000887, -0.000937, -0.000977, -0.001008, -0.001029, -0.001040, -0.001040, -0.001031, -0.001012, -0.000983, -0.000945, -0.000898, -0.000843, -0.000780, -0.000711, -0.000635, -0.000554, -0.000468, -0.000378, -0.000286, -0.000191, -0.000096, -0.000000, 0.000095, 0.000188, 0.000278, 0.000365, 0.000448, 0.000526, 0.000597, 0.000663, 0.000722, 0.000773, 0.000816, 0.000851, 0.000878, 0.000895, 0.000905, 0.000905, 0.000897, 0.000880, 0.000854, 0.000821, 0.000780, 0.000732, 0.000678, 0.000617, 0.000551, 0.000481, 0.000406, 0.000328, 0.000248, 0.000166, 0.000083, 0.000000, -0.000082, -0.000163, -0.000241, -0.000316, -0.000388, -0.000455, -0.000517, -0.000573, -0.000624, -0.000668, -0.000705, -0.000735, -0.000758, -0.000773, -0.000781, -0.000781, -0.000773, -0.000758, -0.000736, -0.000708, -0.000672, -0.000631, -0.000584, -0.000531, -0.000474, -0.000413, -0.000349, -0.000282, -0.000213, -0.000142, -0.000071, -0.000000, 0.000070, 0.000140, 0.000207, 0.000271, 0.000332, 0.000390, 0.000443, 0.000491, 0.000534, 0.000572, 0.000603, 0.000629, 0.000648, 0.000661, 0.000667, 0.000667, 0.000660, 0.000648, 0.000629, 0.000604, 0.000573, 0.000538, 0.000497, 0.000453, 0.000404, 0.000352, 0.000297, 0.000240, 0.000181, 0.000121, 0.000061, 0.000000, -0.000060, -0.000119, -0.000175, -0.000230, -0.000282, -0.000331, -0.000375, -0.000416, -0.000453, -0.000484, -0.000511, -0.000532, -0.000548, -0.000559, -0.000564, -0.000564, -0.000558, -0.000547, -0.000531, -0.000510, -0.000484, -0.000454, -0.000419, -0.000381, -0.000340, -0.000296, -0.000250, -0.000202, -0.000152, -0.000102, -0.000051, -0.000000, 0.000050, 0.000099, 0.000147, 0.000193, 0.000236, 0.000277, 0.000314, 0.000348, 0.000379, 0.000405, 0.000427, 0.000445, 0.000458, 0.000467, 0.000471, 0.000470, 0.000465, 0.000456, 0.000442, 0.000424, 0.000403, 0.000377, 0.000349, 0.000317, 0.000283, 0.000246, 0.000208, 0.000168, 0.000126, 0.000084, 0.000042, 0.000000, -0.000042, -0.000082, -0.000122, -0.000160, -0.000195, -0.000229, -0.000260, -0.000287, -0.000312, -0.000334, -0.000352, -0.000366, -0.000377, -0.000384, -0.000387, -0.000386, -0.000382, -0.000374, -0.000363, -0.000348, -0.000330, -0.000309, -0.000286, -0.000259, -0.000231, -0.000201, -0.000170, -0.000137, -0.000103, -0.000069, -0.000034, -0.000000, 0.000034, 0.000067, 0.000099, 0.000130, 0.000159, 0.000186, 0.000211, 0.000233, 0.000253, 0.000270, 0.000285, 0.000296, 0.000305, 0.000310, 0.000312, 0.000312, 0.000308, 0.000302, 0.000292, 0.000280, 0.000265, 0.000248, 0.000229, 0.000208, 0.000186, 0.000161, 0.000136, 0.000109, 0.000082, 0.000055, 0.000027, 0.000000, -0.000027, -0.000053, -0.000079, -0.000103, -0.000126, -0.000148, -0.000167, -0.000185, -0.000201, -0.000214, -0.000226, -0.000234, -0.000241, -0.000245, -0.000247, -0.000246, -0.000243, -0.000238, -0.000230, -0.000220, -0.000209, -0.000195, -0.000180, -0.000163, -0.000145, -0.000126, -0.000106, -0.000086, -0.000064, -0.000043, -0.000021, -0.000000, 0.000021, 0.000042, 0.000061, 0.000080, 0.000098, 0.000114, 0.000129, 0.000143, 0.000155, 0.000165, 0.000174, 0.000181, 0.000185, 0.000188, 0.000189, 0.000189, 0.000186, 0.000182, 0.000176, 0.000168, 0.000159, 0.000149, 0.000137, 0.000124, 0.000110, 0.000096, 0.000081, 0.000065, 0.000049, 0.000032, 0.000016, 0.000000, -0.000016, -0.000031, -0.000046, -0.000060, -0.000073, -0.000086, -0.000097, -0.000107, -0.000116, -0.000123, -0.000129, -0.000134, -0.000138, -0.000140, -0.000140, -0.000140, -0.000138, -0.000134, -0.000130, -0.000124, -0.000117, -0.000109, -0.000101, -0.000091, -0.000081, -0.000070, -0.000059, -0.000047, -0.000035, -0.000024, -0.000012, -0.000000, 0.000011, 0.000023, 0.000033, 0.000043, 0.000053, 0.000061, 0.000069, 0.000076, 0.000083, 0.000088, 0.000092, 0.000095, 0.000098, 0.000099, 0.000099, 0.000099, 0.000097, 0.000094, 0.000091, 0.000087, 0.000082, 0.000076, 0.000070, 0.000063, 0.000056, 0.000048, 0.000041, 0.000033, 0.000024, 0.000016, 0.000008, 0.000000, -0.000008, -0.000015, -0.000023, -0.000029, -0.000036, -0.000041, -0.000047, -0.000051, -0.000055, -0.000059, -0.000062, -0.000064, -0.000065, -0.000066, -0.000066, -0.000065, -0.000064, -0.000062, -0.000060, -0.000057, -0.000053, -0.000049, -0.000045, -0.000041, -0.000036, -0.000031, -0.000026, -0.000021, -0.000016, -0.000010, -0.000005, -0.000000, 0.000005, 0.000010, 0.000014, 0.000018, 0.000022, 0.000026, 0.000029, 0.000032, 0.000034, 0.000036, 0.000037, 0.000038, 0.000039, 0.000039, 0.000039, 0.000039, 0.000038, 0.000037, 0.000035, 0.000033, 0.000031, 0.000029, 0.000026, 0.000024, 0.000021, 0.000018, 0.000015, 0.000012, 0.000009, 0.000006, 0.000003, 0.000000, -0.000003, -0.000005, -0.000008, -0.000010, -0.000012, -0.000014, -0.000015, -0.000017, -0.000018, -0.000019, -0.000019, -0.000020, -0.000020, -0.000020, -0.000020, -0.000019, -0.000019, -0.000018, -0.000017, -0.000016, -0.000015, -0.000014, -0.000012, -0.000011, -0.000010, -0.000008, -0.000007, -0.000005, -0.000004, -0.000003, -0.000001, -0.000000, 0.000001, 0.000002, 0.000003, 0.000004, 0.000005, 0.000006, 0.000006, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000006, 0.000006, 0.000005, 0.000005, 0.000004, 0.000004, 0.000003, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001, 0.000001, 0.000000, 0.000000, -0.000000, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000};



u16 run_holmes(u16 klatthead)
{
  extern u8 holmes_trigger;
  unsigned nelm=ELM_LEN; // ELm=len is 48= XXX phonemes = how many frames approx ???? - in test case we have 87 frames 
  // how can we make sure that frames/nelm fills the buffer at least
  // and to trigger/schedule so we don't overwrite ourselves
  unsigned char *elm=test_elm; // is our list of phonemes in order phon_number, duration, stress - we cycle through it
  short *samp=audio_buffer;

  // put into struct and init elsewhere...

	filter_t flt[nEparm];
	klatt_frame_t pars;
	Elm_ptr le = &Elements[0];
	unsigned i = 0;
	unsigned tstress = 0;
	unsigned ntstress = 0;
	slope_t stress_s;
	slope_t stress_e;
	float top = 1.1 * def_pars.F0hz10;

	//.....

	int j;
	pars = def_pars;
	pars.FNPhz = le->p[fn].stdy;
	pars.B1phz = pars.B1hz = 60;
	pars.B2phz = pars.B2hz = 90;
	pars.B3phz = pars.B3hz = 150;
	pars.B4phz = def_pars.B4phz;

	/* flag new utterance */
	parwave_init(&klatt_global);

	/* Set stress attack/decay slope */
	stress_s.t = 40;
	stress_e.t = 40;
	stress_e.v = 0.0;

	for (j = 0; j < nEparm; j++)
	{
		flt[j].v = le->p[j].stdy;
		flt[j].a = frac;
		flt[j].b = (float) 1.0 - (float) frac;
	}
	  
	while (i < nelm)
	{
		Elm_ptr ce = &Elements[elm[i++]];
		unsigned dur = elm[i++];
		i++; /* skip stress */
		/* Skip zero length elements which are only there to affect
		boundary values of adjacent elements
		*/
		if (dur > 0)
		{
			Elm_ptr ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
			slope_t startyy[nEparm];
			slope_t end[nEparm];
			unsigned t;

			if (ce->rk > le->rk)
			{
				set_trans(startyy, ce, le, 0, 's');
				/* we dominate last */
			}
			else
			{
				set_trans(startyy, le, ce, 1, 's');
				/* last dominates us */
			}

			if (ne->rk > ce->rk)
			{
				set_trans(end, ne, ce, 1, 'e');
				/* next dominates us */
			}
			else
			{
				set_trans(end, ce, ne, 0, 'e');
				/* we dominate next */
			}


			for (t = 0; t < dur; t++, tstress++)
			{
				float base = top * 0.8 /* 3 * top / 5 */;
				float tp[nEparm];
				int j;

				if (tstress == ntstress)
				{
					unsigned j = i;
					stress_s = stress_e;
					tstress = 0;
					ntstress = dur;
					#ifdef DEBUG_STRESS
					printf("Stress %g -> ", stress_s.v);
					#endif
					while (j <= nelm)
					{
						Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
						unsigned du = (j < nelm) ? elm[j++] : 0;
						unsigned s  = (j < nelm) ? elm[j++] : 3;
						if (s || e->feat & vwl)
						{
							unsigned d = 0;
							if (s)
								stress_e.v = (float) s / 3;
							else
								stress_e.v = (float) 0.1;
							do
							{
								d += du;
								#ifdef DEBUG_STRESS
								printf("%s", (e && e->dict) ? e->dict : "");
								#endif
								e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
								du = elm[j++];
							}
							while ((e->feat & vwl) && elm[j++] == s);
							ntstress += d / 2;
							break;
						}
						ntstress += du;
					}
					#ifdef DEBUG_STRESS
					printf(" %g @ %d\n", stress_e.v, ntstress);
					#endif
				}

				for (j = 0; j < nEparm; j++)
					tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

				/* Now call the synth for each frame */

				pars.F0hz10 = base + (top - base) *
				interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

				pars.AVdb = pars.AVpdb = tp[av];
				pars.AF = tp[af];
				pars.FNZhz = tp[fn];
				pars.ASP = tp[asp];
				pars.Aturb = tp[avc];
				pars.B1phz = pars.B1hz = tp[b1];
				pars.B2phz = pars.B2hz = tp[b2];
				pars.B3phz = pars.B3hz = tp[b3];
				pars.F1hz = tp[f1];
				pars.F2hz = tp[f2];
				pars.F3hz = tp[f3];
				/* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
				Needs to be fixed properly in tables
				*/
				/*
				pars.ANP  = AMP_ADJ + tp[an];
				*/
				pars.AB = AMP_ADJ + tp[ab];
				pars.A5 = AMP_ADJ + tp[a5];
				pars.A6 = AMP_ADJ + tp[a6];
				pars.A1 = AMP_ADJ + tp[a1];
				pars.A2 = AMP_ADJ + tp[a2];
				pars.A3 = AMP_ADJ + tp[a3];
				pars.A4 = AMP_ADJ + tp[a4];

				klatthead=new_parwave(&klatt_global, &pars, samp,klatthead);
				
				//				samp += klatt_global.nspfr;
				/* Declination of f0 envelope 0.25Hz / cS */
				top -= 0.5;
			}
		}
		le = ce;
	} // i<nelm so endless loop here unless get out!

	return klatthead;
	//	return (samp - samp_base);
}


void holmesrun(int16_t* outgoing, u8 size){
  u8 x=0;
  static u8 nextelement=1;
  static short samplenumber=0;
  static u8 newframe=0;
  static Elm_ptr ce; 
  unsigned nelm=ELM_LEN; // 10 phonemes = how many frames approx ???? - in test case we have 87 frames 
  unsigned char *elm=test_elm; // is our list of phonemes in order phon_number, duration, stress - we cycle through it
  u8 j; 
  static u8 dur;
  slope_t startyy[nEparm];
  slope_t end[nEparm];
  static klatt_frame_t pars;

  while(x<size){

  if (i>nelm){   // NEW utterance which means we hit nelm=0 in our cycling:
    i=0;
    le = &Elements[0];
        top = 1.1 * def_pars.F0hz10;
    //    top= 200+ adc_buffer[SELZ];
    pars = def_pars;
    pars.FNPhz = le->p[fn].stdy;
    pars.B1phz = pars.B1hz = 60;
    pars.B2phz = pars.B2hz = 90;
    pars.B3phz = pars.B3hz = 150;
    pars.B4phz = def_pars.B4phz;

    parwave_init(&klatt_global);

    /* Set stress attack/decay slope */
    stress_s.t = 40;
    stress_e.t = 40;
    stress_e.v = 0.0;

    for (j = 0; j < nEparm; j++)
      {
	flt[j].v = le->p[j].stdy;
	flt[j].a = frac;
	flt[j].b = (float) 1.0 - (float) frac;
      }
    nextelement=1;
  }

  //////// are we on first or next element
  if (nextelement==1){
    ce = &Elements[elm[i++]];
    dur = elm[i++];
    i++; /* skip stress */
    /* Skip zero length elements which are only there to affect
       boundary values of adjacent elements
    */

    if (dur == 0) { // do what? NOTHING
    }
    else
      { // startyy to process next frames
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];

	if (ce->rk > le->rk)
	  {
	    set_trans(startyy, ce, le, 0, 's');
	    /* we dominate last */
	  }
	else
	  {
	    set_trans(startyy, le, ce, 1, 's');
	    /* last dominates us */
	  }

	if (ne->rk > ce->rk)
	  {
	    set_trans(end, ne, ce, 1, 'e');
	    /* next dominates us */
	  }
	else
	  {
	    set_trans(end, ce, ne, 0, 'e');
	    /* we dominate next */
	  }
	// next set of frames what do we need to init?
	t=0;
	ne = (i < nelm) ? &Elements[elm[i]] : &Elements[0];
	newframe=1;
      }
  }
  
  if (newframe==1) { // this is a new frame - so we need new parameters
    newframe=0;
    // inc and are we at end of frames in which case we need next element?

    if (t<dur){
            float base = top * 0.8 /* 3 * top / 5 */;
      //float base =      200+ adc_buffer[SELZ];
      float tp[nEparm];

           if (tstress == ntstress)
	{
	  j = i;
	  stress_s = stress_e;
	  tstress = 0;
	  ntstress = dur;

	  while (j <= nelm)
	    {
	      Elm_ptr e   = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
	      unsigned du = (j < nelm) ? elm[j++] : 0;
	      unsigned s  = (j < nelm) ? elm[j++] : 3;
	      if (s || e->feat & vwl)
		{
		  unsigned d = 0;
		  if (s)
		    stress_e.v = (float) s / 3;
		  else
		    stress_e.v = (float) 0.1;
		  do
		    {
		      d += du;
		      e = (j < nelm) ? &Elements[elm[j++]] : &Elements[0];
		      du = elm[j++];
		    }
		  while ((e->feat & vwl) && elm[j++] == s);
		  ntstress += d / 2;
		  break;
		}
	      ntstress += du;
	    }
	    }

      for (j = 0; j < nEparm; j++)
	tp[j] = filter(flt + j, interpolate(ce->name, Ep_name[j], &startyy[j], &end[j], (float) ce->p[j].stdy, t, dur));

      /* Now call the synth for each frame */

      pars.F0hz10 = base + (top - base) *
	interpolate("", "f0", &stress_s, &stress_e, (float) 0, tstress, ntstress);

      pars.AVdb = pars.AVpdb = tp[av];
      pars.AF = tp[af];
      pars.FNZhz = tp[fn];
      pars.ASP = tp[asp];
      pars.Aturb = tp[avc];
      pars.B1phz = pars.B1hz = tp[b1];
      pars.B2phz = pars.B2hz = tp[b2];
      pars.B3phz = pars.B3hz = tp[b3];
      pars.F1hz = tp[f1];
      pars.F2hz = tp[f2];
      pars.F3hz = tp[f3];
      /* AMP_ADJ + is a bodge to get amplitudes up to klatt-compatible levels
	 Needs to be fixed properly in tables
      */
      /*
	pars.ANP  = AMP_ADJ + tp[an];
      */
      pars.AB = AMP_ADJ + tp[ab];
      pars.A5 = AMP_ADJ + tp[a5];
      pars.A6 = AMP_ADJ + tp[a6];
      pars.A1 = AMP_ADJ + tp[a1];
      pars.A2 = AMP_ADJ + tp[a2];
      pars.A3 = AMP_ADJ + tp[a3];
      pars.A4 = AMP_ADJ + tp[a4];
      initparwave(&klatt_global, &pars);
      nextelement=0;
      tstress++; t++;
    }
    else { // hit end of DUR number of frames...
      nextelement=1;
      le = ce; // where we can put this?????? TODO!!!
    }
  }

  if (nextelement==0){
    // always run through samples till we hit next frame
    parwavesample(&klatt_global, &pars, outgoing, samplenumber,x); x++;
  //  outgoing[samplenumber]=rand()%32768;
    samplenumber++;
    if (samplenumber>klatt_global.nspfr) {
      // end of frame so...????
      newframe=1;
      samplenumber=0;
      top -= 0.5; // where we can put this?
    }
  }
}
}
      //ABOVE      le = ce; // where we can put this?????? TODO!!!



arm_biquad_casd_df1_inst_f32 df[5][5] __attribute__ ((section (".ccmdata")));
float coeffs[5][5][5] __attribute__ ((section (".ccmdata")));//{a0 a1 a2 -b1 -b2} b1 and b2 negate


// from simpleklatt


 for (y=0;y<40;y++){ // init frame
   //        frame[y]=simpleklattset.val[y];
	dir[y]=rand()%3;
	}

void generate_new_frame(int16_t* frame){
unsigned char y;
// put frame together from wormings

    for (y=0;y<40;y++){

      // direction change 0,1-back,2-forwards
      switch(dir[y]){
      case 0:
	// no change
	break;
      case 1:
	// forwards
	val[y]+=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]>maxs[y]) dir[y]=2;
	break;
      case 2:
	// backwards
	val[y]-=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]<mins[y]) dir[y]=1;
	break;
      }
//	printf("%d ",val[y]);
// we need to fill frame parameters:

//      frame=val;
      // copy it:
      frame[y]=val[y];
      
      }
      }
// these are constraints see klatt_params - TODO as a struct - in progress

// ref here:

int16_t val[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};


static wormedparamset simpleklattset={40,
    {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60},
    {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0},
{4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60}
  };

wormy* straightwormy;


/////
/*
void dosimpleklatt(void){
unsigned char y;
// put frame together from wormings

    for (y=0;y<40;y++){

      // direction change 0,1-back,2-forwards
      switch(dir[y]){
      case 0:
	// no change
	break;
      case 1:
	// forwards
	val[y]+=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]>maxs[y]) dir[y]=2;
	break;
      case 2:
	// backwards
	val[y]-=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]<mins[y]) dir[y]=1;
	break;
      }
//	printf("%d ",val[y]);
// we need to fill frame parameters:

//*framepointers[y]=val[y];
  }

// what does output point to?

//simple_parwave(globals, frame);

}
*/



void dosimpleklattsamples(int16_t* outgoing, u8 size){
  u8 x=0;
  static short samplenumber=0;
  static u8 newframe=1;
  while(x<size){
 
    // is it a new frame? - generate new frame
    if (newframe==1){
            generate_worm_frame();
      //                  generate_new_frame(frame);
    }

    single_parwave(globals,simpleklattset.val,newframe,samplenumber,x,outgoing);
    //    outgoing[x]=rand()%32768;

    if (newframe==1) newframe=0;
    x++;
    samplenumber++;
    if (samplenumber>globals->nspfr) { // greater than what???? 
      // end of frame so...????
      newframe=1;
      samplenumber=0;
  }
  }
}




void generate_worm_frame(){
  // there is both speed of the worm and how many times we run frame per worm
  // eg.

  for (int16_t x=0;x<adc_buffer[SPEED]>>4; x++){
  wormvaluedint(&simpleklattset,straightwormy, 1.0, 0, 0, 10); // paramset, worm, speed/float, offsetx/float, offsety/float, wormparam
  }
}



// get_sample_benddelta - exy to 32

void digitalker_benddelta(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 x=0,readpos;
  float remainder;
  extent nextent={31,33.0f};
  samplespeed=samplespeed*32.0f;

   if (samplespeed<=1){ 
     while (x<size){
       doadc();
	 u8 xaxis=_selx*nextent.maxplus; //0-16 for r, but now 14 params 0-13
	 MAXED(xaxis,nextent.max);
	 xaxis=nextent.max-xaxis;
	 exy[xaxis]=_sely;
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample_benddelta());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[x]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (x<size){
       doadc();
	 u8 xaxis=_selx*nextent.maxplus; 
	 MAXED(xaxis,nextent.max);
	 xaxis=nextent.max-xaxis;
	 exy[xaxis]=_sely;
	 samplel=(digitalk_get_sample_benddelta());//<<6)-32768; 
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       if (incoming[x]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;
	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
   }
}

//sam.h

//char input[]={"/HAALAOAO MAYN NAAMAEAE IHSTT SAEBAASTTIHAAN \x9b\x9b\0"};
//unsigned char input[]={"/HAALAOAO \x9b\0"};
//unsigned char input[]={"AA \x9b\0"};
//unsigned char input[] = {"GUH5DEHN TAEG\x9b\0"};

//unsigned char input[]={"/HEH3LOW2, /HAW AH YUX2 TUXDEY. AY /HOH3P YUX AH FIYLIHNX OW4 KEY.\x9b\0"};
//unsigned char input[]={"/HEY2, DHIHS IH3Z GREY2T. /HAH /HAH /HAH.AYL BIY5 BAEK.\x9b\0"};
//unsigned char input[]={"/HAH /HAH /HAH \x9b\0"};
//unsigned char input[]={"/HAH /HAH /HAH.\x9b\0"};
//unsigned char input[]={".TUW BIY5Y3,, OHR NAA3T - TUW BIY5IYIY., DHAE4T IHZ DHAH KWEH4SCHAHN.\x9b\0"};
//unsigned char input[]={"/HEY2, DHIHS \x9b\0"};

//unsigned char input[]={" IYIHEHAEAAAHAOOHUHUXERAXIX  \x9b\0"};
//unsigned char input[]={" RLWWYMNNXBDGJZZHVDH \x9b\0"};
//unsigned char input[]={" SSHFTHPTKCH/H \x9b\0"};

//unsigned char input[]={" EYAYOYAWOWUW ULUMUNQ YXWXRXLX/XDX\x9b\0"};




/* FOR TMS on lap
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



#define delay()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)

#define delayxx()						 do {	\
    register unsigned int ix;					\
    for (ix = 0; ix < 1000; ++ix)				\
      __asm__ __volatile__ ("nop\n\t":::"memory");		\
  } while (0)


////////////////////[[[[[[[[[[ TESTING LPC residual as excitation for sp0256

// this works but quite high volume

void lpc_error(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32], lastbuffer[32];
//(DelayN.ar(input,delaytime, delaytime)- LPCAnalyzer.ar(input,source,1024,MouseX.kr(1,256))).poll(10000)
//  do_impulse(carrierbuffer, 32, adc_buffer[SELX]>>2);
//  dowormwavetable(carrierbuffer, &wavtable, adc_buffer[SELX], size);
  int_to_floot(incoming,voicebuffer,size);
  //  LPC_cross(voicebuffer,carrierbuffer, lastbuffer,size);
  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  //  NTube_do(&tuber, otherbuffer, lastbuffer, 32);
    floot_to_int(outgoing,lastbuffer,size);
};



void sp0256_within_noLPC(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  TTS=0;
  // added trigger
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay

  float voicebuffer[32],lastbuffer[32];
  int16_t outgo[32];
  //  int_to_floot(incoming,voicebuffer,size);
  //  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  //  floot_to_int(outgo,voicebuffer,size);
  for (u8 x=0;x<size;x++) outgo[x]=(incoming[x])>>4; // say 11 bits

    u8 xx=0,x=0,readpos;
  float remainder;
    samplespeed/=8.0;
  //  samplespeed=1.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample_withLPC(outgo[xx]); // check this!
	 //	 	 samplel=outgo[xx];
	 samplepos-=1.0f;
	        }
       remainder=samplepos; 
       //       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample_withLPC(outgo[xx]);
	      //	 samplel=outgo[xx];

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
     }
};

void sp0256_within(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ 

  TTS=0;
  // added trigger
  if (trigger==1) sp0256_newsay1219(); // selector is in newsay

  float voicebuffer[32],lastbuffer[32];
  int16_t outgo[32];
  int_to_floot(incoming,voicebuffer,size);
  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  floot_to_int(outgo,lastbuffer,size);
  for (u8 x=0;x<size;x++) outgo[x]=(outgo[x])>>4; // say 11 bits

    u8 xx=0,x=0,readpos;
  float remainder;
    samplespeed/=8.0;
  //  samplespeed=1.0f;
   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample_withLPC(outgo[x++]); // check this!
	 //	 	 samplel=outgo[xx];
	 samplepos-=1.0f;
	        }
       remainder=samplepos; 
       //       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
              samplel=sp0256_get_sample_withLPC(outgo[xx]);
	      //	 samplel=outgo[xx];

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
     }
};




int16_t sp0256_get_sample_withLPC(INT16 samp){
  static int16_t output; 
   
      u8 howmany=0;
            while(howmany==0){ 
   
   if (m_halted==1 && m_filt.rpt <= 0)     {
     sp0256_newsay1219();
   }

      micro();
      howmany=lpc12_update_withsample(&m_filt, samp, &output);
          }
   return output;
 }





// adding lpc12_update_withsample

static inline u8 lpc12_update_withsample(struct lpc12_t *f, INT16 samp, INT16* out)
{
	u8 j;
	//	INT16 samp;
	u8 do_int;

	/* -------------------------------------------------------------------- */
	/*  Iterate up to the desired number of samples.  We actually may       */
	/*  break out early if our repeat count expires.                        */
	/* -------------------------------------------------------------------- */
		/* ---------------------------------------------------------------- */
		/*  Generate a series of periodic impulses, or random noise.        */
		/* ---------------------------------------------------------------- */
		do_int = 0;
		//		samp   = 0;
		if (f->per_orig)
		{
		  int16_t val = (_selx*142.0f);
		  //		  int16_t val=70;
		  MAXED(val,140)
		    val=140-val;
		  f->per=f->per_orig+(70 - val);//+(adc_buffer[SELY]>>5);
		  //		  f->per=f->per_orig;
			if (f->cnt <= 0)
			{
				f->cnt += f->per;
				//	samp    = f->amp;
				f->rpt--;
				do_int  = f->interp;

				for (j = 0; j < 6; j++)
					f->z_data[j][1] = f->z_data[j][0] = 0;

			} else
			{
			  //  samp = 0; /// ??? keeps noise out?
			  f->cnt--;
			}

		} else // do noise
		{
			u8 bit;

			if (--f->cnt <= 0)
			{
				do_int = f->interp;
				f->cnt = PER_NOISE;
				f->rpt--;
				for (j = 0; j < 6; j++)
					f->z_data[j][0] = f->z_data[j][1] = 0;
			}

			bit = f->rng & 1;
			f->rng = (f->rng >> 1) ^ (bit ? 0x4001 : 0);

						if (bit) { samp =  f->amp; }
						else     { samp = -f->amp; }
		}

		/* ---------------------------------------------------------------- */
		/*  If we need to, process the interpolation registers.             */
		/* ---------------------------------------------------------------- */
		if (do_int)
		{
			f->r[0] += f->r[14];
			f->r[1] += f->r[15];

			f->amp   = (f->r[0] & 0x1F) << (((f->r[0] & 0xE0) >> 5) + 0);
			f->per_orig   = f->r[1];

			do_int   = 0;
		}

		/* ---------------------------------------------------------------- */
		/*  Stop if we expire our repeat counter and return the actual      */
		/*  number of samples we did.                                       */
		/* ---------------------------------------------------------------- */
		if (f->rpt <= 0) return 0; 

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
		//		samp=samp>>2;
				for (j = 0; j < 6; j++)
		{
			samp += (((int32_t)f->b_coef[j] * (int32_t)f->z_data[j][1]) >> 9);
			samp += (((int32_t)f->f_coef[j] * (int32_t)f->z_data[j][0]) >> 8);

			f->z_data[j][1] = f->z_data[j][0];
			f->z_data[j][0] = samp;
			}
		*out= limit(samp)<<2;
		return 1;
}


    // testing changing test_elm
      //      u8 axis=adc_buffer[SELX]>>8; // 16*3=48
      // change element, change length? leave stress as is 0
      //      test_elm[axis*3]=phoneme_prob_remap[adc_buffer[SELY]>>6]; // how many phonemes?=64
      //      test_elm[(axis*3)+1]=(adc_buffer[SELZ]>>7)+1; // length say max 32
    

      //      oldmode=mode;    
      //      mode=adc_buffer[MODE]>>7; // 12 bits to say 32 modes (5 bits)
      //           mode=10; // TESTING

      //       if(lpc_busy() == 0) lpc_newsay(adc_buffer[SELX]>>6);   

      //    if(lpc_busy() != 0)    lpc_running(); // so just writes once otherwise gets messy...


  // if there is a change in mode do something?
  //  if (oldmode!=mode){
  //    maintrigger=1;
  //  }

	   /*  if (maintrigger==1) {writepos=0;trigger=1;} // STRIP_OUT

  switch(mode){
  case 0:// rsynth/klatt-single phoneme
           if (trigger==1){
	     trigger=0;
	     u8 phonemm=phoneme_prob_remap[(adc_buffer[SELX]>>6)]; // 7bits=128 %69//6=64
	     pair xx=klatt_phoneme(writepos,phonemm); 
	     generated=xx.generated;
	     writepos=xx.writepos;
	   }
	   break;
  case 1: // rsynth/klatt-chain of phonemes
    writepos=run_holmes(writepos); 
    break;
  case 2: // vosim free running
    writepos=runVOSIM_SC(writepos);
    break;
  case 3: // VOSIMondemand
    if (trigger==1){
      trigger=0;
      float freqwency = (float)((adc_buffer[SELX])+100);//1500.0f; 
      float cycles = (float)((adc_buffer[SELY]>>4)+2);
      float decay = ((float)(adc_buffer[SELZ])/4096.0f); // TODO as SELZ!
      pair xx=demandVOSIM_SC(writepos,freqwency,cycles,decay); 
      generated=xx.generated;
      writepos=xx.writepos;
    }
    break;
  case 9: // SAM full. no writepos though and just a simple proof here
        if (trigger==0){
    	SAMMain();
	trigger=1;
	     }     
    break;
  case 10:
    if(lpc_busy() == 0) lpc_newsay(adc_buffer[SELX]>>6);   

    if(lpc_busy() != 0)    lpc_running(); // so just writes once otherwise gets messy...
    break;
  case 19: // parwave/simpleklatt
    dosimpleklatt();
    break;

  } // cases

    // now readpos is back to one now that we have written something 
  if (maintrigger==1) {
      readpos=0;
      maintrigger=0;
  }
	   */


int16_t votrax_get_sample_rawparam(){ // TODO: trying new model - but still is kind of noisy
  uint16_t sample; u8 x;
  //  m_cclock = m_mainclock / intervals[(int)(_selz*8.0f)]; // TESTING - might need to be array of intervals ABOVE
    //        lenny=96;
  m_sample_count++;
  if(m_sample_count & 1)
    chip_update();
  //  m_cur_f1=(_selz*126.0f)+1;
  sample=analog_calc();//TODO: check extent of analog_calc value - seems OK
  // hit end and then newsay

  if (sample_count++>=lenny){
    sample_count=0;
    votrax_newsay_rawparam();
  }
  return sample;
}

void votrax_newsay_rawparam(){
  phone_commit_pbend();
  lenny=(int)((1.0f-_selz)*12000.0f)+600; // 600 is lowest we can have lenny
  //  int durry=(int)((1.0f-_selz)*32.0f);
  // lenny=((16*(durry*4+1)*4*9+2)/30);
}

void phone_commit_pbend() // parameter bend
{
	// Only these two counters are reset on phone change, the rest is
	// free-running.
	m_phonetick = 0;
	m_ticks = 0;

  /*
    u8 m_rom_vd, m_rom_cld;                         // Duration in ticks of the "voice" and "closure" delays, 4 bits
    u8 m_rom_fa, m_rom_fc, m_rom_va;                // Analog parameters, noise volume, noise freq cutoff and voice volume, 4 bits each
    u8 m_rom_f1, m_rom_f2, m_rom_f2q, m_rom_f3;     // Analog parameters, formant frequencies and Q, 4 bits eac
   */
	// order which makes most sense

	m_rom_va  = exy[0]*4.0f;
	m_rom_f1  = exy[1]*16.0f;
	m_rom_f2  = exy[2]*16.0f;
	m_rom_f3  = exy[3]*16.0f;
	m_rom_f2q = exy[4]*4.0f;
	m_rom_fc  = exy[5]*16.0f;
	m_rom_fa  = exy[6]*4.0f;
	m_rom_cld = exy[7]*12.0f;
	m_rom_vd  = exy[8]*8.0f;
	//	m_rom_closure  = exy[9]+0.5f; // does this give us 1 or 0? YES!
	//	m_rom_duration = exy[10]*130.0f;
	//m_rom_pause = exy[10]+0.5f;
	m_rom_closure=0;
	m_rom_pause=0; // pause can be zero or one
	// we have 12 of exy
	if(m_rom_cld == 0)
	  m_cur_closure = m_rom_closure;
}



// to one side from 1/2 +

/// - from main.c

extern Formlet *formy;
extern Formant *formanty;
extern Blip *blipper;
extern RLPF *RLPFer;
extern NTube tuber;
extern Wavetable wavtable;
extern wormy myworm;
extern biquad* newBB;

  //       LPCAnalyzer_init();
  init_synth(); // which one? --> klatt rsynth !!!! RENAME!
  lpc_init(); 
/*   simpleklatt_init(); */
/* sam_init(); */
/* sam_newsay(); // TEST! */
/* tms5200_init(); */
/* tms5200_newsay(); */
/* channelv_init(); */
/* tube_init(); */
/* tube_newsay(); */
/* BANDS_Init_(); */
/* Vocoder_Init(32000.0f); */
/* digitalk_init(); */
/* digitalk_newsay(0); */
/* nvp_init(); */
/* sample_rate_init(); */
/* initbraidworm(); // re_name */
/* initvoicform(); */
/*   formy=malloc(sizeof(Formlet)); */
/*   formanty=malloc(sizeof(Formant)); */
/*   blipper=malloc(sizeof(Blip)); */
/*   RLPFer=malloc(sizeof(RLPF)); */
/*   Formlet_init(formy); */
/*   Formant_init(formanty); */
/*   Blip_init(blipper); */
/*   RLPF_init(RLPFer); */
/*   NTube_init(&tuber); */
/*    wavetable_init(&wavtable, crowtable_slower, 283); // now last arg as length of table=less than 512 */
/*   //    wavetable_init(&wavtable, plaguetable_simplesir, 328); // now last arg as length of table=less than 512 */
/*   //  wavetable_init(&wavtable, table_kahrs000, 160); // now last arg as length of table=less than 512 */
/*     //  addwormsans(&myworm, 10.0f,10.0f,200.0f, 200.0f, wanderworm); */
/*   //  RavenTube_init(); */
//   newBB=BiQuad_new(LPF, 1.0, 1500, 32000, 0.68); // TEST? 
//        votrax_init(); 



/// - from audio.c

RLPF *RLPFer;
Formlet *formy;
Formant *formanty;
Blip *blipper;
NTube tuber;
Wavetable wavtable;
wormy myworm;
biquad* newBB;


  /*
      for (x=0;x<sz/2;x++){ // STRIP_OUT - leave for TESTING
      readpos=samplepos;
      mono_buffer[x]=audio_buffer[readpos];
      samplepos+=samplespeed;
      if (readpos>=ending) samplepos=0.0f;    
      }
    */


void tubes(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

   if (samplespeed<=1){ 
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tube_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { 
     while (xx<size){
       samplel=tube_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }
};


void sammy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  static u8 triggered=0;
  u8 x=0,readpos;
  static u8 howmany=0;
  float remainder;
  //  float xx,xxx;
  //  float tmpbuffer[32];

     // test lowpass/biquad:
     /*
     int_to_floot(outgoing,tmpbuffer,32);
    for (x=0;x<32;x++){
    xxx=tmpbuffer[x];
    xx=BiQuad(xxx,newB); 
    tmpbuffer[x]=xx;
      }
    floot_to_int(outgoing,tmpbuffer,32);
     */

 
   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (x<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 while (howmany==0) howmany=(sam_get_sample(&samplel)); 
	 howmany--;
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[x]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[x]=samplel;

       // TEST trigger: 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

       x++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (x<size){
       while (howmany==0)	 howmany=(sam_get_sample(&samplel)); 
       howmany--;
       if (samplepos>=samplespeed) {       
	 outgoing[x]=samplel;
       // TEST trigger: 
       if (incoming[x]>THRESH && !triggered) {
	 sam_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[x]<THRESHLOW && triggered) triggered=0;

	 x++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
       }
       }

  // refill back counter etc.
}

void tms5220talkie(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(lpc_get_sample()<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 lpc_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=(lpc_get_sample()<<6)-32768; 

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 lpc_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
};

void newvotrax(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=votrax_get_sample(); 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=votrax_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 votrax_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
};


//float flinbuffer[MONO_BUFSZ];
//float flinbufferz[MONO_BUFSZ];
//float floutbuffer[MONO_BUFSZ];
//float floutbufferz[MONO_BUFSZ];

void fullklatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params SELZ is pitch bend commentd now OUT
  //  klattrun(outgoing, size);
  float tmpbuffer[32]; float xxx,x;
  u8 xx=0;
     while (xx<size){
       outgoing[xx]=klatt_get_sample();
       xx++;
     }

     // test lowpass/biquad:
     
     int_to_floot(outgoing,tmpbuffer,32);
    for (xx=0;xx<32;xx++){
    xxx=tmpbuffer[xx];
    xxx=BiQuad(xxx,newBB); 
    tmpbuffer[xx]=xxx;
      }
    floot_to_int(outgoing,tmpbuffer,32);
    };

void tms5200mame(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // MODEL GENERATOR: TODO is speed and interpolation options
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(tms5200_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tms5200_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=(tms5200_get_sample());//<<6)-32768; 

       /// interpol filter but in wavetable case was 2x UPSAMPLE????
       //        samplel = doFIRFilter(wavetable->FIRFilter, interpolatedValue, i);

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tms5200_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

	 xx++;
	 samplepos-=samplespeed;
       }
       samplepos+=1.0f;
     }
   }

  // refill back counter etc.
};


/*void nvpSR(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // test samplerate conversion
  dosamplerate(incoming, outgoing, samplespeed, size);
  }*/

void foffy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  for (u8 x=0;x<32;x++){
    outgoing[x]=fof_get_sample();
  }
}

void voicformy(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // GENERAL TESTING!
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32], lastbuffer[32];
  //  dochannelvexcite(carrierbuffer,size); // voicform has own excitation
  //    dovoicform(carrierbuffer, otherbuffer, size);
  //Formlet_process(Formlet *unit, int inNumSamples, float* inbuffer, float* outbuffer){
  //  Formlet_setfreq(formy,adc_buffer[SELY]);
  //  Formlet_process(formy, 32, carrierbuffer,otherbuffer);
  //    Formant_process(formanty, adc_buffer[SELX], adc_buffer[SELY], adc_buffer[SELZ], size, otherbuffer); // fundfreq: 440, formfreq: 1760, bwfreq>funfreq: 880 TODO- figure out where is best for these to lie/freq ranges
  //  int_to_floot(incoming,voicebuffer,size);
//  doVOSIM_SC(voicebuffer, otherbuffer,size); // needs float in for trigger

//  RenderVosim(incoming, outgoing, size, adc_buffer[SELX]<<3, adc_buffer[SELY]<<3, adc_buffer[SELZ]<<1);// last is pITCH 
//  RenderVowel(incoming, outgoing, size, adc_buffer[SELX]<<4, adc_buffer[SELY], adc_buffer[SELZ]>>2, adc_buffer[SPEED]<<3);// last is pITCH 
  float freq=(float)(adc_buffer[SELX]>>4);

/*

APEX:
	*ar { arg fo=100, invQ=0.1, scale=1.4, mul=1;
		var flow;
		flow = RLPF.ar(Blip.ar(fo, mul: 10000), scale*fo, invQ, invQ/fo);
		^HPZ1.ar(flow, mul);   // +6 dB/octave caret ^ returns from the method

 */

// vocal/glottal source
  Blip_do(blipper, carrierbuffer, size, freq,2,10000.0);
  RLPF_do(RLPFer, carrierbuffer, voicebuffer, 100, 1.4f, 32, 0.001);
  HPZ_do(carrierbuffer,otherbuffer,32);
  //  Formlet_setfreq(formy,adc_buffer[SELY]>>2);
  //  Formlet_process(formy, 32, otherbuffer,lastbuffer);
  NTube_do(&tuber, otherbuffer, lastbuffer, 32);

  /*  size_t vowel_index = param1 >> 12; // 4 bits?
  uint16_t balance = param2 & 0x0fff; // 4096
  uint16_t formant_shift = (200 + (param3)); // as 10 bits
  */
  //  for (u8 xx=0;xx<32;xx++) outgoing[xx]=incoming[xx];

   floot_to_int(outgoing,lastbuffer,size);
}

void nvp(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // MODEL GENERATOR: TODO is speed and interpolation options
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(nvp_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay(); // selector is in newsay
	 triggered=1;
	   }
       else if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       if (samplepos>=samplespeed) {       
       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 nvp_newsay(); // selector is in newsay
	 triggered=1;
	   }
       else if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 samplel=(nvp_get_sample());//<<6)-32768; 
	 outgoing[xx]=samplel;
	 xx++;
	 samplepos-=samplespeed;
       }
       else samplel=(nvp_get_sample());//<<6)-32768; 
       samplepos+=1.0f;
     }
   }

}; // use this as NEW template

void digitalker(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  TTS=0;
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  samplespeed=samplespeed*8.0f;

   if (tmpsamplespeed<=1){ 
     while (xx<size){
       doadc();
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); 
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
       xx++;
       samplepos+=tmpsamplespeed;
     }
   }
   else { 
     while (xx<size){
       doadc();
       samplel=(digitalk_get_sample());//<<6)-32768; 
       if (samplepos>=tmpsamplespeed) {       
	 outgoing[xx]=samplel;
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;
	 xx++;
	 samplepos-=tmpsamplespeed;
       }
       samplepos+=1.0f;
     }
   }
};



void simpleklatt(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // first without speed or any params - break down as above in tms, spo256 - pull out size
  dosimpleklattsamples(outgoing, size);
};


void channelv(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  dochannelv(incoming,outgoing, size);
}
;
// testing various vocoder and filter implementations:

void testvoc(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  // NO SPEED as is live

  //  dochannelv(incoming,outgoing, size);
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32];
  int_to_floot(incoming,voicebuffer,size);
  dochannelvexcite(carrierbuffer,32);
//  mdavocal_process(&mdavocall, voicebuffer, carrierbuffer, 32);
  //  runVocoder(vocoderr, voicebuffer, carrierbuffer, otherbuffer, size);
  //void runSVFtest_(SVF* svf, float* incoming, float* outgoing, u8 band_size){
  //  runBANDStest_(voicebuffer, otherbuffer, size);

  Vocoder_Process(voicebuffer, carrierbuffer, otherbuffer, size); // wvocoder.c
  floot_to_int(outgoing,otherbuffer,size);
};

/*void LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int inNumSamples);

void LPCanalyzer(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float voicebuffer[32],otherbuffer[32];

  //  LPCAnalyzer_next(float *inoriginal, float *indriver, float *out, int p, int testE, float delta, int inNumSamples) {
    //  convert in to float
    //  exciter=indriver to float
       int_to_floot(incoming,voicebuffer,size);
       LPCAnalyzer_next(NULL, voicebuffer, otherbuffer, 10, size); //poles=10 - CROW TEST!
	//    out from float to int
   floot_to_int(mono_buffer,otherbuffer,size);
};*/

void lpc_error(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32], lastbuffer[32];
//(DelayN.ar(input,delaytime, delaytime)- LPCAnalyzer.ar(input,source,1024,MouseX.kr(1,256))).poll(10000)
//  do_impulse(carrierbuffer, 32, adc_buffer[SELX]>>2);
//  dowormwavetable(carrierbuffer, &wavtable, adc_buffer[SELX], size);
  int_to_floot(incoming,voicebuffer,size);
  //  LPC_cross(voicebuffer,carrierbuffer, lastbuffer,size);
  LPC_residual(voicebuffer, lastbuffer,size); // WORKING!
  //  NTube_do(&tuber, otherbuffer, lastbuffer, 32);
    floot_to_int(outgoing,lastbuffer,size);
};

void test_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){ // how we choose the wavetable - table of tables?
  float lastbuffer[32];
  //  dowavetable(lastbuffer, &wavtable, 2, size);
  //  dowavetable(lastbuffer, &wavtable, 2+(1024-(adc_buffer[SPEED]>>2)), size);

  dowavetable(lastbuffer, &wavtable, 2.0f + (1024.0f*_speed), size);
  floot_to_int(outgoing,lastbuffer,size);
}  

void test_worm_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float lastbuffer[32], otherbuffer[32];
        dowavetable(otherbuffer, &wavtable, adc_buffer[SELX], size);
  //  dowormwavetable(otherbuffer, &wavtable, adc_buffer[SELX], size);
	//  NTube_do(&tuber, otherbuffer, lastbuffer, 32);
  //donoise(otherbuffer,32);

	//	RavenTube_next(otherbuffer, lastbuffer, size);
  floot_to_int(outgoing,lastbuffer,size);
}  


//void wormunfloat(wormy* wormyy, float speed, float param, float *x, float *y){ // for worm as float and no constraints
void wormas_wave(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){
  float lastbuffer[32]; float x,y,speed; u8 param;
  speed=(float)adc_buffer[SELX]/1024.0f;
  param=adc_buffer[SELY]>>4;
  for (u8 xx=0;xx<size;xx++){
    wormunfloat(&myworm, speed, param, &x, &y); // needs to be slower
    lastbuffer[xx]=y;
  }
  floot_to_int(outgoing,lastbuffer,size);
}

void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *(src++);
		sz--;
		*(rdst++) = *(src++);
		//		*(rdst++) = 0;
		sz--;
	}
}

inline void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = (*rsrc++);
		sz--;
	}
}

/*extern u8 trigger;
extern u16 generated;
extern u16 writepos;
extern u8 mode; /// ????? TODO do we need these?
*/


void (*generators[])(int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size)={tms5220talkie, fullklatt, sp0256, simpleklatt, sammy, tms5200mame, tubes, channelv, testvoc, digitalker, nvp, foffy, voicformy, lpc_error, test_wave, wormas_wave, test_worm_wave, newvotrax, sp0256, sp0256TTS, sp0256vocabone, sp0256vocabtwo, sp0256_1219, sp0256rawone, sp0256rawtwo, sp0256bend};

