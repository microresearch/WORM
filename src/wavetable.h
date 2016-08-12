typedef struct {
    float FIRData[49], *FIRCoef;
    int FIRPtr, numberTaps;
} TRMFIRFilter;

typedef struct _Wavetable {
  TRMFIRFilter *FIRFilter;
    float *wavetable;
    float basicIncrement;
    float currentPosition;
int16_t length;
} Wavetable;

void wavetable_init(Wavetable* wavtable, const float *tableitself, int16_t length); // need to declare wavetable struct and and ourtable we use
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, int16_t length);
void dowormwavetable(float* outgoing, Wavetable *wavetable, float frequency, int16_t length);