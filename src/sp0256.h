#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdint.h"
#include "arm_const_structs.h"

void sp0256_raw_init(void);
void sp0256_initbend(void);
void sp0256_init(void);

void sp0256_retriggerTTS(void);

int16_t sp0256_get_sample(void);
void sp0256_newsay(void);

int16_t sp0256_get_samplebend(void);
void sp0256_newsaybend(void);

int16_t sp0256_get_sampleTTS(void);
void sp0256_newsayTTS(void);

int16_t sp0256_get_samplevocabbankone(void);
void sp0256_newsayvocabbankone(void);

int16_t sp0256_get_samplevocabbanktwo(void);
void sp0256_newsayvocabbanktwo(void);

int16_t sp0256_get_sample1219(void);
void sp0256_newsay1219(void);

int16_t sp0256_get_sampleraw(void);
void sp0256_newsayraw(void);
