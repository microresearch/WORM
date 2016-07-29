#include "audio.h"
#include <math.h>
#include "arm_const_structs.h"

extern uint16_t adc_buffer[10];


#define	P_MAX	10	/* order p of LPC analysis, typically 8..14 */

float            	    /* returns minimum mean square error    */
			levinson_durbin(
			float const * ac,  /*  in: [0...p] autocorrelation values  */
	float	     * lpc) /*      [0...p-1] LPC coefficients      */
{
	int i, j;  float r, error = ac[0];

	if (ac[0] == 0) {
	  //		for (i = 0; i < P_MAX; i++) ref[i] = 0; 
		return 0; }

		for (i = 0; i < P_MAX; i++) {

		/* Sum up this iteration's reflection coefficient.
		*/
			r = -ac[i + 1];
			for (j = 0; j < i; j++) r -= lpc[j] * ac[i - j];
			//			ref[i] = r /= error;

		/*  Update LPC coefficients and total error.
		*/
			lpc[i] = r;
			for (j = 0; j < i/2; j++) {
				float tmp  = lpc[j];
				lpc[j]     += r * lpc[i-1-j];
				lpc[i-1-j] += r * tmp;
			}
			if (i % 2) lpc[j] += lpc[j] * r;

			error *= 1.0 - r * r;
		}
		return error;
}

void ac_calc(float * input, float * ac){

  for (u8 j=0;j< P_MAX; j++){ // 10 as 
    float sum=0;
    for (uint16_t k=0;k< 256-j-1; k++){ // 256 window
      sum+=input[j+k]*input[k];
    }
    ac[j]=sum;
  }
}

void compute_residual(float *input, float *residual){ // all for 256

  float ac[256];
  float lpc[10];

  ac_calc(input, ac);
  float error=levinson_durbin(ac, lpc);

    for (uint16_t k=0;k< 256; k++){ // 256 window
      float sum=0;
      for (u8 i = 0; i < P_MAX; i++) {
	//	sum+=lpc[i]* input[k-i] but only if k-i is >0 so for k>10 
      }
    }
}
