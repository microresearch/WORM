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

int16_t tms_get_sample_bend5200(); // for allphons - with     tms_newsay_allphon();

int16_t tms_get_sample_5100pitchtable(); // with straight newsay just for tests and restrict to one vocab LATER TODO!

int16_t tms_get_sample_5100ktable(); // with straight newsay just for tests and restrict to one vocab LATER TODO!
