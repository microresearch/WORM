void votrax_init();

void votrax_newsay();
int16_t votrax_get_sample();

void votrax_newsaygorf(u8 reset);
int16_t votrax_get_samplegorf();

void votrax_newsaywow(u8 reset);
int16_t votrax_get_samplewow();

void votrax_newsayTTS();
int16_t votrax_get_sampleTTS();

void votrax_newsay_rawparam();
int16_t votrax_get_sample_rawparam();

void votrax_newsay_bend(u8 reset);
int16_t votrax_get_sample_bend();


void votrax_retriggerTTS();
