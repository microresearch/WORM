enum FilterMode {
  FILTER_MODE_LOW_PASS,
  FILTER_MODE_BAND_PASS,
  FILTER_MODE_BAND_PASS_NORMALIZED,
  FILTER_MODE_HIGH_PASS
};

typedef struct SVF{
  float f_;
  float fq_;
  float x_[2];
  float lp_[2];
  float bp_[2];
} SVF;


// functions also
void SVF_Reset(SVF* svf);
void SVF_Init(SVF* svf);
void SVF_Init_();
void BANDS_Init_();
void runBANDStest_(float* incoming, float* outgoing, u8 band_size);

void runSVFtest_(float* incoming, float* outgoing, u8 band_size);

inline void set_f_fq(SVF* svf, float f, float fq) {
    svf->f_ = f;
    svf->fq_ = fq;
  }
  
inline void SVF_Process(SVF* svf, const float* in, float* out, u8 size, u8 mode) {
    float lp_1 = svf->lp_[0];
    float bp_1 = svf->bp_[0];
    float lp_2 = svf->lp_[1];
    float bp_2 = svf->bp_[1];
    float x_1 = svf->x_[0];
    float x_2 = svf->x_[1];
    const float fq = svf->fq_;
    const float f = svf->f_;
    while (size--) {
      lp_1 += f * bp_1;
      bp_1 += -fq * bp_1 -f * lp_1 + *in;
      if (mode == FILTER_MODE_BAND_PASS ||
          mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        bp_1 += x_1;
      }
      x_1 = *in++;
      
      float y;
      if (mode == FILTER_MODE_LOW_PASS) {
        y = lp_1 * f;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        y = bp_1 * f;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        y = bp_1 * fq;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        y = x_1 - lp_1 * f - bp_1 * fq;
      }
      
      lp_2 += f * bp_2;
      bp_2 += -fq * bp_2 -f * lp_2 + y;
      if (mode == FILTER_MODE_BAND_PASS ||
          mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        bp_2 += x_2;
      }
      x_2 = y;
      
      if (mode == FILTER_MODE_LOW_PASS) {
        *out++ = lp_2 * f;
      } else if (mode == FILTER_MODE_BAND_PASS) {
        *out++ = bp_2 * f;
      } else if (mode == FILTER_MODE_BAND_PASS_NORMALIZED) {
        *out++ = bp_2 * fq;
      } else if (mode == FILTER_MODE_HIGH_PASS) {
        *out++ = x_2 - lp_2 * f - bp_2 * fq;
      }
    }
    svf->lp_[0] = lp_1;
    svf->bp_[0] = bp_1;
    svf->lp_[1] = lp_2;
    svf->bp_[1] = bp_2;
    svf->x_[0] = x_1;
    svf->x_[1] = x_2;
  }
  
