#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <malloc.h>

#define twopi 6.28318530717952646f


const int kSineSize = 8192;

void main(void){
  int i;
double sineIndexToPhase = twopi / kSineSize;
for (i=0; i < kSineSize; ++i) {
  double phase = i * sineIndexToPhase;
  float d = sin(phase);
  //  gSine[i] = d;
   printf("%f, ",d);
  // printf("%d, ",i);
 }

}
