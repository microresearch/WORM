void sample_rate_init();
void dosamplerate(int16_t* in, int16_t* out, float factor, u8 size);
void samplerate(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void));
