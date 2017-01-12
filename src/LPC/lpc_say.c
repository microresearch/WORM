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

#include "roms/vocab_spkspellone.h"
#include "roms/vocab_spkspelltwo.h"
#include "roms/vocab_mpf.h"
#include "roms/vocab_testroms.h"
#include "roms/vocab_2304.h"
#include "roms/vocab_2303.h"
#include "roms/vocab_2321.h"
#include "roms/vocab_2322.h"
#include "roms/vocab_2352.h"
#include "roms/vocab_2350.h"

#include "roms/vocab_D000.h"
#include "roms/vocab_D001.h"
#include "roms/vocab_D002.h"
#include "roms/vocab_D003.h"
#include "roms/vocab_D004.h"
#include "roms/vocab_D005.h"
#include "roms/vocab_D006.h"
#include "roms/vocab_D007.h"
#include "roms/vocab_D008.h"
#include "roms/vocab_D009.h"
#include "roms/vocab_D010.h"
#include "roms/vocab_D011.h"
#include "roms/vocab_D012.h"
#include "roms/vocab_D013.h"
#include "roms/vocab_D014.h"
#include "roms/vocab_D015.h"
#include "roms/vocab_D016.h"
#include "roms/vocab_D017.h"
#include "roms/vocab_D018.h"
#include "roms/vocab_D019.h"
#include "roms/vocab_D020.h"
#include "roms/vocab_D021.h"
#include "roms/vocab_D022.h"
#include "roms/vocab_D023.h"
#include "roms/vocab_D024.h"
#include "roms/vocab_D025.h"
#include "roms/vocab_D026.h"
#include "roms/vocab_D027-34.h"

uint16_t lpc_get_sample(void);

#define INTERP_PERIOD 25  // samples per subframe
#define SUBFRAME_PERIOD 8 // subframes per frame

int didntjump=1;

const uint8_t* ptrAddr;
uint8_t ptrBit;
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

/*
 * TMS5xxx LPC coefficient tables
 */

#define PERIOD_BITS 6 /// 5 for 5100, 6 for 5200
#define CHIRP_SIZE 52 // was 41 for older chirp

/* 5100=

   5 bits for period

	TI_0280_PATENT_ENERGY
	TI_0280_2801_PATENT_PITCH
	TI_0280_PATENT_LPC
	TI_0280_PATENT_CHIRP
 */

/* 5200=

   6 bits for period

	TI_0285_LATER_ENERGY
	TI_2501E_PITCH
	{
	TI_2801_2501E_LPC
	},
	TI_LATER_CHIRP
 */

/* 5220=

   6 bits for period

	TI_0285_LATER_ENERGY
	TI_5220_PITCH
	{
	TI_5110_5220_LPC
	TI_LATER_CHIRP
 */

/* #define TI_0280_PATENT_ENERGY \ */
/* 		/\* E  *\/\ */
/* 		{   0,  0,  1,  1,  2,  3,  5,  7, \ */
/* 			10, 15, 21, 30, 43, 61, 86, COEFF_ENERGY_SENTINEL }, */
/*  // last rom value is actually really 0, but the tms5110.c code still requires the sentinel value to function correctly, until it is properly updated or merged with tms5220.c */



const uint8_t tmsEnergy5100[0x10] = {0,  0,  1,  1,  2,  3,  5,  7, 10, 15, 21, 30, 43, 61, 86, 0 }; 
const uint8_t tmsEnergyTALKIE[0x10] = {0x00,0x02,0x03,0x04,0x05,0x07,0x0a,0x0f,0x14,0x20,0x29,0x39,0x51,0x72,0xa1,0xff};
const uint8_t tmsEnergy5200[0x10] = {0,  1,  2,  3,  4,  6,  8, 11, 16, 23, 33, 47, 63, 85,114, 0 };

const uint8_t tmsPeriod5100[0x40] =   	{  0,   41,  43,  45,  47,  49,  51,  53,  \
		55,  58,  60,  63,  66,  70,  73,  76,  \
		79,  83,  87,  90,  94,  99,  103, 107,  \
					   112, 118, 123, 129, 134, 140, 147, 153 };

const uint8_t tmsPeriod5200[0x40] = 	{   0,  14,  15,  16,  17,  18,  19,  20,  \
		21,  22,  23,  24,  25,  26,  27,  28,  \
		29,  30,  31,  32,  34,  36,  38,  40,  \
		41,  43,  45,  48,  49,  51,  54,  55,  \
		57,  60,  62,  64,  68,  72,  74,  76,  \
		81,  85,  87,  90,  96,  99, 103, 107, \
		112, 117, 122, 127, 133, 139, 145, 151, \
					    157, 164, 171, 178, 186, 194, 202, 211};

const uint8_t tmsPeriod5220[0x40] = {   0,  15,  16,  17,  18,  19,  20,  21,  \
		22,  23,  24,  25,  26,  27,  28,  29,  \
		30,  31,  32,  33,  34,  35,  36,  37,  \
		38,  39,  40,  41,  42,  44,  46,  48,  \
		50,  52,  53,  56,  58,  60,  62,  65,  \
		68,  70,  72,  76,  78,  80,  84,  86,  \
		91,  94,  98, 101, 105, 109, 114, 118, \
					122, 127, 132, 137, 142, 148, 153, 159};

const uint8_t tmsPeriodTALKIE[0x40] = {0x00,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2D,0x2F,0x31,0x33,0x35,0x36,0x39,0x3B,0x3D,0x3F,0x42,0x45,0x47,0x49,0x4D,0x4F,0x51,0x55,0x57,0x5C,0x5F,0x63,0x66,0x6A,0x6E,0x73,0x77,0x7B,0x80,0x85,0x8A,0x8F,0x95,0x9A,0xA0};

const int8_t chirp5100[CHIRP_SIZE] = { 0x00, 0x2a, 0xd4, 0x32, 0xb2, 0x12, 0x25, 0x14,\
		0x02, 0xe1, 0xc5, 0x02, 0x5f, 0x5a, 0x05, 0x0f,\
		0x26, 0xfc, 0xa5, 0xa5, 0xd6, 0xdd, 0xdc, 0xfc,\
		0x25, 0x2b, 0x22, 0x21, 0x0f, 0xff, 0xf8, 0xee,\
		0xed, 0xef, 0xf7, 0xf6, 0xfa, 0x00, 0x03, 0x02,\
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
					    0x00, 0x00, 0x00, 0x00 };


const int8_t chirp5200[CHIRP_SIZE] = 	{   0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50,\
		0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13,\
		0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
					    0x00, 0x00, 0x00, 0x00 };


const int8_t chirpTALKIE[CHIRP_SIZE] = {0x00,0x2a,0xd4,0x32,0xb2,0x12,0x25,0x14,0x02,0xe1,0xc5,0x02,0x5f,0x5a,0x05,0x0f,0x26,0xfc,0xa5,0xa5,0xd6,0xdd,0xdc,0xfc,0x25,0x2b,0x22,0x21,0x0f,0xff,0xf8,0xee,0xed,0xef,0xf7,0xf6,0xfa,0x00,0x03,0x02,0x01}; // this is as chirp5100 above
					
// these are coeffs converted according to: https://github.com/going-digital/Talkie/issues/6 for 5200 not 5220
// from #define TI_2801_2501E_LPC = 5200 coeffs NOT 5220 but there are no other talkie coeffs

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

// coeff tables from mame for: 

		/* K1  */
const int16_t K1[]=		{ -501, -498, -497, -495, -493, -491, -488, -482,\
			-478, -474, -469, -464, -459, -452, -445, -437,\
			-412, -380, -339, -288, -227, -158,  -81,   -1,\
				  80,  157,  226,  287,  337,  379,  411,  436 };
		/* K2  */
const int16_t K2[]=		{ -328, -303, -274, -244, -211, -175, -138,  -99,\
			-59,  -18,   24,   64,  105,  143,  180,  215,\
			248,  278,  306,  331,  354,  374,  392,  408,\
				  422,  435,  445,  455,  463,  470,  476,  506 };
		/* K3  */
const int16_t K3[]=		{ -441, -387, -333, -279, -225, -171, -117,  -63,\
				  -9,   45,   98,  152,  206,  260,  314,  368  };
		/* K4  */
const int16_t K4[]=		{ -328, -273, -217, -161, -106,  -50,    5,   61,\
				  116,  172,  228,  283,  339,  394,  450,  506  };
		/* K5  */
const int16_t K5[]=		{ -328, -282, -235, -189, -142,  -96,  -50,   -3,\
				  43,   90,  136,  182,  229,  275,  322,  368  };
		/* K6  */
const int16_t K6[]=		{ -256, -212, -168, -123,  -79,  -35,   10,   54,\
				  98,  143,  187,  232,  276,  320,  365,  409  };
		/* K7  */
const int16_t K7[]=		{ -308, -260, -212, -164, -117,  -69,  -21,   27,\
				  75,  122,  170,  218,  266,  314,  361,  409  };
		/* K8  */
const int16_t K8[]=		{ -256, -161,  -66,   29,  124,  219,  314,  409  };
		/* K9  */
const int16_t K9[]=		{ -256, -176,  -96,  -15,   65,  146,  226,  307  };
		/* K10 */
const int16_t K10[]=		{ -205, -132,  -59,   14,   87,  160,  234,  307  };

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
      for (y=0;y<PERIOD_BITS;y++){
	bit=(pitch>>((PERIOD_BITS-1)-y))&1; 
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
	//	data = (*ptrAddr)<<8;
	if (ptrBit+num_bits > 8)
	{
	  data |= byte_rev[*(ptrAddr+1)];
	  //    	  data |= *(ptrAddr+1);
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
  int16_t samplel=lpc_get_sample()>>2;//>>8; // TODO or scale samples/speed??? was >>2
  
  printf("%c",samplel);
}

/*
 * main entry point to speak a string of LPC data from a ROM
 */
void lpc_say(const uint8_t* addr)
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
	if(++synth_subframe_ctr == SUBFRAME_PERIOD)// || starty==1)
	{
	  ///	  printf("UPDATE\n");

	  	  starty=0;
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
		  nextEnergy = tmsEnergy5200[energy];
			repeat = lpc_getBits(1);
			int origpitch = lpc_getBits(PERIOD_BITS); // 5110 - place these in defines
			//	nextPeriod = tmsPeriod[lpc_getBits(6)];
			nextPeriod=tmsPeriod5200[origpitch];
			//		fprintf(stderr,"ENERGY: %d REPEAT: %d PITCH: %d\n", energy, repeat, origpitch);

			/* A repeat frame uses the last coefficients */
			if(!repeat)
			{
				/* All frames use the first 4 coefficients */
			  nextK[0] = tmsK1[lpc_getBits(5)]; //TALKIE TODO/TESTY!
			  nextK[1] = tmsK2[lpc_getBits(5)];
			  nextK[2] = tmsK3[lpc_getBits(4)]<<8;
			  nextK[3] = tmsK4[lpc_getBits(4)]<<8;
			  if(nextPeriod)
				{
					nextK[4] = tmsK5[lpc_getBits(4)]<<8;
					nextK[5] = tmsK6[lpc_getBits(4)]<<8;
					nextK[6] = tmsK7[lpc_getBits(4)]<<8;
					nextK[7] = tmsK8[lpc_getBits(3)]<<8;
					nextK[8] = tmsK9[lpc_getBits(3)]<<8;
					nextK[9] = tmsK10[lpc_getBits(3)]<<8;
					/*			  nextK[0] = K1[lpc_getBits(5)]; //TALKIE TODO/TESTY!
				nextK[1] = K2[lpc_getBits(5)];
				nextK[2] = K3[lpc_getBits(4)];
				nextK[3] = K4[lpc_getBits(4)];
				if(nextPeriod)
				{
					nextK[4] = K5[lpc_getBits(4)];
					nextK[5] = K6[lpc_getBits(4)];
					nextK[6] = K7[lpc_getBits(4)];
					nextK[7] = K8[lpc_getBits(3)];
					nextK[8] = K9[lpc_getBits(3)];
					nextK[9] = K10[lpc_getBits(3)];*/
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
		  ulpc[10] = ((chirp5200[periodCounter]) * (uint32_t) synthEnergy) >> 8;
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
	  ulpc[i] = ulpc[i+1]-(((int32_t)synthK[i]*xlpc[i]) >> 15); // was >>15
	
	/* Output clamp */
		if (ulpc[0] > 511)
		ulpc[0] = 511;
	else if (ulpc[0] < -512)
		ulpc[0] = -512;
	
	/*
	while (a>511) { a-=1024; }
	while (a<-512) { a+=1024; }
	while (b>16383) { b-=32768; }
	while (b<-16384) { b+=32768; }
	*/

	/* Lattice filter reverse path */
	for(i=9;i>0;i--)
	  xlpc[i] = xlpc[i-1] + (((int32_t)synthK[i-1]*ulpc[i-1]) >> 15); // was >>15
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

   //const uint8_t sp_ffD008nn172[]={0xCC, 0xAD, 0x41, 0x48, 0xA6, 0x6C, 0x33, 0xBB, 0x48, 0x25, 0xED, 0x72, 0xC4, 0xD, 0xA1, 0x34, 0x6B, 0x20, 0x39, 0x69, 0xD, 0xDB, 0x94, 0xA, 0x6F, 0x78, 0x19, 0x4C, 0xCF, 0xCA, 0xBC, 0x9E, 0x64, 0x10, 0x43, 0xA9, 0xF2, 0x7A, 0x92, 0x89, 0xF, 0xA2, 0x6B, 0x53, 0x9A, 0x48, 0x3D, 0xA8, 0xE6, 0x9D, 0x79, 0xCC, 0xC9, 0x64, 0x1B, 0x5F, 0x8D, 0xAB, 0xAC, 0xC7, 0x2E, 0x52, 0xF6, 0xAA, 0xB1, 0x99, 0x2A, 0xA9, 0xC1, 0x85, 0xD5, 0x56, 0xC6, 0xA0, 0xAB, 0x9C, 0x19, 0x67, 0xA9, 0x9C, 0xA9, 0x4A, 0x7A, 0x6C, 0x39, 0x32, 0xB6, 0x6A, 0xAE, 0xB5, 0xE9, 0x4A, 0xD9, 0x62, 0xBA, 0xD5, 0x94, 0x1D, 0xE5, 0xF0, 0x51, 0x6E, 0x99, 0xA9, 0xA5, 0xCB, 0x3C, 0xB1, 0x42, 0x6D, 0x2A, 0x15, 0xF1, 0xCD, 0x70, 0xD1, 0xA9, 0x54, 0x28, 0x88, 0x35, 0x48, 0xCB, 0x8A, 0x12, 0x2F, 0x79, 0x33, 0xD, 0x4F, 0xFC, };

   const uint8_t test00[]={116, 33, 72, 14, 125, 94, 33, 81, 125, 116, 29, 125, 2, 81, 125, 84, 71, 255};
   


   
   //      lpc_say(test00);
   //   lpc_say(wordlist_tron[uffset]+uuffset);
   //                   while(synth_running) lpc_running();
   
   //   lpc_say(wordlist_[uffset]+uuffset);
   //   for (x=0;x<18;x++){
   x=0;
   while (test00[x]!=255){
      lpc_say(wordlist_alphons[test00[x++]]);
// lpc_say(sp_ffD008nn172);
//	printf("NEXT\n");
//		lpc_update_coeffs();

	// or we try c7 etc as length?
      while(synth_running) {
	lpc_running();

      }
	
   }

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
