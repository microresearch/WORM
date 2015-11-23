#include "stmlib/utils/dsp.h"
#include "stmlib/utils/random.h"


// vosim//vowel/vowelfof

struct VowelSynthesizerState {
  uint32_t formant_increment[3];
  uint32_t formant_phase[3];
  uint32_t formant_amplitude[3];
  uint16_t consonant_frames;
  uint16_t noise;
};

struct Fof {
  uint32_t phase;
  uint32_t phase_increment;
  uint16_t amplitude;
};

struct FofState {
  Fof fof[kNumOverlappingFof][kNumFormants];
  uint32_t envelope_phase[kNumOverlappingFof];
  uint32_t envelope_phase_increment[kNumOverlappingFof];
  uint8_t lru_fof;
  int16_t prevous_sample;
};

  uint32_t phase_;
  uint32_t phase_increment_;
  uint32_t delay_;

  int16_t parameter_[2];
  int16_t previous_parameter_[2];
  int32_t smoothed_parameter_;
  int16_t pitch_;


uint32_t ComputePhaseIncrement(int16_t midi_pitch) {
  if (midi_pitch >= kPitchTableStart) {
    midi_pitch = kPitchTableStart - 1;
  }
  
  int32_t ref_pitch = midi_pitch;
  ref_pitch -= kPitchTableStart;
  
  size_t num_shifts = 0;
  while (ref_pitch < 0) {
    ref_pitch += kOctave;
    ++num_shifts;
  }
  
  uint32_t a = lut_oscillator_increments[ref_pitch >> 4];
  uint32_t b = lut_oscillator_increments[(ref_pitch >> 4) + 1];
  uint32_t phase_increment = a + \
      (static_cast<int32_t>(b - a) * (ref_pitch & 0xf) >> 4);
  phase_increment >>= num_shifts;
  return phase_increment;
}

///////////////////////////////////

void VosimInit(){

}

void VowelInit(){

}

void VowelFOFInit(){

}


void RenderVosim(
		 uint8_t sync, // sync was sync buffer now just signal
		 int16_t* buffer,
		 size_t size) {

// TODO: interpolate

  for (size_t i = 0; i < 2; ++i) {
    state_.vow.formant_increment[i] = ComputePhaseIncrement(parameter_[i] >> 1);
  }
  while (size--) {
    phase_ += phase_increment_;
    if (sync==1) {
      phase_ = 0;
    }
    int32_t sample = 16384 + 8192;
    state_.vow.formant_phase[0] += state_.vow.formant_increment[0];
    sample += Interpolate824(wav_sine, state_.vow.formant_phase[0]) >> 1;
    
    state_.vow.formant_phase[1] += state_.vow.formant_increment[1];
    sample += Interpolate824(wav_sine, state_.vow.formant_phase[1]) >> 2;
    
    sample = sample * (Interpolate824(lut_bell, phase_) >> 1) >> 15;
    if (phase_ < phase_increment_) {
      state_.vow.formant_phase[0] = 0;
      state_.vow.formant_phase[1] = 0;
      sample = 0;
    }
    sample -= 16384 + 8192;
    *buffer++ = sample;
  }
}

struct PhonemeDefinition {
  uint8_t formant_frequency[3];
  uint8_t formant_amplitude[3];
};

const PhonemeDefinition vowels_data[9] = {
    { { 27,  40,  89 }, { 15,  13,  1 } },
    { { 18,  51,  62 }, { 13,  12,  6 } },
    { { 15,  69,  93 }, { 14,  12,  7 } },
    { { 10,  84, 110 }, { 13,  10,  8 } },
    { { 23,  44,  87 }, { 15,  12,  1 } },
    { { 13,  29,  80 }, { 13,   8,  0 } },
    { {  6,  46,  81 }, { 12,   3,  0 } },
    { {  9,  51,  95 }, { 15,   3,  0 } },
    { {  6,  73,  99 }, {  7,   3,  14 } }
};

const PhonemeDefinition consonant_data[8] = {
    { { 6, 54, 121 }, { 9,  9,  0 } },
    { { 18, 50, 51 }, { 12,  10,  5 } },
    { { 11, 24, 70 }, { 13,  8,  0 } },
    { { 15, 69, 74 }, { 14,  12,  7 } },
    { { 16, 37, 111 }, { 14,  8,  1 } },
    { { 18, 51, 62 }, { 14,  12,  6 } },
    { { 6, 26, 81 }, { 5,  5,  5 } },
    { { 6, 73, 99 }, { 7,  10,  14 } },
};


void RenderVowel(
    uint8_t sync,
    int16_t* buffer,
    size_t size) {
  size_t vowel_index = parameter_[0] >> 12;
  uint16_t balance = parameter_[0] & 0x0fff;
  uint16_t formant_shift = (200 + (parameter_[1] >> 6));
  if (strike_) {
    strike_ = false;
    state_.vow.consonant_frames = 160;
    uint16_t index = (Random::GetSample() + 1) & 7;
    for (size_t i = 0; i < 3; ++i) {
      state_.vow.formant_increment[i] = \
          static_cast<uint32_t>(consonant_data[index].formant_frequency[i]) * \
          0x1000 * formant_shift;
      state_.vow.formant_amplitude[i] = consonant_data[index].formant_amplitude[i];
    }
    state_.vow.noise = index >= 6 ? 4095 : 0;
  }
  
  if (state_.vow.consonant_frames) {
    --state_.vow.consonant_frames;
  } else {
    for (size_t i = 0; i < 3; ++i) {
      state_.vow.formant_increment[i] = 
          (vowels_data[vowel_index].formant_frequency[i] * (0x1000 - balance) + \
           vowels_data[vowel_index + 1].formant_frequency[i] * balance) * \
           formant_shift;
      state_.vow.formant_amplitude[i] =
          (vowels_data[vowel_index].formant_amplitude[i] * (0x1000 - balance) + \
           vowels_data[vowel_index + 1].formant_amplitude[i] * balance) >> 12;
    }
    state_.vow.noise = 0;
  }
  int32_t noise = state_.vow.noise;
  
  while (size--) {
    phase_ += phase_increment_;
    size_t phaselet;
    int16_t sample = 0;
    state_.vow.formant_phase[0] += state_.vow.formant_increment[0];
    phaselet = (state_.vow.formant_phase[0] >> 24) & 0xf0;
    sample += wav_formant_sine[phaselet | state_.vow.formant_amplitude[0]];

    state_.vow.formant_phase[1] += state_.vow.formant_increment[1];
    phaselet = (state_.vow.formant_phase[1] >> 24) & 0xf0;
    sample += wav_formant_sine[phaselet | state_.vow.formant_amplitude[1]];
    
    state_.vow.formant_phase[2] += state_.vow.formant_increment[2];
    phaselet = (state_.vow.formant_phase[2] >> 24) & 0xf0;
    sample += wav_formant_square[phaselet | state_.vow.formant_amplitude[2]];
    
    sample *= 255 - (phase_ >> 24);
    int32_t phase_noise = Random::GetSample() * noise;
    if ((phase_ + phase_noise) < phase_increment_) {
      state_.vow.formant_phase[0] = 0;
      state_.vow.formant_phase[1] = 0;
      state_.vow.formant_phase[2] = 0;
      sample = 0;
    }
    sample = Interpolate88(ws_moderate_overdrive, sample + 32768);
    *buffer++ = sample;
  }
}

const int16_t formant_f_data[kNumFormants][kNumFormants][kNumFormants] = {
  // bass
  {
    { 9519, 10738, 12448, 12636, 12892 }, // a
    { 8620, 11720, 12591, 12932, 13158 }, // e
    { 7579, 11891, 12768, 13122, 13323 }, // i
    { 8620, 10013, 12591, 12768, 13010 }, // o
    { 8324, 9519, 12591, 12831, 13048 } // u
  },
  // tenor
  {
    { 9696, 10821, 12810, 13010, 13263 }, // a
    { 8620, 11827, 12768, 13228, 13477 }, // e
    { 7908, 12038, 12932, 13263, 13452 }, // i
    { 8620, 10156, 12768, 12932, 13085 }, // o
    { 8324, 9519, 12852, 13010, 13296 } // u
  },
  // countertenor
  {
    { 9730, 10902, 12892, 13085, 13330 }, // a
    { 8832, 11953, 12852, 13085, 13296 }, // e
    { 7749, 12014, 13010, 13330, 13483 }, // i
    { 8781, 10211, 12852, 13085, 13296 }, // o
    { 8448, 9627, 12892, 13085, 13363 } // u
  },
  // alto
  {
    { 10156, 10960, 12932, 13427, 14195 }, // a
    { 8620, 11692, 12852, 13296, 14195 }, // e
    { 8324, 11827, 12852, 13550, 14195 }, // i
    { 8881, 10156, 12956, 13427, 14195 }, // o
    { 8160, 9860, 12708, 13427, 14195 } // u
  },
  // soprano
  {
    { 10156, 10960, 13010, 13667, 14195 }, // a
    { 8324, 12187, 12932, 13489, 14195 }, // e
    { 7749, 12337, 13048, 13667, 14195 }, // i
    { 8881, 10156, 12956, 13609, 14195 }, // o
    { 8160, 9860, 12852, 13609, 14195 } // u
  }
};

const int16_t formant_a_data[kNumFormants][kNumFormants][kNumFormants] = {
  // bass
  {
    { 16384, 7318, 5813, 5813, 1638 }, // a
    { 16384, 4115, 5813, 4115, 2062 }, // e
    { 16384, 518, 2596, 1301, 652 }, // i
    { 16384, 4617, 1460, 1638, 163 }, // o
    { 16384, 1638, 411, 652, 259 } // u
  },
  // tenor
  {
    { 16384, 8211, 7318, 6522, 1301 }, // a
    { 16384, 3269, 4115, 3269, 1638 }, // e
    { 16384, 2913, 2062, 1638, 518 }, // i
    { 16384, 5181, 4115, 4115, 821 }, // o
    { 16384, 1638, 2314, 3269, 821 } // u
  },
  // countertenor
  {
    { 16384, 8211, 1159, 1033, 206 }, // a
    { 16384, 3269, 2062, 1638, 1638 }, // e
    { 16384, 1033, 1033, 259, 259 }, // i
    { 16384, 5181, 821, 1301, 326 }, // o
    { 16384, 1638, 1159, 518, 326 } // u
  },
  // alto
  {
    { 16384, 10337, 1638, 259, 16 }, // a
    { 16384, 1033, 518, 291, 16 }, // e
    { 16384, 1638, 518, 259, 16 }, // i
    { 16384, 5813, 2596, 652, 29 }, // o
    { 16384, 4115, 518, 163, 10 } // u
  },
  // soprano
  {
    { 16384, 8211, 411, 1638, 51 }, // a
    { 16384, 1638, 2913, 163, 25 }, // e
    { 16384, 4115, 821, 821, 103 }, // i
    { 16384, 4617, 1301, 1301, 51 }, // o
    { 16384, 2596, 291, 163, 16 } // u
  }
};

int16_t InterpolateFormantParameter(
    const int16_t table[][kNumFormants][kNumFormants],
    int16_t x,
    int16_t y,
    uint8_t formant) {
  uint16_t x_index = x >> 13;
  uint16_t x_mix = x << 3;
  uint16_t y_index = y >> 13;
  uint16_t y_mix = y << 3;
  int16_t a = table[x_index][y_index][formant];
  int16_t b = table[x_index + 1][y_index][formant];
  int16_t c = table[x_index][y_index + 1][formant];
  int16_t d = table[x_index + 1][y_index + 1][formant];
  a = a + ((b - a) * x_mix >> 16);
  c = c + ((d - c) * x_mix >> 16);
  return a + ((c - a) * y_mix >> 16);
}

void RenderVowelFof(
  uint8_t sync,
  int16_t* buffer,
  size_t size) {
  
  uint16_t sine_gain = 0;
  if (pitch_ >= (72 << 7)) {
    uint32_t g = pitch_ - (72 << 7);
    g *= 24;
    if (g > 65535) {
      g = 65535;
    }
    sine_gain = g;
  }
  
  // This thing is running at SR / 2.
  phase_increment_ <<= 1;
  
  int16_t previous_sample = state_.fof.prevous_sample;
  while (size) {
    phase_ += phase_increment_;
    int32_t sample = 0;
    
    if (sine_gain != 65535) {
      for (size_t i = 0; i < kNumOverlappingFof; ++i) {
        if (state_.fof.envelope_phase[i] < 0x01000000) {
          Fof* f = state_.fof.fof[i];
          int32_t s;
          int32_t fof_set_sample = 0;
          f[0].phase += f[0].phase_increment;
          s = wav_sine[f[0].phase >> 24];
          fof_set_sample += s * f[0].amplitude >> 16;
        
          f[1].phase += f[1].phase_increment;
          s = wav_sine[f[1].phase >> 24];
          fof_set_sample += s * f[1].amplitude >> 16;

          f[2].phase += f[2].phase_increment;
          s = wav_sine[f[2].phase >> 24];
          fof_set_sample += s * f[2].amplitude >> 16;

          f[3].phase += f[3].phase_increment;
          s = wav_sine[f[3].phase >> 24];
          fof_set_sample += s * f[3].amplitude >> 16;

          f[4].phase += f[4].phase_increment;
          s = wav_sine[f[4].phase >> 24];
          fof_set_sample += s * f[4].amplitude >> 16;

          sample += fof_set_sample * \
              lut_fof_envelope[state_.fof.envelope_phase[i] >> 14] >> 16;
          state_.fof.envelope_phase[i] += \
              state_.fof.envelope_phase_increment[i];
        }
      }
      // Overlap a new set of grains.
      if (phase_ < phase_increment_) {
        size_t i = state_.fof.lru_fof;
        for (size_t j = 0; j < kNumFormants; ++j) {
          state_.fof.fof[i][j].phase_increment = ComputePhaseIncrement(
              InterpolateFormantParameter(
                  formant_f_data,
                  parameter_[1],
                  parameter_[0],
                  j)) << 1;
          state_.fof.fof[i][j].amplitude = InterpolateFormantParameter(
              formant_a_data,
              parameter_[1],
              parameter_[0],
              j);
          state_.fof.fof[i][j].phase = 8192;
        }
        state_.fof.envelope_phase[i] = 0;
        state_.fof.envelope_phase_increment[i] = 16384 + 8192;
        // Make sure that the envelope duration does not exceed N periods.
        // If this happens, this would cause a discontinuity as we only have
        // N overlapping FOFs.
        uint32_t period = phase_increment_ >> 8;
        uint32_t limit = period / kNumOverlappingFof;
        if (state_.fof.envelope_phase_increment[i] <= limit) {
          state_.fof.envelope_phase_increment[i] = limit - 1;
        }
        state_.fof.lru_fof = (i + 1) % kNumOverlappingFof;
      }
    }

    int16_t sine = Interpolate824(wav_sine, phase_) >> 1;
    sample = Interpolate88(ws_moderate_overdrive, sample + 32768);
    sample = Mix(sample, sine, sine_gain);
    
    *buffer++ = (previous_sample + sample) >> 1;
    *buffer++ = sample;
    previous_sample = sample;
    size -= 2;
  }
  state_.fof.prevous_sample = previous_sample;
}
