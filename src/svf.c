// from stmlib

#include "audio.h"
#include "svf.h"

extern __IO uint16_t adc_buffer[10];


//class CrossoverSvf {
// Two passes of modified Chamberlin SVF with the same coefficients -
// to implement Linkwitz–Riley (Butterworth squared) crossover filters.

/// how to calculate coeffs... fq? - test filter - calc in filter_my.py

/* EMS2000 but what is width - see those calcs?

- ems 2000: 140, 185, 270, 367, 444, 539, 653, 791, 958, 1161, 1406, 1703, 2064, 2700, 4000, 5388 - 30 db/octave

*/

typedef struct Band{
  float gain;
  float coeffs[16];
  SVF svf[2]; // 2 passes
  float samples[32];
} Band; // each band

typedef struct Filterbank{  
  Band band_[17];
} Filterbank;

Filterbank Filterbankk;

float coefficients[]={-0.0282793973573, 0.00959297446756, -0.0163167758833, 0.0230064882424, -0.0393651013703, 0.00641386905172, -0.0334160506508, 0.00544619053794, -0.0574054032856, 0.00934663014085, -0.0487363845568, 0.00793888326107, -0.0779545431939, 0.0126823781741, -0.0661927558613, 0.0107763298717, -0.0942366298986, 0.015321870747, -0.0800287301336, 0.013023431221, -0.114286604369, 0.0185679468016, -0.097072311225, 0.0157895175849, -0.138288264423, 0.0224478486855, -0.117484032941, 0.0190998615243, -0.167253501531, 0.0271218210932, -0.14213102646, 0.023094469639, -0.202167149801, 0.0327441793954, -0.171861964199, 0.027910744163, -0.244389527147, 0.0395277773228, -0.207852602085, 0.0337403624366, -0.29500629759, 0.0476391521105, -0.251056229345, 0.0407420466369, -0.355824872964, 0.0573583693292, -0.303060665772, 0.0491839233527, -0.428878649093, 0.0689993382983, -0.365679453184, 0.0593844544377, -0.55499261433, 0.0890290943739, -0.474238488247, 0.0772216105788, -0.800710156926, 0.127936456871, -0.687889474666, 0.113347474541, -0.883201326334, 0.283602362272, -0.98933441725, 0.859117052889}; // total 64 is 16*4 yes

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

// SVF_wrappers

//SVF svf[2];

void BANDS_Init_(){
float gain=0.06f;
for (u8 i = 0; i < 16; i++) {

Filterbankk.band_[i].gain=gain;
//gain-=0.06;

for (u8 pass = 0; pass < 2; ++pass) {
SVF_Init(&Filterbankk.band_[i].svf[pass]);
set_f_fq(&Filterbankk.band_[i].svf[pass],coefficients[(i*4)+(pass * 2)],coefficients[(i*4)+(pass * 2)+1]);
    }
}
}

/*
void SVF_Init_(){
    for (u8 pass = 0; pass < 2; ++pass) {
      SVF_Init(&svf[pass]);
      set_f_fq(&svf[pass],coefficients[pass * 2],coefficients[pass * 2+1]);
    }
    }*/

static float mult_table[16]={0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f,0.06f};

     
void runBANDStest_(float* incoming, float* outgoing, u8 band_size){
    for (u8 y = 0; y < band_size; y++) {
      outgoing[y]=0.0f;
    }

  u8 which=(adc_buffer[SELX]>>8);// 4 bits to test
  mult_table[which]=(float)(adc_buffer[SELY]>>4)/4096.0f; // 


  for (u8 i=0;i<16;i++){ // each bank

    for (u8 pass = 0; pass < 2; ++pass) {
      const float* source = pass == 0 ? incoming : Filterbankk.band_[i].samples;
      float* destination = Filterbankk.band_[i].samples;
      if (i == 0) {
        SVF_Process(&(Filterbankk.band_[i].svf[pass]),source, destination, band_size, FILTER_MODE_LOW_PASS);
      } else if (i == 15) {
	SVF_Process(&(Filterbankk.band_[i].svf[pass]),source, destination, band_size, FILTER_MODE_HIGH_PASS);
      } else {
	SVF_Process(&(Filterbankk.band_[i].svf[pass]),source, destination, band_size, FILTER_MODE_BAND_PASS_NORMALIZED);
      }
    }

    float *output=Filterbankk.band_[i].samples;

    for (u8 y = 0; y < band_size; y++) {
      output[y] *= mult_table[i];
      //      output[y] *= 0.1f;
            outgoing[y]+=output[y];
    }
  }
}

/*void runSVFtest_(float* incoming, float* outgoing, u8 band_size){
    for (int32_t pass = 0; pass < 2; ++pass) {
      const float* source = pass == 0 ? incoming : outgoing;
      float* destination = outgoing;
//        SVF_Process(&svf[pass],source, destination, band_size, FILTER_MODE_LOW_PASS);
	SVF_Process(&svf[pass],source, destination, band_size, FILTER_MODE_BAND_PASS_NORMALIZED);
	}*/
    // Apply post-gain
//    const float gain = b.post_gain;
//    float* output = b.samples;
//    for (size_t i = 0; i < band_size; ++i) {
//      output[i] *= gain;
//    }
//}

