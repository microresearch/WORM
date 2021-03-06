#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "forlap.h"
#include "resources.h"
#include "LPC/roms/vocab_2303.h"


typedef struct {
  float samplepos;
  u8 phonem, trigger;
  u16 storeone,storetwo,storethree;
  int16_t lastsample;
  int16_t prevsample;
}genny;

genny* allgen;

int16_t sp0256_get_sample(){
  static int16_t xx=0;
  xx+=1;
  return xx;

}

const float data[48][39]  __attribute__ ((section (".flash"))) ={
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 75.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 , 0 , 0.0 },
{ 290 , 610 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 60.0 , 45.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 290 , 610 , 2150 , 3300 , 3750 , 4900 , 50 , 80 , 60 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 0 , 0.75 },
{ 650.0 , 1430.0 , 2500.0 , 3300.0 , 3750.0 , 4900.0 , 250.0 , 200.0 , 116.6 , 76.5 , 178.0 , 250.0 , 200.0 , 1000.0 , 100.0 , 100.0 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 530 , 1310 , 2400 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 80 , 50 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 620.0 , 1100.0 , 2520.0 , 3300.0 , 3750.0 , 4900.0 , 250.0 , 200.0 , 115.5 , 52.5 , 86.25 , 250.0 , 200.0 , 1000.0 , 100.0 , 100.0 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 450 , 870 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 99.0 , 75.0 , 60.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 600 , 990 , 2570 , 3300 , 3750 , 4900 , 90 , 100 , 80 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 45.0 , 82.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 100 , 60 , 110 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 200 , 1100 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 75.0 , 97.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1100 , 2150 , 3300 , 3750 , 4900 , 60 , 100 , 130 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1.05 , 1.0 , 1.0 , 0.0 },
{ 200 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 75.0 , 127.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1600 , 2600 , 3300 , 3750 , 4900 , 60 , 100 , 170 , 250 , 200 , 1000 , 0 , 0.333333333333 , 0.333333333333 , 0.0 , 0.0 , 0.833333333333 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 340 , 1100 , 2080 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 90.0 , 112.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 340 , 1100 , 2080 , 3300 , 3750 , 4900 , 200 , 120 , 150 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.95 , 1.0 , 0 , 0.0 },
{ 360 , 1800 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 50 , 100 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 350 , 1800 , 2820 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 67.5 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 350 , 1800 , 2820 , 3300 , 3750 , 4900 , 200 , 90 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.366666666667 , 0.5 , 0.433333333333 , 0.433333333333 , 0.0 , 1.0 , 0 , 0.0 },
{ 310 , 1050 , 2880 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 1050 , 2880 , 3300 , 3750 , 4900 , 50 , 100 , 280 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 280 , 1700 , 2740 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 225.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 1340 , 2470 , 3300 , 3750 , 4900 , 40 , 300 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 400 , 1100 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 330.0 , 112.5 , 165.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1100 , 2150 , 3300 , 3750 , 4900 , 300 , 150 , 220 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1.1 , 1.0 , 0 , 0.0 },
{ 400 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 330.0 , 90.0 , 187.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1600 , 2600 , 3300 , 3750 , 4900 , 300 , 120 , 250 , 250 , 200 , 1000 , 0 , 0.416666666667 , 0.416666666667 , 0.0 , 0.0 , 1.0 , 0.0 , 1.0 , 0 , 0.0 },
{ 220 , 1100 , 2080 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 90.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 220 , 1100 , 2080 , 3300 , 3750 , 4900 , 60 , 90 , 120 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.95 , 1.0 , 1.0 , 0.0 },
{ 240 , 1390 , 2530 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 135.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 240 , 1390 , 2530 , 3300 , 3750 , 4900 , 70 , 60 , 180 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.866666666667 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 300 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 176.0 , 82.5 , 157.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1600 , 2600 , 3300 , 3750 , 4900 , 160 , 110 , 210 , 250 , 200 , 1000 , 0 , 0.0 , 0.316666666667 , 0.433333333333 , 0.5 , 0.516666666667 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 260 , 2070 , 3020 , 3300 , 3750 , 4900 , 250 , 200.0 , 44.0 , 187.5 , 375.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 260 , 2070 , 3020 , 3300 , 3750 , 4900 , 40 , 250 , 500 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 405 , 900 , 2420 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 75.0 , 60.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 450 , 1100 , 2350 , 3300 , 3750 , 4900 , 80 , 100 , 80 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 80 , 50 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 70 , 60 , 280 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 550 , 960 , 2400 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 97.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 550 , 960 , 2400 , 3300 , 3750 , 4900 , 80 , 50 , 130 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 100 , 150 , 200 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 90.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 100 , 150 , 200 , 3300 , 3750 , 4900 , 60 , 90 , 120 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 0 , 0.75 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 320 , 1290 , 2540 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 67.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 320 , 1290 , 2540 , 3300 , 3750 , 4900 , 200 , 90 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.466666666667 , 0.633333333333 , 1.0 , 0 , 0.0 },
{ 640 , 1230 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 52.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 640 , 1230 , 2550 , 3300 , 3750 , 4900 , 80 , 70 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 50 , 100 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 480 , 2000 , 2900 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 225.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 2000 , 2900 , 3300 , 3750 , 4900 , 40 , 300 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 75.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 , 0 , 0.0 },
{ 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 143.0 , 52.5 , 120.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 45.0 , 82.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 100 , 60 , 110 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 530 , 1680 , 2500 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 530 , 1680 , 2500 , 3300 , 3750 , 4900 , 60 , 90 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 660 , 1200 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 52.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 660 , 1200 , 2550 , 3300 , 3750 , 4900 , 100 , 70 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 112.5 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 60 , 150 , 280 , 250 , 200 , 1000 , 0 , 0.5 , 0.45 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 480 , 1720 , 2520 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 75.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 480 , 1720 , 2520 , 3300 , 3750 , 4900 , 70 , 100 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 112.5 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1990 , 2650 , 3300 , 3750 , 4900 , 60 , 150 , 200 , 250 , 200 , 1000 , 0 , 0.8 , 0.65 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 , 1.0 , 0.0 },
{ 620 , 1660 , 2430 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 112.5 , 240.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1660 , 2430 , 3300 , 3750 , 4900 , 70 , 150 , 320 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 310 , 2020 , 2960 , 3300 , 3750 , 4900 , 250 , 200.0 , 49.5 , 150.0 , 300.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 2020 , 2960 , 3300 , 3750 , 4900 , 45 , 200 , 400 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 300 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 275.0 , 120.0 , 247.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1990 , 2650 , 3300 , 3750 , 4900 , 250 , 130 , 200 , 250 , 200 , 1000 , 0 , 0.8 , 0.633333333333 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 , 0 , 0.0 },
{ 472 , 1100 , 2130 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 150.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 1270 , 2130 , 3300 , 3750 , 4900 , 40 , 200 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 540 , 1100 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 52.5 , 52.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 540 , 1100 , 2300 , 3300 , 3750 , 4900 , 80 , 70 , 70 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 270 , 1290 , 2540 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 60.0 , 127.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 270 , 1290 , 2540 , 3300 , 3750 , 4900 , 60 , 80 , 170 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.466666666667 , 0.633333333333 , 1.0 , 1.0 , 0.0 },
{ 320 , 1390 , 2530 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 60.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 320 , 1390 , 2530 , 3300 , 3750 , 5250 , 200 , 80 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.866666666667 , 0.0 , 1.0 , 0 , 0.0 },
{ 290 , 1350 , 2280 , 3300 , 3750 , 4900 , 250 , 200.0 , 71.5 , 82.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 350 , 1250 , 2200 , 3300 , 3750 , 4900 , 65 , 110 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 290 , 610 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 60.0 , 45.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 290 , 610 , 2150 , 3300 , 3750 , 4900 , 50 , 80 , 60 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 },
{ 310 , 1050 , 1350 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 75.0 , 112.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 1050 , 2050 , 3300 , 3750 , 4900 , 70 , 100 , 150 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 , 1.0 , 0.0 }
};


u16 sp0256(genny* genstruct, int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // MODEL GENERATOR: TODO is speed and interpolation options DONE
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
float samplepos=genstruct->samplepos;
int16_t samplel=genstruct->lastsample;
int16_t lastval=genstruct->prevsample;

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=UPSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       printf("outgoing %d xx: %d\n",outgoing[xx],xx);
       //       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=sp0256_get_sample();

       // say speed is 2.0 we want every second sample....
       if (samplepos>=samplespeed){
	 outgoing[xx]=samplel;
       printf("outgoing %d xx: %d\n",outgoing[xx],xx);
       samplepos-=samplespeed;
     
       // TEST trigger: 
	 xx++;
       }
    samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 genstruct->samplepos=samplepos;
 genstruct->lastsample=samplel;
 genstruct->prevsample=lastval;
 return size;
};

float speedOfSound(float temperature)
{
    return (331.4 + (0.6 * temperature));
}

float indexy[39];

float testindex(){
  return indexy[0];
}

static unsigned char state_=1;
static float target_=0.0,rate_=0.001;

float doenvelope(){
  static float value_=0.0;
  if ( state_ ) {
    if ( target_ > value_ ) {
      value_ += rate_;
      if ( value_ >= target_ ) {
        value_ = target_;
        state_ = 0;
      }
    }
    else {
      value_ -= rate_;
      if ( value_ <= target_ ) {
        value_ = target_;
        state_ = 0;
      }
    }
  }
    return value_;
   
}

typedef struct{ // 44 bytes
	float m_freq, m_decayTime, m_attackTime;
	float m_y01, m_y02; //changes
	float m_b01, m_b02;
	float m_y11, m_y12;//changes
	float m_b11, m_b12;
} Formlet;



void Formlet_setfreq(Formlet *unit, float frequency){
  const float log001=logf(0.001);
  const float mRadiansPerSample=(2 * 3.12) /32000.0f;
  float b01,b02,b11,b12;
  float attackTime = unit->m_attackTime;
  float decayTime = unit->m_decayTime;
  float ffreq = frequency * mRadiansPerSample;
  
  float R = decayTime == 0.f ? 0.f : expf(log001/(decayTime * 32000.0f));
  float twoR = 2.f * R;
  float R2 = R * R;
  float temp, sint;
  //  arm_sin_cos_f32(57.29578 *ffreq, &sint, &temp); 
  //    temp=arm_cos_f32(ffreq);
    temp=cosf(ffreq);
  //  float cost = (twoR * cosf(ffreq)) / (1.f + R2);
  float cost = (twoR * temp) / (1.f + R2);

  b01 = twoR * cost;
  b02 = -R2;
  
  R = attackTime == 0.f ? 0.f : expf(log001/(attackTime * 32000.0f));
  twoR = 2.f * R;
  R2 = R * R;
  //cost = (twoR * cosf(ffreq)) / (1.f + R2);
  cost = (twoR * temp) / (1.f + R2);
  b11 = twoR * cost;
  b12 = -R2;

  // add slopes?
  //  printf("%f %f %f %f\n",b01,b02,b11,b12);
  unit->m_b01 = b01;
  unit->m_b02 = b02;
  unit->m_b11 = b11;
  unit->m_b12 = b12;
}

void Formlet_init(Formlet* unit){
  //  unit->m_freq = frequency;
  //  unit->m_bw = bandwidth;
  unit->m_attackTime = 0.01f; // was both 0.001
  unit->m_decayTime = 0.5f;
  unit->m_y01 = 0.f;
  unit->m_y02 = 0.f;
  unit->m_y11 = 0.f;
  unit->m_y12 = 0.f;
  Formlet_setfreq(unit,1000);
}

void Formlet_process(Formlet *unit, int inNumSamples, float* inbuffer, float* outbuffer){

  float y00;
  float y10;
  float y01 = unit->m_y01;
  float y11 = unit->m_y11;
  float y02 = unit->m_y02;
  float y12 = unit->m_y12;

  float b01 = unit->m_b01;
  float b11 = unit->m_b11;
  float b02 = unit->m_b02;
  float b12 = unit->m_b12;
  float ain;

  /*  for (u8 i=0;i<inNumSamples;i++){
  ain = inbuffer[i];
  y00 = ain + b01 * y01 + b02 * y02;
  y10 = ain + b11 * y11 + b12 * y12;
  printf("Y00 %f %f %f %f\n",y00,y10,y01,y02);
    outbuffer[i] = 0.25* ((y00 - y02) - (y10 - y12)); //was 0.25*
  //  outbuffer[i]=inbuffer[i];
    printf("%f, ",outbuffer[i]);
  y02 = y01;
  y01 = y00;
  y12 = y11;
  y11 = y10;
  }
  */
  unit->m_y01 = y01;
  unit->m_y02 = y02;
  unit->m_y11 = y11;
  unit->m_y12 = y12;
}

#define WIDTH 16                /* this controls the number of neighboring samples
				   which are used to interpolate the new samples.  The
				   processing time is linearly related to this width */

#define SAMPLES_PER_ZERO_CROSSING 32    /* this defines how finely the sinc function */

float sinc_table[WIDTH * SAMPLES_PER_ZERO_CROSSING] = { 0.0 };

#define PI 3.14159263

void make_sinc()
{
    int i;
    double temp,win_freq,win;
    win_freq = PI / WIDTH / SAMPLES_PER_ZERO_CROSSING;
    sinc_table[0] = 1.0;
    for (i=1;i<WIDTH * SAMPLES_PER_ZERO_CROSSING;i++)   {
	temp = (double) i * PI / SAMPLES_PER_ZERO_CROSSING;
	sinc_table[i] = sin(temp) / temp;
	win = 0.5 + 0.5 * cos(win_freq * i);
	sinc_table[i] *= win;
	printf("%f, ", sinc_table[i]);

    }
}
#define XXXX



void main(){
  //  indexy={1.0,1.0}; 
  float x=0.0f;
  int y=0;
  int xx;
  int xv=0xf;
  printf("xxxxxxx %d\n", xv);
  int m_pc=128;
  int d0;
  unsigned char X=16;
  float samplespeed=128.0f;
  allgen=malloc(sizeof(genny));		
  allgen->samplepos=0.0f;
  int16_t mono_buffer[32];
  int16_t sample_buffer[32];
  float carrierbuffer[32], voicebuffer[32],otherbuffer[32];
  float value,   smoothed_adc_value=0, filter_coeff=0.05f;
  //(0x7f ^ (m_inflection << 4) ^ m_filt_f1) + 1) // 7f=127
  unsigned char m_rom_f2=8;
  //   unsigned char tmp=m_rom_f2+(64-(_sely*128.0f));
  //   printf("TMPPPP:     %d\n", tmp);


  //  printf("*sample=(((int16_t)(X)-8)<<12); //1 byte %d", (((int16_t)(-4)-1)<<10)); //1 byte

  printf("32768>>1 %d >>2 %d >>3 %d\n",32768>>1, 32768>>2, 32768>>3); 
  
#ifdef XXXX
  printf("xxxxxxxxxxxxxxxxx\n\n\n\n");
#endif  

typedef struct TMS_vocab__ {
  // pointer to const
  const uint8_t **wordlist;	

  uint16_t extent;
  float extentplus;
} TMS_vocab;

 TMS_vocab vocab_2303={wordlist_spell2303, 102, 104.0f};

int16_t mins[40]= {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};
int16_t maxs[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

 int pittch=-1;
 

 // printf("\n\nPICHBASE %f\n", (440.0f * powf(2.0f,(((float)(pittch+3.0f))/12.0f))));

 // test compost

 signed char dir=1;

 uint16_t comp_counter=0;

 float max=100.0f;
 float valuee=70.0f;
 float our=1.0f;

 float ending= our*valuee; // our skews the whole logarithmically
 
 // printf("\nRANGED: %f\n", ending);

 
 for (int bbb=0;bbb<3200000;bbb++){


   
   u16 startx=rand()%32768;
   u16 endy=rand()%32768;
 if (startx>endy){
    dir=-1;
    if (comp_counter<=endy) comp_counter=startx;
  }
  else {
    dir=1;
    if (comp_counter>=endy) comp_counter=startx;
  }
    comp_counter+=dir;
 
    //    if (comp_counter>32768)  printf("XXXXXXX comp count %d start %d end %d dir %d\n", comp_counter, startx, endy, dir);

 }
 


 


 for (X=0;X<40;X++){
   //   printf("%d, ", maxs[X]-mins[X]);
 }
 // printf("\n\n");

 float contour[3]={0.1,0.2,0.3};

 // printf("VOCAB %d\n", *(vocab_2303.wordlist[0]+1));

  make_sinc();

#define TOTAL_SECTIONS            10 
 float controlRate=16.0;

 float length=18.0;
 float cc = speedOfSound(20.0f);
 int controlPeriod =   rint((cc * TOTAL_SECTIONS * 100.0) /(length * controlRate));
 int sampleRate = controlRate * controlPeriod;
 sampleRate= rint((cc * TOTAL_SECTIONS * 100.0)/length);

 float actualTubeLength = (cc * TOTAL_SECTIONS * 100.0) / sampleRate;
 float nyquist = (float)sampleRate / 2.0;

 // printf("TUBES: controlp %d samplerate %d tubelength %f nyquist %f\n",controlPeriod, sampleRate, actualTubeLength, nyquist);



 /*
	if (modus&1) {
	  u8 val=_selx*130.0f;
	  MAXED(val,127);
	  pitchmod=pitches[Y]*logpitch[val];
	}
	else if (modus&8) {
	  u8 val=exy[0]*130.0f;
	  MAXED(val,127);
	  pitchmod=pitches[Y]*logpitch[val];
	}	
	else
	  pitchmod=pitches[Y];
 */
 printf("\n\n");
 
 for (int u=0;u<128;u++){
   //   	  else f->per=64.0f*logspeed[val];
   int fper=64.0f*logpitch[u];
   printf("%d   ",fper);
 }
 
static const int pitch_vals[32] = {
	97, 95, 92, 89, 87, 84, 82, 80, 77, 75, 73, 71, 69, 67, 65, 63,
	61, 60, 58, 56, 55, 53, 52, 50, 49, 48, 46, 45, 43, 42, 41, 40
};


 unsigned char val=1.0*127.0f; 
 // int m_pitch = pitch_vals[10] * logpitch[val];
 //  printf("mmmmmmmmmmmmmmmmmPPPP: %d\n",m_pitch); 


  // with exy=0.0// exy=0.99
  float _sely=0.95f;
  float exy=1.0f-_sely; // no multiplier and inverted here
  int m_new_frame_energy_idx = exy*15.0f; //
  //  printf("sely=0.9: result=%d\n",m_new_frame_energy_idx); 
  m_new_frame_energy_idx=16;
  exy=0.45;
  m_new_frame_energy_idx*=2*(1.0f-exy);
  //  printf("DDDDDDDDDDDDDDDDDDDDDDDDDDDd: result=%d\n",m_new_frame_energy_idx); 

   
  // test coeff for ADC:
  for (y=0;y<128;y++){
  value=(rand()%255)/255.0f;
  smoothed_adc_value += filter_coeff * (value - smoothed_adc_value);
  //  printf(" %f smoothed: %f\n",value, smoothed_adc_value);
  }
  
  int16_t m_inflection=64;
  int16_t m_filt_f1=16;
  int16_t pitch=(0x7f ^ (m_inflection) ^ m_filt_f1) + 1; // TTS uses inflection // nothing else - what are m_filt_f1 values?
  //  printf("PPPP: %d\n",pitch); 
  
  int m_mainclock = 720000; // TODO as we need m_mainclock - 	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000)

  float m_sclock = m_mainclock / 18.0f;
  //  printf("cllll: %f\n",m_sclock); 
  //  float exy=0.4f;
  int xzy=320;
  //
  
  int m_rom_closure  = exy+0.5f; // does this give us 1 or 0?
  //  printf("is it: %d\n",xzy-(exy*640.0f)); 


  
  //  for( int i = 11 - 1; i > 0; i-- ) printf("%d\n", i);

  //(256-(exy[2 + 2*j]*512.0));

  float tester=0.5f; 
  y=(256-(tester*512.0));
  //  printf("FFFFF: %d\n",y); 

  for (y=0;y<3200;y++){
    x=sinf(y);
    //    printf("sinf(%d)=%f\n",y,x);
  }


  y=128;

  //  xx= (0x4000 << 3);
  //  printf("XXXX %d",xx);
  target_=0.00000;

  Formlet* formy;
  formy=malloc(sizeof(Formlet));
  Formlet_init(formy);
  for (y=0;y<32;y++){
    carrierbuffer[y]=(float) ( 2.0 * rand() / (RAND_MAX + 1.0) - 1.0 );
  }
  Formlet_process(formy, 32, carrierbuffer,otherbuffer);
  for (y=0;y<32;y++){
    //   carrierbuffer[y]=(float)rand()/32768.0f;
    //    printf("%f %f\n",otherbuffer[y],carrierbuffer[y]);
  }



  for (y=0;y<39;y++){
    x=doenvelope();

    //const float data[48][39]  __attribute__ ((section (".flash"))) ={
    //    printf("DOENV: %f\n",x);
    }

  //  float c = speedOfSound(32);
  //  int controlPeriod =    rint((c * 10 * 100.0) /(18.0 * 1.0));
  //  int sampleRate = 4.0 * controlPeriod;

  //  printf("controlperiod %d\n",controlPeriod);

  float ind=testindex();

  //  printf("indexy %f\n",ind);

#define TABLE_LENGTH              512
#define TUBEPI                        3.1415927f


  /*  for (int i = 0; i < TABLE_LENGTH; i++) {
    float wavetable = sinf( ((float)i/(float)TABLE_LENGTH) * 2.0 * TUBEPI );
    printf("%f, ", wavetable);
    }*/

  int i,j; float wavetable;
 
  float tp=30.0f;
  float tnMin=10.0f;
  float tnMax=50.0f;

  int tableDiv1 = rint(TABLE_LENGTH * (tp / 100.0));
  int tableDiv2 = rint(TABLE_LENGTH * ((tp + tnMax) / 100.0));
  float  tnLength = tableDiv2 - tableDiv1;
  float tnDelta = rint(TABLE_LENGTH * ((tnMax - tnMin) / 100.0));
  // print also tndelta
 
  //const int    tableDiv1=154; // this is with 30, 32 
  //const int    tableDiv2=317; //
  //const float tnLength=163.0f; // how is that calc -> glottal calc with tp and tnmax

  printf("//{ 0, %d, %d, %f, %f,   },\n\n", tableDiv1, tableDiv2, tnLength, tnDelta);

  printf("static const float pulsetableXXX[512] __attribute__ ((section (\".flash\"))) = {");

	for (i = 0; i < tableDiv1; i++) {
	    float x = (float)i / (float)tableDiv1;
	    float x2 = x * x;
	    float x3 = x2 * x;
	    wavetable = (3.0 * x2) - (2.0 * x3);
	    printf("%f, ", wavetable);
	}

	/*  CALCULATE FALL PORTION OF WAVE TABLE  */
	for (i = tableDiv1, j = 0; i < tableDiv2; i++, j++) {
	    float x = (float)j / tnLength;
	    wavetable = 1.0 - (x * x);
	    printf("%f, ", wavetable);
	}

	/*  SET CLOSED PORTION OF WAVE TABLE  */
	for (i = tableDiv2; i < TABLE_LENGTH; i++){
	    wavetable = 0.0;
	printf("%f, ", wavetable);
}
	printf("};\n\n");


  /*  while(1){

    //  x=generators[mode](allgen,sample_buffer,mono_buffer,samplespeed,sz/2); 
    xx=sp0256(allgen,sample_buffer,mono_buffer,samplespeed,32);
    }*/

  //  int idx0 = (m_pc    ) >> 3, d0; //???

  //  printf("IDX %d\n", idx0);

  //#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))


  //printf("ELEMENTS: %d\n", NELEMS(crow_coeffs));

  //  printf("SIZE %d",sizeof(sam_vocab));

  // signed short yyy=((X&15)<<12)-32768;

    //int yyy=((X-8)<<12); //1 byte

  // printf("%d\n",yyy);

  /*  for (xx=0;xx<100;xx++){
    x+=11143521654621;
    y=(int)x;
    //    printf("%d %f\n",y,x);

    printf("%d\n",a[y%10]);

    //  if (x==y) printf("OKAY\n");
    }*/
}
