void initbraidworm(void);

void RenderVosim(
		 uint8_t sync, // sync was sync buffer now just signal
		 int16_t* buffer,
		 size_t size, u16 param1,u16 param2, int16_t pitch_);

void RenderVowel(
    uint8_t sync,
    int16_t* buffer,
    size_t size, u16 param1,u16 param2,u16 param3, int16_t pitch_);

void RenderVowelFof(
  uint8_t sync,
  int16_t* buffer,
  size_t size, u16 param1,u16 param2, int16_t pitch_);

int16_t fof_get_sample();
