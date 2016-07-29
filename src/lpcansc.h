void do_impulse(float *out, u8 numSamples, uint16_t freq);
void do_delay(float *in, float *out, uint16_t delaytime, u8 numSamples);
void LPCAnalysis_update(float * newinput, float * output, int numSamples);
void LPCAnalyzer_init();
