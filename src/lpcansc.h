typedef float LPCfloat;


void do_impulse(float *out, u8 numSamples, uint16_t freq);
void do_delay(float *in, float *out, uint16_t delaytime, u8 numSamples);
void LPCAnalyzer_init();
void LPCAnalyzer_init4();
void LPC_residual(LPCfloat * newinput, LPCfloat * output, int numSamples);
void LPC_cross(LPCfloat * newinput, LPCfloat *newsource, LPCfloat * output, int numSamples);
