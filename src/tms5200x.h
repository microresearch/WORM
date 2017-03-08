int16_t tms_init();

int16_t tms_get_sample();
int16_t tms_newsay();

int16_t tms_get_sample_TTS();
int16_t tms_newsay_TTS();
void tms_retriggerTTS();

int16_t tms_get_sample_allphon();
int16_t tms_newsay_allphon();

int16_t tms_get_sample_bendlength();

int16_t tms_get_sample_lowbit();
int16_t tms_newsay_lowbit();

int16_t tms_get_sample_raw5100();
int16_t tms_newsay_raw5100();

int16_t tms_get_sample_raw5200();
int16_t tms_newsay_raw5200();

int16_t tms_get_sample_raw5220();
int16_t tms_newsay_raw5220();

int16_t tms_get_sample_5100pitchtable(u8 fix);  // vocab=0 ADD VOCABS

int16_t tms_get_sample_5100ktable(u8 fix);  // vocab=0 ADD VOCABS

int16_t tms_get_sample_5200ktable(u8 fix);  // vocab=1 ADD VOCABS

int16_t tms_newsay_specific(u8 whichbank);

/// new to TEST in audio.c and extra vocabs for each and above modes as independent functions

int16_t tms_get_sample_bend5200(u8 fix); // vocab=1 which will change for allphons ADD VOCABS

int16_t tms_get_sample_bend5100(u8 fix); // vocab=0 ADD VOCABS

int16_t tms_get_sample_5200pitchtable(u8 fix); // vocab=1 which will change for allphons + we have more pitches = 64 - reflect this in audio.c ADD VOCABS

int16_t tms_get_sample_5100kandpitchtable(u8 fix); // for 5100 we have 32+168 in exy= 200 5200 is 232 ADD VOCABS

int16_t tms_get_sample_5200kandpitchtable(u8 fix); // for 5100 we have 32+168 in exy= 200 5200 is 232 ADD VOCABS
