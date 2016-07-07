#include "audio.h"
//#include "render.h"
#include "RenderTabs.h"
#include <stdio.h>

#define abs(a)	   (((a) < 0) ? -(a) : (a))

extern __IO uint16_t adc_buffer[10];

//timetable for more accurate c64 simulation
u8 timetable[5][5] =
{
	{162, 167, 167, 127, 128},
	{226, 60, 60, 0, 0},
	{225, 60, 59, 0, 0},
	{200, 0, 0, 54, 55},
	{199, 0, 0, 54, 54}
};

/*void Output(int index, unsigned char A)
{
  static unsigned oldtimetableindex = 0, oldbufferpos, older=5;
	int k;
	// but is the NEXT index which determines our length?

	bufferpos += timetable[oldtimetableindex][index];
	//	printf("+= %d old %d INDEX %d BUFF %d\n",timetable[oldtimetableindex][index],oldtimetableindex,index,bufferpos/50);
	//printf("diffpos %d oldind %d ind %d\n",(bufferpos/50)-(oldbufferpos/50), oldtimetableindex, index);
	oldtimetableindex = index;
	// write a little bit in advance
	// so is overwritten NEXT TIME with index
	//	older=5;
	for(k=0; k<older; k++)
	  printf("%c",(A & 15)*16);
	//	for(k=0; k<5; k++)
	//	  buffer[bufferpos/50 + k] = (A & 15)*16;
	
	older=	(bufferpos/50)-(oldbufferpos/50);
	oldbufferpos=bufferpos;
	}*/

extern int32_t bufferpos;//oldbufferpos=0;


static inline unsigned char Output(unsigned char index) // is one step behind what it should be
/* 
   nextbufferpos-currentbufferpos 
   but we don't have that yet...

   always keep next sample in reserve... this is now done in sam.c but is all still glitchy

 */

{
  static unsigned char oldtimetableindex = 0;//, older=5;
  unsigned char k;
	bufferpos += timetable[oldtimetableindex][index];
	oldtimetableindex = index;
	//	older=	(bufferpos/50)-(oldbufferpos/50);
	//	oldbufferpos=bufferpos;
	//	k=older;
		k=5;
	return k;
}

unsigned char wait1 = 7;
unsigned char wait2 = 6;

extern unsigned char A, X, Y;
extern unsigned char mem44;
extern unsigned char mem47;
extern unsigned char mem49;
extern unsigned char mem39;
extern unsigned char mem50;
extern unsigned char mem51;
extern unsigned char mem53;
extern unsigned char mem56;
extern unsigned char mem66;

extern unsigned char speedd;
extern unsigned char pitch;
extern int singmode;

//unsigned char phase1 = 0;  //mem43
unsigned char phase2;
unsigned char phase3;
//unsigned char mem66;
unsigned char mem38;
unsigned char mem40;
unsigned char speedcounter; //mem45
unsigned char mem48;
unsigned char mem48stored;

extern unsigned char phonemeIndexOutput[60]; //tab47296
extern unsigned char stressOutput[60]; //tab47365
extern unsigned char phonemeLengthOutput[60]; //tab47416

unsigned char pitches[256]; // tab43008

unsigned char frequency1[256];
unsigned char frequency2[256];
unsigned char frequency3[256];

unsigned char amplitude1[256];
unsigned char amplitude2[256];
unsigned char amplitude3[256];

unsigned char sampledConsonantFlag[256]; // tab44800


void AddInflection(unsigned char mem48, unsigned char phase1);
unsigned char trans(unsigned char mem39212, unsigned char mem39213);

//written by me because of different table positions.
// mem[47] = ...
// 168=pitches
// 169=frequency1
// 170=frequency2
// 171=frequency3
// 172=amplitude1
// 173=amplitude2
// 174=amplitude3
unsigned char Read(unsigned char p, unsigned char Y)
{
	switch(p)
	{
	case 168: return pitches[Y];
	case 169: return frequency1[Y];
	case 170: return frequency2[Y];
	case 171: return frequency3[Y];
	case 172: return amplitude1[Y];
	case 173: return amplitude2[Y];
	case 174: return amplitude3[Y];
	}
	return 0;
}

void Write(unsigned char p, unsigned char Y, unsigned char value)
{

	switch(p)
	{
	case 168: pitches[Y] = value; return;
	case 169: frequency1[Y] = value;  return;
	case 170: frequency2[Y] = value;  return;
	case 171: frequency3[Y] = value;  return;
	case 172: amplitude1[Y] = value;  return;
	case 173: amplitude2[Y] = value;  return;
	case 174: amplitude3[Y] = value;  return;
	}
}



// -------------------------------------------------------------------------
//Code48227
// Render a sampled sound from the sampleTable.
//
//   Phoneme   Sample Start   Sample End
//   32: S*    15             255
//   33: SH    257            511
//   34: F*    559            767
//   35: TH    583            767
//   36: /H    903            1023
//   37: /X    1135           1279
//   38: Z*    84             119
//   39: ZH    340            375
//   40: V*    596            639
//   41: DH    596            631
//
//   42: CH
//   43: **    399            511
//
//   44: J*
//   45: **    257            276
//   46: **
// 
//   66: P*
//   67: **    743            767
//   68: **
//
//   69: T*
//   70: **    231            255
//   71: **
//
// The SampledPhonemesTable[] holds flags indicating if a phoneme is
// voiced or not. If the upper 5 bits are zero, the sample is voiced.
//
// Samples in the sampleTable are compressed, with bits being converted to
// bytes from high bit to low, as follows:
//
//   unvoiced 0 bit   -> X
//   unvoiced 1 bit   -> 5
//
//   voiced 0 bit     -> 6
//   voiced 1 bit     -> 24
//
// Where X is a value from the table:
//
//   { 0x18, 0x1A, 0x17, 0x17, 0x17 };
//
// The index into this table is determined by masking off the lower
// 3 bits from the SampledPhonemesTable:
//
//        index = (SampledPhonemesTable[i] & 7) - 1;
//
// For voices samples, samples are interleaved between voiced output.

static inline u8 rendervoicedsample(unsigned char *mem66, int16_t* sample, u8 state, u8* howmany){

  static unsigned char phase1;
  u8 tempA;
  signed char pitchmod=(adc_buffer[SELX]>>5)-64; // -64 to +64 I hope
  
  if (state==0){ // beginning /////////
	// current phoneme's index
	mem49 = Y;

	// mask low three bits and subtract 1 get value to 
	// convert 0 bits on unvoiced samples.
	A = mem39&7;
	X = A-1;

    // store the result
	mem56 = X;
	
	// determine which offset to use from table { 0x18, 0x1A, 0x17, 0x17, 0x17 }
	// T, S, Z                0          0x18
	// CH, J, SH, ZH          1          0x1A
	// P, F*, V, TH, DH       2          0x17
	// /H                     3          0x17
	// /X                     4          0x17

    // get value from the table
	mem53 = tab48426[X];
	mem47 = X;      //46016+mem[56]*256
	
	// voiced sample?
	Y = mem49;
	pitchmod+=pitches[mem49];
	if (pitchmod>126) pitchmod=126;
	else if (pitchmod<1) pitchmod=1;
	A = (pitchmod) >> 4;

	// handle voiced samples here
	// number of samples?
	phase1 = A ^ 255;
	Y = *mem66;
	state=2; // jump to outer loop
  } // 0 state

  // outer loop

  if (state==2) {
    mem56 = 8;
    //A = Read(mem47, Y);
    // fetch value from table
    A = sampleTable[mem47*256+Y];
    state=1;
  }

  // inner loop
  if (state==1) {
    //48327: ASL A
    //48328: BCC 48337
			
    // left shift and check high bit
    tempA = A;
    A = A << 1;
    if ((tempA & 128) != 0)
      {
	// if bit set, output 26
	X = 26;
	//		Output(3, X);
	*howmany=Output(3);
	//		*sample=((X)<<12)-28672; // check >>12???
			*sample=((X-8)<<12); //1 byte
			
      } else
      {
	//timetable 4
	// bit is not set, output a 6
	X=6;
	//	Output(4, X);
	*howmany=Output(4);
	//		*sample=((X)<<12)-28672; // check >>12???
			*sample=((X-8)<<12); //1 byte

      }

    mem56--;
    if (mem56==0) {
      state=2; // outer loop
      Y++;
      // continue until counter done
      phase1++;

    if (phase1 == 0) { //started - first inc we don't check
      state=0;
	// restore values and return - when?
      	A = 1;
	mem44 = 1;
	*mem66 = Y;
	Y = mem49;
	//	return 0;
    }
    }
    ///////////////

  }
  return state;
		////////////////////////
  }

static inline u8 renderunvoicedsample(unsigned char *mem66, int16_t* sample, u8 state, u8* howmany){
	u8 tempA;

  if (state==3) goto pos48274;
  else if (state==5) goto pos48280;
  else if (state==6) goto pos48295;

  // A&248 !=0

	// current phoneme's index
	mem49 = Y;

	// mask low three bits and subtract 1 get value to 
	// convert 0 bits on unvoiced samples.
	A = mem39&7;
	X = A-1;

    // store the result
	mem56 = X;
	
	// determine which offset to use from table { 0x18, 0x1A, 0x17, 0x17, 0x17 }
	// T, S, Z                0          0x18
	// CH, J, SH, ZH          1          0x1A
	// P, F*, V, TH, DH       2          0x17
	// /H                     3          0x17
	// /X                     4          0x17

    // get value from the table
	mem53 = tab48426[X];
	mem47 = X;      //46016+mem[56]*256
	
	// voiced sample?
	A = mem39 & 248;

	Y = A ^ 255;

pos48274:
         
    // step through the 8 bits in the sample
	mem56 = 8;
	
	// get the next sample from the table
    // mem47*256 = offset to start of samples
	A = sampleTable[mem47*256+Y];

pos48280:

    // left shift to get the high bit
	tempA = A;
	A = A << 1;
	//48281: BCC 48290
	
	// bit not set?
	if ((tempA & 128) == 0)
	{
        // convert the bit to value from table
		X = mem53;
		//mem[54296] = X;
        // output the byte
		//		Output(1, X);
		*howmany=Output(1);

		//		*sample=((X&15)<<12)-28672; // check >>12??? .. but we can't output further one?
				*sample=(((X&15)-8)<<12); //1 byte
		//	*sample=(rand()%65536)-32768;

		if (X!=0) goto pos48296;
		else return 6;
	}		// if X != 0, exit loop
		//		if(X != 0) goto pos48296;
pos48295:
	//			Output(2, 5);
	*howmany=Output(2);

	//	*sample=((5)<<12)-28672; // check >>12???
		*sample=((-3)<<12); //1 byte
	//		*sample=(rand()%65536)-32768;


pos48296:
	X = 0;
    // decrement counter
	mem56--;
	if (mem56 != 0) return 5;
	
	// increment position
	Y++;
	//	if (Y != 0) goto pos48274
	if (Y != 0) return 3;
	
	// restore values and return
	mem44 = 1;
	Y = mem49;
	return 0; // return state which is - ended?

}

void renderupdate(){

  printf("mem49 %d speedcounter %d Y %d X %d mem38 %d mem44 %d mem48 %d mem66 %d\n",mem49,speedcounter,Y,X,mem38,mem44,mem48,mem66);

}

void    sam_frame_rerun() {
  signed char pitchmod=(adc_buffer[SELX]>>5)-64; // -64 to +64 I hope

  //	phase1 = 0;
	phase2 = 0;
	phase3 = 0;
	mem49 = 0;
	speedcounter = speedd; //sam standard speed
	//	speedcounter = (adc_buffer[SELY]>>4)+1;

	mem48=mem48stored;

	Y = 0;
	pitchmod+=pitches[0];
	if (pitchmod>126) pitchmod=126;
	else if (pitchmod<1) pitchmod=1;
	A = (pitchmod) >> 4;

	//	A = pitches[0];
	mem44 = A;
	X = A;
	mem38 = A - (A>>2);     // 3/4*A ???
	mem66=0;
}

u8 rendersamsample(int16_t* sample,u8* ending){
  signed char pitchmod=(adc_buffer[SELX]>>5)-64; // -64 to +64 I hope
  static u8 state=0;
  static unsigned char phase1 = 0;  //mem43
  u8 carry=0;
  static u8 secondstate=0;
  u8 nosample=1; u8 howmany=0;
  //  printf("mem49 %d speedcounter %d Y %d X %d mem38 %d mem44 %d mem48 %d\n",mem49,speedcounter,Y,X,mem38,mem44,mem48);

  
  while (nosample){

    if (state==3 || state==4 || state==5 || state==6){ // in process of rendering unvoiced sample
      state=renderunvoicedsample(&mem66,sample,state,&howmany);
      if (state==0){			// skip ahead two in the phoneme buffer - once we're done
		  Y += 2;
		  mem48 -= 2;
		  state=1; secondstate=0;
		  if(mem48 == 0) 	{
		    *ending=1;
		    state=0;
		    sam_frame_rerun();
		    return howmany; // ended
		  }
		  		  speedcounter = speedd;
		  //		  speedcounter = (adc_buffer[SELY]>>4)+1;
      }
      return howmany;
    }
        // get the sampled information on the phoneme
    else  if (state==0)
    {
		A = sampledConsonantFlag[Y];
		mem39 = A;
		
		// unvoiced sampled phoneme?
		A = A & 248;
		if(A != 0)
		{
            // render the sample for the phoneme
		  //FILL IN			RenderSample(&mem66);
		  state=renderunvoicedsample(&mem66,sample,state,&howmany);
		  return howmany;
		} else
		  ///////
		  {
            // simulate the glottal pulse and formants
			mem56 = multtable[sinus[phase1] | amplitude1[Y]];
			carry = 0;
			if ((mem56+multtable[sinus[phase2] | amplitude2[Y]] ) > 255) carry = 1;
			mem56 += multtable[sinus[phase2] | amplitude2[Y]];
			A = mem56 + multtable[rectangle[phase3] | amplitude3[Y]] + (carry?1:0);
			// output the accumulated value
			A = ((A + 136) & 255) >> 4; //there must be also a carry
			//mem[54296] = A;
			
			// output the accumulated value
						//			Output(0, A);
			howmany=Output(0);
			*sample=((A-8)<<12); //1 byte
			speedcounter--;
			if (speedcounter != 0) { //goto pos48155;
			  secondstate=0;
			  state=1;
			  return howmany;
			}
			//			else{
			Y++; //go to next amplitude
			// decrement the frame count
			mem48--;
			if(mem48 == 0) {
			  state=0; // NON?
			  sam_frame_rerun();
			  *ending=1;
			  return howmany; // ended frame
			}	
						speedcounter = speedd;
						//speedcounter = (adc_buffer[SELY]>>4)+1;

			state=1; secondstate=0;
			return howmany;
			//			} // else
		} // A/0
    } // state is zero // we always OUT
		
  else if (state==1){

    if (secondstate!=0) {
      secondstate=rendervoicedsample(&mem66,sample,secondstate, &howmany);
		  if (secondstate==0) {

			pitchmod+=pitches[Y];
			if (pitchmod>126) pitchmod=126;
			else if (pitchmod<1) pitchmod=1;

			A = pitchmod;
			mem44 = A;
			A = A - (A>>2);
			mem38 = A;
			
			// reset the formant wave generators to keep them in 
			// sync with the glottal pulse
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			//			continue;
			state = 0;  // but state shouldn't reset until secondstate is 0;
		//		goto pos48159;
			//    }
			return 0;
		  }
    }
    else {
        // decrement the remaining length of the glottal pulse
		mem44--;
		
		// finished with a glottal pulse?
		if(mem44 == 0)
		{
		  //		pos48159:
            // fetch the next glottal pulse length
			pitchmod+=pitches[Y];
			if (pitchmod>126) pitchmod=126;
			else if (pitchmod<1) pitchmod=1;

			A = pitchmod;
			mem44 = A;
			A = A - (A>>2);
			mem38 = A;
			
			// reset the formant wave generators to keep them in 
			// sync with the glottal pulse
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			state = 0; // is this so? - but we haven't returned a sample/???
			continue;
			//			return 0;
		}
		
		// decrement the count
		mem38--;
		
		// is the count non-zero and the sampled flag is zero?
		if((mem38 != 0) || (mem39 == 0))
		{
            // reset the phase of the formants to match the pulse
			phase1 += frequency1[Y];
			phase2 += frequency2[Y];
			phase3 += frequency3[Y];
			state = 0; // is this so? no sample so circulate
			continue;
			//			return 0;
		}
		
		// voiced sampled phonemes interleave the sample with the
		// glottal pulse. The sample flag is non-zero, so render
		// the sample for the phoneme.

		//FILL IN		RenderSample(&mem66);
		secondstate=rendervoicedsample(&mem66,sample,secondstate,&howmany);
		  if (secondstate==0) { // finish render
		    pitchmod+=pitches[Y];
		    if (pitchmod>126) pitchmod=126;
		    else if (pitchmod<1) pitchmod=1;

			A = pitchmod;
			mem44 = A;
			A = A - (A>>2);
			mem38 = A;
			
			// reset the formant wave generators to keep them in 
			// sync with the glottal pulse
			phase1 = 0;
			phase2 = 0;
			phase3 = 0;
			//			continue;
			state = 0;  // but state shouldn't reset until secondstate is 0;
		//		goto pos48159;
			//    }
		  }
    }
			return 0;
  }
}
}

void renderframe(){

	unsigned char phase1 = 0;  //mem43
	int16_t i;
	//	u8 carry;
	if (phonemeIndexOutput[0] == 255) return; //exit if no data

	A = 0;
	X = 0;
	mem44 = 0;


// CREATE FRAMES
//
// The length parameter in the list corresponds to the number of frames
// to expand the phoneme to. Each frame represents 10 milliseconds of time.
// So a phoneme with a length of 7 = 7 frames = 70 milliseconds duration.
//
// The parameters are copied from the phoneme to the frame verbatim.


// pos47587:
do
{
    // get the index
	Y = mem44;
	// get the phoneme at the index
	A = phonemeIndexOutput[mem44];
	mem56 = A;
	
	// if terminal phoneme, exit the loop
	if (A == 255) break;
	
	// period phoneme *.
	if (A == 1)
	{
       // add rising inflection
		A = 1;
		mem48 = 1;
		//goto pos48376;
		AddInflection(mem48, phase1);
	}
	/*
	if (A == 2) goto pos48372;
	*/
	
	// question mark phoneme?
	if (A == 2)
	{
        // create falling inflection
		mem48 = 255;
		AddInflection(mem48, phase1);
	}
	//	pos47615:

    // get the stress amount (more stress = higher pitch)
	phase1 = tab47492[stressOutput[Y] + 1];
	
    // get number of frames to write
	phase2 = phonemeLengthOutput[Y];
	Y = mem56;
	
	// copy from the source to the frames list
	do
	{
		frequency1[X] = freq1data[Y];     // F1 frequency
		frequency2[X] = freq2data[Y];     // F2 frequency
		frequency3[X] = freq3data[Y];     // F3 frequency
		amplitude1[X] = ampl1data[Y];     // F1 amplitude
		amplitude2[X] = ampl2data[Y];     // F2 amplitude
		amplitude3[X] = ampl3data[Y];     // F3 amplitude
		sampledConsonantFlag[X] = sampledConsonantFlags[Y];        // phoneme data for sampled consonants
		pitches[X] = pitch + phase1;      // pitch
		X++;
		phase2--;
	} while(phase2 != 0);
	mem44++;
} while(mem44 != 0);
// -------------------
//pos47694:


// CREATE TRANSITIONS
//
// Linear transitions are now created to smoothly connect each
// phoeneme. This transition is spread between the ending frames
// of the old phoneme (outBlendLength), and the beginning frames 
// of the new phoneme (inBlendLength).
//
// To determine how many frames to use, the two phonemes are 
// compared using the blendRank[] table. The phoneme with the 
// smaller score is used. In case of a tie, a blend of each is used:
//
//      if blendRank[phoneme1] ==  blendRank[phomneme2]
//          // use lengths from each phoneme
//          outBlendFrames = outBlend[phoneme1]
//          inBlendFrames = outBlend[phoneme2]
//      else if blendRank[phoneme1] < blendRank[phoneme2]
//          // use lengths from first phoneme
//          outBlendFrames = outBlendLength[phoneme1]
//          inBlendFrames = inBlendLength[phoneme1]
//      else
//          // use lengths from the second phoneme
//          // note that in and out are swapped around!
//          outBlendFrames = inBlendLength[phoneme2]
//          inBlendFrames = outBlendLength[phoneme2]
//
//  Blend lengths can't be less than zero.
//
// For most of the parameters, SAM interpolates over the range of the last
// outBlendFrames-1 and the first inBlendFrames.
//
// The exception to this is the Pitch[] parameter, which is interpolates the
// pitch from the center of the current phoneme to the center of the next
// phoneme.

	A = 0;
	mem44 = 0;
	mem49 = 0; // mem49 starts at as 0
	X = 0;
	while(1) //while No. 1
	{
 
        // get the current and following phoneme
		Y = phonemeIndexOutput[X];
		A = phonemeIndexOutput[X+1];
		X++;

		// exit loop at end token
		if (A == 255) break;//goto pos47970;


        // get the ranking of each phoneme
		X = A;
		mem56 = blendRank[A];
		A = blendRank[Y];
		
		// compare the rank - lower rank value is stronger
		if (A == mem56)
		{
            // same rank, so use out blend lengths from each phoneme
			phase1 = outBlendLength[Y];
			phase2 = outBlendLength[X];
		} else
		if (A < mem56)
		{
            // first phoneme is stronger, so us it's blend lengths
			phase1 = inBlendLength[X];
			phase2 = outBlendLength[X];
		} else
		{
            // second phoneme is stronger, so use it's blend lengths
            // note the out/in are swapped
			phase1 = outBlendLength[Y];
			phase2 = inBlendLength[Y];
		}

		Y = mem44;
		A = mem49 + phonemeLengthOutput[mem44]; // A is mem49 + length
		mem49 = A; // mem49 now holds length + position
		A = A + phase2; //Maybe Problem because of carry flag

		//47776: ADC 42
		speedcounter = A;
		mem47 = 168;
		phase3 = mem49 - phase1; // what is mem49
		A = phase1 + phase2; // total transition?
		mem38 = A;
		
		X = A;
		X -= 2;
		if ((X & 128) == 0)
		do   //while No. 2
		{
			//pos47810:

          // mem47 is used to index the tables:
          // 168  pitches[]
          // 169  frequency1
          // 170  frequency2
          // 171  frequency3
          // 172  amplitude1
          // 173  amplitude2
          // 174  amplitude3

			mem40 = mem38;

			if (mem47 == 168)     // pitch
			{
                      
               // unlike the other values, the pitches[] interpolates from 
               // the middle of the current phoneme to the middle of the 
               // next phoneme
                      
				unsigned char mem36, mem37;
				// half the width of the current phoneme
				mem36 = phonemeLengthOutput[mem44] >> 1;
				// half the width of the next phoneme
				mem37 = phonemeLengthOutput[mem44+1] >> 1;
				// sum the values
				mem40 = mem36 + mem37; // length of both halves
				mem37 += mem49; // center of next phoneme
				mem36 = mem49 - mem36; // center index of current phoneme
				A = Read(mem47, mem37); // value at center of next phoneme - end interpolation value
				//A = mem[address];
				
				Y = mem36; // start index of interpolation
				mem53 = A - Read(mem47, mem36); // value to center of current phoneme
			} else
			{
                // value to interpolate to
				A = Read(mem47, speedcounter);
				// position to start interpolation from
				Y = phase3;
				// value to interpolate from
				mem53 = A - Read(mem47, phase3);
			}
			
			//Code47503(mem40);
			// ML : Code47503 is division with remainder, and mem50 gets the sign
			
			// calculate change per frame
			mem50 = (((char)(mem53) < 0) ? 128 : 0);
			mem51 = abs((char)mem53) % mem40;
			mem53 = (unsigned char)((char)(mem53) / mem40);

            // interpolation range
			X = mem40; // number of frames to interpolate over
			Y = phase3; // starting frame


            // linearly interpolate values

			mem56 = 0;
			//47907: CLC
			//pos47908:
			while(1)     //while No. 3
			{
				A = Read(mem47, Y) + mem53; //carry alway cleared

				mem48 = A;
				Y++;
				X--;
				if(X == 0) break;

				mem56 += mem51;
				if (mem56 >= mem40)  //???
				{
					mem56 -= mem40; //carry? is set
					//if ((mem56 & 128)==0)
					if ((mem50 & 128)==0)
					{
						//47935: BIT 50
						//47937: BMI 47943
						if(mem48 != 0) mem48++;
					} else mem48--;
				}
				//pos47945:
				Write(mem47, Y, mem48);
			} //while No. 3

			//pos47952:
			mem47++;
			//if (mem47 != 175) goto pos47810;
		} while (mem47 != 175);     //while No. 2
		//pos47963:
		mem44++;
		X = mem44;
	}  //while No. 1

	//goto pos47701;
	//pos47970:

    // add the length of this phoneme
	mem48 = mem49 + phonemeLengthOutput[mem44];
	

// ASSIGN PITCH CONTOUR
//
// This subtracts the F1 frequency from the pitch to create a
// pitch contour. Without this, the output would be at a single
// pitch level (monotone).

/*	
	// don't adjust pitch if in sing mode
	if (!singmode)
	{
        // iterate through the buffer
		for(i=0; i<256; i++) {
            // subtract half the frequency of the formant 1.
            // this adds variety to the voice
    		pitches[i] -= (frequency1[i] >> 1);
        }
	}
*/
	phase1 = 0;
	phase2 = 0;
	phase3 = 0;
	mem49 = 0;
	speedcounter = 72; //sam standard speed

// RESCALE AMPLITUDE
//
// Rescale volume from a linear scale to decibels.
//

	//amplitude rescaling
	for(i=255; i>=0; i--)
	{
		amplitude1[i] = amplitudeRescale[amplitude1[i]];
		amplitude2[i] = amplitudeRescale[amplitude2[i]];
		amplitude3[i] = amplitudeRescale[amplitude3[i]];
	}

	Y = 0;
	A = pitches[0];
	mem44 = A;
	X = A;
	mem38 = A - (A>>2);     // 3/4*A ???
	mem48stored=mem48;

}


// RENDER THE PHONEMES IN THE LIST
//
// The phoneme list is converted into sound through the steps:
//
// 1. Copy each phoneme <length> number of times into the frames list,
//    where each frame represents 10 milliseconds of sound.
//
// 2. Determine the transitions lengths between phonemes, and linearly
//    interpolate the values across the frames.
//
// 3. Offset the pitches by the fundamental frequency.
//
// 4. Render the each frame.

//void Code47574()

// Create a rising or falling inflection 30 frames prior to 
// index X. A rising inflection is used for questions, and 
// a falling inflection is used for statements.

void AddInflection(unsigned char mem48, unsigned char phase1)
{
	//pos48372:
	//	mem48 = 255;
//pos48376:
           
    // store the location of the punctuation
	mem49 = X;
	A = X;
	int Atemp = A;
	
	// backup 30 frames
	A = A - 30; 
	// if index is before buffer, point to start of buffer
	if (Atemp <= 30) A=0;
	X = A;

	// FIXME: Explain this fix better, it's not obvious
	// ML : A =, fixes a problem with invalid pitch with '.'
	while( (A=pitches[X]) == 127) X++;


pos48398:
	//48398: CLC
	//48399: ADC 48
	
	// add the inflection direction
	A += mem48;
	phase1 = A;
	
	// set the inflection
	pitches[X] = A;
pos48406:
         
    // increment the position
	X++;
	
	// exit if the punctuation has been reached
	if (X == mem49) return; //goto pos47615;
	if (pitches[X] == 255) goto pos48406;
	A = phase1;
	goto pos48398;
}

/*
    SAM's voice can be altered by changing the frequencies of the
    mouth formant (F1) and the throat formant (F2). Only the voiced
    phonemes (5-29 and 48-53) are altered.
*/
void SetMouthThroat(unsigned char mouth, unsigned char throat)
{
	unsigned char initialFrequency;
	unsigned char newFrequency = 0;
	//unsigned char mouth; //mem38880
	//unsigned char throat; //mem38881

	// mouth formants (F1) 5..29
	unsigned char mouthFormants5_29[30] = {
		0, 0, 0, 0, 0, 10,
		14, 19, 24, 27, 23, 21, 16, 20, 14, 18, 14, 18, 18,
		16, 13, 15, 11, 18, 14, 11, 9, 6, 6, 6};

	// throat formants (F2) 5..29
	unsigned char throatFormants5_29[30] = {
	255, 255,
	255, 255, 255, 84, 73, 67, 63, 40, 44, 31, 37, 45, 73, 49,
	36, 30, 51, 37, 29, 69, 24, 50, 30, 24, 83, 46, 54, 86};

	// there must be no zeros in this 2 tables
	// formant 1 frequencies (mouth) 48..53
	unsigned char mouthFormants48_53[6] = {19, 27, 21, 27, 18, 13};
       
	// formant 2 frequencies (throat) 48..53
	unsigned char throatFormants48_53[6] = {72, 39, 31, 43, 30, 34};

	unsigned char pos = 5; //mem39216
//pos38942:
	// recalculate formant frequencies 5..29 for the mouth (F1) and throat (F2)
	while(pos != 30)
	{
		// recalculate mouth frequency
		initialFrequency = mouthFormants5_29[pos];
		if (initialFrequency != 0) newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
               
		// recalculate throat frequency
		initialFrequency = throatFormants5_29[pos];
		if(initialFrequency != 0) newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		pos++;
	}

//pos39059:
	// recalculate formant frequencies 48..53
	pos = 48;
	Y = 0;
    while(pos != 54)
    {
		// recalculate F1 (mouth formant)
		initialFrequency = mouthFormants48_53[Y];
		newFrequency = trans(mouth, initialFrequency);
		freq1data[pos] = newFrequency;
           
		// recalculate F2 (throat formant)
		initialFrequency = throatFormants48_53[Y];
		newFrequency = trans(throat, initialFrequency);
		freq2data[pos] = newFrequency;
		Y++;
		pos++;
	}
}


//return = (mem39212*mem39213) >> 1
unsigned char trans(unsigned char mem39212, unsigned char mem39213)
{
	//pos39008:
	unsigned char carry;
	int temp;
	unsigned char mem39214, mem39215;
	A = 0;
	mem39215 = 0;
	mem39214 = 0;
	X = 8;
	do
	{
		carry = mem39212 & 1;
		mem39212 = mem39212 >> 1;
		if (carry != 0)
		{
			/*
						39018: LSR 39212
						39021: BCC 39033
						*/
			carry = 0;
			A = mem39215;
			temp = (int)A + (int)mem39213;
			A = A + mem39213;
			if (temp > 255) carry = 1;
			mem39215 = A;
		}
		temp = mem39215 & 1;
		mem39215 = (mem39215 >> 1) | (carry?128:0);
		carry = temp;
		//39033: ROR 39215
		X--;
	} while (X != 0);
	temp = mem39214 & 128;
	mem39214 = (mem39214 << 1) | (carry?1:0);
	carry = temp;
	temp = mem39215 & 128;
	mem39215 = (mem39215 << 1) | (carry?1:0);
	carry = temp;

	return mem39215;
}


