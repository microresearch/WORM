/*
 * lpc.c
 * 03-26-2015 E. Brombaugh
 * borrows heavily from Peter Knight's Talkie library for Arduino
 * This code is released under GPLv2 license
 */

// stderr redirect etc =  ./say 0 0 2>&1 > /dev/dsp
 
#include "stdio.h"
#include "stdlib.h"
#include "math.h"


//#include "lpc.h"
//#include "audio.h"

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef unsigned short u16;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

uint16_t lpc_get_sample(void);

uint16_t lastbyte;

#define INTERP_PERIOD 25  // samples per subframe
#define SUBFRAME_PERIOD 8 // subframes per frame

int didntjump=1;
   unsigned char *xxx;

uint8_t* ptrAddr, ptrBit;
uint8_t synth_running, synth_subframe_ctr=8, synth_sample_ctr;
uint8_t starty, nextPeriod, synthPeriod;
uint16_t nextEnergy, synthEnergy;
int16_t synthK[10], nextK[10];
uint8_t periodCounter;
int16_t xlpc[10], ulpc[11];
uint16_t synthRand;
uint8_t byte_rev[256];

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

#define CHIRP_SIZE 52
const int8_t chirp[CHIRP_SIZE] = {0x00,0x2a,0xd4,0x32,0xb2,0x12,0x25,0x14,0x02,0xe1,0xc5,0x02,0x5f,0x5a,0x05,0x0f,0x26,0xfc,0xa5,0xa5,0xd6,0xdd,0xdc,0xfc,0x25,0x2b,0x22,0x21,0x0f,0xff,0xf8,0xee,0xed,0xef,0xf7,0xf6,0xfa,0x00,0x03,0x02,0x01};

const int8_t chirpnew[52] = { 0x00, 0x03, 0x0f, 0x28, 0x4c, 0x6c, 0x71, 0x50,\
		0x25, 0x26, 0x4c, 0x44, 0x1a, 0x32, 0x3b, 0x13,\
		0x37, 0x1a, 0x25, 0x1f, 0x1d, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,\
					    0x00, 0x00, 0x00, 0x00 };


const uint8_t tmsPeriod5110[0x40] =	{   0,  15,  16,  17,  19,  21,  22,  25,  \
		26,  29,  32,  36,  40,  42,  46,  50,  \
		55,  60,  64,  68,  72,  76,  80,  84,  \
					    86,  93, 101, 110, 120, 132, 144, 159};

const uint8_t tmsPeriod5200[0x40] = 	{   0,  14,  15,  16,  17,  18,  19,  20,  \
		21,  22,  23,  24,  25,  26,  27,  28,  \
		29,  30,  31,  32,  34,  36,  38,  40,  \
		41,  43,  45,  48,  49,  51,  54,  55,  \
		57,  60,  62,  64,  68,  72,  74,  76,  \
		81,  85,  87,  90,  96,  99, 103, 107, \
		112, 117, 122, 127, 133, 139, 145, 151, \
					    157, 164, 171, 178, 186, 194, 202, 211};

/*
 * Parse frame parameter bits from the ROM data stream.
 */
uint8_t lpc_getBits(uint8_t num_bits)
{
	uint8_t value;
	uint16_t data;
	
		data = byte_rev[*ptrAddr]<<8;
		//	data = (*ptrAddr)<<8;
	if (ptrBit+num_bits > 8)
	{
	        data |= byte_rev[*(ptrAddr+1)];
	  //  	   data |= *(ptrAddr+1);
	}
	data <<= ptrBit;
	value = data >> (16-num_bits);
	ptrBit += num_bits;
	didntjump=1;
	if (ptrBit >= 8)
	{
	  //	  fprintf(stderr, "OFF: %d\n", ptrAddr-xxx);
	  //	  fprintf(stderr, "lastbyte: 0x%x\n", *ptrAddr);
	  lastbyte=ptrAddr-xxx;
	  	  printf("0x%X, ",*ptrAddr);
		ptrBit -= 8;
		ptrAddr++;
		//		didntjump=2;
		if (ptrBit==0) {
		  	  didntjump=0;
		}
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
	synth_subframe_ctr = 7;
	synth_sample_ctr = 0;
	synthRand = 1;
}

//extern int16_t audio_buffer[AUDIO_BUFSZ];

void lpc_running(){  // write into audio buffer
  //  lpc_get_sample();
  static u16 counterrr=0;
  int16_t samplel=lpc_get_sample()>>2; // TODO or scale samples/speed???
  
  //          printf("%c",samplel);
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
			//ptrAddr++; ptrBit=0;
			if (didntjump){
			  			  printf("0x%X, ",*ptrAddr);

			  ptrBit =0;
			  ptrAddr++;
			} 
			//			didntjump=0;
			starty=1;
			//	sleep(1);
	synth_subframe_ctr = 0;
	synth_sample_ctr = 0;
	//	printf("};\n{");
	//	fprintf(stderr, "OFF: %d\n", ptrAddr-xxx);


		}
		else
		{
			/* All other energy types */
			nextEnergy = tmsEnergy[energy];
			repeat = lpc_getBits(1);
			//			ptrBit =0; // TEST for ALPHONS
			//			ptrAddr++;
									nextPeriod = tmsPeriod5200[lpc_getBits(6)]; // TEST for 5110
			//						nextPeriod = tmsPeriod5110[lpc_getBits(5)]; // TEST for 5110
			//			ptrBit =0; // TEST for ALPHONS
			//			ptrAddr++;
			//			nextPeriod=64;
			//			nextPeriod=128;
			/* A repeat frame uses the last coefficients */
			if(!repeat)
			{
				/* All frames use the first 4 coefficients */
				nextK[0] = tmsK1[lpc_getBits(5)];
				//			ptrBit =0; // TEST for ALPHONS
				//			ptrAddr++;

				nextK[1] = tmsK2[lpc_getBits(5)];
				//			ptrBit =0; // TEST for ALPHONS
				//			ptrAddr++;
				nextK[2] = tmsK3[lpc_getBits(4)]<<8;
				//			ptrBit =0; // TEST for ALPHONS
				//			ptrAddr++;
				nextK[3] = tmsK4[lpc_getBits(4)]<<8;
				if(nextPeriod)
				{
					/* Voiced frames use 6 extra coefficients. */
				  //			ptrBit =0; // TEST for ALPHONS
				  //			ptrAddr++;

					nextK[4] = tmsK5[lpc_getBits(4)]<<8;
					//			ptrBit =0; // TEST for ALPHONS
					//			ptrAddr++;
					nextK[5] = tmsK6[lpc_getBits(4)]<<8;
					//			ptrBit =0; // TEST for ALPHONS
					//			ptrAddr++;
					nextK[6] = tmsK7[lpc_getBits(4)]<<8;
					//			ptrBit =0; // TEST for ALPHONS
					//			ptrAddr++;
					nextK[7] = tmsK8[lpc_getBits(3)]<<8;
					//			ptrBit =0; // TEST for ALPHONS
					//			ptrAddr++;
					nextK[8] = tmsK9[lpc_getBits(3)]<<8;
					//			ptrBit =0; // TEST for ALPHONS
					//			ptrAddr++;
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
		//	       		fprintf(stderr,"ENERGY: %d REPEAT: %d PITCH: %d\n", nextEnergy, repeat, nextPeriod);
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
		  ulpc[10] = ((chirpnew[periodCounter]) * (uint32_t) synthEnergy) >> 8; // TESTY
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



void main(int argc, char *argv[]){
  lpc_init();
   int uffset=atoi(argv[1]);
   int uuffset=atoi(argv[2]);
   int lengthy,flag=0;
   FILE *fp = fopen(argv[3], "r");
   fseek(fp,0, SEEK_END);
   // read in and how long is it?
   lengthy=ftell(fp);
   fseek(fp,0, SEEK_SET);

   // malloc and reverse into buffer
   xxx=malloc(lengthy+1);
   fread(xxx,lengthy,1,fp);
   fclose(fp);
   //   xxx[lengthy]=0;
   		  printf("{");

   // speak that buffer
   //         while(flag==0){  
   //     	      	fprintf(stderr, "OFF: %d\n\n", uffset);
		lpc_say(xxx+uffset);
	     //while(synth_running && (ptrAddr-xxx)<uuffset) lpc_running();
	     	     while(synth_running) lpc_running();
		     //		     	     	     fprintf(stderr, "LAST: %d\n\n", lastbyte);
	     //	     lpc_running();
	     //	     	  while(1) lpc_running();
	  	  uffset++;
	  	  if (uffset>lengthy) flag=1;
		  printf("}\n\n");
		  //		  printf("%d\n", uffset);
		  //	  	                while(1) lpc_running();
		  //		  sleep(2);
		  //		  		  		  	   }
}
