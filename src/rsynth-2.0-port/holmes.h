/* holmes.h,
*/
u16 run_holmes(u16 klatthead);
unsigned holmes (unsigned nelm,unsigned char *elm,unsigned nsamp, short *samp_base);
void init_holmes(void);
void term_holmes (void);
extern int speed;
void holmesrun(int16_t* outgoing, u8 size);
int16_t klatt_get_sample(void);
void klatt_newsay(void);
extern int samp_rate;
