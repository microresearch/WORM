// from stmlib

#include "audio.h"
#include "svf.h"

//class CrossoverSvf {
// Two passes of modified Chamberlin SVF with the same coefficients -
// to implement Linkwitz–Riley (Butterworth squared) crossover filters.

/// how to calculate coeffs... fq?

// in warps there are 2 passes for each filter so 2 sets of coeffs

// delay is a question though!

    
void SVF_Reset(SVF* svf) {
  svf->lp_[0] = svf->bp_[0] = svf->lp_[1] = svf->bp_[1] = 0.0f;
  svf->x_[0] = 0.0f;
  svf->x_[1] = 0.0f;
}
  
void SVF_Init(SVF* svf) {
    SVF_Reset(svf);
  }

