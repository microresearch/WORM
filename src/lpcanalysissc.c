/*
 *  LPCAnalysis.h
 *
 *  Created by Nicholas Collins on 10/09/2009.
 *  Copyright 2009 Nicholas M Collins. All rights reserved.
 *
 */

typedef float LPCfloat;


	int windowsize;
	float * windowfunction; //can add later if start cross fading consecutive windows with hop
	int windowtype; //0 is rectangular, 1 is triangular, 2 is squared cosine, 3 is custom passed in buffer?

	float * input;
	//float * output;
	//int blocksize;
	int numpoles;
	int pos;
	float * coeff;
	float * last;


	int testdelta;
	LPCfloat delta;
	LPCfloat latesterror;
	float G; //gain;

	//matrix calculations at double resolution to help avoid errors?
	//storage for Levinson-Durbin iteration to calculate coefficients
	LPCfloat * R;
	LPCfloat * preva;
	LPCfloat * a;

LPCAnalysis(int _windowsize, int _windowtype=0, int offset=0): windowsize(_windowsize), windowtype(_windowtype) {
		//need to know about stupid unit->mWorld
		input= new float[windowsize];
		windowfunction= new float[windowsize];
		//output= new float[blocksize];
		coeff= new float[windowsize];
		last= new float[windowsize];

		R= new LPCfloat[windowsize+1];
		preva= new LPCfloat[windowsize+1];
		a= new LPCfloat[windowsize+1];

		zeroAll();

		pos=offset;
	};


	void zeroAll() {

		int i;

		numpoles=10;

		pos=0;

		for (i=0; i<windowsize;++i) {
			input[i]= 0.0;
			coeff[i]=0.0;
			last[i]=0.0;
		}

		int half= windowsize>>1;

		switch(windowtype) {

		default:
		case 0:
			//rectangular window
			for (i=0; i<windowsize;++i) {
				windowfunction[i]= 1.0;
			}
			break;

		case 1:
				//triangular window
				float mult= 1.0/(float)half;
				for (i=0; i<half;++i) {
					float value= mult*((float)i);
					windowfunction[i]= value;
					windowfunction[half+i]=1.0-value;
				}


			break;
		//
//		case 2:
//
//
//			break;
//
		}


		//for (i=0; i<blocksize;++i) {
//			output[i]= 0.0;
//		}

		testdelta=0;
		delta= 0.999;
		latesterror=0.0;

		G=0.0; //gain starts at zero;
	}


	//latest value of p is number of poles for fitting; if -1 adapt using criteria
	//assess if tonal or transient via error?
	//source is driver signal for filtering, newinput is data arriving for analysis

/*
 *  LPCAnalyzer.cpp
 *  xSC3ExtPlugins-Universal
 *
 *  Created by Nick Collins on 10/04/2009.
 *  Copyright 2009 Nick Collins. All rights reserved.
 *
 */
LPCAnalysis * lpc;
LPCAnalysis * lpc2; //two of them, for corssfading when changing filter

//also can do test to return convergence point; clue to transient vs tonal?
//toggle to freeze or not on current filter coefficients?

void LPCAnalyzer_Ctor(LPCAnalyzer* unit) {

	int windowsize= (int)ZIN0(2);
	int windowtype= (int)ZIN0(6);

	int blocksize= unit->mWorld->mFullRate.mBufLength;

	if(windowsize<blocksize) windowsize=blocksize;
	//must be divisible by two?
	if((windowsize & 0x01))
		windowsize= windowsize+1;
	if(windowsize>1024) windowsize=1024;


	//overloaded new operator so realtime safe
	//unit->mWorld->mFullRate.mBufLength, no need to pass blocksize now
	unit->lpc= (LPCAnalysis*) new(unit->mWorld, ft) LPCAnalysis(windowsize,windowtype,0,unit->mWorld, ft);

	if(windowtype>0)
		unit->lpc2= (LPCAnalysis*) new(unit->mWorld, ft) LPCAnalysis(windowsize,windowtype,windowsize/2,unit->mWorld, ft);
	else
		unit->lpc2=NULL;

	//put them out of sync by half window for crossfading purposes
	//unit->lpc2->pos= windowsize/2;

	SETCALC(LPCAnalyzer_next);

}

void LPCAnalyzer_next(LPCAnalyzer *unit, int inNumSamples) {

	float * inoriginal= IN(0);
	float * indriver= IN(1);
	int p= (int)ZIN0(3);
	float * out= OUT(0);

	int testE= (int)ZIN0(4);
	LPCfloat delta= (LPCfloat)ZIN0(5);

	for (int i=0; i<inNumSamples; ++i) {
		out[i]= 0.0;
	}

	unit->lpc->testdelta= testE;
	unit->lpc->delta= delta;
	unit->lpc->update(inoriginal, indriver, out, inNumSamples, p);

	if(unit->lpc2) {
		unit->lpc2->testdelta= testE;
		unit->lpc2->delta= delta;
		unit->lpc2->update(inoriginal, indriver, out, inNumSamples, p);
	}

}


//blocksize MUST be less than or equal to windowsize

void update(float * newinput, float * newsource, float * output, int numSamples, int p) {

	int i;

	int left= windowsize-pos;

	if(numSamples>=left) {

		for (i=0; i<left;++i) {

			input[pos++]= newinput[i];
		}

		//output up to here
		calculateOutput(newsource, output, windowsize-left, left);

		//update
		numpoles=p;
		calculatePoles();

		pos=0;

		int remainder= numSamples-left;

		for (i=0; i<remainder;++i) {

			input[pos++]= newinput[left+i];

		}

		//output too
		calculateOutput(newsource+left, output+left, pos-remainder, remainder);


	} else {

		for (i=0; i<numSamples;++i) {

			input[pos++]= newinput[i];
		}

		//output
		calculateOutput(newsource, output, pos-numSamples, numSamples);


	}

	//return output;
}


void calculateOutput(float * source, float * target, int startpos, int num) {
	int i,j;

	int basepos,posnow;

	for(i=0; i<num; ++i) {

		basepos= startpos+i+windowsize-1; //-1 since coefficients for previous values starts here

		float sum=0.0;

		for(j=0; j<numpoles; ++j) {
			posnow= (basepos-j)%windowsize;

			//where is pos used?
			sum += last[posnow]*coeff[j]; //was coeff i
		}

		sum= G*source[i]-sum; //scale factor G calculated by squaring energy E below

		last[startpos+i]=sum;

		//ZXP(out)=
		target[i]+= sum*windowfunction[startpos+i];

	}

}

//recalculate poles based on recent window of input
void calculatePoles() {

	//can test for convergence by looking for 1-((Ei+1)/Ei)<d

	int i, j;
	LPCfloat sum;

	//safety
	if(numpoles<1) numpoles=1;
	if(numpoles>windowsize) numpoles=windowsize;

	//printf("p? %d",p);

	//calculate new LPC filter coefficients following (Makhoul 1975) autocorrelation, deterministic signal, Durbin iterative matrix solver

	//float R[21];//autocorrelation coeffs;
	//float preva[21];
	//float a[21];
	LPCfloat E, k;

	//faster calculation of autocorrelation possible?

	for(i=0; i<=numpoles; ++i) {
		sum=0.0;

		for (j=0; j<= windowsize-1-i; ++j)
			sum+= input[j]*input[j+i];

		R[i]=sum;
	}

	E= R[0];
	k=0;

	if(E<0.00000000001) {

		//zero power, so zero all coeff
		for (i=0; i<numpoles;++i)
			coeff[i]=0.0;
//
		latesterror= E;
		G=0.0;
		//printf("zero power %f\n", E);
		return;

	};

	//rescaling may help with numerical instability issues?
//	float mult= 1.0/E;
//	for(i=1; i<=numpoles; ++i)
//		R[i]= R[i]*mult;
//
	for(i=0; i<=(numpoles+1); ++i) {
		a[i]=0.0;
		preva[i]=0.0; //CORRECTION preva[j]=0.0;
	}


//	for(i=0; i<numpoles; ++i) {
//		printf("sanity check a %f R %1.15f ",a[i+1], R[i]);
//	}
//	printf("\n");


	LPCfloat prevE= E;

	for(i=1; i<=numpoles; ++i) {

		sum=0.0;

		for(j=1;j<i;++j)
			sum+= a[j]*R[i-j];

		k=(-1.0*(R[i]+sum))/E;

		a[i]=k;

		for(j=1;j<=(i-1);++j)
			a[j]=preva[j]+(k*preva[i-j]);

		for(j=1;j<=i;++j)
			preva[j]=a[j];

		E= (1-k*k)*E;

		//printf("E check %f %d k was %f\n", E,i,k);

		//check for instability; all E must be greater than zero
		if(E<0.00000000001) {

			//zero power, so zero all coeff

			//leave coeff as previous values
			//for (j=0; j<numpoles;++j)
//				coeff[j]=0.0;

			latesterror= E;
			//printf("early return %1.15f %d\n", E,i);
			return;

		};

		if(testdelta) {

			LPCfloat ratio= E/prevE;
			if(ratio>delta) {
				//printf("variable order chose %d\n", i);
				break; //done to error bound
			}
			prevE= E;

		}


	}

	G= sqrt(E);

	latesterror= E;

	//solution is the final set of a
	for(i=0; i<numpoles; ++i) {
		//coeff[numpoles-1-i]=a[i+1];
		coeff[i]=a[i+1];
		//printf("a %f R %f",a[i+1], R[i]);
	}

	//MUST CHECK gain?

}
