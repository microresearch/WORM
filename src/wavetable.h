typedef struct {
    float FIRData[49], *FIRCoef;
    int FIRPtr, numberTaps;
} TRMFIRFilter;

typedef struct _Wavetable {
  TRMFIRFilter *FIRFilter;
    const float *wavetable;
    float basicIncrement;
    float currentPosition;
  int16_t length;
} Wavetable;

void wavetable_init(Wavetable* wavtable, const float *tableitself, int16_t length); // need to declare wavetable struct and and ourtable we use
void dowavetable(float* outgoing, Wavetable *wavetable, float frequency, u8 length);
void dowormwavetable(float* outgoing, Wavetable *wavetable, float frequency, u8 length);
float dosinglewavetable(Wavetable *wavetable, float frequency);//

void wave_newsay(void);
int16_t wave_get_sample(void);
