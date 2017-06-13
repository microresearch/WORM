int16_t tms_init();

int16_t tms_get_sample();
int16_t tms_get_sample_sing();
void tms_newsay();

int16_t tms_get_sample_TTS();
void tms_newsay_TTS();
void tms_retriggerTTS();

int16_t tms_get_sample_allphon();
int16_t tms_get_sample_allphon_sing();
void tms_newsay_allphon();

int16_t tms_get_sample_bendlength();

int16_t tms_get_sample_lowbit();
void tms_newsay_lowbit();

int16_t tms_get_sample_raw5100();
void tms_newsay_raw5100();

int16_t tms_get_sample_raw5200();
void tms_newsay_raw5200();

int16_t tms_get_sample_raw5220();
void tms_newsay_raw5220();

int16_t tms_get_sample_5100pitchtablew();  // vocab=0 ADD VOCABS
int16_t tms_get_sample_5100ktablew();  // vocab=0 ADD VOCABS
int16_t tms_get_sample_5200ktablea();// allphon ADD

void tms_newsay_specific(u8 whichbank);
void tms_newsay_specifica(); // allphons
void tms_newsay_specific5100(); // 0

/// new to TEST in audio.c and extra vocabs for each and above modes as independent functions

int16_t tms_get_sample_bend5200a(); // vocab=1 which will change for allphons ADD VOCABS

int16_t tms_get_sample_bend5100w(); // vocab=0 ADD VOCABS

int16_t tms_get_sample_5200pitchtablea(); // vocab=1 which will change for allphons + we have more pitches = 64 - reflect this in audio.c ADD VOCABS

int16_t tms_get_sample_5100kandpitchtablew(); // for 5100 we have 32+168 in exy= 200 5200 is 232 ADD VOCABS

int16_t tms_get_sample_5200kandpitchtablea(); // for 5100 we have 32+168 in exy= 200 5200 is 232 ADD VOCABS
