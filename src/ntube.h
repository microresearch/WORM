
typedef struct 
{
	unsigned char numtubes;
	float delayright[4][512], delayleft[4][512]; //tubes -> was malloced
	int position; //can be same for all lines!
	int maxlength, modulo;
  //	float delayconversion;
	float f1in, f1out;	//averaging filters f1, f2 for frequency dependent losses; need a storage slot for previous values
	float f2in, f2out;

	//convenience variables for copying particular input data
  //	float * losses;
  //	float * scattering;
  //	float * delays;

	float rightouts[4];
	float leftouts[4];

}NTube;

void NTube_init(NTube* unit);
void NTube_do(NTube *unit, float *in, float *out, int inNumSamples);
