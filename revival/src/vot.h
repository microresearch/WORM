void votrax_init();

void votrax_newsay();
int16_t votrax_get_sample();

void votrax_newsaygorf(u8 reset);
void votrax_newsaygorfr();

int16_t votrax_get_samplegorf();

void votrax_newsaywow(u8 reset);
void votrax_newsaywowr();
int16_t votrax_get_samplewow();

void votrax_newsayTTS();
int16_t votrax_get_sampleTTS();

void votrax_newsay_rawparam();
int16_t votrax_get_sample_rawparam();

void votrax_newsay_bend(u8 reset);
void votrax_newsay_bendr();
int16_t votrax_get_sample_bend();

void votrax_newsay_sing();
int16_t votrax_get_sample_sing();

void votrax_newsaywow_bendfilter(u8 reset); 
void votrax_newsaywow_bendfilterr(); 
int16_t votrax_get_samplewow_bendfilter();
void votrax_retriggerTTS();
void votrax_rawparam_newsay();
