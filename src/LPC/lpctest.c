/*
 * lpc.c
 * 03-26-2015 E. Brombaugh
 * borrows heavily from Peter Knight's Talkie library for Arduino
 * This code is released under GPLv2 license
 */

#include "stdio.h"
#include "math.h"

unsigned char buffer[327680];//143360
//int offset=0x13c3;
//int offset=0x1408;
int offset=0x00;
int printer=0;

#define INTERP_PERIOD 25  // samples per subframe
#define SUBFRAME_PERIOD 8 // subframes per frame

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;


const uint8_t* ptrAddr;
uint8_t ptrBit=0;
uint8_t synth_running, synth_subframe_ctr, synth_sample_ctr;
uint8_t starty, nextPeriod, synthPeriod;
uint16_t nextEnergy, synthEnergy;
int16_t synthK[10], nextK[10];
uint8_t periodCounter;
int16_t xlpc[10], ulpc[11];
uint16_t synthRand;
uint8_t byte_rev[256];

int didntjump=1;

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

int accessed;

uint8_t lpc_getBits(uint8_t num_bits)
{
	uint8_t value;
	uint16_t data;

	//	printf("addr %X byte %X\n", ptrAddr-buffer, byte_rev[*ptrAddr]);
		
	//	data = byte_rev[*ptrAddr]<<8;
	data = (*ptrAddr)<<8;
	accessed=0;
	if (ptrBit+num_bits > 8)
	{
		data |= *(ptrAddr+1);
		accessed=1;
		//		didntjump=1;
		//		printf("ACCESSED");
	}
	data <<= ptrBit;
	value = data >> (16-num_bits);
	ptrBit += num_bits;
	didntjump=1;
	if (ptrBit >= 8)
	{
		ptrBit -= 8;
		ptrAddr++;
	if (printer==1)	printf("0x%X, ", byte_rev[*ptrAddr]);
		if (ptrBit==0) {
		  didntjump=0;
		}
		accessed=0;
		//		printf("JUMPED");
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
	
}

int counter=0;
int justafter=0;
int firstbits;

int lpc_update_coeffs(void)
{
	int8_t i;
	uint8_t repeat;
	uint16_t energy;
		
		/* Read speech data, processing the variable size frames. */
		energy = lpc_getBits(4);
		//		printf("ENERGY %d\n",energy);
		if(energy == 0)
		{
			/* Energy = 0: rest frame */
			nextEnergy = 0;
			if (justafter==1) { //can be start or padding? can't decide!

			  /*			if (didntjump){
			ptrBit =0;
			ptrAddr++;
			}
			didntjump=0;*/
			justafter=1; // paddings?
			}
			//		else justafter=0;
			return 1;
		}
		else if(energy == 0xf)
		{
			/* Energy = 15: stop frame. Silence the synthesiser. */
		  //		  printf("ZZZZZZZZZZZZZZZZZZZZZZ--------------------------------->stop frame %d %X \n",counter,(ptrAddr-buffer));
		  if (printer==1) printf("\n\n");
			if (justafter==1) return 0;
			counter++;
			//next one
			justafter=1;
			if (accessed){
			  //			  printf("ACCESSED");

			ptrBit =0;
			ptrAddr++;
			}
						if (didntjump){
			ptrBit =0;
			ptrAddr++;
			if (printer==1)	printf("0x%X, ", byte_rev[*ptrAddr]);
			} 
			didntjump=0;
			return 1;
		}
		else if(energy>0x0f)
		  {
		    //		    printf("BOO\n");
		    return 0;
		  }
		else 
		{
		  if (justafter==1){
		    //new 
		    //		    printf("\n");

		  }

			/* All other energy types */
		  		  //		  		  printf("ENERGY %d\n", energy);
			repeat = lpc_getBits(1);
			//				printf("REPEAT %d\n", repeat);
			if (justafter==1 && repeat) return 0; // nothing to repeat
			justafter=0;
			nextPeriod = tmsPeriod[lpc_getBits(6)];
			//			printf("repeat %d\n",repeat);
			/* A repeat frame uses the last coefficients */
			if(!repeat)
			{
				/* All frames use the first 4 coefficients */
			  //				  printf("frame\n");
				nextK[0] = tmsK1[lpc_getBits(5)];
				nextK[1] = tmsK2[lpc_getBits(5)];
				nextK[2] = tmsK3[lpc_getBits(4)]<<8;
				nextK[3] = tmsK4[lpc_getBits(4)]<<8;
				if(nextPeriod)
				{
					/* Voiced frames use 6 extra coefficients. */
				  //				  printf("voiced\n");
					nextK[4] = tmsK5[lpc_getBits(4)]<<8;
					nextK[5] = tmsK6[lpc_getBits(4)]<<8;
					nextK[6] = tmsK7[lpc_getBits(4)]<<8;
					nextK[7] = tmsK8[lpc_getBits(3)]<<8;
					nextK[8] = tmsK9[lpc_getBits(3)]<<8;
					nextK[9] = tmsK10[lpc_getBits(3)]<<8;
				}
			}
		}
	
	return 1;
}

int alt_lpc_update_coeffs(void)
{
	int8_t i;
	uint8_t repeat;
	uint16_t energy;
		
		/* Read speech data, processing the variable size frames. */
		energy = lpc_getBits(4);
		//		printf("ENERGY %d\n",energy);
		if(energy == 0)
		{
			/* Energy = 0: rest frame */
			nextEnergy = 0;
		}
		else if(energy == 0xf)
		{
			return 0;
		}
		else if(energy>0x0f)
		  {
		    //		    printf("BOO\n");
		    return 0;
		  }
		else 
		{

			/* All other energy types */
		  		  //		  		  printf("ENERGY %d\n", energy);
			repeat = lpc_getBits(1);
			//				printf("REPEAT %d\n", repeat);
			if (justafter==1 && repeat) return 0; // nothing to repeat
			justafter=0;
			nextPeriod = tmsPeriod[lpc_getBits(6)];
			//			printf("repeat %d\n",repeat);
			/* A repeat frame uses the last coefficients */
			if(!repeat)
			{
				/* All frames use the first 4 coefficients */
			  //				  printf("frame\n");
				nextK[0] = tmsK1[lpc_getBits(5)];
				nextK[1] = tmsK2[lpc_getBits(5)];
				nextK[2] = tmsK3[lpc_getBits(4)]<<8;
				nextK[3] = tmsK4[lpc_getBits(4)]<<8;
				if(nextPeriod)
				{
					/* Voiced frames use 6 extra coefficients. */
				  //				  printf("voiced\n");
					nextK[4] = tmsK5[lpc_getBits(4)]<<8;
					nextK[5] = tmsK6[lpc_getBits(4)]<<8;
					nextK[6] = tmsK7[lpc_getBits(4)]<<8;
					nextK[7] = tmsK8[lpc_getBits(3)]<<8;
					nextK[8] = tmsK9[lpc_getBits(3)]<<8;
					nextK[9] = tmsK10[lpc_getBits(3)]<<8;
				}
			}
		}
	
	return 1;
}



int main(){

  lpc_init();
  int maxcounter=0,maxoffset=0;
  // read in rom offset and see if we can match

   FILE *fp;
   int xx,yy,bits=0,xxy;
   unsigned char rever;
   /* Open file for both reading and writing */
   int length=23475;
   
   //      fp = fopen("/root/Downloads/TI99/speech/spchrom.bin", "r");
   //      fp = fopen("/root/Downloads/TI99/apple/ciderpress/linux/echo2dsk.LPC", "r");
      fp = fopen("/root/WORDS_D000", "r");
   fread(buffer, 1, length, fp);

   //      for (xx=0;xx<32768;xx++){
   //      printf("0x%02X, ",byte_rev[buffer[0x13c3]]); // checks out okay
	//      }
   printer=0;

   /*
         while(offset<length){
        counter=0;


   //   printf("%s\n", buffer);
	ptrAddr=buffer+offset; ptrBit=0;
	xxy=1;      justafter=1;

      while((ptrAddr-buffer<(length-offset)) && xxy==1){
	xxy=lpc_update_coeffs();
	//	printf("ptrAddr-buffer %d\n",ptrAddr-buffer);

      }
      //      printf("......................XXXXXXXXXXXXXXXXXXXXXXXXXXXXstopped\n");

      //   if (counter>1) printf("count: %d offset: %d xxy %d\n",counter,offset,xxy);
      if (counter>maxcounter){
	maxcounter=counter;
	maxoffset=offset;
      }
   

      offset++;

   }
	 printf("maxcounter %d maxoffset 0x%X reved 0x%x,0x%x\n",maxcounter,maxoffset,byte_rev[buffer[maxoffset]],byte_rev[buffer[maxoffset+1]]);
	 fclose(fp);
*/

   	 maxoffset=0;
   
	 // seek and print with breaks from here;;
	 //	 ptrAddr=buffer+maxoffset; ptrBit=0;
	 //	 printer=1; xxy=1;

	 //	 printf("0x%X, ", byte_rev[*ptrAddr]);
	 //	 while((ptrAddr-buffer<(length-maxoffset))){
	 //	xxy=alt_lpc_update_coeffs();
	//		printf("ptrAddr-buffer %d\n",ptrAddr-buffer);
	//		if (xxy==0) printf("\n\n");
	 //	}
    ptrAddr=buffer;
   // dump reversed
   for (xx=0;xx<length;xx++){
     printf("0x%X, ", byte_rev[*ptrAddr+xx]);
     
     }

}
