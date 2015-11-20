typedef struct 
{
	int32_t m_phase1, m_phase2, m_phase3;
	float m_cpstoinc; // was double
} Formant; 

void Formant_init(Formant *unit);
//void Formant_process(Formant *unit, int32_t freq1, int32_t freq2, int32_t freq3, int inNumSamples, float* outbuffer);
void Formant_process(Formant *unit, float freq1in, float freq2in, float freq3in, int inNumSamples, float* outbuffer);
