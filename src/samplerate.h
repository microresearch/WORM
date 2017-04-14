void sample_rate_init();

void samplerate(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio);
void samplerate_simple(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio);

void samplerate_exy(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio, u8 extent); // exy template

void samplerate_simple_exy(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), float sampleratio, u8 extent); // exy template
