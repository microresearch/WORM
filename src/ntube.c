// from SLUGens.cpp SC

//UGens by Nick Collins
//SLUGens released under the GNU GPL as extensions for SuperCollider 3, by Nick Collins http://composerprogrammer.com/index.html

// what is relation of number of tubes and samplerate?

//average length of human male vocal tract 16.9cm (14.1cm adult female)  speed of sound 340.29 m/s. So delay of vocal tract is 
//0.169/340.29 = 0.00049663522289812 seconds
//0.0005*44100 is about 22 samples, so less than one sample per section of the throat if more than 22 measurements used! 
//need higher sampling rate, or less sections in model

// or for 32000 we have 16 samples 

// Length in seconds of each tube's paired delay line (i.e., each waveguide section, N of them). There must be at least 2 samples per length at the synthesis sampling rate. 

// but let's settle on say 4 tubes

#include "audio.h"
#include "ntube.h"
#include "scformant.h"
#include <math.h>
#include "arm_const_structs.h"

extern RLPF *RLPFer; // TODO - do not re-use this instance


extern __IO uint16_t adc_buffer[10];

inline float somenoise(){
float xx=(float) ( 0.5 * rand() / (RAND_MAX + 0.5) - 0.5 );
 return xx;
}

// see NTube.schelp
//{(NTube.ar(WhiteNoise.ar*SinOsc.ar(0.5),`[0.97, 1.0, 1.0, 1.0, 0.97], `[0.5,MouseY.kr(-1.0,1.0),0.2],`([0.01,0.02,0.01,0.005]*MouseX.kr(0.01,1.0)))*0.1).dup}.play
//

static float losses[5]={0.95, 1.0, 1.0, 1.0, 0.97};//N+1
static float scatteringcoefficients[3]={0.5, 0.01, 0.2};//N-1 eg 0.01 // try vary second option -1 to +1 THIS IS K
static float delays[4]={3, 3, 3, 3};//N - but delays in samples not seconds = length of tube which is????

// try as just 2 tubes. we can make this one just with setting ___ as XX

// so we have samplenumber/32000 gives us seconds. which * 340.29 m/s gives us length of section in metres
// so for 4 we have 4/32000 =0.000125 =0.043 which is 4cm - 4 x 4 is human tract   ... for raven is 13cm = 4x3 say


	//	float * losses= unit->losses;
	//	float * scatteringcoefficients= unit->scattering;
	//	float * delays= unit->delays;


////output= NTube.ar(input, loss, karray, delaylengtharray);
void NTube_init(NTube* unit) {

  u8 i; int j;

	//	const u8 numinputs = unit->mNumInputs;
	const u8 numtubes=4; ///(numinputs-1)/3;  //NOW 1+ (N+1) + N-1 + N 3N+1//WAS 1+1+N-1+N = 2N+1 TRY as 2 NOW TEST!
	unit->numtubes= numtubes;

	unit->maxlength= 512; //no frequencies below about 50 Hz for an individual section - this can be reduced for 32k samplerate to say 512
	unit->modulo= unit->maxlength-1;

	//	unit->delayconversion= 32000.0f; //multiplies delay time in seconds to make delay time in samples

	//printf("num tubes only %d and delayconversion %f \n", numtubes, unit->delayconversion);

	//	unit->delayright= (float**)malloc(numtubes * sizeof(float *));
	//	unit->delayleft= (float**)malloc(numtubes * sizeof(float *));

	for (i=0; i<numtubes; ++i) {

	  //	unit->delayright[i]= 	(float*)malloc(unit->maxlength * sizeof(float));
	  //	unit->delayleft[i]= 	(float*)malloc(unit->maxlength * sizeof(float));

	  //		float * pointer1 = 	unit->delayright[i];
	  //	float * pointer2 = 	unit->delayleft[i];


		for (j=0; j<unit->maxlength; ++j) {
			unit->delayright[i][j]= 0.0;
			unit->delayleft[i][j]= 0.0;
		}
	}

	//	unit->losses= (float*)malloc((numtubes+1) * sizeof(float)); // fixed arrays
	//	unit->scattering= (float*)malloc((numtubes-1) * sizeof(float));
	//	unit->delays= (float*)malloc(numtubes * sizeof(float));

	//	unit->rightouts= (float*)malloc(numtubes * sizeof(float));
	//	unit->leftouts= (float*)malloc(numtubes * sizeof(float));


	unit->position=0;

	unit->f1in= 0.0;
	unit->f1out= 0.0;
	unit->f2in=0.0;
	unit->f2out=0.0;

	//	SETCALC(NTube_next);
}

void NTube_do(NTube *unit, float *in, float *out, int inNumSamples) {

  u8 i,j; static float count=0.0f; 

	u8 numtubes= unit->numtubes;

	//	float ** right= unit->delayright;
	int pos= unit->position;
	//	float ** left= unit->delayleft;

	//GET FREQUENCIES AND SCATTERING COEFFICIENTS
	//	float * losses= unit->losses;
	//	float * scatteringcoefficients= unit->scattering;
	//	float * delays= unit->delays;

	//int arg=1;

	//used to be single argument
	//float loss= (float)ZIN0(1);

	/*
	for (i=0; i<(numtubes+1); ++i)	{

	  losses[i]= ZIN0(arg); // TODO - losses array! = N+1 length
	  ++arg;
	  }*/

	/*	for (i=0; i<(numtubes-1); ++i) {

	  scatteringcoefficients[i]= ZIN0(arg); // TODO - scattering array = N-1 length check schelp
		++arg;
		}*/

	//	scatteringcoefficients[1]=((float)adc_buffer[SELZ]/2048.0f)-1.0f; 

	// try this as worming??? there are 3 coeffs - values are -1 to 1 TESTY!!!
	
	scatteringcoefficients[(int)count%3]=sinf((float)count)*0.8f;
	count+=(float)adc_buffer[SPEED]/4096.0f;

	int maxlength= unit->maxlength;
	float maxlengthf= (float) maxlength;
	float maxlengthfminus1= (float) (maxlength-1);
	int modulo= unit->modulo;
	/*
		float delayconv= unit->delayconversion;

		for (i=0; i<numtubes; ++i) {

	  float delayinsec= ZIN0(arg); // TODO - delay array N
		float delayinsamples= delayconv*delayinsec;

		if(delayinsamples<0.0) delayinsamples=0.0;
		if(delayinsamples>maxlengthfminus1) delayinsamples= maxlengthfminus1;

		delays[i]= delayinsamples; //ZIN0(arg);

		//printf("delay %d is %f \n", i, delays[i]);
		++arg;
		}*/

	//have to store filter state around loop; probably don't need to store output, but oh well
	float f1in=unit->f1in;
	float f2in=unit->f2in;
	float f2out=unit->f2out;
	float f1out=unit->f1out;

	float * delayline;
	float * delayline2;
	float past;
	int pos1, pos2;
	float interp; //for linear interpolation of position

	float * rightouts= unit->rightouts;
	float * leftouts= unit->leftouts;

	for (i=0; i<inNumSamples; ++i) {

		//update all outs

		for (j=0; j<numtubes; ++j) {

			//calculate together since share position calculation, same delay length in each tube section
			delayline= unit->delayright[j];
			delayline2= unit->delayleft[j];

			past = fmodf(pos+maxlengthf- delays[j], maxlengthf);

			pos1= past; //round down
			interp= past-pos1;
			pos2= (pos1+1)&modulo;
			rightouts[j]= ((1.0-interp)*delayline[pos1]) + (interp*delayline[pos2]);
			leftouts[j]= ((1.0-interp)*delayline2[pos1]) + (interp*delayline2[pos2]);

		}


		//printf("got to here! %d \n",i);


		//output value
		out[i]=rightouts[numtubes-1]; // last of the rightouts

		//including filters at the ends:

		//update all filters
		f1out= losses[0]*0.5*(f1in+leftouts[0]);
		f1in= leftouts[0];

		//should change s factor later, and independent time varying gains...
		f2out= losses[numtubes]*(0.5*f2in+0.5*rightouts[numtubes-1]);
		f2in= - rightouts[numtubes-1]; // final end reflection?>? TODO_ TESTY!

		//// and if we LPF f2out?
		//		RLPF_do_single(RLPFer, &f2out, &f2out, 100, 1.4f, 0.001);

		delayline= unit->delayright[0];
		delayline2= unit->delayleft[numtubes-1];

		delayline[pos]= in[i]+f1out; 
		delayline2[pos]= f2out;

		// then update all other ins via numtubes-1 scattering junctions
		// this is where we can choose to add white noise of GAIN SPEED? TODO!

		for (j=0; j<(numtubes-1); ++j) {

			float k = scatteringcoefficients[j];

			delayline= unit->delayright[j+1];
			delayline2= unit->delayleft[j];


			//version one: no internal friction, too long
			//delayline[pos]= rightouts[j]*(1+k)+ ((-k)*leftouts[j+1]);
			//delayline2[pos]= rightouts[j]*k+ ((1-k)*leftouts[j+1]);

			float loss= losses[j+1];
			//always a loss at interface to avoid continual recirculation; separate internal loss parameter?
			if (j==1) delayline[pos]= rightouts[j]*(1+k)+ (loss*(-k)*leftouts[j+1]);//+somenoise(); 
			else delayline[pos]= rightouts[j]*(1+k)+ (loss*(-k)*leftouts[j+1]); // is this reflection -k so last would be out ?
			delayline2[pos]= (rightouts[j]*k*loss)+ ((1-k)*leftouts[j+1]);


			//calculate inputs of all delays
			//d2right[d2rightpos]= d1rightout*(1+k)+ ((-k)*d2leftout);
			//d1left[d1leftpos]= d1rightout*k+ ((1-k)*d2leftout);

		}

		//update common delay line position pointer
		//update position
		pos= (pos+1)&modulo;


		//printf("got to here 3! %d %d \n",i,pos);
	}

	unit->f1in= f1in;
	unit->f2in= f2in;
	unit->f2out=f2out;
	unit->f1out= f1out;


	unit->position= pos;
}
