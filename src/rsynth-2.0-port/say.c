#include <config.h>

/* $Id: say.c,v 1.13 1994/11/08 13:30:50 a904209 Exp a904209 $
   $Log: say.c,v $
 * Revision 1.13  1994/11/08  13:30:50  a904209
 * 2.0 release
 *
 * Revision 1.12  1994/11/04  13:32:31  a904209
 * 1.99.1 - Change configure stuff
 *
 * Revision 1.11  1994/11/02  10:55:31  a904209
 * Add autoconf. Tested on SunOS/Solaris
 *
 * Revision 1.10  1994/10/04  17:12:50  a904209
 * 3rd pre-release
 *
 * Revision 1.9  1994/10/04  09:08:27  a904209
 * Next Patch merge
 *
 * Revision 1.8  1994/10/03  08:41:47  a904209
 * 2nd pre-release                                                         
 *
 * Revision 1.7  1994/09/19  15:48:29  a904209
 * Split hplay.c, gdbm dictionary, start of f0 contour, netaudio and HP ports
 *
 * Revision 1.6  1994/04/15  16:47:37  a904209
 * Edits for Solaris2.3 (aka SunOs 5.3)
 *
 * Revision 1.5  1994/02/24  15:03:05  a904209
 * Added contributed linux, NeXT and SGI ports.
 *
 * Revision 1.4  93/11/18  16:29:06  a904209
 * Migrated nsyth.c towards Jon's scheme - merge still incomplete
 * 
 * Revision 1.3  93/11/16  14:32:44  a904209
 * Added RCS Ids, partial merge of Jon's new klatt/parwave
 *
 * Revision 1.3  93/11/16  14:00:58  a904209
 * Add IDs and merge Jon's klatt sources - incomplete
 *
 */
extern char *Revision;

#include "stm32f4xx.h"
//#include "platform_config.h"
//#include "hw_config.h"

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "nsynth.h"
#include "darray.h"
#include "holmes.h"
#include "phtoelm.h"
#include "say.h"

//short *pWavBuffer;
int16_t pWavBuffer[3840] __attribute__ ((section (".data")));

darray_t wav_elm;
unsigned short wav_len;

void PhonemeToWaveData(char *phone, int len, int verbose)
{
  unsigned frames; //int16_t *pwav;
 
	int i, j;
 
	darray_init(&wav_elm, sizeof(char), len);
	
	//	frames=len;
		if((frames = phone_to_elm(phone, len, &wav_elm))){
	
		unsigned max_samples = frames * klatt_global.nspfr;

		//		int xxxy=crashfun();

		if (verbose){
			printf("max_samples = %d\n", max_samples);
		}
   
		//		pWavBuffer = (short *) malloc(sizeof(short) * max_samples);
		//		pwav=&pWavBuffer[0];

		if (verbose){
			printf("%.*s\n", len, phone);
		}
			
		//		if (pWavBuffer){
			wav_len = holmes(	wav_elm.items, 
								(unsigned char *) darray_find(&wav_elm, 0),
						max_samples, pWavBuffer	);

			if (verbose){
				printf("wav_len = %d\n", wav_len);
		
				j = wav_len;
				if(j>256) j = 256;
				for(i=0;i<j;i++){
					printf("%04X %04X\n", pWavBuffer[i], (0x1000 + pWavBuffer[i]));
				}
			}

			// free(pWavBuffer);
			//				}
			//		}else{
		  //			fprintf(stderr, "memory shortage (PhonemeToWaveData)\n");
			//					}
		}
	// darray_free(&wav_elm);
}



void FreePhonemeToWaveData(void)
{
  /*  	if(pWavBuffer){
		free(pWavBuffer);
		pWavBuffer = NULL;
	}
  */	
	darray_free(&wav_elm);
}






