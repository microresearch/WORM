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

#ifndef LAP
#include "stm32f4xx.h"
#else
#include "forlap.h"
#endif
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "nsynth.h"
#include "darray.h"
#include "holmes.h"
#include "phtoelm.h"
#include "elements.h"
#include "say.h"


//short *pWavBuffer;
//int16_t pWavBuffer[10240] __attribute__ ((section (".ccmdata")));

darray_t wav_elm;
unsigned short wav_len;

#define StressDur(e,s) ((e->ud + (e->du - e->ud) * s / 3)*speed)


// test TTS into phonemeToWave with new holmes

// so TTSnewsay generates TTS list - fixed now - which then generates list of elements (wav_elm), of length wav_elm.items

//	PhonemeToWaveData(eng2phonemeOUT-array from xlate, eng2phonemeOUT_ptr=length, 0);

void PhonemeToWaveData(char *phone, int len)
{
	unsigned frames;
 	int i, j;
 	darray_init(&wav_elm, sizeof(char), len); // where is wav_elm and how long is it?
	frames = phone_to_elm(phone, len, &wav_elm);
	}

void PhonemeToWaveDataxxx(u8 phone, int len, int verbose) // len here is 1 // this is for single phoneme?
{
  unsigned int frames; //int16_t *pwav;
    unsigned char intern[9];
    intern[0]=phone; // phoneme

    Elm_ptr p = &Elements[phone];
    intern[1]=StressDur(p,1); // duration
    intern[2]=0; // stress - 0/1/2/3 - set by phoneme as additional possible

    //    intern[3]=24;
    //    intern[4]=15;
    //    intern[5]=0;
 
    //	darray_init(&wav_elm, sizeof(char), len); // char=2
	
        frames=intern[1]; // add the durs - how long is a frame in samples? 256 samples...


	//		frames = phone_to_elm(phone, len, &wav_elm);
	
    unsigned max_samples = frames * klatt_global.nspfr;

    //		wav_len = holmes(	wav_elm.items, 
    //				(unsigned char *) darray_find(&wav_elm, 0),
    // 			max_samples, pWavBuffer	);

    //			    wav_len = holmes(3, 
    //					     intern,
    //					     max_samples, pWavBuffer	);

		//		}
    //    darray_free(&wav_elm);

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






