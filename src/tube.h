// for tube.c

int initializeSynthesizer(void);
void synthesize(void);

void tube_init();
void tube_newsay(); 
int16_t tube_get_sample();

void tube_newsay_sing(); 
int16_t tube_get_sample_sing();

void tube_newsay_bend(); 
int16_t tube_get_sample_bend();

void tube_newsay_raw(); 
int16_t tube_get_sample_raw();
