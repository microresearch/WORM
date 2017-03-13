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

  // MODEL GENERATOR: TODO is speed and interpolation options DONE
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=UPSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=tube_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 tube_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=tube_get_sample();

       if (samplepos>=samplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
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

  // refill back counter etc.
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
  // MODEL GENERATOR: TODO is speed and interpolation options
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
  // lpc_running

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate
  float tmpsamplespeed=samplespeed*32.0f;

   if (tmpsamplespeed<=1){ // slower=DOWNSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=(digitalk_get_sample());//<<6)-32768; 
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       //       outgoing[xx]=samplel;

       // TEST trigger: 
       if (incoming[xx]>THRESH && !triggered) {
	 digitalk_newsay(); // selector is in newsay
	 triggered=1;
	   }
       if (incoming[xx]<THRESHLOW && triggered) triggered=0;

       xx++;
       samplepos+=tmpsamplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=(digitalk_get_sample());//<<6)-32768; 

       if (samplepos>=tmpsamplespeed) {       
	 outgoing[xx]=samplel;
       // TEST trigger: 
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

