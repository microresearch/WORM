typedef struct 
{
	int32_t m_phase1, m_phase2, m_phase3;
	float m_cpstoinc; // was double
} Formant; 

typedef struct 
{
	int32_t m_phase, m_numharm, m_N;
	float m_freqin, m_scale;
} Blip;

typedef struct
{
	float m_freq, m_reson;
	float m_y1, m_y2, m_a0, m_b1, m_b2;
} RLPF;



void Blip_do(Blip *unit, float* out, int inNumSamples, float freq, u8 numharm, float mul);
void Blip_init(Blip *unit);

void Formant_init(Formant *unit);
//void Formant_process(Formant *unit, int32_t freq1, int32_t freq2, int32_t freq3, int inNumSamples, float* outbuffer);
void Formant_process(Formant *unit, float freq1in, float freq2in, float freq3in, int inNumSamples, float* outbuffer);

void HPZ_do(float* in, float* out, int inNumSamples);

void RLPF_init(RLPF* unit);
void RLPF_do(RLPF* unit, float* in, float* out, float freq, float reson, int inNumSamples, float mul);
