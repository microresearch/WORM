/*
 * lpc.c
 * 03-26-2015 E. Brombaugh
 * borrows heavily from Peter Knight's Talkie library for Arduino
 * This code is released under GPLv2 license
 */

// stderr redirect etc =  ./say 0 0 2>&1 > /dev/dsp
 
#include "stdio.h"
#include "math.h"
#include "time.h"


//#include "lpc.h"
//#include "audio.h"

typedef unsigned char uint8_t;
typedef unsigned char u8; 
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef unsigned short u16;
typedef unsigned short u16;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

uint16_t lpc_get_sample(void);

#define INTERP_PERIOD 25  // samples per subframe
#define SUBFRAME_PERIOD 8 // subframes per frame

int didntjump=1;

uint8_t* ptrAddr, ptrBit;
uint8_t synth_running, synth_subframe_ctr, synth_sample_ctr;
uint8_t starty, nextPeriod, synthPeriod;
uint16_t nextEnergy, synthEnergy;
int16_t synthK[10], nextK[10];
uint8_t periodCounter;
int16_t xlpc[10], ulpc[11];
uint16_t synthRand;
uint8_t byte_rev[256];

/* match below with rom?

1 AE1 - 0fe:

020b 8f15 0908 0606 0a05 0204 0b8f 1509 0806 060a 0502 04

2 AE1N - 115:

02 0b93 1807 0903 0409 0403 040b 9318 0709 0304 0904 0304 

3 AH1 - 12c:

020b 8e0f 0a0c 0a02 0605 0405 0b8e 0f0a 0c0a 0206 0504 05

4 AH1N - 143:

02 0b90 0c0d 0b07 0305 0504 040b 900c 0d0b 0703 0505 0404 

5 AW1 - 15a:

020a 8f0a090f0803050505030a8b0f0e0c070109050304 = 23 bytes

a6=166 aa=170

02 0b 8f = a6 f6

02 0a 8f = aa f6

http://www.unige.ch/medecine/nouspikel/ti99/speech.htm

say: energy 4 bits 02=87  which is 0x57 ???

repeat 1 bit=0

pitch 6 bits=0b =25

// a6 f6 = 1010 0110 , 1111 0110

02 0b 8f = 0000 0010 , 0000 1011, 

*/


const uint8_t sp_0055m[]         __attribute__ ((section (".flash")))  ={0x27, 0xB3, 0xB6, 0x32, 0x94, 0xF5, 0xB5, 0x4C, 0xDB, 0xAA, 0x70, 0xD2, 0x1B, 0x32, 0xAD, 0x2A, 0x42, 0x85, 0x6D, 0x48, 0xAD, 0x68, 0xF, 0x11, 0xA5, 0x25, 0xD6, 0x4B, 0xAD, 0x84, 0x7B, 0x8D, 0xC4, 0x6E, 0xE7, 0x32, 0xCC, 0x33, 0x12, 0x7B, 0x1D, 0x2B, 0x30, 0x4F, 0x4B, 0xDD, 0x71, 0xC8, 0xA4, 0xBC, 0x25, 0x75, 0x83, 0x79, 0x4D, 0x5B, 0x85, 0xD4, 0x47, 0xC5, 0x8D, 0x88, 0x98, 0xB2, 0xD6, 0x8D, 0x94, 0x3C, 0x89, 0x2, 0x34, 0xE7, 0x16, 0x80, 0x60, 0xD5, 0x29, 0x29, 0x42, 0xB8, 0x2B, 0x43, 0xB5, 0x24, 0x8, 0xF5, 0xAA, 0xB6, 0x7C, 0x92, 0xE6, 0x54, 0xD4, 0xBB, 0xF5, 0x49, 0x7A, 0x52, 0x61, 0xAD, 0x2D, 0x27, 0x69, 0xC9, 0x84, 0x2C, 0xB7, 0x9C, 0xA4, 0x26, 0x67, 0xD2, 0xEA, 0x73, 0xD2, 0x52, 0x83, 0xC9, 0xBC, 0xCF, 0xC9, 0x53, 0x71, 0xD2, 0x88, 0x3E, 0xA7, 0x8E, 0xDD, 0xD1, 0xC3, 0x7A, 0x8F, 0x2E, 0x4C, 0xE1, 0xC, 0xEB, 0x5D, 0x7A, 0x3F, 0x99, 0x47, 0xBD, 0x63, 0x19, 0x7C, 0x4E, 0xD8, 0x90, 0x4E, 0xAE, 0xB7, 0x55, 0xE8, 0xD5, 0x57, 0x3C};


uint8_t *wordlist[]={sp_005m,sp_0055m};




/*
 * TMS5xxx LPC coefficient tables
 */
const uint8_t tmsEnergy[0x10] = {0x00,0x02,0x03,0x04,0x05,0x07,0x0a,0x0f,0x14,0x20,0x29,0x39,0x51,0x72,0xa1,0xff};
const uint8_t tmsPeriod[0x40] = {0x00,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2D,0x2F,0x31,0x33,0x35,0x36,0x39,0x3B,0x3D,0x3F,0x42,0x45,0x47,0x49,0x4D,0x4F,0x51,0x55,0x57,0x5C,0x5F,0x63,0x66,0x6A,0x6E,0x73,0x77,0x7B,0x80,0x85,0x8A,0x8F,0x95,0x9A,0xA0};
const int16_t tmsK1[0x20]     = {0x82C0,0x8380,0x83C0,0x8440,0x84C0,0x8540,0x8600,0x8780,0x8880,0x8980,0x8AC0,0x8C00,0x8D40,0x8F00,0x90C0,0x92C0,0x9900,0xA140,0xAB80,0xB840,0xC740,0xD8C0,0xEBC0,0x0000,0x1440,0x2740,0x38C0,0x47C0,0x5480,0x5EC0,0x6700,0x6D40};
const int16_t tmsK2[0x20]     = {0xAE00,0xB480,0xBB80,0xC340,0xCB80,0xD440,0xDDC0,0xE780,0xF180,0xFBC0,0x0600,0x1040,0x1A40,0x2400,0x2D40,0x3600,0x3E40,0x45C0,0x4CC0,0x5300,0x5880,0x5DC0,0x6240,0x6640,0x69C0,0x6CC0,0x6F80,0x71C0,0x73C0,0x7580,0x7700,0x7E80};
const int8_t tmsK3[0x10]      = {0x92,0x9F,0xAD,0xBA,0xC8,0xD5,0xE3,0xF0,0xFE,0x0B,0x19,0x26,0x34,0x41,0x4F,0x5C};
const int8_t tmsK4[0x10]      = {0xAE,0xBC,0xCA,0xD8,0xE6,0xF4,0x01,0x0F,0x1D,0x2B,0x39,0x47,0x55,0x63,0x71,0x7E};
const int8_t tmsK5[0x10]      = {0xAE,0xBA,0xC5,0xD1,0xDD,0xE8,0xF4,0xFF,0x0B,0x17,0x22,0x2E,0x39,0x45,0x51,0x5C};
const int8_t tmsK6[0x10]      = {0xC0,0xCB,0xD6,0xE1,0xEC,0xF7,0x03,0x0E,0x19,0x24,0x2F,0x3A,0x45,0x50,0x5B,0x66};
const int8_t tmsK7[0x10]      = {0xB3,0xBF,0xCB,0xD7,0xE3,0xEF,0xFB,0x07,0x13,0x1F,0x2B,0x37,0x43,0x4F,0x5A,0x66};
const int8_t tmsK8[0x08]      = {0xC0,0xD8,0xF0,0x07,0x1F,0x37,0x4F,0x66};
const int8_t tmsK9[0x08]      = {0xC0,0xD4,0xE8,0xFC,0x10,0x25,0x39,0x4D};
const int8_t tmsK10[0x08]     = {0xCD,0xDF,0xF1,0x04,0x16,0x20,0x3B,0x4D};

#define CHIRP_SIZE 41
const int8_t chirp[CHIRP_SIZE] = {0x00,0x2a,0xd4,0x32,0xb2,0x12,0x25,0x14,0x02,0xe1,0xc5,0x02,0x5f,0x5a,0x05,0x0f,0x26,0xfc,0xa5,0xa5,0xd6,0xdd,0xdc,0xfc,0x25,0x2b,0x22,0x21,0x0f,0xff,0xf8,0xee,0xed,0xef,0xf7,0xf6,0xfa,0x00,0x03,0x02,0x01};

void test_random_params(uint8_t* output, uint8_t length){
  u8 x,y,bitcount=0,bit=0,byte=0,bytecount=0; // length is number of frames
  u8 energy, pitch, repeat, K1,K2,K3,K4,K5,K6,K7,K8,K9,K10;
  
  for (x=0;x<length;x++){
    energy=rand()%15;
    if (energy!=0){
      printf("energy %d\n",energy);
      for (y=0;y<4;y++){
	bit=(energy>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
      // then 1 rpt or not
      bit=rand()%2;
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      repeat=bit;
      //      pitch= // 6 bits
      pitch=rand()%64;
      for (y=0;y<6;y++){
	bit=(pitch>>(5-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
      if (repeat!=0){
	// K1 -> K2 = 5 bits
	K1=rand()%32;
      for (y=0;y<5;y++){
	bit=(K1>>(4-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K2=rand()%32;
      for (y=0;y<5;y++){
	bit=(K1>>(4-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	// K3 -> K4 = 4 bits
	K3=rand()%16;
      for (y=0;y<4;y++){
	bit=(K1>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K4=rand()%16;
      for (y=0;y<4;y++){
	bit=(K1>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }

	if (pitch!=0){
	  // K5 -> K7 = 4 bits
	K4=rand()%16;
      for (y=0;y<4;y++){
	bit=(K1>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K4=rand()%16;
      for (y=0;y<4;y++){
	bit=(K1>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K4=rand()%16;
      for (y=0;y<4;y++){
	bit=(K1>>(3-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	  // K8 -> K10 = 3 bits
	K8=rand()%8;
      for (y=0;y<3;y++){
	bit=(K1>>(2-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K8=rand()%8;
      for (y=0;y<3;y++){
	bit=(K1>>(2-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	K8=rand()%8;
      for (y=0;y<3;y++){
	bit=(K1>>(2-y))&1; 
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
	}
      }
		}
    else {//silence
      //      next 4 bits are zeroed
      for (y=0;y<4;y++){
	bit=0;
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }

    }
  // now add end 0x15 energy at end 
      for (y=0;y<4;y++){
	bit=1;
	byte+=bit<<(bitcount++);
      if (bitcount==8){
	output[bytecount++]=byte;
	byte=0;
	bitcount=0;
	}
      }
    


  }
}

/* HOW to put together bits from parameters // bounds:

Byte rev?

bit by bit:

- 4: energy: 0 is silence, 15 is end

- 1: rpt - if rpt then we just have one pitch pack and no coeffs

- 6: pitch - 0 is unvoiced which has k1-k4 only coeffs

- 5: K1
- 5: K2
- 4: K3
- 4: K4

- 4: K5
- 4: K6
- 4: K7
- 3: K8
- 3: K9
- 3: K10

*/

/*
 * Parse frame parameter bits from the ROM data stream.
 */

void tobits(uint8_t number, uint8_t bits){
  int x;
  for (x=0;x<bits;x++){
    fprintf(stderr, "%d",(number>>x)&1);
  }
//	fprintf(stderr, "\n");
}

uint8_t lpc_getBits(uint8_t num_bits)
{
	uint8_t value;
	uint16_t data;
	
	data = byte_rev[*ptrAddr]<<8;
	//data = (*ptrAddr)<<8;
	if (ptrBit+num_bits > 8)
	{
	  data |= byte_rev[*(ptrAddr+1)];
	  //  	  data |= *(ptrAddr+1);
	}
	data <<= ptrBit;
	value = data >> (16-num_bits);
	ptrBit += num_bits;
	didntjump=1;
	if (ptrBit >= 8)
	{
	  fprintf(stderr,"%x, ",*ptrAddr);
		ptrBit -= 8;
		ptrAddr++;
		//		didntjump=2;
		if (ptrBit==0) {
		  didntjump=0;
		}
	}
//	tobits(value,num_bits);
	return value;
}

/*
 * Initialize the LPC system
 */
void lpc_init(void)
{
	int16_t i,j;

	/* initialize the byte reverse table for the ROM data */
	for(i=0;i<256;i++)
	{
		j = (i>>4) | (i<<4); // Swap in groups of 4
		j = ((j & 0xcc)>>2) | ((j & 0x33)<<2); // Swap in groups of 2
		byte_rev[i] = ((j & 0xaa)>>1) | ((j & 0x55)<<1); // Swap bit pairs
	}
	
	/* initialize the LPC synth parameters */
	synthPeriod = 0;
	synthEnergy = 0;
	for(i=0;i<10;i++)
	{
		synthK[i] = 0;
		xlpc[i]=0;
	}
	starty = 0;
	synth_running = 0;
	synth_subframe_ctr = 0;
	synth_sample_ctr = 0;
	synthRand = 1;
}

//extern int16_t audio_buffer[AUDIO_BUFSZ];

void lpc_running(){  // write into audio buffer
  //  lpc_get_sample();
  static u16 counterrr=0;
  int16_t samplel=lpc_get_sample()>>2; // TODO or scale samples/speed???
  
  printf("%c",samplel);
}

/*
 * main entry point to speak a string of LPC data from a ROM
 */
void lpc_say(uint8_t* addr)
{
	/* initialize ROM pointers */
	ptrAddr = addr;
	ptrBit = 0;
	
	/* Starty the synth */
	synth_running = 1;
	starty = 1;
}


/*
 * non-blocking test to see if currently speaking
 */
uint8_t lpc_busy(void)
{
	return synth_running;
}



/*
 * Update LPC coefficients
 * called at the subframe rate from the sample routine
 */
void lpc_update_coeffs(void)
{
	int8_t i;
	uint8_t repeat;
	uint16_t energy;
	
	/* time for a new frame? */
	if(++synth_subframe_ctr == SUBFRAME_PERIOD)
	{
		/* reset the subframe counter */
		synth_subframe_ctr = 0;
		
		/* Read speech data, processing the variable size frames. */
		energy = lpc_getBits(4);
		if(energy == 0)
		{
			/* Energy = 0: rest frame */
			nextEnergy = 0;
		}
		else if(energy == 0xf)
		{
			/* Energy = 15: stop frame. Silence the synthesiser. */
			nextEnergy = 0;
			for(i=0;i<10;i++)
				nextK[i] = 0;
			synth_running = 0;
			// try jump 
			//						ptrAddr++; ptrBit=0;
			if (didntjump){
			  ptrBit =0;
			  //			  printf("0x%X, ",*ptrAddr);
	  ptrAddr++;
			} 
			//			didntjump=0;
			starty=1;
			//	sleep(1);
	synth_subframe_ctr = 0;
	synth_sample_ctr = 0;
	//	printf("};\n{");
		       

		}
		else
		{
			/* All other energy types */
			nextEnergy = tmsEnergy[energy];
			repeat = lpc_getBits(1);
			int origpitch = lpc_getBits(6);
			//	nextPeriod = tmsPeriod[lpc_getBits(6)];
			nextPeriod=tmsPeriod[origpitch];
			//		fprintf(stderr,"ENERGY: %d REPEAT: %d PITCH: %d\n", energy, repeat, origpitch);

			/* A repeat frame uses the last coefficients */
			if(!repeat)
			{
				/* All frames use the first 4 coefficients */
				nextK[0] = tmsK1[lpc_getBits(5)];
				nextK[1] = tmsK2[lpc_getBits(5)];
				nextK[2] = tmsK3[lpc_getBits(4)]<<8;
				nextK[3] = tmsK4[lpc_getBits(4)]<<8;
				if(nextPeriod)
				{
					/* Voiced frames use 6 extra coefficients. */
					nextK[4] = tmsK5[lpc_getBits(4)]<<8;
					nextK[5] = tmsK6[lpc_getBits(4)]<<8;
					nextK[6] = tmsK7[lpc_getBits(4)]<<8;
					nextK[7] = tmsK8[lpc_getBits(3)]<<8;
					nextK[8] = tmsK9[lpc_getBits(3)]<<8;
					nextK[9] = tmsK10[lpc_getBits(3)]<<8;
				}
			}
		}
	}
	
	/* skip interp on 1st subframe */
		if(starty)
		{
		starty = 0;
		synthPeriod = nextPeriod;
		synthEnergy = nextEnergy;
		for(i=0;i<10;i++)
			synthK[i] = nextK[i] ;
			}
	else
	{
#define	EXPO_SHIFT 2
		synthPeriod += (nextPeriod-synthPeriod)>>EXPO_SHIFT;
		synthEnergy += (nextEnergy-synthEnergy)>>EXPO_SHIFT;
		for(i=0;i<10;i++)
			synthK[i] += (nextK[i]-synthK[i])>>EXPO_SHIFT;
	}
		//	       		fprintf(stderr,"ENERGY: %d REPEAT: %d PITCH: %d\n", nextEnergy, repeat, nextPeriod); // after lookup

}

/*
 * compute a new sample of LPC data
 */
uint16_t lpc_get_sample(void)
{
	int8_t i;
	
	/* if not running just return mid-scale */
	/*		if(synth_running == 0)
	{
		return 512;
		}
	*/
	
	/* Time to update the coeffs? */
	if(++synth_sample_ctr == INTERP_PERIOD)
	{
		/* reset the sample counter */
		synth_sample_ctr = 0;
		
		/* update the coeffs */
		lpc_update_coeffs();
	}
	
	/* Stimulus waveform */
	if(synthPeriod)
	{
		/* Voiced source */
		if(periodCounter < synthPeriod)
			periodCounter++;
		else
			periodCounter = 0;
		
		if (periodCounter < CHIRP_SIZE)
			ulpc[10] = ((chirp[periodCounter]) * (uint32_t) synthEnergy) >> 8;
		else
			ulpc[10] = 0;
	}
	else
	{
		/* Unvoiced source */
	  //		synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
	  synthRand=rand()%32768;
		ulpc[10] = (synthRand & 1) ? synthEnergy : -synthEnergy;
	}
	
	/* Lattice filter forward path */
	for(i=9;i>=0;i--)
		ulpc[i] = ulpc[i+1]-(((int32_t)synthK[i]*xlpc[i]) >> 15);
	
	/* Output clamp */
	if (ulpc[0] > 511)
		ulpc[0] = 511;
	else if (ulpc[0] < -512)
		ulpc[0] = -512;
	
	/* Lattice filter reverse path */
	for(i=9;i>0;i--)
		xlpc[i] = xlpc[i-1] + (((int32_t)synthK[i-1]*ulpc[i-1]) >> 15);
	xlpc[0] = ulpc[0];
	
	/* return 10-bit offset binary */
	return ulpc[0]+512;
}



void main(int argc, char *argv[]){
  int x;
  lpc_init();
   int uffset=atoi(argv[1]);
   int uuffset=atoi(argv[2]);

   srand (time(NULL));
   u8 randy[1280]; // zero it if necessary

//   test_random_params(randy, 10);
//   lpc_say(randy);
//   while(synth_running) lpc_running();

   // and try and read these back well first one
   //   ptrAddr=randy;
   //   int val=lpc_getBits(4);
   //   printf("rand0: %d FINAL %d", randy[0],val);
   //   val=lpc_getBits(4);
   //   printf("rand1: %d FINAL %d", randy[1],val);

   ////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

                lpc_say(wordlist[uffset]+uuffset);
    //            lpc_say(sp77+uffset);

	// or we try c7 etc as length?
                while(synth_running) lpc_running();


   ////////////////////>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // stay with first one which could be THIR or THIRTEE according to
  // TABLE (c40301 which is first of the 03 set D003 comes before THIRTEE and after THIR)

  // offset could be c4 plus XX
  // 279 was approx total offset so 279 minus 77(4d)=202 minus c7=199=3)

  // 279-199/c7=80(0x50)

  // 279 minus 4d/77 = 202 minus c7=199=3

  /* POINTERS:
00 4d00 b900 5f01  /].]h^.^..M..._.
0702 6202 f202 6503 ac03 5504 bb04 2a05  ..b...e...U...*.
8505 e505 8206 fd06 6107 a707 f907 9d08  ........a.......
1909 8e09 060a 7b0a d10a 680b bf0b 3e0c  ......{...h...>.
da0c 4c0d a00d 110e 8d0e 470f 2b10 cc10  ..L.......G.+...
fe10 8811 e011 2012 ae12 4a13 fa13 b714  ...... ...J.....
4315 d815 7116 bd16 3017 cd17 4b18 b918  C...q...0...K...
3a19 a819 271a d51a 321b ce1b 581c c21c  :...'...2...X...
291d 891d d41d 731e f71e 661f cb1f 2820  ).....s...f...( 
ae20 f820 6321 d321 6022 de22 4e23 ce23  . . c!.!`"."N#.#
5224 af24 3025 9c25 2326 7e26 1027 9027  R$.$0%.%#&~&.'.'
0c28 4528 c328 3529 be29 1b2a 8e2a 352b  .(E(.(5).).*.*5+
c12b 2f2c b92c 352d 

4d=77 279-77=202

  */

   //    int offsetsfrompointers[]={0x4d,0xb9,0x5f,0x107,0x262,0x2f2,0x265,0x3ac,0x355,0x4bb};

    //  int offsetsfromtables[]={0xc7,0xc4,0xce,0xd9,0xd4,0xc5,0xc8,0xd4,0xc4,0xd7};

  //  uint8_t *wordlist[22]={sp_000,sp_001,sp_002,sp_003,sp_004,sp_005,sp_006,sp_007,sp_008,sp_009,sp_010,sp_011,sp_012,sp_013,sp_015,sp_016,sp_017};

  //    int offset=offsetsfrompointers[uffset]+offsetsfromtables[uffset]+uuff; // which seems to give us THIN c702c5 - 279 but 
  //  int offset=offsetsfrompointers[uffset]+ 200 + offsetsfromtables[uffset] ;
  //  printf("%d\n",offset);

  //          offset=279;
  //  offset=0;	
//    int xx;
    //    printf("TOTAL OFFSET %d\n\n", offset);
  //  while(1){
    //	        lpc_say(sp_D003_88+offset);
					   /*    for (xx=0;xx<158;xx++){
      lpc_say(wordlist3[xx]+offset);
      while(synth_running) lpc_running();
      fprintf(stderr, "NUM: %d\r", xx);
      }*/
    //    for (x=0;x<126;x++){
//    x=0;
    //            lpc_say(wordlist_alphons[x]+uuffset);
    //            lpc_say(sp77+uffset);

	// or we try c7 etc as length?
    //            while(synth_running) lpc_running();
	    //      }
	//
    //					   while(1) lpc_running();
    //    offset++;
    //    sleep(1);
    //   printf("offset: %d\n",offset);
    //}
}
