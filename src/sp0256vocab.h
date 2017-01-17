// extra vocab from book: jiminez p.  43+

const u8 sp_ampere[] __attribute__ ((section (".flash")))  = {0x18, 0x10, 0x9, 0x34};

const u8 sp_cent[] __attribute__ ((section (".flash")))  = {0x37, 0x37, 7, 0xb, 0x11};

const u8 sp_centi[] __attribute__ ((section (".flash")))  = {0x37, 0x37, 7, 0xb, 0xc};

const u8 sp_danger[] __attribute__ ((section (".flash")))  = {0x21, 7, 0xb, 0x19, 0x33};

const u8 sp_degree[] __attribute__ ((section (".flash")))  = {0x21, 0xc, 0x24, 0x27, 0xc};

const u8 sp_dollar[] __attribute__ ((section (".flash")))  = {0x21, 0xf, 0x20, 0x33};

const u8 sp_feet[] __attribute__ ((section (".flash")))  = {0x28, 0xc, 0x11};

const u8 sp_farads[] __attribute__ ((section (".flash")))  = {0x28, 0xf, 0x27, 0xf, 0x37};

const u8 sp_fuel[] ={ 0x28, 0x13, 0x2d};

const u8 sp_gallon[] __attribute__ ((section (".flash")))  = {0x24, 0xf, 0x2d, 0x25, 0xb};

const u8 sp_go[] __attribute__ ((section (".flash")))  = {0x24, 0x35};

const u8 sp_gram[] __attribute__ ((section (".flash")))  = {0x24, 0x27, 0x1a, 0x10};

const u8 sp_high[] __attribute__ ((section (".flash")))  = {0x39, 6};

const u8 sp_higher[] __attribute__ ((section (".flash")))  = {0x29, 6, 0x33};

const u8 sp_inches[] __attribute__ ((section (".flash")))  = {0x13, 0xb, 0x25, 7, 0x37};

const u8 sp_it[] __attribute__ ((section (".flash")))  = {0xc, 0x11};

const u8 sp_kilo[] __attribute__ ((section (".flash")))  = {0x2a, 0x13, 0x2d, 0x35};

const u8 sp_less[] __attribute__ ((section (".flash")))  = {0x2d, 7, 0x37, 0x37};

const u8 sp_lesser[] __attribute__ ((section (".flash")))  = {0x2d, 7, 0x37, 0x33};

const u8 sp_limitt[] __attribute__ ((section (".flash")))  = {0x2d, 0xc, 0x10, 0xc, 0x11};

const u8 sp_low[] __attribute__ ((section (".flash")))  = {0x2d, 0x35};

const u8 sp_lower[] __attribute__ ((section (".flash")))  = {0x2d, 0x35, 0x33};

const u8 sp_milli[] __attribute__ ((section (".flash")))  = {0x10, 0xc, 0x2d, 0xc};

const u8 sp_microo[] __attribute__ ((section (".flash")))  = {0x10, 6, 8, 0x27, 0x35};

const u8 sp_minus[] __attribute__ ((section (".flash")))  = {0x10, 6, 0xb, 0xf, 0x37};

const u8 sp_number[] __attribute__ ((section (".flash")))  = {0x28, 0xf, 0x10, 0x3f, 0x33};

const u8 sp_off[] __attribute__ ((section (".flash")))  = {0x18, 0x28, 0x28};

const u8 sp_on[] __attribute__ ((section (".flash")))  = {0x18, 0xb};

const u8 sp_percent[] __attribute__ ((section (".flash")))  = {9, 0x33, 0x37, 7, 0x28, 0x11};

const u8 sp_pico[] __attribute__ ((section (".flash")))  = {0x9, 0x33, 0x37, 0x7, 0x38, 0x11};

const u8 sp_please[] __attribute__ ((section (".flash")))  = {0x9, 0x2d, 0x13, 0x37};

const u8 sp_point[] __attribute__ ((section (".flash")))  = {0x9, 0x5, 0xb, 0x11};

const u8 sp_pulses[] __attribute__ ((section (".flash")))  = {0x9, 0x1e, 0x2d, 0x37, 0x7, 0x37};

const u8 sp_rate[] __attribute__ ((section (".flash")))  = {0xe, 0x14, 0x11};

const u8 sp_right[] __attribute__ ((section (".flash")))  = {0xe, 0x6, 0x11};

const u8 sp_rpm[] __attribute__ ((section (".flash")))  = {0x3b, 0x9, 0x13, 0x7, 0x10};

const u8 sp_set[] __attribute__ ((section (".flash")))  = {0x37, 0x7, 0xd};

const u8 sp_speedddd[] __attribute__ ((section (".flash")))  = {0x37, 0x2, 0x9, 0xc, 0x21};

const u8 sp_than[] __attribute__ ((section (".flash")))  = {0x36, 0x1a, 0x18};

const u8 sp_try[] __attribute__ ((section (".flash")))  = {0xd, 0x27, 0x6};

const u8 sp_temperature[] __attribute__ ((section (".flash")))  = {0xd, 0x7, 0x10, 0x9, 0x33, 0x25, 0x34};

const u8 sp_volt[] __attribute__ ((section (".flash")))  = {0x25, 0x35, 0x2d, 0x11};
  

/// length is 276 - where did this come from? retrospeak?

const u8 sp0256thirty[7] __attribute__ ((section (".flash")))  ={29, 52, 1, 2, 13, 19, 255};
const u8 sp0256bathing[6] __attribute__ ((section (".flash")))  ={63, 20, 54, 12, 44, 255};
const u8 sp0256twentieth[13] __attribute__ ((section (".flash")))  ={13, 48, 7, 7, 11, 1, 2, 13, 19, 1, 7, 29, 255};
const u8 sp0256gauges[7] __attribute__ ((section (".flash")))  ={36, 20, 1, 10, 12, 43, 255};
const u8 sp0256switched[10] __attribute__ ((section (".flash")))  ={55, 55, 48, 12, 12, 2, 50, 2, 13, 255};
const u8 sp0256september[14] __attribute__ ((section (".flash")))  ={55, 55, 7, 2, 9, 2, 13, 7, 7, 16, 0, 63, 51, 255};
const u8 sp0256talkers[8] __attribute__ ((section (".flash")))  ={13, 23, 23, 2, 42, 51, 43, 255};
const u8 sp0256ninth[6] __attribute__ ((section (".flash")))  ={11, 6, 11, 1, 29, 255};
const u8 sp0256fifty[10] __attribute__ ((section (".flash")))  ={40, 40, 12, 40, 40, 1, 2, 13, 19, 255};
const u8 sp0256four[4] __attribute__ ((section (".flash")))  ={40, 40, 58, 255};
const u8 sp0256gauged[7] __attribute__ ((section (".flash")))  ={36, 20, 1, 10, 1, 21, 255};
const u8 sp0256gauge[5] __attribute__ ((section (".flash")))  ={36, 20, 1, 10, 255};
const u8 sp0256talks[7] __attribute__ ((section (".flash")))  ={13, 23, 23, 1, 41, 55, 255};
const u8 sp0256yes[6] __attribute__ ((section (".flash")))  ={25, 7, 7, 55, 55, 255};
const u8 sp0256fifth[7] __attribute__ ((section (".flash")))  ={40, 12, 40, 40, 2, 29, 255};
const u8 sp0256spells[9] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 43, 255};
const u8 sp0256fir[3] __attribute__ ((section (".flash")))  ={40, 52, 255};
const u8 sp0256sixteen[12] __attribute__ ((section (".flash")))  ={55, 55, 12, 2, 41, 55, 1, 2, 13, 19, 11, 255};
const u8 sp0256system[11] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 55, 55, 2, 13, 7, 16, 255};
const u8 sp0256pledged[9] __attribute__ ((section (".flash")))  ={9, 45, 7, 7, 2, 10, 1, 21, 255};
const u8 sp0256thursday[7] __attribute__ ((section (".flash")))  ={29, 52, 43, 1, 33, 20, 255};
const u8 sp0256the[4] __attribute__ ((section (".flash")))  ={18, 2, 19, 255};
const u8 sp0256to[3] __attribute__ ((section (".flash")))  ={13, 31, 255};
const u8 sp0256whale[4] __attribute__ ((section (".flash")))  ={46, 20, 62, 255};
const u8 sp0256systems[12] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 55, 55, 2, 13, 7, 16, 43, 255};
const u8 sp0256pledges[9] __attribute__ ((section (".flash")))  ={9, 45, 7, 7, 2, 10, 12, 43, 255};
const u8 sp0256then[5] __attribute__ ((section (".flash")))  ={18, 7, 7, 11, 255};
const u8 sp0256seventy[11] __attribute__ ((section (".flash")))  ={55, 55, 7, 35, 12, 11, 1, 2, 13, 19, 255};
const u8 sp0256coop[5] __attribute__ ((section (".flash")))  ={8, 31, 2, 9, 255};
const u8 sp0256march[5] __attribute__ ((section (".flash")))  ={16, 59, 2, 50, 255};
const u8 sp0256checking[8] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 42, 12, 44, 255};
const u8 sp0256gauging[7] __attribute__ ((section (".flash")))  ={36, 20, 1, 10, 12, 44, 255};
const u8 sp0256freezing[8] __attribute__ ((section (".flash")))  ={40, 40, 14, 19, 43, 12, 44, 255};
const u8 sp0256collide[6] __attribute__ ((section (".flash")))  ={8, 15, 45, 6, 21, 255};
const u8 sp0256october[10] __attribute__ ((section (".flash")))  ={24, 1, 41, 2, 13, 53, 0, 63, 51, 255};
const u8 sp0256five[5] __attribute__ ((section (".flash")))  ={40, 40, 6, 35, 255};
const u8 sp0256month[5] __attribute__ ((section (".flash")))  ={16, 15, 11, 29, 255};
const u8 sp0256correcting[11] __attribute__ ((section (".flash")))  ={42, 52, 7, 7, 1, 41, 1, 13, 12, 44, 255};
const u8 sp0256day[4] __attribute__ ((section (".flash")))  ={33, 7, 20, 255};
const u8 sp0256minute[7] __attribute__ ((section (".flash")))  ={16, 12, 11, 12, 2, 13, 255};
const u8 sp0256bread[7] __attribute__ ((section (".flash")))  ={28, 39, 7, 7, 0, 21, 255};
const u8 sp0256switches[10] __attribute__ ((section (".flash")))  ={55, 55, 48, 12, 12, 2, 50, 12, 43, 255};
const u8 sp0256february[10] __attribute__ ((section (".flash")))  ={40, 7, 7, 0, 28, 39, 31, 47, 19, 255};
const u8 sp0256cognitive[11] __attribute__ ((section (".flash")))  ={8, 24, 24, 34, 11, 12, 2, 13, 12, 35, 255};
const u8 sp0256investigator[18] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 13, 51, 255};
const u8 sp0256h[5] __attribute__ ((section (".flash")))  ={20, 1, 2, 50, 255};
const u8 sp0256l[4] __attribute__ ((section (".flash")))  ={7, 7, 62, 255};
const u8 sp0256clown[5] __attribute__ ((section (".flash")))  ={42, 45, 32, 11, 255};
const u8 sp0256correct[9] __attribute__ ((section (".flash")))  ={42, 52, 7, 7, 1, 41, 1, 17, 255};
const u8 sp0256p[3] __attribute__ ((section (".flash")))  ={9, 19, 255};
const u8 sp0256intrigued[11] __attribute__ ((section (".flash")))  ={12, 11, 2, 13, 39, 19, 0, 34, 1, 21, 255};
const u8 sp0256ninety[7] __attribute__ ((section (".flash")))  ={11, 6, 11, 2, 13, 19, 255};
const u8 sp0256stopped[11] __attribute__ ((section (".flash")))  ={55, 55, 2, 17, 24, 24, 2, 9, 2, 13, 255};
const u8 sp0256q[4] __attribute__ ((section (".flash")))  ={42, 49, 31, 255};
const u8 sp0256january[8] __attribute__ ((section (".flash")))  ={10, 26, 26, 11, 25, 47, 19, 255};
const u8 sp0256zero[4] __attribute__ ((section (".flash")))  ={43, 60, 53, 255};
const u8 sp0256x[7] __attribute__ ((section (".flash")))  ={7, 7, 2, 41, 55, 55, 255};
const u8 sp0256investigators[19] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 13, 51, 43, 255};
const u8 sp0256legislates[15] __attribute__ ((section (".flash")))  ={45, 7, 7, 1, 10, 12, 55, 55, 45, 20, 1, 2, 17, 55, 255};
const u8 sp0256seventeen[12] __attribute__ ((section (".flash")))  ={55, 55, 7, 35, 29, 11, 1, 2, 13, 19, 11, 255};
const u8 sp0256talked[8] __attribute__ ((section (".flash")))  ={13, 23, 23, 2, 41, 2, 13, 255};
const u8 sp0256pledge[7] __attribute__ ((section (".flash")))  ={9, 45, 7, 7, 2, 10, 255};
const u8 sp0256ten[5] __attribute__ ((section (".flash")))  ={13, 7, 7, 11, 255};
const u8 sp0256starts[9] __attribute__ ((section (".flash")))  ={55, 55, 2, 13, 59, 2, 17, 55, 255};
const u8 sp0256sensitivity[19] __attribute__ ((section (".flash")))  ={55, 55, 7, 7, 11, 55, 55, 12, 1, 2, 13, 12, 35, 12, 1, 2, 13, 19, 255};
const u8 sp0256thirteenth[10] __attribute__ ((section (".flash")))  ={29, 51, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256crown[5] __attribute__ ((section (".flash")))  ={42, 39, 32, 11, 255};
const u8 sp0256eighteenth[9] __attribute__ ((section (".flash")))  ={20, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256d[3] __attribute__ ((section (".flash")))  ={33, 19, 255};
const u8 sp0256engaging[11] __attribute__ ((section (".flash")))  ={7, 7, 0, 11, 36, 20, 1, 10, 12, 44, 255};
const u8 sp0256thread[7] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 21, 255};
const u8 sp0256litter[7] __attribute__ ((section (".flash")))  ={45, 12, 12, 2, 13, 51, 255};
const u8 sp0256computer[9] __attribute__ ((section (".flash")))  ={42, 15, 16, 9, 49, 22, 13, 51, 255};
const u8 sp0256talker[7] __attribute__ ((section (".flash")))  ={13, 23, 23, 2, 42, 51, 255};
const u8 sp0256legislated[16] __attribute__ ((section (".flash")))  ={45, 7, 7, 1, 10, 12, 55, 55, 45, 20, 1, 2, 13, 12, 21, 255};
const u8 sp0256escape[9] __attribute__ ((section (".flash")))  ={7, 55, 55, 2, 42, 20, 2, 9, 255};
const u8 sp0256twelve[7] __attribute__ ((section (".flash")))  ={13, 48, 7, 7, 45, 35, 255};
const u8 sp0256pi[5] __attribute__ ((section (".flash")))  ={9, 24, 24, 6, 255};
const u8 sp0256calendar[10] __attribute__ ((section (".flash")))  ={42, 26, 26, 45, 7, 11, 1, 33, 51, 255};
const u8 sp0256enraging[9] __attribute__ ((section (".flash")))  ={7, 11, 14, 20, 1, 10, 12, 44, 255};
const u8 sp0256tenth[7] __attribute__ ((section (".flash")))  ={13, 7, 7, 11, 1, 29, 255};
const u8 sp0256saturday[10] __attribute__ ((section (".flash")))  ={55, 55, 26, 2, 13, 51, 1, 33, 20, 255};
const u8 sp0256checkers[8] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 42, 51, 43, 255};
const u8 sp0256sweats[9] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 55, 255};
const u8 sp0256for[4] __attribute__ ((section (".flash")))  ={40, 40, 58, 255};
const u8 sp0256clock[7] __attribute__ ((section (".flash")))  ={42, 45, 24, 24, 2, 41, 255};
const u8 sp0256memories[8] __attribute__ ((section (".flash")))  ={16, 7, 7, 16, 52, 19, 43, 255};
const u8 sp0256forty[6] __attribute__ ((section (".flash")))  ={40, 58, 2, 13, 19, 255};
const u8 sp0256stops[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 17, 24, 24, 2, 9, 55, 255};
const u8 sp0256score[6] __attribute__ ((section (".flash")))  ={55, 55, 2, 8, 58, 255};
const u8 sp0256sixth[9] __attribute__ ((section (".flash")))  ={55, 55, 12, 2, 41, 55, 1, 29, 255};
const u8 sp0256sweaters[10] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 51, 43, 255};
const u8 sp0256checks[7] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 42, 55, 255};
const u8 sp0256red[6] __attribute__ ((section (".flash")))  ={14, 7, 7, 0, 21, 255};
const u8 sp0256t[3] __attribute__ ((section (".flash")))  ={13, 19, 255};
const u8 sp0256thirtieth[10] __attribute__ ((section (".flash")))  ={29, 52, 1, 2, 13, 19, 1, 7, 29, 255};
const u8 sp0256million[8] __attribute__ ((section (".flash")))  ={16, 12, 12, 45, 49, 15, 11, 255};
const u8 sp0256escaping[11] __attribute__ ((section (".flash")))  ={7, 55, 55, 2, 42, 20, 2, 9, 12, 44, 255};
const u8 sp0256august[9] __attribute__ ((section (".flash")))  ={23, 23, 1, 61, 15, 55, 2, 17, 255};
const u8 sp0256june[4] __attribute__ ((section (".flash")))  ={10, 31, 11, 255};
const u8 sp0256friday[7] __attribute__ ((section (".flash")))  ={40, 39, 6, 1, 33, 20, 255};
const u8 sp0256seventh[9] __attribute__ ((section (".flash")))  ={55, 55, 7, 35, 29, 11, 1, 29, 255};
const u8 sp0256equals[9] __attribute__ ((section (".flash")))  ={19, 1, 2, 8, 48, 15, 62, 43, 255};
const u8 sp0256fifteenth[11] __attribute__ ((section (".flash")))  ={40, 12, 40, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256wednesday[9] __attribute__ ((section (".flash")))  ={46, 7, 7, 11, 43, 1, 33, 20, 255};
const u8 sp0256fourteen[8] __attribute__ ((section (".flash")))  ={40, 58, 1, 2, 13, 19, 11, 255};
const u8 sp0256cookie[5] __attribute__ ((section (".flash")))  ={8, 30, 42, 19, 255};
const u8 sp0256corrects[10] __attribute__ ((section (".flash")))  ={42, 52, 7, 7, 1, 41, 1, 17, 55, 255};
const u8 sp0256extent[9] __attribute__ ((section (".flash")))  ={7, 42, 55, 13, 7, 7, 11, 13, 255};
const u8 sp0256investigating[19] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 13, 12, 44, 255};
const u8 sp0256sixty[11] __attribute__ ((section (".flash")))  ={55, 55, 12, 2, 41, 55, 1, 2, 13, 19, 255};
const u8 sp0256talking[8] __attribute__ ((section (".flash")))  ={13, 23, 23, 2, 42, 12, 44, 255};
const u8 sp0256november[11] __attribute__ ((section (".flash")))  ={56, 53, 35, 7, 7, 16, 0, 63, 63, 51, 255};
const u8 sp0256raspberry[10] __attribute__ ((section (".flash")))  ={14, 15, 55, 55, 2, 28, 52, 39, 19, 255};
const u8 sp0256fifteen[9] __attribute__ ((section (".flash")))  ={40, 12, 40, 1, 2, 13, 19, 11, 255};
const u8 sp0256by[4] __attribute__ ((section (".flash")))  ={63, 24, 6, 255};
const u8 sp0256legislature[15] __attribute__ ((section (".flash")))  ={45, 7, 7, 1, 10, 12, 55, 55, 45, 20, 1, 2, 50, 51, 255};
const u8 sp0256c[4] __attribute__ ((section (".flash")))  ={55, 55, 19, 255};
const u8 sp0256ate[4] __attribute__ ((section (".flash")))  ={20, 2, 13, 255};
const u8 sp0256daughter[5] __attribute__ ((section (".flash")))  ={33, 23, 13, 51, 255};
const u8 sp0256g[3] __attribute__ ((section (".flash")))  ={10, 19, 255};
const u8 sp0256of[4] __attribute__ ((section (".flash")))  ={24, 24, 35, 255};
const u8 sp0256k[4] __attribute__ ((section (".flash")))  ={42, 7, 20, 255};
const u8 sp0256equal[8] __attribute__ ((section (".flash")))  ={19, 1, 2, 8, 48, 15, 62, 255};
const u8 sp0256o[2] __attribute__ ((section (".flash")))  ={53, 255};
const u8 sp0256times[6] __attribute__ ((section (".flash")))  ={13, 24, 6, 16, 43, 255};
const u8 sp0256pinning[7] __attribute__ ((section (".flash")))  ={9, 12, 12, 11, 12, 44, 255};
const u8 sp0256april[8] __attribute__ ((section (".flash")))  ={20, 2, 9, 39, 12, 12, 45, 255};
const u8 sp0256s[5] __attribute__ ((section (".flash")))  ={7, 7, 55, 55, 255};
const u8 sp0256plus[7] __attribute__ ((section (".flash")))  ={9, 45, 15, 15, 55, 55, 255};
const u8 sp0256sweat[8] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 255};
const u8 sp0256w[8] __attribute__ ((section (".flash")))  ={33, 15, 1, 63, 62, 49, 31, 255};
const u8 sp0256whaler[5] __attribute__ ((section (".flash")))  ={46, 20, 45, 51, 255};
const u8 sp0256investigates[18] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 17, 55, 255};
const u8 sp0256intrigue[9] __attribute__ ((section (".flash")))  ={12, 11, 2, 13, 39, 19, 0, 34, 255};
const u8 sp0256first[6] __attribute__ ((section (".flash")))  ={40, 52, 55, 1, 13, 255};
const u8 sp0256key[3] __attribute__ ((section (".flash")))  ={42, 19, 255};
const u8 sp0256enraged[9] __attribute__ ((section (".flash")))  ={7, 11, 14, 20, 1, 10, 1, 21, 255};
const u8 sp0256investigate[17] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 13, 255};
const u8 sp0256threading[9] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 33, 12, 44, 255};
const u8 sp0256tuesday[7] __attribute__ ((section (".flash")))  ={13, 31, 43, 1, 33, 20, 255};
const u8 sp0256one[5] __attribute__ ((section (".flash")))  ={46, 15, 15, 11, 255};
const u8 sp0256sincerely[11] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 11, 55, 55, 60, 45, 19, 255};
const u8 sp0256nipping[9] __attribute__ ((section (".flash")))  ={11, 12, 12, 1, 2, 9, 12, 44, 255};
const u8 sp0256eleventh[10] __attribute__ ((section (".flash")))  ={12, 45, 7, 7, 35, 12, 11, 1, 29, 255};
const u8 sp0256stopper[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 17, 24, 24, 2, 9, 51, 255};
const u8 sp0256enrages[9] __attribute__ ((section (".flash")))  ={7, 11, 14, 20, 1, 10, 12, 43, 255};
const u8 sp0256twelfth[9] __attribute__ ((section (".flash")))  ={13, 48, 7, 7, 45, 40, 1, 29, 255};
const u8 sp0256seventeenth[14] __attribute__ ((section (".flash")))  ={55, 55, 7, 35, 29, 11, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256spelling[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 12, 44, 255};
const u8 sp0256speak[8] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 19, 2, 41, 255};
const u8 sp0256sweating[10] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 12, 44, 255};
const u8 sp0256little[7] __attribute__ ((section (".flash")))  ={45, 12, 12, 2, 13, 62, 255};
const u8 sp0256checked[8] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 41, 1, 13, 255};
const u8 sp0256start[8] __attribute__ ((section (".flash")))  ={55, 55, 2, 13, 59, 2, 13, 255};
const u8 sp0256whales[5] __attribute__ ((section (".flash")))  ={46, 20, 62, 43, 255};
const u8 sp0256twenty[10] __attribute__ ((section (".flash")))  ={13, 48, 7, 7, 11, 1, 2, 13, 19, 255};
const u8 sp0256sweater[9] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 51, 255};
const u8 sp0256engagement[16] __attribute__ ((section (".flash")))  ={7, 7, 0, 11, 36, 20, 1, 10, 16, 7, 7, 11, 1, 2, 13, 255};
const u8 sp0256checker[7] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 42, 51, 255};
const u8 sp0256intrigues[10] __attribute__ ((section (".flash")))  ={12, 11, 2, 13, 39, 19, 0, 34, 43, 255};
const u8 sp0256pinned[7] __attribute__ ((section (".flash")))  ={9, 12, 12, 11, 1, 21, 255};
const u8 sp0256eleven[8] __attribute__ ((section (".flash")))  ={12, 45, 7, 7, 35, 12, 11, 255};
const u8 sp0256beer[3] __attribute__ ((section (".flash")))  ={63, 60, 255};
const u8 sp0256too[3] __attribute__ ((section (".flash")))  ={13, 31, 255};
const u8 sp0256memory[7] __attribute__ ((section (".flash")))  ={16, 7, 7, 16, 52, 19, 255};
const u8 sp0256whalers[6] __attribute__ ((section (".flash")))  ={46, 20, 45, 51, 43, 255};
const u8 sp0256hundred[12] __attribute__ ((section (".flash")))  ={57, 15, 15, 11, 1, 33, 39, 12, 12, 0, 21, 255};
const u8 sp0256sweated[11] __attribute__ ((section (".flash")))  ={55, 55, 46, 7, 7, 2, 13, 12, 2, 21, 255};
const u8 sp0256sincere[9] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 11, 55, 55, 60, 255};
const u8 sp0256today[6] __attribute__ ((section (".flash")))  ={13, 31, 2, 33, 20, 255};
const u8 sp0256freezers[8] __attribute__ ((section (".flash")))  ={40, 40, 14, 19, 43, 51, 43, 255};
const u8 sp0256sixteenth[14] __attribute__ ((section (".flash")))  ={55, 55, 12, 2, 41, 55, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256started[11] __attribute__ ((section (".flash")))  ={55, 55, 2, 13, 59, 2, 13, 12, 0, 33, 255};
const u8 sp0256infinitive[14] __attribute__ ((section (".flash")))  ={12, 11, 40, 40, 12, 12, 11, 12, 1, 2, 13, 12, 35, 255};
const u8 sp0256thousand[10] __attribute__ ((section (".flash")))  ={29, 24, 32, 43, 29, 0, 0, 11, 21, 255};
const u8 sp0256corrected[12] __attribute__ ((section (".flash")))  ={42, 52, 7, 7, 1, 41, 1, 13, 12, 1, 21, 255};
const u8 sp0256sister[9] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 55, 2, 13, 51, 255};
const u8 sp0256fore[4] __attribute__ ((section (".flash")))  ={40, 40, 58, 255};
const u8 sp0256subjectv[14] __attribute__ ((section (".flash")))  ={55, 55, 15, 1, 28, 1, 10, 7, 7, 2, 41, 2, 13, 255};
const u8 sp0256engage[9] __attribute__ ((section (".flash")))  ={7, 7, 0, 11, 36, 20, 1, 10, 255};
const u8 sp0256uncle[6] __attribute__ ((section (".flash")))  ={15, 44, 2, 8, 62, 255};
const u8 sp0256july[5] __attribute__ ((section (".flash")))  ={10, 22, 45, 6, 255};
const u8 sp0256freeze[6] __attribute__ ((section (".flash")))  ={40, 40, 14, 19, 43, 255};
const u8 sp0256nineteen[9] __attribute__ ((section (".flash")))  ={11, 6, 11, 1, 2, 13, 19, 11, 255};
const u8 sp0256starter[9] __attribute__ ((section (".flash")))  ={55, 55, 2, 13, 59, 2, 13, 51, 255};
const u8 sp0256intriguing[11] __attribute__ ((section (".flash")))  ={12, 11, 2, 13, 39, 19, 0, 34, 12, 44, 255};
const u8 sp0256b[3] __attribute__ ((section (".flash")))  ={63, 19, 255};
const u8 sp0256threads[8] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 33, 43, 255};
const u8 sp0256spellers[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 52, 43, 255};
const u8 sp0256hour[3] __attribute__ ((section (".flash")))  ={32, 51, 255};
const u8 sp0256f[5] __attribute__ ((section (".flash")))  ={7, 7, 40, 40, 255};
const u8 sp0256frozen[8] __attribute__ ((section (".flash")))  ={40, 40, 14, 53, 43, 7, 11, 255};
const u8 sp0256legislating[16] __attribute__ ((section (".flash")))  ={45, 7, 7, 1, 10, 12, 55, 55, 45, 20, 1, 2, 13, 12, 44, 255};
const u8 sp0256j[4] __attribute__ ((section (".flash")))  ={10, 7, 20, 255};
const u8 sp0256second[10] __attribute__ ((section (".flash")))  ={55, 55, 7, 2, 42, 12, 11, 1, 21, 255};
const u8 sp0256n[4] __attribute__ ((section (".flash")))  ={7, 7, 11, 255};
const u8 sp0256r[2] __attribute__ ((section (".flash")))  ={59, 255};
const u8 sp0256nine[5] __attribute__ ((section (".flash")))  ={11, 24, 6, 11, 255};
const u8 sp0256three[4] __attribute__ ((section (".flash")))  ={29, 14, 19, 255};
const u8 sp0256error[4] __attribute__ ((section (".flash")))  ={7, 47, 58, 255};
const u8 sp0256stopping[11] __attribute__ ((section (".flash")))  ={55, 55, 2, 17, 24, 24, 2, 9, 12, 44, 255};
const u8 sp0256emotional[9] __attribute__ ((section (".flash")))  ={19, 16, 53, 37, 15, 11, 15, 62, 255};
const u8 sp0256z[6] __attribute__ ((section (".flash")))  ={43, 7, 7, 1, 21, 255};
const u8 sp0256thirteen[8] __attribute__ ((section (".flash")))  ={29, 51, 1, 2, 13, 19, 11, 255};
const u8 sp0256and[6] __attribute__ ((section (".flash")))  ={26, 26, 11, 1, 21, 255};
const u8 sp0256bathe[4] __attribute__ ((section (".flash")))  ={63, 20, 54, 255};
const u8 sp0256seven[8] __attribute__ ((section (".flash")))  ={55, 55, 7, 7, 35, 12, 11, 255};
const u8 sp0256v[3] __attribute__ ((section (".flash")))  ={35, 19, 255};
const u8 sp0256pin[5] __attribute__ ((section (".flash")))  ={9, 12, 12, 11, 255};
const u8 sp0256escapes[10] __attribute__ ((section (".flash")))  ={7, 55, 55, 2, 42, 20, 2, 9, 43, 255};
const u8 sp0256december[11] __attribute__ ((section (".flash")))  ={33, 19, 55, 55, 7, 7, 16, 0, 63, 51, 255};
const u8 sp0256is[4] __attribute__ ((section (".flash")))  ={12, 12, 43, 255};
const u8 sp0256year[3] __attribute__ ((section (".flash")))  ={25, 60, 255};
const u8 sp0256am[5] __attribute__ ((section (".flash")))  ={26, 26, 1, 16, 255};
const u8 sp0256spelled[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 2, 21, 255};
const u8 sp0256escaped[11] __attribute__ ((section (".flash")))  ={7, 55, 55, 2, 42, 20, 2, 9, 1, 13, 255};
const u8 sp0256speller[9] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 52, 255};
const u8 sp0256eighteen[7] __attribute__ ((section (".flash")))  ={20, 1, 2, 13, 19, 11, 255};
const u8 sp0256sincerity[16] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 11, 55, 55, 7, 7, 14, 12, 1, 2, 13, 19, 255};
const u8 sp0256fourteenth[10] __attribute__ ((section (".flash")))  ={40, 58, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256ready[7] __attribute__ ((section (".flash")))  ={14, 7, 7, 0, 33, 19, 255};
const u8 sp0256engages[11] __attribute__ ((section (".flash")))  ={7, 7, 0, 11, 36, 20, 1, 10, 12, 43, 255};
const u8 sp0256check[6] __attribute__ ((section (".flash")))  ={50, 7, 7, 2, 41, 255};
const u8 sp0256physical[10] __attribute__ ((section (".flash")))  ={40, 40, 12, 43, 12, 2, 42, 15, 62, 255};
const u8 sp0256legislate[14] __attribute__ ((section (".flash")))  ={45, 7, 7, 1, 10, 12, 55, 55, 45, 20, 1, 2, 13, 255};
const u8 sp0256spell[8] __attribute__ ((section (".flash")))  ={55, 55, 2, 9, 7, 7, 62, 255};
const u8 sp0256rays[5] __attribute__ ((section (".flash")))  ={14, 7, 20, 43, 255};
const u8 sp0256pledging[9] __attribute__ ((section (".flash")))  ={9, 45, 7, 7, 2, 10, 12, 44, 255};
const u8 sp0256no[4] __attribute__ ((section (".flash")))  ={56, 15, 53, 255};
const u8 sp0256divided[10] __attribute__ ((section (".flash")))  ={33, 12, 35, 6, 1, 33, 12, 1, 21, 255};
const u8 sp0256six[8] __attribute__ ((section (".flash")))  ={55, 55, 12, 12, 2, 41, 55, 255};
const u8 sp0256freezer[7] __attribute__ ((section (".flash")))  ={40, 40, 14, 19, 43, 51, 255};
const u8 sp0256sunday[9] __attribute__ ((section (".flash")))  ={55, 55, 15, 15, 11, 1, 33, 20, 255};
const u8 sp0256fourth[6] __attribute__ ((section (".flash")))  ={40, 40, 58, 1, 29, 255};
const u8 sp0256pins[6] __attribute__ ((section (".flash")))  ={9, 12, 12, 11, 43, 255};
const u8 sp0256ray[4] __attribute__ ((section (".flash")))  ={14, 7, 20, 255};
const u8 sp0256nips[8] __attribute__ ((section (".flash")))  ={11, 12, 12, 1, 2, 9, 55, 255};
const u8 sp0256switching[10] __attribute__ ((section (".flash")))  ={55, 55, 48, 12, 12, 2, 50, 12, 44, 255};
const u8 sp0256investigated[20] __attribute__ ((section (".flash")))  ={12, 12, 11, 35, 7, 7, 55, 1, 2, 13, 12, 0, 36, 20, 1, 13, 12, 1, 21, 255};
const u8 sp0256monday[8] __attribute__ ((section (".flash")))  ={16, 15, 15, 11, 1, 33, 20, 255};
const u8 sp0256may[3] __attribute__ ((section (".flash")))  ={16, 20, 255};
const u8 sp0256stop[9] __attribute__ ((section (".flash")))  ={55, 55, 2, 17, 24, 24, 2, 9, 255};
const u8 sp0256eighty[5] __attribute__ ((section (".flash")))  ={20, 2, 13, 19, 255};
const u8 sp0256robots[9] __attribute__ ((section (".flash")))  ={14, 53, 1, 63, 24, 2, 17, 55, 255};
const u8 sp0256threaders[9] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 33, 51, 43, 255};
const u8 sp0256y[3] __attribute__ ((section (".flash")))  ={46, 6, 255};
const u8 sp0256eight[4] __attribute__ ((section (".flash")))  ={20, 2, 13, 255};
const u8 sp0256two[3] __attribute__ ((section (".flash")))  ={13, 31, 255};
const u8 sp0256letter[7] __attribute__ ((section (".flash")))  ={45, 7, 7, 2, 13, 51, 255};
const u8 sp0256subject[14] __attribute__ ((section (".flash")))  ={55, 55, 15, 15, 1, 28, 1, 10, 7, 2, 41, 2, 13, 255};
const u8 sp0256date[5] __attribute__ ((section (".flash")))  ={33, 20, 2, 13, 255};
const u8 sp0256nip[7] __attribute__ ((section (".flash")))  ={11, 12, 12, 1, 2, 9, 255};
const u8 sp0256eighth[6] __attribute__ ((section (".flash")))  ={20, 2, 13, 1, 29, 255};
const u8 sp0256nipped[9] __attribute__ ((section (".flash")))  ={11, 12, 12, 1, 2, 9, 2, 13, 255};
const u8 sp0256whaling[6] __attribute__ ((section (".flash")))  ={46, 20, 45, 12, 44, 255};
const u8 sp0256a[2] __attribute__ ((section (".flash")))  ={20, 255};
const u8 sp0256threaded[10] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 33, 12, 1, 21, 255};
const u8 sp0256e[2] __attribute__ ((section (".flash")))  ={19, 255};
const u8 sp0256third[5] __attribute__ ((section (".flash")))  ={29, 51, 1, 21, 255};
const u8 sp0256won[5] __attribute__ ((section (".flash")))  ={46, 15, 15, 11, 255};
const u8 sp0256i[3] __attribute__ ((section (".flash")))  ={24, 6, 255};
const u8 sp0256alarm[5] __attribute__ ((section (".flash")))  ={15, 45, 59, 16, 255};
const u8 sp0256sensitive[14] __attribute__ ((section (".flash")))  ={55, 55, 7, 7, 11, 55, 55, 12, 1, 2, 13, 12, 35, 255};
const u8 sp0256m[4] __attribute__ ((section (".flash")))  ={7, 7, 16, 255};
const u8 sp0256robot[8] __attribute__ ((section (".flash")))  ={14, 53, 1, 63, 24, 2, 13, 255};
const u8 sp0256enrage[7] __attribute__ ((section (".flash")))  ={7, 11, 14, 20, 1, 10, 255};
const u8 sp0256switch[8] __attribute__ ((section (".flash")))  ={55, 55, 48, 12, 12, 2, 50, 255};
const u8 sp0256u[3] __attribute__ ((section (".flash")))  ={49, 31, 255};
const u8 sp0256threader[8] __attribute__ ((section (".flash")))  ={29, 14, 7, 7, 1, 33, 51, 255};
const u8 sp0256time[5] __attribute__ ((section (".flash")))  ={13, 24, 6, 16, 255};
const u8 sp0256bather[5] __attribute__ ((section (".flash")))  ={63, 20, 54, 51, 255};
const u8 sp0256nineteenth[11] __attribute__ ((section (".flash")))  ={11, 6, 11, 1, 2, 13, 19, 11, 1, 29, 255};
const u8 sp0256starting[10] __attribute__ ((section (".flash")))  ={55, 55, 2, 13, 59, 2, 13, 12, 44, 255};
const u8 sp0256hello[6] __attribute__ ((section (".flash")))  ={27, 7, 45, 15, 53, 255};
const u8 sp0256talk[6] __attribute__ ((section (".flash")))  ={13, 23, 23, 1, 41, 255};


const uint8_t *vocab_sp0256[276] =  {sp0256thirty, sp0256bathing, sp0256twentieth, sp0256gauges, sp0256switched, sp0256september, sp0256talkers, sp0256ninth, sp0256fifty, sp0256four, sp0256gauged, sp0256gauge, sp0256talks, sp0256yes, sp0256fifth, sp0256spells, sp0256fir, sp0256sixteen, sp0256system, sp0256pledged, sp0256thursday, sp0256the, sp0256to, sp0256whale, sp0256systems, sp0256pledges, sp0256then, sp0256seventy, sp0256coop, sp0256march, sp0256checking, sp0256gauging, sp0256freezing, sp0256collide, sp0256october, sp0256five, sp0256month, sp0256correcting, sp0256day, sp0256minute, sp0256bread, sp0256switches, sp0256february, sp0256cognitive, sp0256investigator, sp0256h, sp0256l, sp0256clown, sp0256correct, sp0256p, sp0256intrigued, sp0256ninety, sp0256stopped, sp0256q, sp0256january, sp0256zero, sp0256x, sp0256investigators, sp0256legislates, sp0256seventeen, sp0256talked, sp0256pledge, sp0256ten, sp0256starts, sp0256sensitivity, sp0256thirteenth, sp0256crown, sp0256eighteenth, sp0256d, sp0256engaging, sp0256thread, sp0256litter, sp0256computer, sp0256talker, sp0256legislated, sp0256escape, sp0256twelve, sp0256pi, sp0256calendar, sp0256enraging, sp0256tenth, sp0256saturday, sp0256checkers, sp0256sweats, sp0256for, sp0256clock, sp0256memories, sp0256forty, sp0256stops, sp0256score, sp0256sixth, sp0256sweaters, sp0256checks, sp0256red, sp0256t, sp0256thirtieth, sp0256million, sp0256escaping, sp0256august, sp0256june, sp0256friday, sp0256seventh, sp0256equals, sp0256fifteenth, sp0256wednesday, sp0256fourteen, sp0256cookie, sp0256corrects, sp0256extent, sp0256investigating, sp0256sixty, sp0256talking, sp0256november, sp0256raspberry, sp0256fifteen, sp0256by, sp0256legislature, sp0256c, sp0256ate, sp0256daughter, sp0256g, sp0256of, sp0256k, sp0256equal, sp0256o, sp0256times, sp0256pinning, sp0256april, sp0256s, sp0256plus, sp0256sweat, sp0256w, sp0256whaler, sp0256investigates, sp0256intrigue, sp0256first, sp0256key, sp0256enraged, sp0256investigate, sp0256threading, sp0256tuesday, sp0256one, sp0256sincerely, sp0256nipping, sp0256eleventh, sp0256stopper, sp0256enrages, sp0256twelfth, sp0256seventeenth, sp0256spelling, sp0256speak, sp0256sweating, sp0256little, sp0256checked, sp0256start, sp0256whales, sp0256twenty, sp0256sweater, sp0256engagement, sp0256checker, sp0256intrigues, sp0256pinned, sp0256eleven, sp0256beer, sp0256too, sp0256memory, sp0256whalers, sp0256hundred, sp0256sweated, sp0256sincere, sp0256today, sp0256freezers, sp0256sixteenth, sp0256started, sp0256infinitive, sp0256thousand, sp0256corrected, sp0256sister, sp0256fore, sp0256subjectv, sp0256engage, sp0256uncle, sp0256july, sp0256freeze, sp0256nineteen, sp0256starter, sp0256intriguing, sp0256b, sp0256threads, sp0256spellers, sp0256hour, sp0256f, sp0256frozen, sp0256legislating, sp0256j, sp0256second, sp0256n, sp0256r, sp0256nine, sp0256three, sp0256error, sp0256stopping, sp0256emotional, sp0256z, sp0256thirteen, sp0256and, sp0256bathe, sp0256seven, sp0256v, sp0256pin, sp0256escapes, sp0256december, sp0256is, sp0256year, sp0256am, sp0256spelled, sp0256escaped, sp0256speller, sp0256eighteen, sp0256sincerity, sp0256fourteenth, sp0256ready, sp0256engages, sp0256check, sp0256physical, sp0256legislate, sp0256spell, sp0256rays, sp0256pledging, sp0256no, sp0256divided, sp0256six, sp0256freezer, sp0256sunday, sp0256fourth, sp0256pins, sp0256ray, sp0256nips, sp0256switching, sp0256investigated, sp0256monday, sp0256may, sp0256stop, sp0256eighty, sp0256robots, sp0256threaders, sp0256y, sp0256eight, sp0256two, sp0256letter, sp0256subject, sp0256date, sp0256nip, sp0256eighth, sp0256nipped, sp0256whaling, sp0256a, sp0256threaded, sp0256e, sp0256third, sp0256won, sp0256i, sp0256alarm, sp0256sensitive, sp0256m, sp0256robot, sp0256enrage, sp0256switch, sp0256u, sp0256threader, sp0256time, sp0256bather, sp0256nineteenth, sp0256starting, sp0256hello, sp0256talk};

const uint8_t *vocab_sp0256extra[42] =  {sp_ampere, sp_cent, sp_centi, sp_danger, sp_degree, sp_dollar, sp_feet, sp_farads, sp_fuel, sp_gallon, sp_go, sp_gram, sp_high, sp_higher, sp_inches, sp_it, sp_kilo, sp_less, sp_lesser, sp_limitt, sp_low, sp_lower, sp_milli, sp_microo, sp_minus, sp_number, sp_off, sp_on, sp_percent, sp_pico, sp_please, sp_point, sp_pulses, sp_rate, sp_right, sp_rpm, sp_set, sp_speedddd, sp_than, sp_try, sp_temperature, sp_volt};
