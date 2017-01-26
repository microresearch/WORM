#include "stm32f4xx.h"
#include "arm_math.h"
#include "stdlib.h"
#include "stdint.h"
#include "arm_const_structs.h"

int16_t sp0256_get_sample(void);
void sp0256_newsay(void);
void sp0256_init(void);

int16_t sp0256_get_sampleTTS(void);
void sp0256_newsayTTS(void);

int16_t sp0256_get_samplevocabbankone(void);
void sp0256_newsayvocabbankone(void);

int16_t sp0256_get_samplevocabbanktwo(void);
void sp0256_newsayvocabbanktwo(void);

int16_t sp0256_get_sample12(void);
void sp0256_newsay12(void);

int16_t sp0256_get_sample19(void);
void sp0256_newsay19(void);

int16_t sp0256_get_samplerawone(void);
void sp0256_newsayrawone(void);
