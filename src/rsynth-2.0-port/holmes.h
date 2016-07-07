/* holmes.h,
*/
extern u16 run_holmes(u16 klatthead);
extern unsigned holmes (unsigned nelm,unsigned char *elm,unsigned nsamp, short *samp_base);
extern void init_holmes(void);
extern void term_holmes (void);
extern int speed;
extern void holmesrun(int16_t* outgoing, u8 size);
extern int16_t klatt_get_sample(void);
