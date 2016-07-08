/******************************************************************************
*
*     Program:       tube
*     
*     Description:   Software (non-real-time) implementation of the Tube
*                    Resonance Model for speech production.
*
*     Author:        Leonard Manzara
*
*     Date:          July 5th, 1994
*
******************************************************************************/

#include "audio.h"

#define FALSE 0
#define TRUE 1

/*  COMPILE WITH OVERSAMPLING OR PLAIN OSCILLATOR  */
#define OVERSAMPLING_OSCILLATOR   1

/*  COMPILE SO THAT INTERPOLATION NOT DONE FOR SOME CONTROL RATE PARAMETERS  */
#define MATCH_DSP                 1


/*  OROPHARYNX REGIONS  */
#define R1                        0      /*  S1  */
#define R2                        1      /*  S2  */
#define R3                        2      /*  S3  */
#define R4                        3      /*  S4 & S5  */
#define R5                        4      /*  S6 & S7  */
#define R6                        5      /*  S8  */
#define R7                        6      /*  S9  */
#define R8                        7      /*  S10  */
#define TOTAL_REGIONS             8

/*  OROPHARYNX SCATTERING JUNCTION COEFFICIENTS (BETWEEN EACH REGION)  */
#define C1                        R1     /*  R1-R2 (S1-S2)  */
#define C2                        R2     /*  R2-R3 (S2-S3)  */
#define C3                        R3     /*  R3-R4 (S3-S4)  */
#define C4                        R4     /*  R4-R5 (S5-S6)  */
#define C5                        R5     /*  R5-R6 (S7-S8)  */
#define C6                        R6     /*  R6-R7 (S8-S9)  */
#define C7                        R7     /*  R7-R8 (S9-S10)  */
#define C8                        R8     /*  R8-AIR (S10-AIR)  */
#define TOTAL_COEFFICIENTS        TOTAL_REGIONS

/*  OROPHARYNX SECTIONS  */
#define S1                        0      /*  R1  */
#define S2                        1      /*  R2  */
#define S3                        2      /*  R3  */
#define S4                        3      /*  R4  */
#define S5                        4      /*  R4  */
#define S6                        5      /*  R5  */
#define S7                        6      /*  R5  */
#define S8                        7      /*  R6  */
#define S9                        8      /*  R7  */
#define S10                       9      /*  R8  */
#define TOTAL_SECTIONS            10

/*  NASAL TRACT SECTIONS  */
#define N1                        0
#define VELUM                     N1
#define N2                        1
#define N3                        2
#define N4                        3
#define N5                        4
#define N6                        5
#define TOTAL_NASAL_SECTIONS      6

/*  NASAL TRACT COEFFICIENTS  */
#define NC1                       N1     /*  N1-N2  */
#define NC2                       N2     /*  N2-N3  */
#define NC3                       N3     /*  N3-N4  */
#define NC4                       N4     /*  N4-N5  */
#define NC5                       N5     /*  N5-N6  */
#define NC6                       N6     /*  N6-AIR  */
#define TOTAL_NASAL_COEFFICIENTS  TOTAL_NASAL_SECTIONS

/*  THREE-WAY JUNCTION ALPHA COEFFICIENTS  */
#define LEFT                      0
#define RIGHT                     1
#define UPPER                     2
#define TOTAL_ALPHA_COEFFICIENTS  3

/*  FRICATION INJECTION COEFFICIENTS  */
#define FC1                       0      /*  S3  */
#define FC2                       1      /*  S4  */
#define FC3                       2      /*  S5  */
#define FC4                       3      /*  S6  */
#define FC5                       4      /*  S7  */
#define FC6                       5      /*  S8  */
#define FC7                       6      /*  S9  */
#define FC8                       7      /*  S10  */
#define TOTAL_FRIC_COEFFICIENTS   8


/*  GLOTTAL SOURCE OSCILLATOR TABLE VARIABLES  */
#define TABLE_LENGTH              512
#define TABLE_MODULUS             (TABLE_LENGTH-1)

/*  WAVEFORM TYPES  */
#define PULSE                     0
#define SINE                      1

/*  OVERSAMPLING FIR FILTER CHARACTERISTICS  */
#define FIR_BETA                  .2
#define FIR_GAMMA                 .1
#define FIR_CUTOFF                .00000001

/*  PITCH VARIABLES  */
#define PITCH_BASE                220.0
#define PITCH_OFFSET              3           /*  MIDDLE C = 0  */
#define LOG_FACTOR                3.32193

/*  RANGE OF ALL VOLUME CONTROLS  */
#define VOL_MAX                   60

/*  SCALING CONSTANT FOR INPUT TO VOCAL TRACT & THROAT (MATCHES DSP)  */
//#define VT_SCALE                  0.03125     /*  2^(-5)  */
// this is a temporary fix only, to try to match dsp synthesizer
#define VT_SCALE                  0.125     /*  2^(-3)  */

/*  FINAL OUTPUT SCALING, SO THAT .SND FILES APPROX. MATCH DSP OUTPUT  */
#define OUTPUT_SCALE              0.25

/*  CONSTANTS FOR THE FIR FILTER  */
#define LIMIT                     200
#define BETA_OUT_OF_RANGE         1
#define GAMMA_OUT_OF_RANGE        2
#define GAMMA_TOO_SMALL           3

/*  CONSTANTS FOR NOISE GENERATOR  */
#define FACTOR                    377.0
#define INITIAL_SEED              0.7892347

/*  MAXIMUM SAMPLE VALUE  */
#define RANGE_MAX                 32767.0

/*  MATH CONSTANTS  */
#define TUBEPI                        3.1415927
#define TWO_PI                    (2.0 * TUBEPI)

/*  FUNCTION RETURN CONSTANTS  */
#define ERROR                     (-1)
#define SUCCESS                   0

/*  BI-DIRECTIONAL TRANSMISSION LINE POINTERS  */
#define TOP                       0
#define BOTTOM                    1


/*  SAMPLE RATE CONVERSION CONSTANTS  */
#define ZERO_CROSSINGS            13                 /*  SRC CUTOFF FRQ      */
#define LP_CUTOFF                 (11.0/13.0)        /*  (0.846 OF NYQUIST)  */

#define N_BITS                    16
#define L_BITS                    8
#define L_RANGE                   256                  /*  must be 2^L_BITS  */
#define M_BITS                    8
#define M_RANGE                   256                  /*  must be 2^M_BITS  */
#define FRACTION_BITS             (L_BITS + M_BITS)
#define FRACTION_RANGE            65536         /*  must be 2^FRACTION_BITS  */
#define FILTER_LENGTH             (ZERO_CROSSINGS * L_RANGE)
#define FILTER_LIMIT              (FILTER_LENGTH - 1)

#define N_MASK                    0xFFFF0000
#define L_MASK                    0x0000FF00
#define M_MASK                    0x000000FF
#define FRACTION_MASK             0x0000FFFF

#define nValue(x)                 (((x) & N_MASK) >> FRACTION_BITS)
#define lValue(x)                 (((x) & L_MASK) >> M_BITS)
#define mValue(x)                 ((x) & M_MASK)
#define fractionValue(x)          ((x) & FRACTION_MASK)

#define BETA                      5.658        /*  kaiser window parameters  */
#define IzeroEPSILON              1E-21 // was 21

#define OUTPUT_SRATE_LOW          22050.0
#define OUTPUT_SRATE_HIGH         44100.0
//#define BUFFER_SIZE               1024                 /*  ring buffer size  */

//extern signed int audio_buffer[32768];
//signed int audio_buffer[32767];
//static unsigned int laststart;

/*  DATA TYPES  **************************************************************/


/*  GLOBAL VARIABLES *********************************************************/

float globglotpitch=0.0; // female

/*  INPUT VARIABLES  */
float  outputRate;                  /*  output sample rate (22.05, 44.1)  */
float  controlRate;                 /*  1.0-1000.0 input tables/second (Hz)  */

float volume;                      /*  master volume (0 - 60 dB)  */
int waveform;                    /*  GS waveform type (0=PULSE, 1=SINE  */
float tp;                          /*  % glottal pulse rise time  */
float tnMin;                       /*  % glottal pulse fall time minimum  */
float tnMax;                       /*  % glottal pulse fall time maximum  */
float breathiness;                 /*  % glottal source breathiness  */

float length;                      /*  nominal tube length (10 - 20 cm)  */
float temperature;                 /*  tube temperature (25 - 40 C)  */
float lossFactor;                  /*  junction loss factor in (0 - 5 %)  */

float apScale;                     /*  aperture scl. radius (3.05 - 12 cm)  */
float mouthCoef;                   /*  mouth aperture coefficient  */
float noseCoef;                    /*  nose aperture coefficient  */

float noseRadius[TOTAL_NASAL_SECTIONS];  /*  fixed nose radii (0 - 3 cm)  */

float throatCutoff;                /*  throat lp cutoff (50 - nyquist Hz)  */
float throatVol;                   /*  throat volume (0 - 48 dB) */

int    modulation;                  /*  pulse mod. of noise (0=OFF, 1=ON)  */
float mixOffset;                   /*  noise crossmix offset (30 - 60 dB)  */


/*  DERIVED VALUES  */
int    controlPeriod;
int    sampleRate;
float actualTubeLength;            /*  actual length in cm  */

float dampingFactor;               /*  calculated damping factor  */
float crossmixFactor;              /*  calculated crossmix factor  */

//float wavetable[TABLE_LENGTH];
float* wavetable;

//float *wavetable;
int    tableDiv1;
int    tableDiv2;
float tnLength;
float tnDelta;
float breathinessFactor;

float basicIncrement;
float currentPosition;

/*  REFLECTION AND RADIATION FILTER MEMORY  */
float a10, b11, a20, a21, b21;

/*  NASAL REFLECTION AND RADIATION FILTER MEMORY  */
float na10, nb11, na20, na21, nb21;

/*  THROAT LOWPASS FILTER MEMORY, GAIN  */
float tb1, ta0, throatGain;

/*  FRICATION BANDPASS FILTER MEMORY  */
float bpAlpha, bpBeta, bpGamma;

/*  TEMPORARY SAMPLE STORAGE VALUES  */
float maximumSampleValue = 0.0;
long int numberSamples = 0;
//FILE  *tempFilePtr;

/*  MEMORY FOR TUBE AND TUBE COEFFICIENTS  */
float oropharynx[TOTAL_SECTIONS][2][2];
float oropharynx_coeff[TOTAL_COEFFICIENTS];

float nasal[TOTAL_NASAL_SECTIONS][2][2];
float nasal_coeff[TOTAL_NASAL_COEFFICIENTS];

float alpha[TOTAL_ALPHA_COEFFICIENTS];
int current_ptr = 1;
int prev_ptr = 0;

/*  MEMORY FOR FRICATION TAPS  */
float fricationTap[TOTAL_FRIC_COEFFICIENTS];

/*  VARIABLES FOR INTERPOLATION  */
struct currentt{
    float glotPitch;
    float glotPitchDelta;
    float glotVol;
    float glotVolDelta;
    float aspVol;
    float aspVolDelta;
    float fricVol;
    float fricVolDelta;
    float fricPos;
    float fricPosDelta;
    float fricCF;
    float fricCFDelta;
    float fricBW;
    float fricBWDelta;
    float radius[TOTAL_REGIONS];
    float radiusDelta[TOTAL_REGIONS];
    float velum;
    float velumDelta;
};

struct currentt current;

/*  VARIABLES FOR FIR LOWPASS FILTER  */
//float FIRData[128], FIRCoef[128];
float *FIRData, *FIRCoef;

int FIRPtr, numberTaps;

/*  VARIABLES FOR SAMPLE RATE CONVERSION  */
float sampleRateRatio;
//float h[FILTER_LENGTH], deltaH[FILTER_LENGTH], buffer[BUFFER_SIZE];
int fillPtr, emptyPtr = 0, padSize, fillSize;
unsigned int timeRegisterIncrement, filterIncrement, phaseIncrement;
unsigned int timeRegister = 0;

/*  GLOBAL FUNCTIONS (LOCAL TO THIS FILE)  ***********************************/

//int initializeSynthesizer(void);

void initializeWavetable(void);
float speedOfSound(float temperature);
void updateWavetable(float amplitude);
void initializeFIR(float beta, float gamma, float cutoff);
float noise(void);
float noiseFilter(float input);
void initializeMouthCoefficients(float coeff);
float reflectionFilter(float input);
float radiationFilter(float input);
void initializeNasalFilterCoefficients(float coeff);
float nasalReflectionFilter(float input);
float nasalRadiationFilter(float input);

void setControlRateParameters();
void sampleRateInterpolation(void);
void initializeNasalCavity(void);
void initializeThroat(void);
void calculateTubeCoefficients(void);
void setFricationTaps(void);
void calculateBandpassCoefficients(void);
float mod0(float value);
void incrementTablePosition(float frequency);
float oscillator(float frequency);
float vocalTract(float input, float frication);
float throaty(float input);
float bandpassFilter(float input);
float amplitude(float decibelLevel);
float frequency(float pitch);
int maximallyFlat(float beta, float gamma, int *np, float *coefficient);
void trim(float cutoff, int *numberCoefficients, float *coefficient);
void rationalApproximation(float number, int *order, int *numerator,
			   int *denominator);
float FIRFilter(float input, int needOutput);
int increment(int pointer, int modulus);
int decrement(int pointer, int modulus);
//void initializeConversion(void);
//void initializeFilter(void);
float Izero(float x);
//void initializeBuffer(void);
//void dataFill(float data);
//void dataEmpty(void);

/////////////////////////////////////////////=???????

// example input to convert to arrays in first INIT
// and transcribe that list of vowels/frames

float parameter_list[21]={60.0, 0, 30.0, 16.0, 32.0, 2.50, 18.0, 32, 1.50, 3.05, 5000.0, 5000.0, 1.35, 1.96, 1.91, 1.3, 0.73, 1500.0, 6.0, 1, 48.0}; // all floats???

float input_frame[16]={-12.5, 54.0, 0.0, 0.0, 4.0, 4400, 600, 0.8, 0.8, 0.4, 0.4, 1.78, 1.78, 1.26, 0.8, 0.1};

//float input_frame[16]=  {-1.000000, 0.000000, 0.000000, 24.000000, 7.000000, 864.000000, 3587.000000, 0.800000, 0.890000, 0.990000, 0.810000, 0.600000, 0.520000, 0.710000, 0.240000, 0.100000};

/// with microint 64 x 16:



void init_parameters(void){

float glotPitch, glotVol, radius[TOTAL_REGIONS], velum, aspVol;
float fricVol, fricPos, fricCF, fricBW;
unsigned char i;

// laststart=0;
 outputRate = 32000;
 controlRate = 4.0;

// what happened to volume?
volume = parameter_list[0];

// glottal source waveform
waveform = parameter_list[1];

//      GET THE GLOTTAL PULSE RISE TIME (tp)  
tp = parameter_list[2];
  
//      GET THE GLOTTAL PULSE FALL TIME MINIMUM (tnMin)  
tnMin = parameter_list[3];
  
//      GET THE GLOTTAL PULSE FALL TIME MAXIMUM (tnMax)  
tnMax = parameter_list[4];

//      GET THE GLOTTAL SOURCE BREATHINESS  
breathiness = parameter_list[5];

//      GET THE NOMINAL TUBE LENGTH  
length = parameter_list[6];

//      GET THE TUBE TEMPERATURE  
temperature = parameter_list[7];

//      GET THE JUNCTION LOSS FACTOR  
lossFactor = parameter_list[8];

//      GET THE APERTURE SCALING RADIUS  
apScale = parameter_list[9];

//    GET THE MOUTH APERTURE COEFFICIENT  
mouthCoef = parameter_list[10];

//      GET THE NOSE APERTURE COEFFICIENT  
noseCoef = parameter_list[11];

//      GET THE NOSE RADII for how many? 6 
noseRadius[1] = parameter_list[12]; // starts at 1 tho
noseRadius[2] = parameter_list[13];
noseRadius[3] = parameter_list[14];
noseRadius[4] = parameter_list[15];
noseRadius[5] = parameter_list[16];

//      GET THE THROAT LOWPASS FREQUENCY CUTOFF  
throatCutoff = parameter_list[17];

//      GET THE THROAT VOLUME  
throatVol = parameter_list[18];

//      GET THE PULSE MODULATION OF NOISE FLAG  
modulation = parameter_list[19];

//      GET THE NOISE CROSSMIX OFFSET  
mixOffset = parameter_list[20];

// then read in frame - so we will have 2 inputtables!

/*
glotPitch = input_frame[0]+globglotpitch;
glotVol = input_frame[1];
aspVol = input_frame[2];
fricVol = input_frame[3];
fricPos = input_frame[4];
fricCF = input_frame[5];
fricBW = input_frame[6];


for (i = 0; i < TOTAL_REGIONS; i++) radius[i] = input_frame[7+i];

velum = input_frame[7+i];

*/

//ADD THE PARAMETERS TO THE INPUT LIST  
//addInput(glotPitch, glotVol, aspVol, fricVol, fricPos, fricCF,fricBW, radius, velum);

// printff("NUMMMMMM %d",numberInputTables);


//FLOAT UP THE LAST INPUT TABLE, TO HELP INTERPOLATION CALCULATIONS  -- adds one
/*if (numberInputTables > 0) {
int lastTable = numberInputTables - 1;
addInput(glotPitchAt(lastTable), glotVolAt(lastTable),
aspVolAt(lastTable), fricVolAt(lastTable),
fricPosAt(lastTable), fricCFAt(lastTable),
fricBWAt(lastTable), radiiAt(lastTable),
velumAt(lastTable));
}*/

// do as last:


//    printff("NUMMMMMM %d",numberInputTables);


}

/*
// parameters from input

//32000.0   ; output sample rate (22050.0, 44100.0)??? FIXED
//4.0       ; input control rate (1 - 1000 Hz) FIXED we skip these
60.0	  ; master volume (0 - 60 dB)
0         ; glottal source waveform type (0 = pulse, 1 = sine)
40.0      ; glottal pulse rise time (5 - 50 % of GP period)
16.0      ; glottal pulse fall time minimum (5 - 50 % of GP period)
32.0      ; glottal pulse fall time maximum (5 - 50 % of GP period)
2.50      ; glottal source breathiness (0 - 10 % of GS amplitude)
18.0      ; nominal tube length (10 - 20 cm)
32        ; tube temperature (25 - 40 degrees celsius)
1.50      ; junction loss factor (0 - 5 % of unity gain)
3.05      ; aperture scaling radius (3.05 - 12 cm)
5000.0	  ; mouth aperture coefficient (100 - nyqyist Hz)
5000.0	  ; nose aperture coefficient (100 - nyquist Hz)
1.35	  ; radius of nose section 1 (0 - 3 cm)
1.96	  ; radius of nose section 2 (0 - 3 cm)
1.91	  ; radius of nose section 3 (0 - 3 cm)
1.3	  ; radius of nose section 4 (0 - 3 cm)
0.73	  ; radius of nose section 5 (0 - 3 cm)
1500.0    ; throat lowpass frequency cutoff (50 - nyquist Hz)
6.0       ; throat volume (0 - 48 dB)
1	  ; pulse modulation of noise (0 = off, 1 = on)
48.0      ; noise crossmix offset (30 - 60 db)

// this is: 

	  GET EACH PARAMETER  
	glotPitch = strtod(ptr, &ptr);
	glotVol = strtod(ptr, &ptr);
	aspVol = strtod(ptr, &ptr);
	fricVol = strtod(ptr, &ptr);
	fricPos = strtod(ptr, &ptr);
	fricCF = strtod(ptr, &ptr);
	fricBW = strtod(ptr, &ptr);
	for (i = 0; i < TOTAL_REGIONS; i++) // 8 values
	    radius[i] = strtod(ptr, &ptr);
	velum = strtod(ptr, &ptr); // last value

-12.0	0.0	0.0	0.0	4.0	4400	600	0.8	0.8	0.4	0.4	1.78	1.78	1.26	0.8	0.0
-12.5	54.0	0.0	0.0	4.0	4400	600	0.8	0.8	0.4	0.4	1.78	1.78	1.26	0.8	0.0
-13.0	60.0	0.0	0.0	4.0	4400	600	0.8	0.8	0.6	0.6	1.58	1.58	1.13	1.01	0.1
-13.5	60.0	0.0	0.0	4.0	4450	550	0.8	0.8	1.28	1.28	1.0	1.0	1.0	0.8	1.0
-14.0	54.0	0.0	0.0	4.0	4500	500	0.8	0.8	1.68	1.58	0.8	0.8	0.5	0.4	1.0
-14.5	51.0	0.0	0.0	4.0	4500	500	0.8	0.8	1.78	1.78	0.2	0.2	0.4	0.0	1.0
*/



float speedOfSound(float temperature)
{
    return (331.4 + (0.6 * temperature));
}

void tube_init(void)
{
    float nyquist;

    init_parameters(); // reads our parameter array

    /*  CALCULATE THE SAMPLE RATE, BASED ON NOMINAL
	TUBE LENGTH AND SPEED OF SOUND  */
	float c = speedOfSound(temperature);
	controlPeriod =
	    rintf((c * TOTAL_SECTIONS * 100.0) /(length * controlRate));
	sampleRate = controlRate * controlPeriod;
	actualTubeLength = (c * TOTAL_SECTIONS * 100.0) / sampleRate;
	nyquist = (float)sampleRate / 2.0;

    /*  CALCULATE THE BREATHINESS FACTOR  */
    breathinessFactor = breathiness / 100.0;

    /*  CALCULATE CROSSMIX FACTOR  */
    crossmixFactor = 1.0 / amplitude(mixOffset);

    /*  CALCULATE THE DAMPING FACTOR  */
    dampingFactor = (1.0 - (lossFactor / 100.0));

    /*  INITIALIZE THE WAVE TABLE  */
        initializeWavetable();

    /*  INITIALIZE THE FIR FILTER  */
        initializeFIR(FIR_BETA, FIR_GAMMA, FIR_CUTOFF);

    /*  INITIALIZE REFLECTION AND RADIATION FILTER COEFFICIENTS FOR MOUTH  */
        initializeMouthCoefficients((nyquist - mouthCoef) / nyquist);

    /*  INITIALIZE REFLECTION AND RADIATION FILTER COEFFICIENTS FOR NOSE  */
        initializeNasalFilterCoefficients((nyquist - noseCoef) / nyquist);

    /*  INITIALIZE NASAL CAVITY FIXED SCATTERING COEFFICIENTS  */ 
        initializeNasalCavity();

    /*  INITIALIZE THE THROAT LOWPASS FILTER  */
        initializeThroat();

    /*  INITIALIZE THE SAMPLE RATE CONVERSION ROUTINES  */
    //    initializeConversion(); // not now

    //    return 1;
}

void initializeWavetable(void)
{
    int i, j;


    /*  ALLOCATE MEMORY FOR WAVETABLE  */
    wavetable = (float *)calloc(TABLE_LENGTH, sizeof(float)); // TODO: as fixed array = 512 floats


    /*  CALCULATE WAVE TABLE PARAMETERS  */
    tableDiv1 = rintf(TABLE_LENGTH * (tp / 100.0));
    tableDiv2 = rintf(TABLE_LENGTH * ((tp + tnMax) / 100.0));
    tnLength = tableDiv2 - tableDiv1;
    tnDelta = rintf(TABLE_LENGTH * ((tnMax - tnMin) / 100.0));
    basicIncrement = (float)TABLE_LENGTH / (float)sampleRate;
    currentPosition = 0;

    /*  INITIALIZE THE WAVETABLE WITH EITHER A GLOTTAL PULSE OR SINE TONE  */
    if (waveform == PULSE) {
	/*  CALCULATE RISE PORTION OF WAVE TABLE  */
	for (i = 0; i < tableDiv1; i++) {
	    float x = (float)i / (float)tableDiv1;
	    float x2 = x * x;
	    float x3 = x2 * x;
	    wavetable[i] = (3.0 * x2) - (2.0 * x3);
	}

	/*  CALCULATE FALL PORTION OF WAVE TABLE  */
	for (i = tableDiv1, j = 0; i < tableDiv2; i++, j++) {
	    float x = (float)j / tnLength;
	    wavetable[i] = 1.0 - (x * x);
	}

	/*  SET CLOSED PORTION OF WAVE TABLE  */
	for (i = tableDiv2; i < TABLE_LENGTH; i++)
	    wavetable[i] = 0.0;
    }
    else {
	/*  SINE WAVE  */
	for (i = 0; i < TABLE_LENGTH; i++) {
	    wavetable[i] = sinf( ((float)i/(float)TABLE_LENGTH) * 2.0 * TUBEPI );
	}
    }	
}

void updateWavetable(float amplitude)
{
    int i, j;


    /*  CALCULATE NEW CLOSURE POINT, BASED ON AMPLITUDE  */
    float newDiv2 = tableDiv2 - rintf(amplitude * tnDelta);
    float newTnLength = newDiv2 - tableDiv1;

    /*  RECALCULATE THE FALLING PORTION OF THE GLOTTAL PULSE  */
    for (i = tableDiv1, j = 0; i < newDiv2; i++, j++) {
	float x = (float)j / newTnLength;
	wavetable[i] = 1.0 - (x * x);
    }

    /*  FILL IN WITH CLOSED PORTION OF GLOTTAL PULSE  */
    for (i = newDiv2; i < tableDiv2; i++)
	wavetable[i] = 0.0;
}


void initializeFIR(float beta, float gamma, float cutoff)
{
    int i, pointer, increment, numberCoefficients;
    float coefficient[LIMIT+1];
  

    /*  DETERMINE IDEAL LOW PASS FILTER COEFFICIENTS  */
    maximallyFlat(beta, gamma, &numberCoefficients, coefficient); 

    /*  TRIM LOW-VALUE COEFFICIENTS  */
    trim(cutoff, &numberCoefficients, coefficient);

    /*  DETERMINE THE NUMBER OF TAPS IN THE FILTER  */
    numberTaps = (numberCoefficients * 2) - 1;

    /*  ALLOCATE MEMORY FOR DATA AND COEFFICIENTS  */
        FIRData = (float *)calloc(numberTaps, sizeof(float)); // NOT FIXED as dependent on BETA!
        FIRCoef = (float *)calloc(numberTaps, sizeof(float));

    /*  INITIALIZE THE COEFFICIENTS  */
    increment = (-1);
    pointer = numberCoefficients;
    for (i = 0; i < numberTaps; i++) {
	FIRCoef[i] = coefficient[pointer];
	pointer += increment;
	if (pointer <= 0) {
	    pointer = 2;
	    increment = 1;
	}
    }

    /*  SET POINTER TO FIRST ELEMENT  */
    FIRPtr = 0;
}

float noise(void)
{
    static float seed = INITIAL_SEED;

    float product = seed * FACTOR;
    seed = product - (int)product;
    return (seed - 0.5);
}

float noiseFilter(float input)
{
    static float noiseX = 0.0;

    float output = input + noiseX;
    noiseX = input;
    return (output);
}

void initializeMouthCoefficients(float coeff)
{
    b11 = -coeff;
    a10 = 1.0 - fabsf(b11);

    a20 = coeff;
    a21 = b21 = -a20;
}


float reflectionFilter(float input)
{
    static float reflectionY = 0.0;

    float output = (a10 * input) - (b11 * reflectionY);
    reflectionY = output;
    return (output);
}


float radiationFilter(float input)
{
    static float radiationX = 0.0, radiationY = 0.0;

    float output = (a20 * input) + (a21 * radiationX) - (b21 * radiationY);
    radiationX = input;
    radiationY = output;
    return (output);
}


void initializeNasalFilterCoefficients(float coeff)
{
    nb11 = -coeff;
    na10 = 1.0 - fabsf(nb11);

    na20 = coeff;
    na21 = nb21 = -na20;
}


float nasalReflectionFilter(float input)
{
    static float nasalReflectionY = 0.0;

    float output = (na10 * input) - (nb11 * nasalReflectionY);
    nasalReflectionY = output;
    return (output);
}

float nasalRadiationFilter(float input)
{
    static float nasalRadiationX = 0.0, nasalRadiationY = 0.0;

    float output = (na20 * input) + (na21 * nasalRadiationX) -
	(nb21 * nasalRadiationY);
    nasalRadiationX = input;
    nasalRadiationY = output;
    return (output);
}

float glotPitchAt()
{
  return input_frame[0]+globglotpitch;
}

float glotVolAt()
{
  return input_frame[1];
}

float *radiiAt()
{
  return &input_frame[7];
}

float radiusAtRegion(u8 region)
{
  return input_frame[7+region];
}

float velumAt()
{
return input_frame[7+TOTAL_REGIONS];
}

float aspVolAt()
{
  return input_frame[2];
}

float fricVolAt()
{
  return input_frame[3];
}

float fricPosAt()
{
  return input_frame[4];
}

float fricCFAt()
{
  return input_frame[5];
}

float fricBWAt()
{
  return input_frame[6];
}

void tube_newsay(void){
  // what we need to reset

  setControlRateParameters(); // TODO - just replace with straight array in

}

int16_t tube_get_sample(void)
{
  static int16_t j=0;
  int16_t sample;
  float f0, ax, ah1, pulse, lp_noise, pulsed_noise, signal, crossmix, absoluteSampleValue,scale=100.0f;
  
    // do we have a new frame? -> newsay 
    if (j>=controlPeriod) {
      tube_newsay();
      j=0;
    }
    f0 = frequency(current.glotPitch); // without interpol these are all the same?
    ax = amplitude(current.glotVol);
    ah1 = amplitude(current.aspVol);
	    calculateTubeCoefficients();
	    setFricationTaps();
	    calculateBandpassCoefficients();

	    lp_noise = noiseFilter(noise());
	    if (waveform == PULSE)		updateWavetable(ax);
	    pulse = oscillator(f0);
	    pulsed_noise = lp_noise * pulse;
	    pulse = ax * ((pulse * (1.0 - breathinessFactor)) +
			  (pulsed_noise * breathinessFactor));

	    if (modulation) {
		crossmix = ax * crossmixFactor;
		crossmix = (crossmix < 1.0) ? crossmix : 1.0;
		signal = (pulsed_noise * crossmix) +
		    (lp_noise * (1.0 - crossmix));
	    }
	    else
		signal = lp_noise;

	    signal = vocalTract(((pulse + (ah1 * signal)) * VT_SCALE), bandpassFilter(signal));

	    signal += throaty(pulse * VT_SCALE);
	    
	    //	    dataFill(signal); // this is where we get samples - no interpolation of samples OR control - just to test
	    absoluteSampleValue = fabsf(signal);
	    if (absoluteSampleValue > maximumSampleValue)
		maximumSampleValue = absoluteSampleValue;
	    
	    //	    sampleRateInterpolation(); // TODO!!! using last input_frame params - lastframeat
	    if (maximumSampleValue > 0)
	      scale = (RANGE_MAX / maximumSampleValue);
	    signal=signal*scale;
	    sample=rintf(signal); 	 
	    j++;
	    //	     sample=rand()%32768;
	    return sample;
}

void setControlRateParameters()
{
    u8 i;
    current.glotPitch = glotPitchAt();

    //    current.glotPitchDelta =
    //	(glotPitchAt(pos) - current.glotPitch) / (float)controlPeriod;

    current.glotVol = glotVolAt();
    //    current.glotVolDelta =
    //	(glotVolAt(pos) - current.glotVol) / (float)controlPeriod;

    current.aspVol = aspVolAt();
#if MATCH_DSP
    current.aspVolDelta = 0.0;
#else
    current.aspVolDelta =
	(aspVolAt(pos) - current.aspVol) / (float)controlPeriod;
#endif

    current.fricVol = fricVolAt();
#if MATCH_DSP
    current.fricVolDelta = 0.0;
#else
    current.fricVolDelta =
	(fricVolAt(pos) - current.fricVol) / (float)controlPeriod;
#endif

    current.fricPos = fricPosAt();
#if MATCH_DSP
    current.fricPosDelta = 0.0;
#else
    current.fricPosDelta =
	(fricPosAt(pos) - current.fricPos) / (float)controlPeriod;
#endif

    current.fricCF = fricCFAt();
#if MATCH_DSP
    current.fricCFDelta = 0.0;
#else
    current.fricCFDelta =
	(fricCFAt(pos) - current.fricCF) / (float)controlPeriod;
#endif

    current.fricBW = fricBWAt();
#if MATCH_DSP
    current.fricBWDelta = 0.0;
#else
    current.fricBWDelta =
	(fricBWAt(pos) - current.fricBW) / (float)controlPeriod;
#endif

    for (i = 0; i < TOTAL_REGIONS; i++) {
      	current.radius[i] = radiusAtRegion(i);
	//	current.radiusDelta[i] =
	//	    (radiusAtRegion(pos,i) - current.radius[i]) /
	//		(float)controlPeriod;
    }

    current.velum = velumAt();
    //    current.velumDelta =
    //	(velumAt(pos) - current.velum) / (float)controlPeriod;
    
}

void sampleRateInterpolation(void)
{
    int i;

    current.glotPitch += current.glotPitchDelta;
    current.glotVol += current.glotVolDelta;
    //    current.aspVol += current.aspVolDelta; // 0.0
    //    current.fricVol += current.fricVolDelta; // 0.0
    //    current.fricPos += current.fricPosDelta; // 0.0
    //    current.fricCF += current.fricCFDelta;// 0.0
    current.fricBW += current.fricBWDelta; // 0.0
    for (i = 0; i < TOTAL_REGIONS; i++)
	current.radius[i] += current.radiusDelta[i];
    current.velum += current.velumDelta;
}

void initializeNasalCavity(void)
{
    int i, j;
    float radA2, radB2;


    /*  CALCULATE COEFFICIENTS FOR INTERNAL FIXED SECTIONS OF NASAL CAVITY  */
    for (i = N2, j = NC2; i < N6; i++, j++) {
	radA2 = noseRadius[i] * noseRadius[i];
	radB2 = noseRadius[i+1] * noseRadius[i+1];
	nasal_coeff[j] = (radA2 - radB2) / (radA2 + radB2);
    }

    /*  CALCULATE THE FIXED COEFFICIENT FOR THE NOSE APERTURE  */
    radA2 = noseRadius[N6] * noseRadius[N6];
    radB2 = apScale * apScale;
    nasal_coeff[NC6] = (radA2 - radB2) / (radA2 + radB2);
}


void initializeThroat(void)
{
    ta0 = (throatCutoff * 2.0)/sampleRate;
    tb1 = 1.0 - ta0;

    throatGain = amplitude(throatVol);
}


void calculateTubeCoefficients(void)
{
    int i;
    float radA2, radB2, r0_2, r1_2, r2_2, sum;


    /*  CALCULATE COEFFICIENTS FOR THE OROPHARYNX  */
    for (i = 0; i < (TOTAL_REGIONS-1); i++) {
	radA2 = current.radius[i] * current.radius[i];
	radB2 = current.radius[i+1] * current.radius[i+1];
	oropharynx_coeff[i] = (radA2 - radB2) / (radA2 + radB2);
    }	

    /*  CALCULATE THE COEFFICIENT FOR THE MOUTH APERTURE  */
    radA2 = current.radius[R8] * current.radius[R8];
    radB2 = apScale * apScale;
    oropharynx_coeff[C8] = (radA2 - radB2) / (radA2 + radB2);

    /*  CALCULATE ALPHA COEFFICIENTS FOR 3-WAY JUNCTION  */
    /*  NOTE:  SINCE JUNCTION IS IN MIDDLE OF REGION 4, r0_2 = r1_2  */
    r0_2 = r1_2 = current.radius[R4] * current.radius[R4];
    r2_2 = current.velum * current.velum;
    sum = 2.0 / (r0_2 + r1_2 + r2_2);
    alpha[LEFT] = sum * r0_2;
    alpha[RIGHT] = sum * r1_2;
    alpha[UPPER] = sum * r2_2;

    /*  AND 1ST NASAL PASSAGE COEFFICIENT  */
    radA2 = current.velum * current.velum;
    radB2 = noseRadius[N2] * noseRadius[N2];
    nasal_coeff[NC1] = (radA2 - radB2) / (radA2 + radB2);
}

void setFricationTaps(void)
{
    int i, integerPart;
    float complement, remainder;
    float fricationAmplitude = amplitude(current.fricVol);


    /*  CALCULATE POSITION REMAINDER AND COMPLEMENT  */
    integerPart = (int)current.fricPos;
    complement = current.fricPos - (float)integerPart;
    remainder = 1.0 - complement;

    /*  SET THE FRICATION TAPS  */
    for (i = FC1; i < TOTAL_FRIC_COEFFICIENTS; i++) {
	if (i == integerPart) {
	    fricationTap[i] = remainder * fricationAmplitude;
	    if ((i+1) < TOTAL_FRIC_COEFFICIENTS)
		fricationTap[++i] = complement * fricationAmplitude;
	}
	else
	    fricationTap[i] = 0.0;
    }
}


void calculateBandpassCoefficients(void)
{
    float tanValue, cosValue;


    tanValue = tanf((TUBEPI * current.fricBW) / sampleRate);
    cosValue = cosf((2.0 * TUBEPI * current.fricCF) / sampleRate);

    bpBeta = (1.0 - tanValue) / (2.0 * (1.0 + tanValue));
    bpGamma = (0.5 + bpBeta) * cosValue;
    bpAlpha = (0.5 - bpBeta) / 2.0;
}


float mod0(float value)
{
    if (value > TABLE_MODULUS)
	value -= TABLE_LENGTH;

    return (value);
}


void incrementTablePosition(float frequency)
{
    currentPosition = mod0(currentPosition + (frequency * basicIncrement));
}


#if OVERSAMPLING_OSCILLATOR
float oscillator(float frequency)  /*  2X OVERSAMPLING OSCILLATOR  */
{
    int i, lowerPosition, upperPosition;
    float interpolatedValue, output;


    for (i = 0; i < 2; i++) {
	/*  FIRST INCREMENT THE TABLE POSITION, DEPENDING ON FREQUENCY  */
	incrementTablePosition(frequency/2.0);
    
	/*  FIND SURROUNDING INTEGER TABLE POSITIONS  */
	lowerPosition = (int)currentPosition;
	upperPosition = mod0(lowerPosition + 1);
    
	/*  CALCULATE INTERPOLATED TABLE VALUE  */
	interpolatedValue = (wavetable[lowerPosition] +
			     ((currentPosition - lowerPosition) *
			      (wavetable[upperPosition] -
			       wavetable[lowerPosition])));

	/*  PUT VALUE THROUGH FIR FILTER  */
	output = FIRFilter(interpolatedValue, i);
    }

    /*  SINCE WE DECIMATE, TAKE ONLY THE SECOND OUTPUT VALUE  */
    return (output);
}
#else
float oscillator(float frequency)  /*  PLAIN OSCILLATOR  */
{
    int lowerPosition, upperPosition;


    /*  FIRST INCREMENT THE TABLE POSITION, DEPENDING ON FREQUENCY  */
    incrementTablePosition(frequency);

    /*  FIND SURROUNDING INTEGER TABLE POSITIONS  */
    lowerPosition = (int)currentPosition;
    upperPosition = mod0(lowerPosition + 1);

    /*  RETURN INTERPOLATED TABLE VALUE  */
    return (wavetable[lowerPosition] +
	    ((currentPosition - lowerPosition) *
	     (wavetable[upperPosition] - wavetable[lowerPosition])));
}
#endif


float vocalTract(float input, float frication)
{
    int i, j, k;
    float delta, output, junctionPressure;


    /*  INCREMENT CURRENT AND PREVIOUS POINTERS  */
    if (++current_ptr > 1)
	current_ptr = 0;
    if (++prev_ptr > 1)
	prev_ptr = 0;

    /*  UPDATE OROPHARYNX  */
    /*  INPUT TO TOP OF TUBE  */
    oropharynx[S1][TOP][current_ptr] =
	(oropharynx[S1][BOTTOM][prev_ptr] * dampingFactor) + input;

    /*  CALCULATE THE SCATTERING JUNCTIONS FOR S1-S2  */
    delta = oropharynx_coeff[C1] *
	(oropharynx[S1][TOP][prev_ptr] - oropharynx[S2][BOTTOM][prev_ptr]);
    oropharynx[S2][TOP][current_ptr] =
	(oropharynx[S1][TOP][prev_ptr] + delta) * dampingFactor;
    oropharynx[S1][BOTTOM][current_ptr] =
	(oropharynx[S2][BOTTOM][prev_ptr] + delta) * dampingFactor;

    /*  CALCULATE THE SCATTERING JUNCTIONS FOR S2-S3 AND S3-S4  */
    for (i = S2, j = C2, k = FC1; i < S4; i++, j++, k++) {
	delta = oropharynx_coeff[j] *
	    (oropharynx[i][TOP][prev_ptr] - oropharynx[i+1][BOTTOM][prev_ptr]);
	oropharynx[i+1][TOP][current_ptr] =
	    ((oropharynx[i][TOP][prev_ptr] + delta) * dampingFactor) +
		(fricationTap[k] * frication);
	oropharynx[i][BOTTOM][current_ptr] =
	    (oropharynx[i+1][BOTTOM][prev_ptr] + delta) * dampingFactor;
    }

    /*  UPDATE 3-WAY JUNCTION BETWEEN THE MIDDLE OF R4 AND NASAL CAVITY  */
    junctionPressure = (alpha[LEFT] * oropharynx[S4][TOP][prev_ptr])+
	(alpha[RIGHT] * oropharynx[S5][BOTTOM][prev_ptr]) +
	(alpha[UPPER] * nasal[VELUM][BOTTOM][prev_ptr]);
    oropharynx[S4][BOTTOM][current_ptr] =
	(junctionPressure - oropharynx[S4][TOP][prev_ptr]) * dampingFactor;
    oropharynx[S5][TOP][current_ptr] =
	((junctionPressure - oropharynx[S5][BOTTOM][prev_ptr]) * dampingFactor)
	    + (fricationTap[FC3] * frication);
    nasal[VELUM][TOP][current_ptr] =
	(junctionPressure - nasal[VELUM][BOTTOM][prev_ptr]) * dampingFactor;

    /*  CALCULATE JUNCTION BETWEEN R4 AND R5 (S5-S6)  */
    delta = oropharynx_coeff[C4] *
	(oropharynx[S5][TOP][prev_ptr] - oropharynx[S6][BOTTOM][prev_ptr]);
    oropharynx[S6][TOP][current_ptr] =
	((oropharynx[S5][TOP][prev_ptr] + delta) * dampingFactor) +
	    (fricationTap[FC4] * frication);
    oropharynx[S5][BOTTOM][current_ptr] =
	(oropharynx[S6][BOTTOM][prev_ptr] + delta) * dampingFactor;

    /*  CALCULATE JUNCTION INSIDE R5 (S6-S7) (PURE DELAY WITH DAMPING)  */
    oropharynx[S7][TOP][current_ptr] =
	(oropharynx[S6][TOP][prev_ptr] * dampingFactor) +
	    (fricationTap[FC5] * frication);
    oropharynx[S6][BOTTOM][current_ptr] =
	oropharynx[S7][BOTTOM][prev_ptr] * dampingFactor;

    /*  CALCULATE LAST 3 INTERNAL JUNCTIONS (S7-S8, S8-S9, S9-S10)  */
    for (i = S7, j = C5, k = FC6; i < S10; i++, j++, k++) {
	delta = oropharynx_coeff[j] *
	    (oropharynx[i][TOP][prev_ptr] - oropharynx[i+1][BOTTOM][prev_ptr]);
	oropharynx[i+1][TOP][current_ptr] =
	    ((oropharynx[i][TOP][prev_ptr] + delta) * dampingFactor) +
		(fricationTap[k] * frication);
	oropharynx[i][BOTTOM][current_ptr] =
	    (oropharynx[i+1][BOTTOM][prev_ptr] + delta) * dampingFactor;
    }

    /*  REFLECTED SIGNAL AT MOUTH GOES THROUGH A LOWPASS FILTER  */
    oropharynx[S10][BOTTOM][current_ptr] =  dampingFactor *
	reflectionFilter(oropharynx_coeff[C8] *
			 oropharynx[S10][TOP][prev_ptr]);

    /*  OUTPUT FROM MOUTH GOES THROUGH A HIGHPASS FILTER  */
    output = radiationFilter((1.0 + oropharynx_coeff[C8]) *
			     oropharynx[S10][TOP][prev_ptr]);


    /*  UPDATE NASAL CAVITY  */
    for (i = VELUM, j = NC1; i < N6; i++, j++) {
	delta = nasal_coeff[j] *
	    (nasal[i][TOP][prev_ptr] - nasal[i+1][BOTTOM][prev_ptr]);
	nasal[i+1][TOP][current_ptr] =
	    (nasal[i][TOP][prev_ptr] + delta) * dampingFactor;
	nasal[i][BOTTOM][current_ptr] =
	    (nasal[i+1][BOTTOM][prev_ptr] + delta) * dampingFactor;
    }

    /*  REFLECTED SIGNAL AT NOSE GOES THROUGH A LOWPASS FILTER  */
    nasal[N6][BOTTOM][current_ptr] = dampingFactor *
	nasalReflectionFilter(nasal_coeff[NC6] * nasal[N6][TOP][prev_ptr]);

    /*  OUTPUT FROM NOSE GOES THROUGH A HIGHPASS FILTER  */
    output += nasalRadiationFilter((1.0 + nasal_coeff[NC6]) *
				   nasal[N6][TOP][prev_ptr]);

    /*  RETURN SUMMED OUTPUT FROM MOUTH AND NOSE  */
    return(output);
}


float throaty(float input)
{
    static float throatY = 0.0;

    float output = (ta0 * input) + (tb1 * throatY);
    throatY = output;
    return (output * throatGain);
}


float bandpassFilter(float input)
{
    static float xn1 = 0.0, xn2 = 0.0, yn1 = 0.0, yn2 = 0.0;
    float output;


    output = 2.0 *
	((bpAlpha * (input - xn2)) + (bpGamma * yn1) - (bpBeta * yn2));

    xn2 = xn1;
    xn1 = input;
    yn2 = yn1;
    yn1 = output;

    return (output);
}


float amplitude(float decibelLevel)
{
    /*  CONVERT 0-60 RANGE TO -60-0 RANGE  */
    decibelLevel -= VOL_MAX;

    /*  IF -60 OR LESS, RETURN AMPLITUDE OF 0  */
    if (decibelLevel <= (-VOL_MAX))
        return(0.0);

    /*  IF 0 OR GREATER, RETURN AMPLITUDE OF 1  */
    if (decibelLevel >= 0.0)
        return(1.0);

    /*  ELSE RETURN INVERSE LOG VALUE  */
    return(powf(10.0,(decibelLevel/20.0)));
}


float frequency(float pitch)
{
    return(PITCH_BASE * powf(2.0,(((float)(pitch+PITCH_OFFSET))/12.0)));
}


int maximallyFlat(float beta, float gamma, int *np, float *coefficient)
{
    float a[LIMIT+1], c[LIMIT+1], betaMinimum, ac;
    int nt, numerator, n, ll, i;


    /*  INITIALIZE NUMBER OF POINTS  */
    (*np) = 0;

    /*  CUT-OFF FREQUENCY MUST BE BETWEEN 0 HZ AND NYQUIST  */
    if ((beta <= 0.0) || (beta >= 0.5))
	return(BETA_OUT_OF_RANGE);

    /*  TRANSITION BAND MUST FIT WITH THE STOP BAND  */
    betaMinimum = ((2.0 * beta) < (1.0 - 2.0 * beta)) ? (2.0 * beta) :
	(1.0 - 2.0 * beta);
    if ((gamma <= 0.0) || (gamma >= betaMinimum))
	return(GAMMA_OUT_OF_RANGE);

    /*  MAKE SURE TRANSITION BAND NOT TOO SMALL  */
    nt = (int)(1.0 / (4.0 * gamma * gamma));
    if (nt > 160)
	return(GAMMA_TOO_SMALL);

    /*  CALCULATE THE RATIONAL APPROXIMATION TO THE CUT-OFF POINT  */
    ac = (1.0 + cosf(TWO_PI * beta)) / 2.0;
    rationalApproximation(ac, &nt, &numerator, np);

    /*  CALCULATE FILTER ORDER  */
    n = (2 * (*np)) - 1;
    if (numerator == 0)
	numerator = 1;


    /*  COMPUTE MAGNITUDE AT NP POINTS  */
    c[1] = a[1] = 1.0;
    ll = nt - numerator;

    for (i = 2; i <= (*np); i++) {
	int j;
	float x, sum = 1.0, y;
	c[i] = cosf(TWO_PI * ((float)(i-1)/(float)n));
	x = (1.0 - c[i]) / 2.0;
	y = x;

	if (numerator == nt)
	    continue;

	for (j = 1; j <= ll; j++) {
	    float z = y;
	    if (numerator != 1) {
		int jj;
		for (jj = 1; jj <= (numerator - 1); jj++)
		    z *= 1.0 + ((float)j / (float)jj);
	    }
	    y *= x;
	    sum += z;
	}
	a[i] = sum * powf((1.0 - x), numerator);
    }


    /*  CALCULATE WEIGHTING COEFFICIENTS BY AN N-POINT IDFT  */
    for (i = 1; i <= (*np); i++) {
	int j;
	coefficient[i] = a[1] / 2.0;
	for (j = 2; j <= (*np); j++) {
	    int m = ((i - 1) * (j - 1)) % n;
	    if (m > nt)
		m = n - m;
	    coefficient[i] += c[m+1] * a[j];
	}
	coefficient[i] *= 2.0/(float)n;
    }

    return(0);
}

void trim(float cutoff, int *numberCoefficients, float *coefficient)
{
    int i;

    for (i = (*numberCoefficients); i > 0; i--) {
	if (fabsf(coefficient[i]) >= fabsf(cutoff)) {
	    (*numberCoefficients) = i;
	    return;
	}
    }
}


void rationalApproximation(float number, int *order,
			   int *numerator, int *denominator)
{
    float fractionalPart, minimumError = 1.0;
    int i, orderMaximum, modulus = 0;


    /*  RETURN IMMEDIATELY IF THE ORDER IS LESS THAN ONE  */
    if (*order <= 0) {
	*numerator = 0;
	*denominator = 0;
	*order = -1;
	return;
    }

    /*  FIND THE ABSOLUTE VALUE OF THE FRACTIONAL PART OF THE NUMBER  */
    fractionalPart = fabsf(number - (int)number);

    /*  DETERMINE THE MAXIMUM VALUE OF THE DENOMINATOR  */
    orderMaximum = 2 * (*order);
    orderMaximum = (orderMaximum > LIMIT) ? LIMIT : orderMaximum;

    /*  FIND THE BEST DENOMINATOR VALUE  */
    for (i = (*order); i <= orderMaximum; i++) {
	float ps = i * fractionalPart;
	int ip = (int)(ps + 0.5);
	float error = fabsf( (ps - (float)ip)/(float)i );
	if (error < minimumError) {
	    minimumError = error;
	    modulus = ip;
	    *denominator = i;
	}
    }

    /*  DETERMINE THE NUMERATOR VALUE, MAKING IT NEGATIVE IF NECESSARY  */
    *numerator = (int)fabsf(number) * (*denominator) + modulus;
    if (number < 0)
	*numerator *= (-1);

    /*  SET THE ORDER  */
    *order = *denominator - 1;

    /*  RESET THE NUMERATOR AND DENOMINATOR IF THEY ARE EQUAL  */
    if (*numerator == *denominator) {
	*denominator = orderMaximum;
	*order = *numerator = *denominator - 1;
    }
}


float FIRFilter(float input, int needOutput)
{
    if (needOutput) {
	int i;
	float output = 0.0;

	/*  PUT INPUT SAMPLE INTO DATA BUFFER  */
	FIRData[FIRPtr] = input;
    
	/*  SUM THE OUTPUT FROM ALL FILTER TAPS  */
	for (i = 0; i < numberTaps; i++) {
	    output += FIRData[FIRPtr] * FIRCoef[i];
	    FIRPtr = increment(FIRPtr, numberTaps);
	}
    
	/*  DECREMENT THE DATA POINTER READY FOR NEXT CALL  */
	FIRPtr = decrement(FIRPtr, numberTaps);
    
	/*  RETURN THE OUTPUT VALUE  */
	return(output);
    }
    else {
	/*  PUT INPUT SAMPLE INTO DATA BUFFER  */
	FIRData[FIRPtr] = input;

	/*  ADJUST THE DATA POINTER, READY FOR NEXT CALL  */
	FIRPtr = decrement(FIRPtr, numberTaps);
    
	return(0.0);
    }
}


int increment(int pointer, int modulus)
{
    if (++pointer >= modulus)
	return(0);
    else
	return(pointer);
}


int decrement(int pointer, int modulus)
{
    if (--pointer < 0)
	return(modulus-1);
    else
	return(pointer);
}


float Izero(float x)
{
    float sum, u, halfx, temp;
    int n;


    sum = u = n = 1;
    halfx = x / 2.0;

    do {
	temp = halfx / (float)n;
	n += 1;
	temp *= temp;
	u *= temp;
	sum += u;
    } while (u >= (IzeroEPSILON * sum));

    return(sum);
}
