#define MAX_BANDS  16
#define AMPLIFIER 16.0
#define LADSPA_Data float


struct bandpass// 44 bytes
{
  LADSPA_Data c, f, att;
  LADSPA_Data freq;
  LADSPA_Data low1, low2;
  LADSPA_Data mid1, mid2;
  LADSPA_Data high1, high2;
  LADSPA_Data y;
};

struct bands_out{
  LADSPA_Data decay;
  LADSPA_Data oldval;
};

/* Instance data for the vocoder plugin */
typedef struct { //44*16*3=2k!
  LADSPA_Data SampleRate;
  struct bandpass bands_formant[MAX_BANDS]; /* one structure per band */
  struct bandpass bands_carrier[MAX_BANDS]; /* one structure per band */
  struct bands_out bands_out[MAX_BANDS]; /* one structure per band */

  /* Ports */

  LADSPA_Data * portFormant;	/* Formant signal port data location */
  LADSPA_Data * portCarrier;	/* Carrier signal port data location */
  LADSPA_Data * portOutput;	/* Output audio port data location */

} VocoderInstance;

int32_t testvocode(void);

void runVocoder(VocoderInstance *vocoder, float *formant, float *carrier, float *out, unsigned int SampleCount);
VocoderInstance* instantiateVocoder(void);
