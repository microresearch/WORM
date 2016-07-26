#include "arm_math.h"
#include "arm_const_structs.h"
#include "effect.h"

// vosim//vowel/vowelfof

extern __IO uint16_t adc_buffer[10];

static uint32_t rng_state_;

#define kNumOverlappingFof 5 // was 3
#define kNumFormants 5
#define false 0

static const uint16_t kPitchTableStart = 128 * 128;
static const uint16_t kOctave = 12 * 128;

static inline uint32_t state() { return rng_state_; }

inline int16_t Mix(int16_t a, int16_t b, uint16_t balance) {
  return (a * (65535 - balance) + b * balance) >> 16;
}

static inline void Seed(uint16_t seed) {
    rng_state_ = seed;
  }

static inline uint32_t GetWord() {
    rng_state_ = rng_state_ * 1664525L + 1013904223L;
    return state();
  }
  
static inline int16_t GetSample() {
  return (int16_t)(GetWord() >> 16);
  }

static inline float GetFloat() {
  return (float)(GetWord()) / 4294967296.0f;
  }

//    sample += Interpolate824(wav_sine, vow.formant_phase[0]) >> 1; and lut bell=uint16_t

inline int16_t iInterpolate824(const int16_t* table, uint32_t phase) {
  int32_t a = table[phase >> 24];
  int32_t b = table[(phase >> 24) + 1];
  return a + ((b - a) * (int32_t)((phase >> 8) & 0xffff) >> 16);
}

/*
inline uint16_t uInterpolate824(const uint16_t* table, uint32_t phase) {
  uint32_t a = table[phase >> 24];
  uint32_t b = table[(phase >> 24) + 1];
  return a + ((b - a) * (uint32_t)((phase >> 8) & 0xffff) >> 16);
  }*/

//    sample = Interpolate88(ws_moderate_overdrive, sample + 32768);

inline int16_t Interpolate88(const int16_t* table, uint16_t index) {
  int32_t a = table[index >> 8];
  int32_t b = table[(index >> 8) + 1];
  return a + ((b - a) * (int32_t)(index & 0xff) >> 8);
}

const int16_t wav_formant_sine[]   __attribute__ ((section (".flash"))) = {
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,      2,      2,      3,
       3,      4,      5,      6,
       7,      8,     10,     12,
      14,     17,     20,     24,
       0,      3,      4,      5,
       6,      7,      9,     10,
      12,     15,     18,     21,
      26,     31,     37,     45,
       0,      4,      5,      6,
       8,      9,     11,     13,
      16,     19,     23,     28,
      34,     40,     49,     58,
       0,      5,      6,      7,
       8,     10,     12,     15,
      17,     21,     25,     30,
      36,     44,     53,     63,
       0,      4,      5,      6,
       8,      9,     11,     13,
      16,     19,     23,     28,
      34,     40,     49,     58,
       0,      3,      4,      5,
       6,      7,      9,     10,
      12,     15,     18,     21,
      26,     31,     37,     45,
       0,      2,      2,      3,
       3,      4,      5,      6,
       7,      8,     10,     12,
      14,     17,     20,     24,
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,      0,      0,      0,
       0,     -2,     -2,     -3,
      -3,     -4,     -5,     -6,
      -7,     -8,    -10,    -12,
     -14,    -17,    -20,    -24,
       0,     -3,     -4,     -5,
      -6,     -7,     -9,    -10,
     -12,    -15,    -18,    -21,
     -26,    -31,    -37,    -45,
       0,     -4,     -5,     -6,
      -8,     -9,    -11,    -13,
     -16,    -19,    -23,    -28,
     -34,    -40,    -49,    -58,
       0,     -5,     -6,     -7,
      -8,    -10,    -12,    -15,
     -17,    -21,    -25,    -30,
     -36,    -44,    -53,    -63,
       0,     -4,     -5,     -6,
      -8,     -9,    -11,    -13,
     -16,    -19,    -23,    -28,
     -34,    -40,    -49,    -58,
       0,     -3,     -4,     -5,
      -6,     -7,     -9,    -10,
     -12,    -15,    -18,    -21,
     -26,    -31,    -37,    -45,
       0,     -2,     -2,     -3,
      -3,     -4,     -5,     -6,
      -7,     -8,    -10,    -12,
     -14,    -17,    -20,    -24,
};
const int16_t wav_formant_square[]  __attribute__ ((section (".flash"))) = {
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,      1,      1,      2,
       2,      3,      3,      4,
       4,      5,      6,      8,
       9,     11,     13,     16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
       0,     -1,     -1,     -2,
      -2,     -3,     -3,     -4,
      -4,     -5,     -6,     -8,
      -9,    -11,    -13,    -16,
};


const int16_t wav_sine[]  __attribute__ ((section (".flash"))) = {
  -32512, -32502, -32473, -32423,
  -32356, -32265, -32160, -32031,
  -31885, -31719, -31533, -31331,
  -31106, -30864, -30605, -30324,
  -30028, -29712, -29379, -29026,
  -28658, -28272, -27868, -27449,
  -27011, -26558, -26089, -25604,
  -25103, -24588, -24056, -23512,
  -22953, -22378, -21793, -21191,
  -20579, -19954, -19316, -18667,
  -18006, -17334, -16654, -15960,
  -15259, -14548, -13828, -13100,
  -12363, -11620, -10868, -10112,
   -9347,  -8578,  -7805,  -7023,
   -6241,  -5453,  -4662,  -3868,
   -3073,  -2274,  -1474,   -674,
     126,    929,   1729,   2527,
    3326,   4123,   4916,   5707,
    6495,   7278,   8057,   8833,
    9601,  10366,  11122,  11874,
   12618,  13353,  14082,  14802,
   15512,  16216,  16906,  17589,
   18260,  18922,  19569,  20207,
   20834,  21446,  22045,  22634,
   23206,  23765,  24311,  24842,
   25357,  25858,  26343,  26812,
   27266,  27701,  28123,  28526,
   28912,  29281,  29632,  29966,
   30281,  30579,  30859,  31118,
   31361,  31583,  31788,  31973,
   32139,  32286,  32412,  32521,
   32608,  32679,  32725,  32757,
   32766,  32757,  32725,  32679,
   32608,  32521,  32412,  32286,
   32139,  31973,  31788,  31583,
   31361,  31118,  30859,  30579,
   30281,  29966,  29632,  29281,
   28912,  28526,  28123,  27701,
   27266,  26812,  26343,  25858,
   25357,  24842,  24311,  23765,
   23206,  22634,  22045,  21446,
   20834,  20207,  19569,  18922,
   18260,  17589,  16906,  16216,
   15512,  14802,  14082,  13353,
   12618,  11874,  11122,  10366,
    9601,   8833,   8057,   7278,
    6495,   5707,   4916,   4123,
    3326,   2527,   1729,    929,
     126,   -674,  -1474,  -2274,
   -3073,  -3868,  -4662,  -5453,
   -6241,  -7023,  -7805,  -8578,
   -9347, -10112, -10868, -11620,
  -12363, -13100, -13828, -14548,
  -15259, -15960, -16654, -17334,
  -18006, -18667, -19316, -19954,
  -20579, -21191, -21793, -22378,
  -22953, -23512, -24056, -24588,
  -25103, -25604, -26089, -26558,
  -27011, -27449, -27868, -28272,
  -28658, -29026, -29379, -29712,
  -30028, -30324, -30605, -30864,
  -31106, -31331, -31533, -31719,
  -31885, -32031, -32160, -32265,
  -32356, -32423, -32473, -32502,
  -32512,
};

const int16_t ws_moderate_overdrive[]   __attribute__ ((section (".flash"))) = {
  -32766, -32728, -32689, -32648,
  -32607, -32564, -32519, -32474,
  -32427, -32378, -32328, -32277,
  -32224, -32170, -32113, -32056,
  -31996, -31935, -31872, -31807,
  -31740, -31671, -31600, -31527,
  -31451, -31374, -31294, -31212,
  -31128, -31041, -30951, -30859,
  -30765, -30667, -30567, -30464,
  -30358, -30250, -30138, -30022,
  -29904, -29782, -29657, -29529,
  -29397, -29261, -29122, -28979,
  -28832, -28681, -28526, -28367,
  -28204, -28036, -27864, -27688,
  -27507, -27321, -27131, -26936,
  -26736, -26531, -26321, -26106,
  -25886, -25660, -25429, -25192,
  -24950, -24702, -24449, -24190,
  -23925, -23654, -23377, -23094,
  -22805, -22510, -22209, -21902,
  -21588, -21268, -20942, -20609,
  -20270, -19924, -19573, -19215,
  -18850, -18479, -18102, -17718,
  -17328, -16932, -16530, -16121,
  -15707, -15286, -14859, -14427,
  -13989, -13545, -13095, -12640,
  -12180, -11715, -11244, -10769,
  -10289,  -9804,  -9315,  -8822,
   -8324,  -7823,  -7319,  -6810,
   -6299,  -5785,  -5268,  -4748,
   -4226,  -3703,  -3177,  -2650,
   -2121,  -1592,  -1062,   -531,
       0,    531,   1062,   1592,
    2122,   2650,   3177,   3703,
    4227,   4749,   5268,   5785,
    6299,   6811,   7319,   7824,
    8325,   8822,   9315,   9804,
   10289,  10769,  11244,  11715,
   12180,  12641,  13095,  13545,
   13989,  14427,  14860,  15286,
   15707,  16122,  16530,  16933,
   17329,  17719,  18102,  18479,
   18850,  19215,  19573,  19925,
   20270,  20609,  20942,  21268,
   21588,  21902,  22209,  22510,
   22806,  23094,  23377,  23654,
   23925,  24190,  24449,  24703,
   24950,  25192,  25429,  25660,
   25886,  26106,  26321,  26531,
   26736,  26936,  27131,  27322,
   27507,  27688,  27865,  28037,
   28204,  28367,  28526,  28681,
   28832,  28979,  29122,  29262,
   29397,  29529,  29658,  29783,
   29904,  30023,  30138,  30250,
   30359,  30465,  30568,  30668,
   30765,  30860,  30952,  31041,
   31128,  31212,  31294,  31374,
   31452,  31527,  31600,  31671,
   31740,  31807,  31872,  31935,
   31996,  32056,  32114,  32170,
   32224,  32277,  32329,  32379,
   32427,  32474,  32520,  32564,
   32607,  32648,  32689,  32728,
   32728,
};

const uint32_t lut_oscillator_increments[]   __attribute__ ((section (".flash"))) = {
  594573364, 598881888, 603221633, 607592826,
  611995694, 616430467, 620897376, 625396654,
  629928536, 634493258, 639091058, 643722175,
  648386851, 653085330, 657817855, 662584675,
  667386036, 672222191, 677093390, 681999888,
  686941940, 691919804, 696933740, 701984010,
  707070875, 712194602, 717355458, 722553711,
  727789633, 733063497, 738375577, 743726151,
  749115497, 754543897, 760011633, 765518991,
  771066257, 776653721, 782281674, 787950409,
  793660223, 799411412, 805204277, 811039119,
  816916243, 822835954, 828798563, 834804379,
  840853716, 846946888, 853084215, 859266014,
  865492610, 871764326, 878081490, 884444431,
  890853479, 897308971, 903811242, 910360631,
  916957479, 923602131, 930294933, 937036233,
  943826384, 950665739, 957554655, 964493491,
  971482608, 978522372, 985613148, 992755307,
  999949221, 1007195266, 1014493818, 1021845258,
  1029249970, 1036708340, 1044220756, 1051787610,
  1059409296, 1067086213, 1074818759, 1082607339,
  1090452358, 1098354226, 1106313353, 1114330156,
  1122405051, 1130538461, 1138730809, 1146982522,
  1155294030, 1163665767, 1172098168, 1180591675,
  1189146729,
};

const uint16_t lut_bell[]   __attribute__ ((section (".flash"))) = {
       0,    670,   2655,   5873,
   10191,  15434,  21387,  27805,
   34427,  40980,  47198,  52824,
   57630,  61417,  64032,  65366,
   65534,  65528,  65517,  65500,
   65477,  65449,  65415,  65376,
   65331,  65280,  65224,  65162,
   65095,  65022,  64944,  64860,
   64770,  64675,  64574,  64468,
   64357,  64240,  64118,  63990,
   63857,  63718,  63575,  63426,
   63271,  63112,  62947,  62777,
   62602,  62421,  62236,  62046,
   61850,  61650,  61444,  61234,
   61018,  60798,  60573,  60343,
   60109,  59870,  59626,  59377,
   59124,  58866,  58604,  58338,
   58067,  57791,  57512,  57228,
   56940,  56648,  56351,  56051,
   55746,  55438,  55126,  54810,
   54490,  54166,  53839,  53508,
   53173,  52835,  52494,  52149,
   51801,  51449,  51094,  50736,
   50375,  50011,  49645,  49275,
   48902,  48526,  48148,  47767,
   47384,  46998,  46610,  46219,
   45826,  45431,  45033,  44633,
   44232,  43828,  43423,  43015,
   42606,  42195,  41783,  41369,
   40953,  40537,  40118,  39699,
   39278,  38856,  38433,  38010,
   37585,  37159,  36733,  36306,
   35879,  35450,  35022,  34593,
   34163,  33734,  33304,  32874,
   32445,  32015,  31585,  31156,
   30727,  30298,  29869,  29442,
   29014,  28588,  28162,  27737,
   27312,  26889,  26467,  26045,
   25625,  25206,  24789,  24373,
   23958,  23545,  23133,  22723,
   22315,  21908,  21504,  21101,
   20700,  20302,  19905,  19511,
   19119,  18730,  18343,  17958,
   17576,  17196,  16819,  16445,
   16074,  15706,  15340,  14978,
   14618,  14262,  13909,  13559,
   13212,  12869,  12529,  12193,
   11860,  11531,  11206,  10884,
   10566,  10252,   9941,   9635,
    9332,   9034,   8740,   8450,
    8164,   7882,   7604,   7331,
    7062,   6798,   6538,   6283,
    6032,   5786,   5544,   5307,
    5075,   4848,   4625,   4407,
    4195,   3987,   3784,   3586,
    3393,   3205,   3022,   2844,
    2671,   2504,   2342,   2185,
    2033,   1887,   1746,   1610,
    1479,   1354,   1235,   1121,
    1012,    909,    811,    719,
     632,    550,    475,    405,
     340,    281,    228,    180,
     138,    101,     70,     45,
      25,     11,      2,      0,
       0,
};
const uint16_t lut_fof_envelope[]  __attribute__ ((section (".flash"))) = {
       0,      9,     39,     89,
     159,    248,    357,    486,
     634,    802,    989,   1196,
    1421,   1666,   1930,   2212,
    2513,   2832,   3170,   3525,
    3898,   4289,   4697,   5122,
    5564,   6022,   6497,   6987,
    7493,   8015,   8552,   9103,
    9668,  10248,  10841,  11448,
   12067,  12700,  13344,  14000,
   14667,  15346,  16035,  16734,
   17443,  18161,  18888,  19624,
   20367,  21118,  21877,  22641,
   23412,  24189,  24970,  25757,
   26548,  27342,  28140,  28941,
   29744,  30548,  31355,  32161,
   32969,  33776,  34583,  35388,
   36192,  36994,  37793,  38589,
   39382,  40171,  40955,  41734,
   42508,  43276,  44037,  44792,
   45539,  46279,  47010,  47733,
   48447,  49151,  49845,  50529,
   51202,  51864,  52514,  53152,
   53778,  54391,  54991,  55578,
   56150,  56709,  57253,  57782,
   58296,  58794,  59276,  59743,
   60193,  60626,  61043,  61442,
   61824,  62189,  62535,  62864,
   63174,  63465,  63738,  63993,
   64228,  64444,  64641,  64818,
   64977,  65115,  65234,  65333,
   65413,  65472,  65512,  65532,
   65534,  65534,  65533,  65532,
   65530,  65528,  65526,  65523,
   65520,  65516,  65512,  65508,
   65503,  65498,  65492,  65486,
   65480,  65473,  65466,  65458,
   65450,  65441,  65432,  65423,
   65414,  65403,  65393,  65382,
   65371,  65359,  65347,  65335,
   65322,  65308,  65295,  65281,
   65266,  65251,  65236,  65220,
   65204,  65188,  65171,  65154,
   65136,  65118,  65099,  65081,
   65061,  65042,  65022,  65001,
   64980,  64959,  64937,  64915,
   64893,  64870,  64847,  64823,
   64799,  64775,  64750,  64725,
   64699,  64673,  64647,  64620,
   64593,  64565,  64537,  64509,
   64480,  64451,  64422,  64392,
   64362,  64331,  64300,  64268,
   64236,  64204,  64172,  64139,
   64105,  64071,  64037,  64003,
   63968,  63933,  63897,  63861,
   63824,  63787,  63750,  63713,
   63675,  63636,  63598,  63558,
   63519,  63479,  63439,  63398,
   63357,  63316,  63274,  63232,
   63189,  63146,  63103,  63059,
   63015,  62971,  62926,  62881,
   62836,  62790,  62744,  62697,
   62650,  62603,  62555,  62507,
   62458,  62410,  62360,  62311,
   62261,  62211,  62160,  62109,
   62058,  62006,  61954,  61901,
   61849,  61796,  61742,  61688,
   61634,  61579,  61524,  61469,
   61414,  61358,  61301,  61245,
   61187,  61130,  61072,  61014,
   60956,  60897,  60838,  60778,
   60719,  60658,  60598,  60537,
   60476,  60414,  60353,  60290,
   60228,  60165,  60102,  60038,
   59974,  59910,  59845,  59781,
   59715,  59650,  59584,  59518,
   59451,  59384,  59317,  59249,
   59182,  59113,  59045,  58976,
   58907,  58837,  58768,  58697,
   58627,  58556,  58485,  58414,
   58342,  58270,  58198,  58125,
   58052,  57979,  57905,  57831,
   57757,  57683,  57608,  57533,
   57457,  57382,  57306,  57229,
   57153,  57076,  56998,  56921,
   56843,  56765,  56687,  56608,
   56529,  56449,  56370,  56290,
   56210,  56129,  56049,  55968,
   55886,  55805,  55723,  55641,
   55558,  55475,  55392,  55309,
   55226,  55142,  55058,  54973,
   54889,  54804,  54718,  54633,
   54547,  54461,  54375,  54288,
   54201,  54114,  54027,  53939,
   53852,  53763,  53675,  53586,
   53498,  53408,  53319,  53229,
   53139,  53049,  52959,  52868,
   52777,  52686,  52595,  52503,
   52411,  52319,  52227,  52134,
   52041,  51948,  51855,  51761,
   51667,  51573,  51479,  51385,
   51290,  51195,  51100,  51004,
   50909,  50813,  50717,  50621,
   50524,  50427,  50331,  50233,
   50136,  50038,  49941,  49843,
   49744,  49646,  49547,  49448,
   49349,  49250,  49151,  49051,
   48951,  48851,  48751,  48650,
   48550,  48449,  48348,  48247,
   48145,  48044,  47942,  47840,
   47738,  47635,  47533,  47430,
   47327,  47224,  47121,  47018,
   46914,  46810,  46706,  46602,
   46498,  46393,  46289,  46184,
   46079,  45974,  45869,  45763,
   45658,  45552,  45446,  45340,
   45234,  45127,  45021,  44914,
   44807,  44700,  44593,  44486,
   44378,  44271,  44163,  44055,
   43947,  43839,  43731,  43622,
   43514,  43405,  43296,  43187,
   43078,  42969,  42860,  42750,
   42641,  42531,  42421,  42312,
   42202,  42091,  41981,  41871,
   41760,  41650,  41539,  41428,
   41317,  41206,  41095,  40984,
   40873,  40761,  40650,  40538,
   40426,  40314,  40202,  40090,
   39978,  39866,  39754,  39642,
   39529,  39417,  39304,  39191,
   39079,  38966,  38853,  38740,
   38627,  38514,  38400,  38287,
   38174,  38060,  37947,  37833,
   37720,  37606,  37492,  37379,
   37265,  37151,  37037,  36923,
   36809,  36695,  36581,  36466,
   36352,  36238,  36124,  36009,
   35895,  35780,  35666,  35551,
   35437,  35322,  35208,  35093,
   34978,  34863,  34749,  34634,
   34519,  34404,  34290,  34175,
   34060,  33945,  33830,  33715,
   33600,  33485,  33370,  33256,
   33141,  33026,  32911,  32796,
   32681,  32566,  32451,  32336,
   32221,  32106,  31991,  31876,
   31761,  31646,  31532,  31417,
   31302,  31187,  31072,  30957,
   30843,  30728,  30613,  30498,
   30384,  30269,  30155,  30040,
   29925,  29811,  29696,  29582,
   29468,  29353,  29239,  29125,
   29011,  28896,  28782,  28668,
   28554,  28440,  28326,  28212,
   28099,  27985,  27871,  27757,
   27644,  27530,  27417,  27304,
   27190,  27077,  26964,  26851,
   26738,  26625,  26512,  26399,
   26286,  26174,  26061,  25949,
   25836,  25724,  25612,  25500,
   25388,  25276,  25164,  25052,
   24940,  24829,  24717,  24606,
   24495,  24383,  24272,  24161,
   24050,  23940,  23829,  23718,
   23608,  23498,  23388,  23277,
   23167,  23058,  22948,  22838,
   22729,  22619,  22510,  22401,
   22292,  22183,  22074,  21966,
   21857,  21749,  21641,  21533,
   21425,  21317,  21210,  21102,
   20995,  20887,  20780,  20673,
   20567,  20460,  20354,  20247,
   20141,  20035,  19929,  19824,
   19718,  19613,  19508,  19403,
   19298,  19193,  19088,  18984,
   18880,  18776,  18672,  18568,
   18465,  18361,  18258,  18155,
   18052,  17950,  17847,  17745,
   17643,  17541,  17439,  17338,
   17237,  17135,  17035,  16934,
   16833,  16733,  16633,  16533,
   16433,  16333,  16234,  16135,
   16036,  15937,  15839,  15740,
   15642,  15544,  15447,  15349,
   15252,  15155,  15058,  14962,
   14865,  14769,  14673,  14577,
   14482,  14387,  14291,  14197,
   14102,  14008,  13914,  13820,
   13726,  13633,  13539,  13446,
   13354,  13261,  13169,  13077,
   12985,  12894,  12802,  12711,
   12620,  12530,  12440,  12350,
   12260,  12170,  12081,  11992,
   11903,  11815,  11726,  11638,
   11551,  11463,  11376,  11289,
   11202,  11116,  11030,  10944,
   10858,  10773,  10688,  10603,
   10519,  10434,  10350,  10267,
   10183,  10100,  10017,   9935,
    9852,   9770,   9689,   9607,
    9526,   9445,   9364,   9284,
    9204,   9124,   9045,   8966,
    8887,   8808,   8730,   8652,
    8574,   8497,   8420,   8343,
    8267,   8190,   8115,   8039,
    7964,   7889,   7814,   7740,
    7666,   7592,   7518,   7445,
    7373,   7300,   7228,   7156,
    7084,   7013,   6942,   6872,
    6801,   6731,   6662,   6593,
    6524,   6455,   6387,   6318,
    6251,   6183,   6116,   6050,
    5983,   5917,   5851,   5786,
    5721,   5656,   5592,   5528,
    5464,   5401,   5338,   5275,
    5213,   5150,   5089,   5027,
    4966,   4906,   4845,   4785,
    4726,   4666,   4608,   4549,
    4491,   4433,   4375,   4318,
    4261,   4205,   4148,   4093,
    4037,   3982,   3927,   3873,
    3819,   3765,   3712,   3659,
    3606,   3554,   3502,   3451,
    3399,   3349,   3298,   3248,
    3198,   3149,   3100,   3051,
    3003,   2955,   2908,   2861,
    2814,   2767,   2721,   2676,
    2630,   2585,   2541,   2497,
    2453,   2409,   2366,   2323,
    2281,   2239,   2198,   2156,
    2116,   2075,   2035,   1995,
    1956,   1917,   1879,   1840,
    1803,   1765,   1728,   1691,
    1655,   1619,   1584,   1549,
    1514,   1480,   1446,   1412,
    1379,   1346,   1314,   1282,
    1250,   1219,   1188,   1157,
    1127,   1098,   1068,   1039,
    1011,    983,    955,    927,
     901,    874,    848,    822,
     797,    772,    747,    723,
     699,    675,    652,    630,
     608,    586,    564,    543,
     523,    502,    482,    463,
     444,    425,    407,    389,
     372,    355,    338,    322,
     306,    290,    275,    260,
     246,    232,    219,    206,
     193,    181,    169,    157,
     146,    136,    125,    116,
     106,     97,     88,     80,
      72,     65,     58,     51,
      45,     39,     34,     29,
      24,     20,     16,     12,
       9,      7,      5,      3,
       1,      0,      0,      0,
       0,
};

typedef struct {
  uint32_t formant_increment[3];
  uint32_t formant_phase[3];
  uint32_t formant_amplitude[3];
  uint16_t consonant_frames;
  uint16_t noise;
} VowelSynthesizerState;

typedef struct {
  uint32_t phase;
  uint32_t phase_increment;
  uint16_t amplitude;
} Fof;

typedef struct {
  Fof fof[kNumOverlappingFof][kNumFormants];
  uint32_t envelope_phase[kNumOverlappingFof];
  uint32_t envelope_phase_increment[kNumOverlappingFof];
  uint8_t lru_fof;
  int16_t prevous_sample;
}  FofState;

static uint32_t phase_ = 0;

//int16_t parameter_[2]; // external parameters
static int16_t previous_parameter_[2];
static int32_t smoothed_parameter_;


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
      ((int32_t)(b - a) * (ref_pitch & 0xf) >> 4);
  phase_increment >>= num_shifts;
  return phase_increment;
}

///////////////////////////////////

static u8 strike_;

VowelSynthesizerState vow; // do these have memory?
FofState fof;

void initbraidworm(){
  rng_state_ = 0x21;

  // rng seed?
    phase_ = 0;
    strike_ = 1;

}


void RenderVosim( // 2 vosims
		 int16_t* sync, 
		 int16_t* buffer,
		 u8 size, u16 param1,u16 param2, int16_t pitch_) {

  uint32_t phase_increment_;

  phase_increment_ = ComputePhaseIncrement(pitch_);

  //  for (size_t i = 0; i < 2; ++i) {
    vow.formant_increment[0] = ComputePhaseIncrement(param1 >> 1);
    vow.formant_increment[1] = ComputePhaseIncrement(param2 >> 1);
    //  }
    static int16_t oldsync;

  while (size--) {
    phase_ += phase_increment_;

            if (*sync>0 && oldsync<0) { // primitive TODO
          phase_ = 0;
        }
	oldsync=*sync; sync++;

    int32_t sample = 16384 + 8192; // subtract below
    vow.formant_phase[0] += vow.formant_increment[0];
    sample += iInterpolate824(wav_sine, vow.formant_phase[0]) >> 1;
    
    vow.formant_phase[1] += vow.formant_increment[1];
    sample += iInterpolate824(wav_sine, vow.formant_phase[1]) >> 2; // was >> 2????
    
    sample = sample * (iInterpolate824(lut_bell, phase_) >> 1) >> 15;
    if (phase_ < phase_increment_) {
      vow.formant_phase[0] = 0;
      vow.formant_phase[1] = 0;
      sample = 0;
    }
    sample -= 16384 + 8192;
    *buffer++ = sample;
  }
}

typedef struct {
  uint8_t formant_frequency[3];
  uint8_t formant_amplitude[3];
} PhonemeDefinition;

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
    int16_t* sync,
    int16_t* buffer,
    size_t size, u16 param1,u16 param2,u16 param3, int16_t pitch_) { // sync is unused?
  uint32_t phase_increment_;

  phase_increment_ = ComputePhaseIncrement(pitch_);
    static int16_t oldsync;


  size_t vowel_index = param1 >> 12; // 4 bits?
  uint16_t balance = param2 & 0x0fff; // 4096
  uint16_t formant_shift = (200 + (param3)); // as 10 bits
  if (*sync>0 && oldsync<0) { // primitive TODO -> implement as new_say!
    //  if (strike_) { /// strike is trigger!
    //    strike_ = false;
    /*    vow.consonant_frames = 160;
    uint16_t index = (GetSample() + 1) & 7; // random index?
        for (size_t i = 0; i < 3; ++i) {
      vow.formant_increment[i] = \
	(uint32_t)(consonant_data[index].formant_frequency[i]) *	\
          0x1000 * formant_shift;
      vow.formant_amplitude[i] = consonant_data[index].formant_amplitude[i];
    }
    vow.noise = index >= 6 ? 4095 : 0;*/
  }
  /*	oldsync=*sync; sync++;
  if (vow.consonant_frames) {
    --vow.consonant_frames;
    } else {*/
    for (size_t i = 0; i < 3; ++i) {
      vow.formant_increment[i] = 
          (vowels_data[vowel_index].formant_frequency[i] * (0x1000 - balance) + \
           vowels_data[vowel_index + 1].formant_frequency[i] * balance) * \
           formant_shift;
      vow.formant_amplitude[i] =
          (vowels_data[vowel_index].formant_amplitude[i] * (0x1000 - balance) + \
           vowels_data[vowel_index + 1].formant_amplitude[i] * balance) >> 12;
    }
    vow.noise = 0;
    //  }
  int32_t noise = vow.noise;
  
  while (size--) {
    phase_ += phase_increment_;
    size_t phaselet;
    int16_t sample = 0;
    vow.formant_phase[0] += vow.formant_increment[0];
    phaselet = (vow.formant_phase[0] >> 24) & 0xf0;
    sample += wav_formant_sine[phaselet | vow.formant_amplitude[0]];

    vow.formant_phase[1] += vow.formant_increment[1];
    phaselet = (vow.formant_phase[1] >> 24) & 0xf0;
    sample += wav_formant_sine[phaselet | vow.formant_amplitude[1]];
    
    vow.formant_phase[2] += vow.formant_increment[2];
    phaselet = (vow.formant_phase[2] >> 24) & 0xf0;
    sample += wav_formant_square[phaselet | vow.formant_amplitude[2]];
    
    sample *= 255 - (phase_ >> 24);
    int32_t phase_noise = GetSample() * noise;
    if ((phase_ + phase_noise) < phase_increment_) {
      vow.formant_phase[0] = 0;
      vow.formant_phase[1] = 0;
      vow.formant_phase[2] = 0;
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

int16_t fof_get_sample(){ // this is rendervowelfof
  uint8_t sync;
  u16 param1=adc_buffer[SELX]<<3; 
  u16 param2=adc_buffer[SELY]<<3; 
  int16_t pitch_=adc_buffer[SELZ]<<1;
  static u8 flagger=0;
  uint32_t phase_increment_;

  if (flagger==1){
    flagger=0;
    return fof.prevous_sample;
  }

  phase_increment_ = ComputePhaseIncrement(pitch_);
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
  
  int16_t previous_sample = fof.prevous_sample;
  //  while (size) {
    phase_ += phase_increment_;
    int32_t sample = 0;
    
    if (sine_gain != 65535) {
      for (size_t i = 0; i < kNumOverlappingFof; ++i) {
        if (fof.envelope_phase[i] < 0x01000000) {
          Fof* f = fof.fof[i];
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
              lut_fof_envelope[fof.envelope_phase[i] >> 14] >> 16;
          fof.envelope_phase[i] += \
              fof.envelope_phase_increment[i];
        }
      }
      // Overlap a new set of grains.
      if (phase_ < phase_increment_) {
        size_t i = fof.lru_fof;
        for (size_t j = 0; j < kNumFormants; ++j) {
          fof.fof[i][j].phase_increment = ComputePhaseIncrement(
              InterpolateFormantParameter(
                  formant_f_data,
                  param2,
                  param1,
                  j)) << 1;
          fof.fof[i][j].amplitude = InterpolateFormantParameter(
              formant_a_data,
              param2,
              param1,
              j);
          fof.fof[i][j].phase = 8192;
        }
        fof.envelope_phase[i] = 0;
        fof.envelope_phase_increment[i] = 16384 + 8192;
        // Make sure that the envelope duration does not exceed N periods.
        // If this happens, this would cause a discontinuity as we only have
        // N overlapping FOFs.
        uint32_t period = phase_increment_ >> 8;
        uint32_t limit = period / kNumOverlappingFof;
        if (fof.envelope_phase_increment[i] <= limit) {
          fof.envelope_phase_increment[i] = limit - 1;
        }
        fof.lru_fof = (i + 1) % kNumOverlappingFof;
      }
    }

    int16_t sine = iInterpolate824(wav_sine, phase_) >> 1;
    sample = Interpolate88(ws_moderate_overdrive, sample + 32768);
    sample = Mix(sample, sine, sine_gain);

    // here there are two samples:
    flagger=1;
    fof.prevous_sample = sample;
    return ((previous_sample + sample) >> 1);
    //    *buffer++ = (previous_sample + sample) >> 1;
    //    *buffer++ = sample;
    //    previous_sample = sample;
    //    size -= 2;
    //  }
    //  fof.prevous_sample = previous_sample;
}

