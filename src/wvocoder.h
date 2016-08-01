void Vocoder_Init(float sample_rate);

void Vocoder_Process(
    const float* modulator,
    const float* carrier,
    float* out,
    u8 size);
