// perry cook srconvert.c:

#include "audio.h"
#include "nvp.h"


#define TRUE 1
#define WIDTH 16                /* this controls the number of neighboring samples
				   which are used to interpolate the new samples.  The
				   processing time is linearly related to this width */
#define DELAY_SIZE 40

#define USE_TABLE 1          /* this controls whether a linearly interpolated lookup
				   table is used for sinc function calculation, or the
				   sinc is calculated by floating point trig function calls.  */

#define SAMPLES_PER_ZERO_CROSSING 8   /* this defines how finely the sinc function 
					   is sampled for storage in the table  */

//float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING] = { 0.0 }; // TODO: if we use table then store as const
//float sinc_table[1] = { 0.0 };

extern __IO uint16_t adc_buffer[10];
extern float _selx, _sely, _selz;
extern float smoothed_adc_value[5];

inline void doadc(){
  float value;
  
  value =(float)adc_buffer[SELX]/65536.0f; 
  smoothed_adc_value[2] += 0.1f * (value - smoothed_adc_value[2]);
  _selx=smoothed_adc_value[2];
  CONSTRAIN(_selx,0.0f,1.0f);

  value =(float)adc_buffer[SELY]/65536.0f; 
  smoothed_adc_value[3] += 0.1f * (value - smoothed_adc_value[3]);
  _sely=smoothed_adc_value[3];
  CONSTRAIN(_sely,0.0f,1.0f);

  value =(float)adc_buffer[SELZ]/65536.0f; 
  smoothed_adc_value[4] += 0.1f * (value - smoothed_adc_value[4]);
  _selz=smoothed_adc_value[4];
  CONSTRAIN(_selz,0.0f,1.0f);
}


// this is for width 16, crossing 8

const float sinc_table[]={0.974349, 0.899774, 0.783151, 0.635087, 0.468759, 0.298481, 0.138189, -0.000000, -0.106962, -0.177365, -0.210003, -0.207638, -0.176405, -0.124857, -0.062790, 0.000000, 0.054864, 0.095233, 0.117213, 0.119807, 0.104753, 0.076025, 0.039083, -0.000000, -0.035425, -0.062441, -0.077914, -0.080624, -0.071277, -0.052247, -0.027101, 0.000000, 0.024946, 0.044263, 0.055562, 0.057805, 0.051352, 0.037806, 0.019687, 0.000000, -0.018242, -0.032456, -0.040840, -0.042578, -0.037893, -0.027940, -0.014568, 0.000000, 0.013523, 0.024072, 0.030300, 0.031593, 0.028114, 0.020724, 0.010800, 0.000000, -0.010010, -0.017800, -0.022378, -0.023301, -0.020703, -0.015234, -0.007924, 0.000000, 0.007312, 0.012972, 0.016265, 0.016889, 0.014961, 0.010974, 0.005689, -0.000000, -0.005212, -0.009210, -0.011501, -0.011890, -0.010485, -0.007654, -0.003948, -0.000000, 0.003578, 0.006285, 0.007800, 0.008012, 0.007018, 0.005087, 0.002604, 0.000000, -0.002322, -0.004044, -0.004974, -0.005060, -0.004387, -0.003146, -0.001592, 0.000000, 0.001385, 0.002380, 0.002885, 0.002890, 0.002465, 0.001737, 0.000863, -0.000000, -0.000720, -0.001208, -0.001428, -0.001392, -0.001152, -0.000786, -0.000377, -0.000000, 0.000289, 0.000462, 0.000516, 0.000473, 0.000364, 0.000229, 0.000099, -0.000000, -0.000059, -0.000080, -0.000072, -0.000049, -0.000025, -0.000009, -0.000001};

// this is generated by test.c using srconvrt.c with width 64 and zero cross as 32

//const float sinc_table[]={0.998394, 0.993585, 0.985600, 0.974486, 0.960307, 0.943145, 0.923099, 0.900282, 0.874827, 0.846878, 0.816594, 0.784147, 0.749719, 0.713503, 0.675702, 0.636524, 0.596185, 0.554905, 0.512908, 0.470417, 0.427659, 0.384858, 0.342233, 0.300004, 0.258380, 0.217567, 0.177761, 0.139149, 0.101909, 0.066204, 0.032188, 0.000000, -0.030235, -0.058407, -0.084420, -0.108195, -0.129669, -0.148794, -0.165541, -0.179894, -0.191855, -0.201440, -0.208684, -0.213633, -0.216349, -0.216908, -0.215398, -0.211919, -0.206583, -0.199510, -0.190832, -0.180685, -0.169214, -0.156570, -0.142906, -0.128379, -0.113150, -0.097376, -0.081217, -0.064829, -0.048366, -0.031979, -0.015811, -0.000000, 0.015322, 0.030032, 0.044015, 0.057167, 0.069394, 0.080610, 0.090743, 0.099730, 0.107523, 0.114081, 0.119380, 0.123403, 0.126147, 0.127622, 0.127844, 0.126845, 0.124664, 0.121350, 0.116962, 0.111566, 0.105236, 0.098052, 0.090101, 0.081475, 0.072268, 0.062579, 0.052508, 0.042159, 0.031632, 0.021030, 0.010454, 0.000000, -0.010236, -0.020163, -0.029695, -0.038751, -0.047256, -0.055141, -0.062346, -0.068815, -0.074503, -0.079372, -0.083391, -0.086538, -0.088801, -0.090175, -0.090663, -0.090276, -0.089035, -0.086965, -0.084102, -0.080485, -0.076163, -0.071187, -0.065617, -0.059514, -0.052945, -0.045980, -0.038691, -0.031152, -0.023438, -0.015624, -0.007787, -0.000000, 0.007664, 0.015134, 0.022344, 0.029229, 0.035728, 0.041787, 0.047354, 0.052386, 0.056841, 0.060687, 0.063895, 0.066446, 0.068325, 0.069522, 0.070038, 0.069876, 0.069048, 0.067572, 0.065469, 0.062769, 0.059506, 0.055718, 0.051448, 0.046744, 0.041656, 0.036236, 0.030542, 0.024631, 0.018561, 0.012393, 0.006186, 0.000000, -0.006107, -0.012078, -0.017858, -0.023394, -0.028637, -0.033541, -0.038063, -0.042164, -0.045812, -0.048977, -0.051635, -0.053766, -0.055357, -0.056398, -0.056888, -0.056826, -0.056221, -0.055085, -0.053434, -0.051291, -0.048680, -0.045634, -0.042184, -0.038370, -0.034231, -0.029810, -0.025152, -0.020306, -0.015318, -0.010238, -0.005116, -0.000000, 0.005060, 0.010018, 0.014826, 0.019442, 0.023821, 0.027927, 0.031721, 0.035172, 0.038250, 0.040929, 0.043188, 0.045010, 0.046382, 0.047295, 0.047746, 0.047735, 0.047266, 0.046349, 0.044996, 0.043226, 0.041059, 0.038519, 0.035636, 0.032438, 0.028961, 0.025240, 0.021312, 0.017218, 0.012999, 0.008694, 0.004347, 0.000000, -0.004306, -0.008531, -0.012635, -0.016579, -0.020327, -0.023847, -0.027105, -0.030073, -0.032725, -0.035040, -0.036998, -0.038583, -0.039784, -0.040592, -0.041004, -0.041019, -0.040641, -0.039876, -0.038735, -0.037233, -0.035386, -0.033217, -0.030747, -0.028004, -0.025016, -0.021814, -0.018430, -0.014897, -0.011252, -0.007530, -0.003767, -0.000000, 0.003736, 0.007405, 0.010972, 0.014404, 0.017670, 0.020739, 0.023584, 0.026179, 0.028502, 0.030532, 0.032253, 0.033651, 0.034715, 0.035436, 0.035813, 0.035842, 0.035527, 0.034874, 0.033891, 0.032591, 0.030988, 0.029101, 0.026949, 0.024555, 0.021944, 0.019143, 0.016180, 0.013084, 0.009887, 0.006619, 0.003313, 0.000000, -0.003288, -0.006519, -0.009663, -0.012691, -0.015574, -0.018286, -0.020802, -0.023100, -0.025159, -0.026961, -0.028491, -0.029737, -0.030688, -0.031337, -0.031680, -0.031717, -0.031450, -0.030882, -0.030022, -0.028880, -0.027469, -0.025805, -0.023904, -0.021788, -0.019478, -0.016997, -0.014371, -0.011625, -0.008787, -0.005885, -0.002946, -0.000000, 0.002926, 0.005803, 0.008604, 0.011303, 0.013875, 0.016296, 0.018544, 0.020598, 0.022441, 0.024055, 0.025428, 0.026546, 0.027403, 0.027990, 0.028305, 0.028346, 0.028114, 0.027614, 0.026853, 0.025838, 0.024582, 0.023099, 0.021403, 0.019514, 0.017449, 0.015230, 0.012880, 0.010422, 0.007880, 0.005278, 0.002643, 0.000000, -0.002626, -0.005210, -0.007727, -0.010153, -0.012466, -0.014645, -0.016669, -0.018520, -0.020181, -0.021638, -0.022878, -0.023890, -0.024666, -0.025201, -0.025490, -0.025532, -0.025329, -0.024884, -0.024203, -0.023293, -0.022166, -0.020832, -0.019307, -0.017606, -0.015747, -0.013747, -0.011628, -0.009411, -0.007117, -0.004768, -0.002388, -0.000000, 0.002374, 0.004710, 0.006987, 0.009183, 0.011277, 0.013250, 0.015084, 0.016762, 0.018269, 0.019592, 0.020718, 0.021638, 0.022345, 0.022834, 0.023100, 0.023142, 0.022962, 0.022563, 0.021949, 0.021128, 0.020108, 0.018902, 0.017521, 0.015980, 0.014295, 0.012482, 0.010560, 0.008547, 0.006465, 0.004332, 0.002170, 0.000000, -0.002158, -0.004282, -0.006353, -0.008351, -0.010257, -0.012053, -0.013724, -0.015253, -0.016627, -0.017833, -0.018861, -0.019702, -0.020349, -0.020796, -0.021042, -0.021083, -0.020922, -0.020561, -0.020004, -0.019259, -0.018332, -0.017235, -0.015978, -0.014575, -0.013039, -0.011387, -0.009635, -0.007800, -0.005900, -0.003954, -0.001981, -0.000000, 0.001970, 0.003910, 0.005802, 0.007628, 0.009371, 0.011013, 0.012541, 0.013940, 0.015198, 0.016302, 0.017244, 0.018015, 0.018609, 0.019020, 0.019247, 0.019287, 0.019142, 0.018814, 0.018307, 0.017627, 0.016781, 0.015778, 0.014629, 0.013346, 0.011941, 0.010429, 0.008825, 0.007145, 0.005405, 0.003623, 0.001815, 0.000000, -0.001806, -0.003585, -0.005319, -0.006994, -0.008592, -0.010100, -0.011502, -0.012786, -0.013941, -0.014956, -0.015821, -0.016530, -0.017077, -0.017456, -0.017666, -0.017705, -0.017573, -0.017273, -0.016809, -0.016186, -0.015411, -0.014491, -0.013437, -0.012260, -0.010970, -0.009582, -0.008109, -0.006566, -0.004968, -0.003330, -0.001669, -0.000000, 0.001660, 0.003296, 0.004891, 0.006431, 0.007902, 0.009289, 0.010580, 0.011762, 0.012825, 0.013760, 0.014558, 0.015211, 0.015715, 0.016066, 0.016260, 0.016297, 0.016177, 0.015902, 0.015476, 0.014904, 0.014191, 0.013345, 0.012375, 0.011292, 0.010105, 0.008827, 0.007471, 0.006049, 0.004577, 0.003068, 0.001538, 0.000000, -0.001530, -0.003038, -0.004509, -0.005929, -0.007285, -0.008564, -0.009755, -0.010846, -0.011827, -0.012690, -0.013426, -0.014030, -0.014495, -0.014820, -0.015000, -0.015035, -0.014925, -0.014673, -0.014281, -0.013753, -0.013096, -0.012316, -0.011422, -0.010422, -0.009327, -0.008148, -0.006897, -0.005585, -0.004226, -0.002833, -0.001420, -0.000000, 0.001413, 0.002805, 0.004164, 0.005476, 0.006729, 0.007911, 0.009011, 0.010020, 0.010927, 0.011724, 0.012406, 0.012964, 0.013395, 0.013695, 0.013863, 0.013896, 0.013795, 0.013563, 0.013201, 0.012714, 0.012107, 0.011386, 0.010560, 0.009636, 0.008625, 0.007535, 0.006378, 0.005165, 0.003908, 0.002620, 0.001313, 0.000000, -0.001307, -0.002595, -0.003852, -0.005066, -0.006225, -0.007319, -0.008338, -0.009271, -0.010111, -0.010849, -0.011480, -0.011997, -0.012397, -0.012675, -0.012830, -0.012861, -0.012769, -0.012554, -0.012219, -0.011769, -0.011208, -0.010541, -0.009777, -0.008922, -0.007985, -0.006976, -0.005905, -0.004783, -0.003619, -0.002427, -0.001216, -0.000000, 0.001210, 0.002404, 0.003568, 0.004692, 0.005766, 0.006780, 0.007723, 0.008588, 0.009366, 0.010051, 0.010635, 0.011115, 0.011485, 0.011744, 0.011888, 0.011917, 0.011832, 0.011633, 0.011323, 0.010906, 0.010386, 0.009769, 0.009061, 0.008269, 0.007401, 0.006466, 0.005474, 0.004433, 0.003355, 0.002249, 0.001127, 0.000000, -0.001122, -0.002228, -0.003308, -0.004350, -0.005346, -0.006286, -0.007161, -0.007963, -0.008684, -0.009319, -0.009861, -0.010306, -0.010650, -0.010890, -0.011024, -0.011051, -0.010972, -0.010788, -0.010501, -0.010115, -0.009633, -0.009060, -0.008404, -0.007669, -0.006864, -0.005997, -0.005077, -0.004112, -0.003112, -0.002086, -0.001046, -0.000000, 0.001041, 0.002067, 0.003068, 0.004036, 0.004959, 0.005831, 0.006643, 0.007387, 0.008057, 0.008646, 0.009149, 0.009562, 0.009881, 0.010104, 0.010228, 0.010254, 0.010181, 0.010010, 0.009744, 0.009385, 0.008938, 0.008407, 0.007798, 0.007117, 0.006370, 0.005565, 0.004711, 0.003816, 0.002888, 0.001936, 0.000971, 0.000000, -0.000966, -0.001918, -0.002848, -0.003745, -0.004603, -0.005412, -0.006165, -0.006856, -0.007478, -0.008024, -0.008492, -0.008875, -0.009171, -0.009378, -0.009493, -0.009517, -0.009449, -0.009291, -0.009044, -0.008711, -0.008296, -0.007803, -0.007238, -0.006605, -0.005912, -0.005166, -0.004373, -0.003542, -0.002680, -0.001797, -0.000901, -0.000000, 0.000897, 0.001781, 0.002643, 0.003476, 0.004272, 0.005023, 0.005723, 0.006364, 0.006941, 0.007449, 0.007882, 0.008238, 0.008513, 0.008705, 0.008812, 0.008834, 0.008771, 0.008624, 0.008395, 0.008086, 0.007701, 0.007243, 0.006718, 0.006131, 0.005488, 0.004795, 0.004059, 0.003287, 0.002488, 0.001668, 0.000836, 0.000000, -0.000832, -0.001653, -0.002453, -0.003227, -0.003966, -0.004663, -0.005312, -0.005907, -0.006442, -0.006913, -0.007316, -0.007646, -0.007901, -0.008079, -0.008179, -0.008199, -0.008141, -0.008004, -0.007791, -0.007505, -0.007147, -0.006723, -0.006235, -0.005690, -0.005093, -0.004450, -0.003767, -0.003051, -0.002309, -0.001548, -0.000776, -0.000000, 0.000772, 0.001534, 0.002277, 0.002994, 0.003680, 0.004327, 0.004929, 0.005481, 0.005978, 0.006415, 0.006789, 0.007095, 0.007332, 0.007497, 0.007589, 0.007608, 0.007553, 0.007427, 0.007229, 0.006963, 0.006631, 0.006237, 0.005785, 0.005279, 0.004725, 0.004128, 0.003495, 0.002830, 0.002142, 0.001436, 0.000720, 0.000000, -0.000716, -0.001423, -0.002112, -0.002778, -0.003413, -0.004013, -0.004572, -0.005084, -0.005545, -0.005950, -0.006296, -0.006580, -0.006800, -0.006953, -0.007038, -0.007055, -0.007005, -0.006887, -0.006704, -0.006457, -0.006149, -0.005784, -0.005364, -0.004895, -0.004382, -0.003828, -0.003240, -0.002624, -0.001986, -0.001332, -0.000667, -0.000000, 0.000664, 0.001319, 0.001958, 0.002575, 0.003164, 0.003721, 0.004238, 0.004713, 0.005140, 0.005515, 0.005836, 0.006099, 0.006302, 0.006444, 0.006523, 0.006539, 0.006492, 0.006383, 0.006213, 0.005984, 0.005698, 0.005360, 0.004971, 0.004536, 0.004060, 0.003547, 0.003002, 0.002432, 0.001840, 0.001234, 0.000618, 0.000000, -0.000615, -0.001222, -0.001814, -0.002385, -0.002931, -0.003446, -0.003926, -0.004365, -0.004761, -0.005108, -0.005405, -0.005649, -0.005837, -0.005968, -0.006041, -0.006055, -0.006012, -0.005911, -0.005753, -0.005541, -0.005276, -0.004962, -0.004602, -0.004200, -0.003759, -0.003284, -0.002780, -0.002251, -0.001703, -0.001142, -0.000572, -0.000000, 0.000570, 0.001131, 0.001679, 0.002208, 0.002713, 0.003189, 0.003633, 0.004040, 0.004405, 0.004727, 0.005001, 0.005227, 0.005400, 0.005521, 0.005589, 0.005602, 0.005561, 0.005468, 0.005322, 0.005125, 0.004880, 0.004590, 0.004257, 0.003884, 0.003476, 0.003037, 0.002570, 0.002082, 0.001575, 0.001056, 0.000529, 0.000000, -0.000527, -0.001046, -0.001552, -0.002041, -0.002508, -0.002948, -0.003358, -0.003734, -0.004072, -0.004369, -0.004622, -0.004830, -0.004991, -0.005102, -0.005164, -0.005177, -0.005139, -0.005052, -0.004917, -0.004735, -0.004509, -0.004240, -0.003932, -0.003588, -0.003211, -0.002805, -0.002374, -0.001923, -0.001455, -0.000975, -0.000489, -0.000000, 0.000486, 0.000965, 0.001433, 0.001884, 0.002315, 0.002722, 0.003100, 0.003447, 0.003759, 0.004033, 0.004266, 0.004458, 0.004606, 0.004709, 0.004766, 0.004777, 0.004742, 0.004661, 0.004537, 0.004369, 0.004160, 0.003912, 0.003628, 0.003310, 0.002962, 0.002587, 0.002190, 0.001773, 0.001342, 0.000899, 0.000451, 0.000000, -0.000448, -0.000890, -0.001321, -0.001737, -0.002134, -0.002509, -0.002858, -0.003177, -0.003464, -0.003717, -0.003932, -0.004109, -0.004245, -0.004339, -0.004392, -0.004401, -0.004369, -0.004295, -0.004179, -0.004024, -0.003832, -0.003603, -0.003341, -0.003048, -0.002728, -0.002383, -0.002016, -0.001633, -0.001235, -0.000828, -0.000415, -0.000000, 0.000413, 0.000819, 0.001216, 0.001599, 0.001964, 0.002309, 0.002630, 0.002924, 0.003188, 0.003420, 0.003618, 0.003780, 0.003905, 0.003992, 0.004040, 0.004048, 0.004018, 0.003950, 0.003844, 0.003701, 0.003524, 0.003313, 0.003072, 0.002803, 0.002508, 0.002190, 0.001854, 0.001501, 0.001135, 0.000761, 0.000381, 0.000000, -0.000379, -0.000753, -0.001117, -0.001469, -0.001805, -0.002121, -0.002416, -0.002685, -0.002928, -0.003141, -0.003322, -0.003471, -0.003586, -0.003665, -0.003709, -0.003717, -0.003689, -0.003626, -0.003528, -0.003397, -0.003234, -0.003041, -0.002819, -0.002572, -0.002301, -0.002010, -0.001701, -0.001377, -0.001041, -0.000698, -0.000350, -0.000000, 0.000348, 0.000690, 0.001025, 0.001347, 0.001655, 0.001945, 0.002215, 0.002462, 0.002684, 0.002879, 0.003045, 0.003181, 0.003286, 0.003358, 0.003398, 0.003405, 0.003379, 0.003321, 0.003231, 0.003111, 0.002962, 0.002784, 0.002581, 0.002355, 0.002107, 0.001840, 0.001557, 0.001260, 0.000953, 0.000639, 0.000320, 0.000000, -0.000318, -0.000632, -0.000937, -0.001232, -0.001514, -0.001779, -0.002025, -0.002251, -0.002454, -0.002632, -0.002784, -0.002908, -0.003004, -0.003070, -0.003106, -0.003112, -0.003088, -0.003035, -0.002953, -0.002843, -0.002706, -0.002544, -0.002358, -0.002151, -0.001924, -0.001680, -0.001422, -0.001151, -0.000870, -0.000583, -0.000292, -0.000000, 0.000291, 0.000577, 0.000855, 0.001124, 0.001381, 0.001623, 0.001848, 0.002054, 0.002239, 0.002401, 0.002539, 0.002652, 0.002739, 0.002799, 0.002832, 0.002837, 0.002815, 0.002766, 0.002691, 0.002591, 0.002466, 0.002318, 0.002148, 0.001959, 0.001753, 0.001530, 0.001295, 0.001048, 0.000793, 0.000531, 0.000266, 0.000000, -0.000264, -0.000525, -0.000779, -0.001023, -0.001257, -0.001477, -0.001681, -0.001868, -0.002036, -0.002184, -0.002309, -0.002412, -0.002491, -0.002545, -0.002574, -0.002579, -0.002559, -0.002514, -0.002446, -0.002354, -0.002240, -0.002106, -0.001952, -0.001780, -0.001592, -0.001390, -0.001176, -0.000952, -0.000720, -0.000482, -0.000241, -0.000000, 0.000240, 0.000476, 0.000706, 0.000928, 0.001140, 0.001340, 0.001525, 0.001694, 0.001847, 0.001980, 0.002094, 0.002186, 0.002258, 0.002307, 0.002333, 0.002337, 0.002319, 0.002278, 0.002216, 0.002132, 0.002029, 0.001907, 0.001767, 0.001612, 0.001441, 0.001258, 0.001064, 0.000861, 0.000651, 0.000436, 0.000219, 0.000000, -0.000217, -0.000431, -0.000639, -0.000840, -0.001031, -0.001211, -0.001379, -0.001532, -0.001669, -0.001789, -0.001892, -0.001975, -0.002040, -0.002084, -0.002107, -0.002111, -0.002094, -0.002057, -0.002000, -0.001925, -0.001832, -0.001721, -0.001595, -0.001454, -0.001300, -0.001135, -0.000960, -0.000777, -0.000587, -0.000393, -0.000197, -0.000000, 0.000196, 0.000388, 0.000576, 0.000756, 0.000929, 0.001091, 0.001242, 0.001379, 0.001503, 0.001611, 0.001703, 0.001778, 0.001836, 0.001875, 0.001896, 0.001899, 0.001884, 0.001850, 0.001799, 0.001731, 0.001647, 0.001547, 0.001434, 0.001307, 0.001169, 0.001020, 0.000862, 0.000698, 0.000527, 0.000353, 0.000177, 0.000000, -0.000176, -0.000348, -0.000517, -0.000679, -0.000833, -0.000979, -0.001114, -0.001237, -0.001348, -0.001445, -0.001527, -0.001594, -0.001645, -0.001680, -0.001699, -0.001701, -0.001687, -0.001657, -0.001611, -0.001550, -0.001474, -0.001385, -0.001283, -0.001170, -0.001046, -0.000912, -0.000771, -0.000624, -0.000472, -0.000316, -0.000158, -0.000000, 0.000157, 0.000311, 0.000462, 0.000606, 0.000744, 0.000874, 0.000994, 0.001104, 0.001203, 0.001289, 0.001363, 0.001422, 0.001468, 0.001499, 0.001515, 0.001517, 0.001504, 0.001477, 0.001436, 0.001381, 0.001314, 0.001234, 0.001143, 0.001042, 0.000931, 0.000812, 0.000687, 0.000555, 0.000420, 0.000281, 0.000141, 0.000000, -0.000140, -0.000277, -0.000410, -0.000539, -0.000662, -0.000777, -0.000884, -0.000981, -0.001069, -0.001145, -0.001210, -0.001263, -0.001303, -0.001330, -0.001345, -0.001346, -0.001334, -0.001310, -0.001273, -0.001224, -0.001164, -0.001094, -0.001013, -0.000923, -0.000825, -0.000719, -0.000608, -0.000492, -0.000372, -0.000249, -0.000124, -0.000000, 0.000124, 0.000245, 0.000363, 0.000477, 0.000585, 0.000686, 0.000781, 0.000867, 0.000944, 0.001011, 0.001068, 0.001114, 0.001150, 0.001174, 0.001186, 0.001187, 0.001177, 0.001155, 0.001122, 0.001079, 0.001026, 0.000963, 0.000892, 0.000813, 0.000726, 0.000633, 0.000535, 0.000433, 0.000327, 0.000219, 0.000109, 0.000000, -0.000109, -0.000215, -0.000319, -0.000419, -0.000514, -0.000603, -0.000685, -0.000761, -0.000828, -0.000887, -0.000937, -0.000977, -0.001008, -0.001029, -0.001040, -0.001040, -0.001031, -0.001012, -0.000983, -0.000945, -0.000898, -0.000843, -0.000780, -0.000711, -0.000635, -0.000554, -0.000468, -0.000378, -0.000286, -0.000191, -0.000096, -0.000000, 0.000095, 0.000188, 0.000278, 0.000365, 0.000448, 0.000526, 0.000597, 0.000663, 0.000722, 0.000773, 0.000816, 0.000851, 0.000878, 0.000895, 0.000905, 0.000905, 0.000897, 0.000880, 0.000854, 0.000821, 0.000780, 0.000732, 0.000678, 0.000617, 0.000551, 0.000481, 0.000406, 0.000328, 0.000248, 0.000166, 0.000083, 0.000000, -0.000082, -0.000163, -0.000241, -0.000316, -0.000388, -0.000455, -0.000517, -0.000573, -0.000624, -0.000668, -0.000705, -0.000735, -0.000758, -0.000773, -0.000781, -0.000781, -0.000773, -0.000758, -0.000736, -0.000708, -0.000672, -0.000631, -0.000584, -0.000531, -0.000474, -0.000413, -0.000349, -0.000282, -0.000213, -0.000142, -0.000071, -0.000000, 0.000070, 0.000140, 0.000207, 0.000271, 0.000332, 0.000390, 0.000443, 0.000491, 0.000534, 0.000572, 0.000603, 0.000629, 0.000648, 0.000661, 0.000667, 0.000667, 0.000660, 0.000648, 0.000629, 0.000604, 0.000573, 0.000538, 0.000497, 0.000453, 0.000404, 0.000352, 0.000297, 0.000240, 0.000181, 0.000121, 0.000061, 0.000000, -0.000060, -0.000119, -0.000175, -0.000230, -0.000282, -0.000331, -0.000375, -0.000416, -0.000453, -0.000484, -0.000511, -0.000532, -0.000548, -0.000559, -0.000564, -0.000564, -0.000558, -0.000547, -0.000531, -0.000510, -0.000484, -0.000454, -0.000419, -0.000381, -0.000340, -0.000296, -0.000250, -0.000202, -0.000152, -0.000102, -0.000051, -0.000000, 0.000050, 0.000099, 0.000147, 0.000193, 0.000236, 0.000277, 0.000314, 0.000348, 0.000379, 0.000405, 0.000427, 0.000445, 0.000458, 0.000467, 0.000471, 0.000470, 0.000465, 0.000456, 0.000442, 0.000424, 0.000403, 0.000377, 0.000349, 0.000317, 0.000283, 0.000246, 0.000208, 0.000168, 0.000126, 0.000084, 0.000042, 0.000000, -0.000042, -0.000082, -0.000122, -0.000160, -0.000195, -0.000229, -0.000260, -0.000287, -0.000312, -0.000334, -0.000352, -0.000366, -0.000377, -0.000384, -0.000387, -0.000386, -0.000382, -0.000374, -0.000363, -0.000348, -0.000330, -0.000309, -0.000286, -0.000259, -0.000231, -0.000201, -0.000170, -0.000137, -0.000103, -0.000069, -0.000034, -0.000000, 0.000034, 0.000067, 0.000099, 0.000130, 0.000159, 0.000186, 0.000211, 0.000233, 0.000253, 0.000270, 0.000285, 0.000296, 0.000305, 0.000310, 0.000312, 0.000312, 0.000308, 0.000302, 0.000292, 0.000280, 0.000265, 0.000248, 0.000229, 0.000208, 0.000186, 0.000161, 0.000136, 0.000109, 0.000082, 0.000055, 0.000027, 0.000000, -0.000027, -0.000053, -0.000079, -0.000103, -0.000126, -0.000148, -0.000167, -0.000185, -0.000201, -0.000214, -0.000226, -0.000234, -0.000241, -0.000245, -0.000247, -0.000246, -0.000243, -0.000238, -0.000230, -0.000220, -0.000209, -0.000195, -0.000180, -0.000163, -0.000145, -0.000126, -0.000106, -0.000086, -0.000064, -0.000043, -0.000021, -0.000000, 0.000021, 0.000042, 0.000061, 0.000080, 0.000098, 0.000114, 0.000129, 0.000143, 0.000155, 0.000165, 0.000174, 0.000181, 0.000185, 0.000188, 0.000189, 0.000189, 0.000186, 0.000182, 0.000176, 0.000168, 0.000159, 0.000149, 0.000137, 0.000124, 0.000110, 0.000096, 0.000081, 0.000065, 0.000049, 0.000032, 0.000016, 0.000000, -0.000016, -0.000031, -0.000046, -0.000060, -0.000073, -0.000086, -0.000097, -0.000107, -0.000116, -0.000123, -0.000129, -0.000134, -0.000138, -0.000140, -0.000140, -0.000140, -0.000138, -0.000134, -0.000130, -0.000124, -0.000117, -0.000109, -0.000101, -0.000091, -0.000081, -0.000070, -0.000059, -0.000047, -0.000035, -0.000024, -0.000012, -0.000000, 0.000011, 0.000023, 0.000033, 0.000043, 0.000053, 0.000061, 0.000069, 0.000076, 0.000083, 0.000088, 0.000092, 0.000095, 0.000098, 0.000099, 0.000099, 0.000099, 0.000097, 0.000094, 0.000091, 0.000087, 0.000082, 0.000076, 0.000070, 0.000063, 0.000056, 0.000048, 0.000041, 0.000033, 0.000024, 0.000016, 0.000008, 0.000000, -0.000008, -0.000015, -0.000023, -0.000029, -0.000036, -0.000041, -0.000047, -0.000051, -0.000055, -0.000059, -0.000062, -0.000064, -0.000065, -0.000066, -0.000066, -0.000065, -0.000064, -0.000062, -0.000060, -0.000057, -0.000053, -0.000049, -0.000045, -0.000041, -0.000036, -0.000031, -0.000026, -0.000021, -0.000016, -0.000010, -0.000005, -0.000000, 0.000005, 0.000010, 0.000014, 0.000018, 0.000022, 0.000026, 0.000029, 0.000032, 0.000034, 0.000036, 0.000037, 0.000038, 0.000039, 0.000039, 0.000039, 0.000039, 0.000038, 0.000037, 0.000035, 0.000033, 0.000031, 0.000029, 0.000026, 0.000024, 0.000021, 0.000018, 0.000015, 0.000012, 0.000009, 0.000006, 0.000003, 0.000000, -0.000003, -0.000005, -0.000008, -0.000010, -0.000012, -0.000014, -0.000015, -0.000017, -0.000018, -0.000019, -0.000019, -0.000020, -0.000020, -0.000020, -0.000020, -0.000019, -0.000019, -0.000018, -0.000017, -0.000016, -0.000015, -0.000014, -0.000012, -0.000011, -0.000010, -0.000008, -0.000007, -0.000005, -0.000004, -0.000003, -0.000001, -0.000000, 0.000001, 0.000002, 0.000003, 0.000004, 0.000005, 0.000006, 0.000006, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000007, 0.000006, 0.000006, 0.000005, 0.000005, 0.000004, 0.000004, 0.000003, 0.000003, 0.000002, 0.000002, 0.000001, 0.000001, 0.000001, 0.000000, 0.000000, -0.000000, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000001, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000, -0.000000};

int16_t delay_buffer[3*WIDTH] = { 0 };

int16_t gimme_data(int16_t j)
{
     return delay_buffer[(int) j + WIDTH];
}

void new_data(int16_t data)
{
    u8 ii;
    for (ii=0;ii<DELAY_SIZE-5;ii++)	delay_buffer[ii] = delay_buffer[ii+1];
    delay_buffer[DELAY_SIZE-5] = data;
}

float linear_interp(float first_number,float second_number,float fraction)
{
    return (first_number + ((second_number - first_number) * fraction));
}



float sinc(float x)
{
    int low;
    float temp,delta;
    if (fabsf(x)>=WIDTH-1)
	return 0.0f;
    else {
	temp = fabsf(x) * (float) SAMPLES_PER_ZERO_CROSSING;
	low = temp;          /* these are interpolation steps */
	delta = temp - low;  /* and can be ommited if desired */
	return linear_interp(sinc_table[low],sinc_table[low + 1],delta);
	//return sinc_table[low];
    }
}

/*
float sinc(float x)
{
  return t_sinc(x);
}
*/

void dosamplerate(int16_t* in, int16_t* out, float factor, u8 size){
float one_over_factor,delta_factor,final_factor,initial_factor;
float alpha;
//    u8 mode;
 int16_t data_in;
float temp1=0.0f,temp3;
static float time_now=0.0f;
static int32_t total_written=0,j;
int32_t left_limit,right_limit;
static int32_t int_time=0,last_time=0;
 int16_t x=0;
for (u8 ii=0;ii<size;ii++){
temp1 = 0.0f;

left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;
//if (right_limit>size) right_limit = size;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
out[ii] = (int) temp1;
}

 else    {
one_over_factor = 1.0f / factor;
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * one_over_factor *
    sinc(one_over_factor * (time_now - (float) j));
out[ii] = (int) temp1;
}

/* }// not SINCMODE but interpol
 else {
alpha = time_now - (float) int_time;
out[ii] = (delay_buffer[DELAY_SIZE-5] * alpha)
  + (delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
 out[ii] = data_in;
 }*/


//            fwrite(&data_out,2,1,sound_out);
total_written++;
time_now += factor;
last_time = int_time;
int_time = time_now;
while(last_time<int_time)      {
new_data(in[x%size]);
 data_in=in[x%size];
 x++;
last_time += 1;
}

}
//            factor  = initial_factor + (time_now * delta_factor);
}

/// test code 

void samplerate(int16_t* in, int16_t* out, float factor, u8 size, int16_t(*getsample)(void), void(*newsay)(void), u8 trigger, float sampleratio){
  float one_over_factor;
  float alpha;
  float temp1=0.0f;
  static float time_now=0.0f;
  long j;
  long left_limit,right_limit,last_time=0;
  static long int_time=0;
  static u8 triggered=0;
  factor*=sampleratio;
 
  if (trigger==1) newsay();   // first trigger from mode-change

for (u8 ii=0;ii<size;ii++){
  temp1 = 0.0f;
  // factor=1.0f;

  if (time_now>327680.0){
    int_time-=time_now; // preserve???
    time_now=0.0f;
  }

  // deal also with trigger
  if (in[ii]>THRESH && !triggered) {
    newsay();
    triggered=1;
  }
  else if (in[ii]<THRESHLOW && triggered) triggered=0;


  //if (SINCMODE)       {
left_limit = time_now - WIDTH + 1;      /* leftmost neighboring sample used for interp.*/
right_limit = time_now + WIDTH; /* rightmost leftmost neighboring sample used for interp.*/
if (left_limit<0) left_limit = 0;
//if (right_limit>size) right_limit = size;

if (factor<1.0f) {
for (j=left_limit;j<right_limit;j++)
  temp1 += gimme_data(j-int_time) * 
    sinc(time_now - (float) j);
 out[ii] = (int) temp1;
}
 else    {
   one_over_factor = 1.0f / factor;
   for (j=left_limit;j<right_limit;j++)
     temp1 += gimme_data(j-int_time) * one_over_factor *
       sinc(one_over_factor * (time_now - (float) j));
   out[ii] = (int) temp1;
}
/* }////////////////////////////////////////// not SINCMODE but interpol
 else {
alpha = time_now - (float) int_time;
out[ii] = (delay_buffer[DELAY_SIZE-5] * alpha)
  + (delay_buffer[DELAY_SIZE-6] * (1.0f - alpha));
// out[ii] = getsample();
// out[ii]=rand()%32768;
}*/

time_now += factor;
last_time = int_time;
int_time = time_now;
 doadc();
while(last_time<int_time)      {
  int16_t val=getsample();
  new_data(val);
  //  data_in=val;
  // x++;
last_time += 1;
}

 }
//            factor  = initial_factor + (time_now * delta_factor);
}

