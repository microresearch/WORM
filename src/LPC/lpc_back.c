/*
 * lpc.c
 * 03-26-2015 E. Brombaugh
 * borrows heavily from Peter Knight's Talkie library for Arduino
 * This code is released under GPLv2 license
 */
 
#include "lpc.h"
//#include "audio.h"

#define INTERP_PERIOD 25  // samples per subframe
#define SUBFRAME_PERIOD 8 // subframes per frame

uint8_t* ptrAddr, ptrBit;
uint8_t synth_running, synth_subframe_ctr, synth_sample_ctr;
uint8_t starty, nextPeriod, synthPeriod;
uint16_t nextEnergy, synthEnergy;
int16_t synthK[10], nextK[10];
uint8_t periodCounter;
int16_t xlpc[10], ulpc[11];
uint16_t synthRand;
uint8_t byte_rev[256];

uint8_t spWHISKY[]         = {0x04,0x88,0xAE,0x8C,0x03,0x12,0x08,0x51,0x74,0x65,0xE9,0xEC,0x68,0x24,0x59,0x46,0x78,0x41,0xD7,0x13,0x37,0x6D,0x62,0xC3,0x5B,0x6F,0xDC,0xD2,0xEA,0x54,0xD2,0xE3,0x89,0x01,0x7E,0x2B,0xF7,0x80,0x07,0x14,0xD0,0xE5,0x15,0x38,0x60,0x8C,0x70,0x03,0x04,0x29,0x36,0xBA,0x5E,0x14,0x34,0x72,0xF6,0xE8,0xA7,0x6F,0x82,0xF4,0x2D,0x73,0xEA,0x47,0x3A,0x67,0x6A,0xC0,0xF0,0x2F,0xF1,0x4E,0xCF,0xA8,0x8A,0x1C,0xB9,0xD8,0xFF,0xEE,0x1F,0xBB,0x59,0xD0,0xD6,0xFE,0x3F};

uint8_t spMYWORM[]={0xc1, 0xb5, 0x45, 0x5d, 0x43, 0xa3, 0x44, 0x37, 0x4, 0x2e, 0x6b, 0xa8, 0x6b, 0xa4, 0x10, 0xa8, 0x6a, 0xa1, 0xa6, 0xa1, 0x93, 0xc3, 0x8a, 0x85, 0x86, 0x5a, 0x2e, 0xc, 0x5e, 0xe1, 0x58, 0x62, 0xa5, 0x90, 0xe8, 0x9a, 0x71, 0x8a, 0xe5, 0x42, 0x91, 0xa7, 0xc6, 0x29, 0x95, 0x13, 0x85, 0x8e, 0x39, 0x3b, 0x47, 0x6e, 0x14, 0xea, 0xe5, 0x54, 0x62, 0xba, 0x90, 0x68, 0xba, 0x71, 0x49, 0xe8, 0x42, 0xe1, 0xed, 0xca, 0x2d, 0xa1, 0x1b, 0xc9, 0xb6, 0x2a, 0xaf, 0x88, 0x19, 0x94, 0x18, 0xa6, 0xbc, 0x2a, 0x64, 0x90, 0xa2, 0x84, 0xe1, 0xb8, 0xd0, 0x45, 0xea, 0x10, 0x8e, 0x9d, 0x46, 0x97, 0xa0, 0xab, 0x29, 0x9d, 0x11, 0x5b, 0xbe, 0x2e, 0x61, 0x34, 0x2e, 0x6c, 0x51, 0x26, 0xbb, 0xf2, 0xb8, 0xb0, 0x45, 0xea, 0xe8, 0x86, 0x13, 0xc2, 0x86, 0xaf, 0x8b, 0x88, 0xac, 0x12, 0x5b, 0xa4, 0x2d, 0xaa, 0x7c, 0x46, 0x6c, 0x51, 0x3a, 0xbb, 0xf2, 0x9a, 0xb0, 0xe5, 0xa9, 0xec, 0x46, 0x63, 0xc4, 0x96, 0x24, 0xb3, 0x29, 0x8f, 0x9, 0x1b, 0x94, 0x2c, 0xa2, 0xb2, 0x4a, 0x6e, 0x48, 0xaa, 0x98, 0xc8, 0x9a, 0xb0, 0x21, 0xa9, 0x6e, 0xaa, 0x23, 0xc2, 0x86, 0x2c, 0xaa, 0xaa, 0xac, 0xa, 0x6b, 0x1, 0xaf, 0x22, 0xba, 0x66, 0xac, 0x5, 0xa4, 0xa9, 0x6a, 0x9b, 0x99, 0x14, 0xe0, 0x2a, 0x6e, 0x65, 0x91, 0x93, 0x84, 0x9a, 0xba, 0x85, 0x59, 0x49, 0x32, 0xae, 0x1a, 0x1a, 0x16, 0x39, 0x29, 0x2c, 0x5b, 0x70, 0x68, 0xe4, 0xa4, 0xc0, 0x62, 0x29, 0x21, 0xb1, 0x13, 0x5, 0xab, 0x85, 0xa4, 0x5a, 0x49, 0x14, 0xce, 0xe2, 0x1a, 0x16, 0x3a, 0xc8, 0xac, 0xa9, 0x6b, 0xa8, 0xe5, 0x40, 0xa9, 0xac, 0x29, 0xa6, 0x96, 0x43, 0x60, 0xb2, 0xb9, 0xb8, 0x45, 0xe, 0x9e, 0x4a, 0xe2, 0x16, 0x16, 0x39, 0xa4, 0xb2, 0xa8, 0x5b, 0x98, 0xa5, 0xe4, 0xaa, 0xac, 0xa1, 0xa1, 0x96, 0x53, 0xcf, 0x8b, 0x85, 0xb8, 0x5a, 0x4e, 0xa9, 0x2a, 0xe6, 0x1a, 0x6a, 0x39, 0x68, 0x3a, 0x6b, 0x68, 0x98, 0xe5, 0x10, 0x9b, 0xec, 0x61, 0x26, 0x51, 0x1e};

uint8_t spKATI[]={0xc1, 0x95, 0x49, 0x43, 0x5c, 0x22, 0x7, 0x47, 0x65, 0xb, 0x35, 0xb1, 0x1c, 0x42, 0x97, 0x2d, 0xd8, 0xd5, 0xf6, 0x31, 0xb2, 0xb3, 0x94, 0x90, 0xa8, 0x4b, 0xf7, 0xc6, 0x53, 0x42, 0xea, 0x1e, 0xd9, 0xbb, 0x8, 0xa, 0x6e, 0x76, 0xc4, 0x10, 0xdd, 0x38, 0x68, 0x59, 0x33, 0x78, 0x34, 0xd7, 0x94, 0xba, 0x55, 0x4b, 0xa2, 0xd, 0x96, 0x1a, 0x1c, 0x3b, 0x49, 0x26, 0x59, 0xa8, 0xab, 0x9d, 0x60, 0xca, 0x62, 0xa6, 0xa1, 0x56, 0x1c, 0x90, 0x9d, 0xd7, 0xe1, 0x62, 0x34, 0xd7, 0x54, 0xd7, 0x87, 0x8d, 0xde, 0x5d, 0x4b, 0xd5, 0x1c, 0x36, 0x6, 0xb, 0x2f, 0x56, 0x73, 0xb8, 0x18, 0xdc, 0xbd, 0xc9, 0xc9, 0x61, 0xa3, 0x8b, 0xb0, 0x26, 0x27, 0x87, 0x89, 0x26, 0xd2, 0x8b, 0xd5, 0x1c, 0xc6, 0x87, 0x48, 0x37, 0x71, 0x7d, 0x98, 0xe0, 0xbd, 0x43, 0x25, 0xf6, 0x11, 0xbc, 0x95, 0x76, 0x97, 0xba, 0x4b, 0xc0, 0xd5, 0x47, 0xdd, 0x96, 0xe, 0x11, 0x4f, 0x4b, 0x8d, 0x9c, 0x3b, 0x64, 0x34, 0x35, 0x35, 0xbd, 0x4d, 0xd3, 0xc0, 0xb2, 0xd2, 0xd0, 0xa6, 0xdd, 0x2c, 0x1e, 0x5a, 0x56, 0xea, 0x1a, 0x27, 0x91, 0xe8, 0x58, 0x59, 0x68, 0xec, 0x44, 0xc3, 0x6d, 0xa5, 0x6e, 0xb1, 0x53, 0x0, 0x96, 0xa7, 0xa6, 0xc5, 0x2e, 0x2c, 0x2b, 0x9a, 0x9a, 0x11, 0xbb, 0x44, 0xb8, 0x6a, 0x99, 0x5a, 0xdc, 0x91, 0x2a, 0xa3, 0xa9, 0xe5, 0x71, 0x5a, 0xca, 0xa2, 0x98, 0xb7, 0xd4, 0x6d, 0x39, 0xf, 0xa2, 0x36, 0xd2, 0x64, 0x64, 0xde, 0x53, 0x68, 0x5a, 0x94, 0x92, 0xaa, 0x48, 0x61, 0xa1, 0x55, 0x4b, 0xaa, 0x12, 0xb9, 0xa7, 0x9a, 0x49, 0xb9, 0x4e, 0x64, 0x16, 0x6a, 0x35, 0x14, 0x22, 0x72, 0x98, 0xab, 0xe5, 0x90, 0xa8, 0xcc, 0xee, 0x21, 0x56, 0x52, 0xa6, 0x93, 0x86, 0xb8, 0xc6, 0x8e, 0x61, 0xe8, 0x55, 0xd0, 0x50, 0x33, 0x2b, 0xc1, 0x35, 0x51, 0xc3, 0x4c, 0x23, 0x3f};

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

/*
 * Parse frame parameter bits from the ROM data stream.
 */
uint8_t lpc_getBits(uint8_t num_bits)
{
	uint8_t value;
	uint16_t data;
	
	data = byte_rev[*ptrAddr]<<8;
	if (ptrBit+num_bits > 8)
	{
		data |= byte_rev[*(ptrAddr+1)];
	}
	data <<= ptrBit;
	value = data >> (16-num_bits);
	ptrBit += num_bits;
	if (ptrBit >= 8)
	{
		ptrBit -= 8;
		ptrAddr++;
	}
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

extern int16_t audio_buffer[AUDIO_BUFSZ];

void lpc_running(){  // write into audio buffer
  //  lpc_get_sample();
  static u16 counterrr=0;
  int16_t samplel=(lpc_get_sample()<<6)-32768; // TODO or scale samples/speed???
  audio_buffer[counterrr++]=samplel;
  if (counterrr>AUDIO_BUFSZ) counterrr=0;
  audio_buffer[counterrr++]=samplel;
  if (counterrr>AUDIO_BUFSZ) counterrr=0;
  audio_buffer[counterrr++]=samplel;
  if (counterrr>AUDIO_BUFSZ) counterrr=0;
  audio_buffer[counterrr++]=samplel;
  if (counterrr>AUDIO_BUFSZ) counterrr=0;
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


void lpc_newsay(void)
{
	/* initialize ROM pointers */
	ptrAddr = spKATI;
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
		}
		else
		{
			/* All other energy types */
			nextEnergy = tmsEnergy[energy];
			repeat = lpc_getBits(1);
			nextPeriod = tmsPeriod[lpc_getBits(6)];
			
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
		/* targets are currents */
		synthPeriod = nextPeriod;
		synthEnergy = nextEnergy;
		for(i=0;i<10;i++)
			synthK[i] = nextK[i] ;
	}
	else
	{
		/* expo decay to target interpolation */
#define EXPO_SHIFT 2
		synthPeriod += (nextPeriod-synthPeriod)>>EXPO_SHIFT;
		synthEnergy += (nextEnergy-synthEnergy)>>EXPO_SHIFT;
		for(i=0;i<10;i++)
			synthK[i] += (nextK[i]-synthK[i])>>EXPO_SHIFT;
	}
}

/*
 * compute a new sample of LPC data
 */
uint16_t lpc_get_sample(void)
{
	int8_t i;
	
	/* if not running just return mid-scale */
	if(synth_running == 0)
	{
		return 512;
		}
	
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
		synthRand = (synthRand >> 1) ^ ((synthRand & 1) ? 0xB800 : 0);
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
