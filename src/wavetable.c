#include "stdio.h"
#include "math.h"
#include "forlap.h"

// varying wavetable implementations: params - oversampling, interpolating... length and calc frequency?

// note that BRAIDs uses multiple indexes into giant wavetable

// from tubes:

#define TABLE_LENGTH              512
#define TABLE_MODULUS             (TABLE_LENGTH-1)

#define OVERSAMPLING_OSCILLATOR 0

// TODO: FIR filter for above. generate table

float ourtable[512]={0.000000, 0.012296, 0.024589, 0.036879, 0.049164, 0.061441, 0.073708, 0.085965, 0.098208, 0.110437, 0.122649, 0.134842, 0.147016, 0.159166, 0.171293, 0.183394, 0.195467, 0.207511, 0.219523, 0.231502, 0.243446, 0.255353, 0.267222, 0.279050, 0.290836, 0.302578, 0.314275, 0.325923, 0.337523, 0.349071, 0.360567, 0.372008, 0.383393, 0.394720, 0.405988, 0.417194, 0.428337, 0.439415, 0.450426, 0.461370, 0.472244, 0.483046, 0.493776, 0.504430, 0.515009, 0.525509, 0.535931, 0.546271, 0.556528, 0.566702, 0.576789, 0.586790, 0.596702, 0.606524, 0.616253, 0.625890, 0.635432, 0.644878, 0.654227, 0.663477, 0.672626, 0.681674, 0.690618, 0.699458, 0.708193, 0.716820, 0.725339, 0.733748, 0.742047, 0.750233, 0.758306, 0.766264, 0.774106, 0.781832, 0.789439, 0.796926, 0.804293, 0.811539, 0.818662, 0.825661, 0.832535, 0.839284, 0.845905, 0.852399, 0.858764, 0.864999, 0.871103, 0.877076, 0.882916, 0.888622, 0.894194, 0.899631, 0.904932, 0.910096, 0.915122, 0.920010, 0.924759, 0.929369, 0.933837, 0.938165, 0.942350, 0.946394, 0.950294, 0.954050, 0.957662, 0.961130, 0.964452, 0.967628, 0.970658, 0.973541, 0.976278, 0.978866, 0.981306, 0.983599, 0.985742, 0.987736, 0.989581, 0.991277, 0.992822, 0.994218, 0.995463, 0.996558, 0.997502, 0.998295, 0.998937, 0.999428, 0.999768, 0.999958, 0.999995, 0.999882, 0.999617, 0.999202, 0.998635, 0.997917, 0.997049, 0.996029, 0.994859, 0.993539, 0.992068, 0.990448, 0.988678, 0.986758, 0.984689, 0.982471, 0.980105, 0.977590, 0.974928, 0.972118, 0.969162, 0.966058, 0.962809, 0.959414, 0.955874, 0.952190, 0.948362, 0.944390, 0.940275, 0.936019, 0.931620, 0.927081, 0.922402, 0.917584, 0.912626, 0.907531, 0.902298, 0.896929, 0.891425, 0.885785, 0.880012, 0.874106, 0.868067, 0.861898, 0.855598, 0.849168, 0.842611, 0.835925, 0.829114, 0.822177, 0.815116, 0.807931, 0.800625, 0.793197, 0.785650, 0.777984, 0.770200, 0.762299, 0.754284, 0.746154, 0.737912, 0.729558, 0.721093, 0.712520, 0.703839, 0.695051, 0.686159, 0.677162, 0.668064, 0.658864, 0.649565, 0.640167, 0.630673, 0.621083, 0.611400, 0.601624, 0.591757, 0.581801, 0.571756, 0.561626, 0.551410, 0.541111, 0.530730, 0.520269, 0.509729, 0.499112, 0.488420, 0.477654, 0.466816, 0.455907, 0.444929, 0.433884, 0.422773, 0.411598, 0.400361, 0.389064, 0.377708, 0.366295, 0.354826, 0.343304, 0.331729, 0.320105, 0.308432, 0.296713, 0.284948, 0.273141, 0.261292, 0.249404, 0.237478, 0.225517, 0.213521, 0.201493, 0.189434, 0.177347, 0.165233, 0.153094, 0.140932, 0.128748, 0.116545, 0.104325, 0.092088, 0.079838, 0.067576, 0.055303, 0.043022, 0.030735, 0.018443, 0.006148, -0.006148, -0.018443, -0.030735, -0.043022, -0.055303, -0.067576, -0.079838, -0.092088, -0.104325, -0.116545, -0.128748, -0.140932, -0.153094, -0.165233, -0.177347, -0.189434, -0.201493, -0.213521, -0.225517, -0.237479, -0.249404, -0.261293, -0.273141, -0.284949, -0.296713, -0.308432, -0.320105, -0.331730, -0.343304, -0.354826, -0.366295, -0.377708, -0.389064, -0.400362, -0.411599, -0.422773, -0.433884, -0.444929, -0.455907, -0.466816, -0.477654, -0.488420, -0.499112, -0.509729, -0.520269, -0.530730, -0.541111, -0.551410, -0.561626, -0.571757, -0.581801, -0.591757, -0.601624, -0.611400, -0.621084, -0.630673, -0.640168, -0.649565, -0.658864, -0.668064, -0.677163, -0.686159, -0.695051, -0.703839, -0.712520, -0.721093, -0.729558, -0.737912, -0.746154, -0.754284, -0.762299, -0.770200, -0.777984, -0.785650, -0.793197, -0.800625, -0.807932, -0.815116, -0.822177, -0.829114, -0.835926, -0.842611, -0.849168, -0.855598, -0.861898, -0.868068, -0.874106, -0.880012, -0.885786, -0.891425, -0.896929, -0.902298, -0.907531, -0.912626, -0.917584, -0.922402, -0.927082, -0.931621, -0.936019, -0.940275, -0.944390, -0.948362, -0.952190, -0.955874, -0.959414, -0.962809, -0.966058, -0.969162, -0.972118, -0.974928, -0.977590, -0.980105, -0.982471, -0.984689, -0.986758, -0.988678, -0.990448, -0.992068, -0.993539, -0.994859, -0.996029, -0.997049, -0.997917, -0.998635, -0.999202, -0.999617, -0.999882, -0.999995, -0.999958, -0.999768, -0.999428, -0.998937, -0.998295, -0.997502, -0.996558, -0.995463, -0.994218, -0.992822, -0.991277, -0.989581, -0.987736, -0.985742, -0.983599, -0.981306, -0.978866, -0.976277, -0.973541, -0.970658, -0.967628, -0.964452, -0.961130, -0.957662, -0.954050, -0.950294, -0.946394, -0.942350, -0.938165, -0.933837, -0.929368, -0.924759, -0.920010, -0.915122, -0.910096, -0.904932, -0.899631, -0.894194, -0.888622, -0.882915, -0.877076, -0.871103, -0.864999, -0.858764, -0.852399, -0.845905, -0.839284, -0.832535, -0.825661, -0.818662, -0.811539, -0.804293, -0.796926, -0.789438, -0.781831, -0.774106, -0.766264, -0.758306, -0.750233, -0.742047, -0.733748, -0.725339, -0.716820, -0.708193, -0.699458, -0.690618, -0.681673, -0.672626, -0.663476, -0.654227, -0.644878, -0.635432, -0.625890, -0.616253, -0.606523, -0.596702, -0.586790, -0.576789, -0.566702, -0.556528, -0.546271, -0.535930, -0.525509, -0.515009, -0.504430, -0.493775, -0.483046, -0.472244, -0.461370, -0.450426, -0.439414, -0.428336, -0.417193, -0.405988, -0.394720, -0.383393, -0.372008, -0.360567, -0.349071, -0.337523, -0.325923, -0.314274, -0.302578, -0.290836, -0.279050, -0.267222, -0.255353, -0.243446, -0.231502, -0.219523, -0.207511, -0.195467, -0.183394, -0.171293, -0.159166, -0.147015, -0.134842, -0.122649, -0.110437, -0.098208, -0.085965, -0.073708, -0.061440, -0.049163, -0.036879, -0.024589, -0.012295, 0.000000};

typedef struct _Wavetable {
  //    TRMFIRFilter *FIRFilter;
    float *wavetable;
    float basicIncrement;
    float currentPosition;
} Wavetable;


static float mod0(float value);
static void WavetableIncrementPosition(Wavetable *wavetable, float frequency);

static float mod0(float value)
{
    while (value > TABLE_MODULUS)
        value -= TABLE_LENGTH;
    return value;
}


// Increments the position in the wavetable according to the desired frequency.
static void WavetableIncrementPosition(Wavetable *wavetable, float frequency)
{
    wavetable->currentPosition = mod0(wavetable->currentPosition + (frequency * wavetable->basicIncrement));
}

#if OVERSAMPLING_OSCILLATOR
void WavetableOscillator(float* outgoing, Wavetable *wavetable, float frequency, int16_t length)  //  2X oversampling oscillator
{
    int i, lowerPosition, upperPosition;
    float interpolatedValue, sample;

    for (int ii = 0; ii < length; ii++) {
    for (i = 0; i < 2; i++) {
        //  First increment the table position, depending on frequency
        WavetableIncrementPosition(wavetable, frequency / 2.0);

        //  Find surrounding integer table positions
        lowerPosition = (int)wavetable->currentPosition;
        upperPosition = mod0(lowerPosition + 1);

        //  Calculate interpolated table value
        interpolatedValue = (wavetable->wavetable[lowerPosition] +
                             ((wavetable->currentPosition - lowerPosition) *
                              (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

        //  Put value through FIR filter
        sample = FIRFilter(wavetable->FIRFilter, interpolatedValue, i); // TODO!
    }
    outgoing[ii]=sample;
    //  Since we decimate, take only the second output value
    }
}
#else
void WavetableOscillator(float* outgoing, Wavetable *wavetable, float frequency, int16_t length)  //  Plain oscillator
{
    int lowerPosition, upperPosition;
    for (int ii = 0; ii < length; ii++) {

    //  First increment the table position, depending on frequency
    WavetableIncrementPosition(wavetable, frequency);

    //  Find surrounding integer table positions
    lowerPosition = (int)wavetable->currentPosition;
    upperPosition = mod0(lowerPosition + 1);

    //  Return interpolated table value
    float sample= (wavetable->wavetable[lowerPosition] +
            ((wavetable->currentPosition - lowerPosition) *
             (wavetable->wavetable[upperPosition] - wavetable->wavetable[lowerPosition])));

    outgoing[ii]=sample;
    }
}
#endif

// 

void main(){
  //  float ourtable[512]={1.0}; // generate tabel
  Wavetable wavtable; float out[128];
  wavtable.wavetable=ourtable;
  wavtable.basicIncrement=(float)TABLE_LENGTH/8000.0f; // 32000 for real thing - test lap is 8000
  wavtable.currentPosition=0.0;

  while(1){
  WavetableOscillator(out, &wavtable, 50.0f, 128);  //  Plain oscillator

  for (int x=0;x<128;x++){
    printf("%c",(char)(out[x]*128.0f));
  }
  }
}
