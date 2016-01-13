/// testing first on laptop
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
//#include "stm32f4xx.h"
//#include "audio.h"

typedef unsigned int u16;


float data[48][37]={
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 75.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 },
{ 290 , 610 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 60.0 , 45.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 290 , 610 , 2150 , 3300 , 3750 , 4900 , 50 , 80 , 60 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 650.0 , 1430.0 , 2500.0 , 3300.0 , 3750.0 , 4900.0 , 250.0 , 200.0 , 116.6 , 76.5 , 178.0 , 250.0 , 200.0 , 1000.0 , 100.0 , 100.0 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 530 , 1310 , 2400 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 80 , 50 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 620.0 , 1100.0 , 2520.0 , 3300.0 , 3750.0 , 4900.0 , 250.0 , 200.0 , 115.5 , 52.5 , 86.25 , 250.0 , 200.0 , 1000.0 , 100.0 , 100.0 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 450 , 870 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 99.0 , 75.0 , 60.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 600 , 990 , 2570 , 3300 , 3750 , 4900 , 90 , 100 , 80 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 45.0 , 82.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 100 , 60 , 110 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 200 , 1100 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 75.0 , 97.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1100 , 2150 , 3300 , 3750 , 4900 , 60 , 100 , 130 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1.05 , 1.0 },
{ 200 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 75.0 , 127.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1600 , 2600 , 3300 , 3750 , 4900 , 60 , 100 , 170 , 250 , 200 , 1000 , 0 , 0.333333333333 , 0.333333333333 , 0.0 , 0.0 , 0.833333333333 , 0.0 , 1.0 },
{ 340 , 1100 , 2080 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 90.0 , 112.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 340 , 1100 , 2080 , 3300 , 3750 , 4900 , 200 , 120 , 150 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.95 , 1.0 },
{ 360 , 1800 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 50 , 100 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 350 , 1800 , 2820 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 67.5 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 350 , 1800 , 2820 , 3300 , 3750 , 4900 , 200 , 90 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.366666666667 , 0.5 , 0.433333333333 , 0.433333333333 , 0.0 , 1.0 },
{ 310 , 1050 , 2880 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 1050 , 2880 , 3300 , 3750 , 4900 , 50 , 100 , 280 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 280 , 1700 , 2740 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 225.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 1340 , 2470 , 3300 , 3750 , 4900 , 40 , 300 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 400 , 1100 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 330.0 , 112.5 , 165.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1100 , 2150 , 3300 , 3750 , 4900 , 300 , 150 , 220 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 1.1 , 1.0 },
{ 400 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 330.0 , 90.0 , 187.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1600 , 2600 , 3300 , 3750 , 4900 , 300 , 120 , 250 , 250 , 200 , 1000 , 0 , 0.416666666667 , 0.416666666667 , 0.0 , 0.0 , 1.0 , 0.0 , 1.0 },
{ 220 , 1100 , 2080 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 90.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 220 , 1100 , 2080 , 3300 , 3750 , 4900 , 60 , 90 , 120 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.95 , 1.0 },
{ 240 , 1390 , 2530 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 135.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 240 , 1390 , 2530 , 3300 , 3750 , 4900 , 70 , 60 , 180 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.866666666667 , 0.0 , 1.0 },
{ 300 , 1600 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 176.0 , 82.5 , 157.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1600 , 2600 , 3300 , 3750 , 4900 , 160 , 110 , 210 , 250 , 200 , 1000 , 0 , 0.0 , 0.316666666667 , 0.433333333333 , 0.5 , 0.516666666667 , 0.0 , 1.0 },
{ 260 , 2070 , 3020 , 3300 , 3750 , 4900 , 250 , 200.0 , 44.0 , 187.5 , 375.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 260 , 2070 , 3020 , 3300 , 3750 , 4900 , 40 , 250 , 500 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 405 , 900 , 2420 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 75.0 , 60.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 450 , 1100 , 2350 , 3300 , 3750 , 4900 , 80 , 100 , 80 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1220 , 2550 , 3300 , 3750 , 4900 , 80 , 50 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 70 , 60 , 280 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 },
{ 550 , 960 , 2400 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 37.5 , 97.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 550 , 960 , 2400 , 3300 , 3750 , 4900 , 80 , 50 , 130 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 100 , 150 , 200 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 90.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 100 , 150 , 200 , 3300 , 3750 , 4900 , 60 , 90 , 120 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 45.0 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 },
{ 320 , 1290 , 2540 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 67.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 320 , 1290 , 2540 , 3300 , 3750 , 4900 , 200 , 90 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.466666666667 , 0.633333333333 , 1.0 },
{ 640 , 1230 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 52.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 640 , 1230 , 2550 , 3300 , 3750 , 4900 , 80 , 70 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 75.0 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 400 , 1800 , 2570 , 3300 , 3750 , 4900 , 50 , 100 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 480 , 2000 , 2900 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 225.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 2000 , 2900 , 3300 , 3750 , 4900 , 40 , 300 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 75.0 , 225.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1840 , 2750 , 3300 , 3750 , 4900 , 200 , 100 , 300 , 250 , 200 , 1000 , 0 , 0.0 , 0.466666666667 , 0.4 , 0.4 , 0.383333333333 , 0.0 , 1.0 },
{ 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 250 , 200.0 , 143.0 , 52.5 , 120.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 700 , 1220 , 2600 , 3300 , 3750 , 4900 , 130 , 70 , 160 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 45.0 , 82.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 500 , 1400 , 2300 , 3300 , 3750 , 4900 , 100 , 60 , 110 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 530 , 1680 , 2500 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 67.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 530 , 1680 , 2500 , 3300 , 3750 , 4900 , 60 , 90 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 660 , 1200 , 2550 , 3300 , 3750 , 4900 , 250 , 200.0 , 110.0 , 52.5 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 660 , 1200 , 2550 , 3300 , 3750 , 4900 , 100 , 70 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 112.5 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 60 , 150 , 280 , 250 , 200 , 1000 , 0 , 0.5 , 0.45 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 },
{ 480 , 1720 , 2520 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 75.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 480 , 1720 , 2520 , 3300 , 3750 , 4900 , 70 , 100 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 200 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 112.5 , 210.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 200 , 1990 , 2650 , 3300 , 3750 , 4900 , 60 , 150 , 200 , 250 , 200 , 1000 , 0 , 0.8 , 0.65 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 },
{ 620 , 1660 , 2430 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 112.5 , 240.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 620 , 1660 , 2430 , 3300 , 3750 , 4900 , 70 , 150 , 320 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 310 , 2020 , 2960 , 3300 , 3750 , 4900 , 250 , 200.0 , 49.5 , 150.0 , 300.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 2020 , 2960 , 3300 , 3750 , 4900 , 45 , 200 , 400 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 300 , 1990 , 2850 , 3300 , 3750 , 4900 , 250 , 200.0 , 275.0 , 120.0 , 247.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 300 , 1990 , 2650 , 3300 , 3750 , 4900 , 250 , 130 , 200 , 250 , 200 , 1000 , 0 , 0.8 , 0.633333333333 , 0.366666666667 , 0.383333333333 , 0.383333333333 , 0.0 , 1.0 },
{ 472 , 1100 , 2130 , 3300 , 3750 , 4900 , 450 , 216.0 , 44.0 , 150.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 1.0 , 480 , 1270 , 2130 , 3300 , 3750 , 4900 , 40 , 200 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 540 , 1100 , 2300 , 3300 , 3750 , 4900 , 250 , 200.0 , 88.0 , 52.5 , 52.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 540 , 1100 , 2300 , 3300 , 3750 , 4900 , 80 , 70 , 70 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 270 , 1290 , 2540 , 3300 , 3750 , 4900 , 250 , 200.0 , 66.0 , 60.0 , 127.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 270 , 1290 , 2540 , 3300 , 3750 , 4900 , 60 , 80 , 170 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.466666666667 , 0.633333333333 , 1.0 },
{ 320 , 1390 , 2530 , 3300 , 3750 , 4900 , 250 , 200.0 , 220.0 , 60.0 , 150.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 320 , 1390 , 2530 , 3300 , 3750 , 5250 , 200 , 80 , 200 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.866666666667 , 0.0 , 1.0 },
{ 290 , 1350 , 2280 , 3300 , 3750 , 4900 , 250 , 200.0 , 71.5 , 82.5 , 105.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 350 , 1250 , 2200 , 3300 , 3750 , 4900 , 65 , 110 , 140 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 290 , 610 , 2150 , 3300 , 3750 , 4900 , 250 , 200.0 , 55.0 , 60.0 , 45.0 , 250 , 200 , 1000 , 100 , 100 , 0 , 290 , 610 , 2150 , 3300 , 3750 , 4900 , 50 , 80 , 60 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 },
{ 310 , 1050 , 1350 , 3300 , 3750 , 4900 , 250 , 200.0 , 77.0 , 75.0 , 112.5 , 250 , 200 , 1000 , 100 , 100 , 0 , 310 , 1050 , 2050 , 3300 , 3750 , 4900 , 70 , 100 , 150 , 250 , 200 , 1000 , 0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0 }
};


/*
This file is a part of the NV Speech Player project. 
URL: https://bitbucket.org/nvaccess/speechplayer
Copyright 2014 NV Access Limited.
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2.0, as published by
the Free Software Foundation.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
This license can be found at:
http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
*/

/*
Based on klsyn-88, found at http://linguistics.berkeley.edu/phonlab/resources/
*/

/*
finish c port// need frame management?

*/

unsigned int sampleRRate=32000;

typedef unsigned char bool;

#define false 0
#define true 1



inline float calculateValueAtFadePosition(float oldVal, float newVal, float curFadeRatio) {
	return oldVal+((newVal-oldVal)*curFadeRatio);
}


//#define _USE_MATH_DEFINES

typedef float speechPlayer_frameParam_t;

/// the parameters - see below from python

/*
  
  frame.preFormantGain=1.0
  frame.outputGain=2.0

vary these:
frame.preFormantGain=1.0
frame.vibratoPitchOffset=0.1
frame.vibratoSpeed=5.5
frame.voicePitch=150


	_curPitch=118
	_curVoice='Adam'
	_curInflection=0.5
	_curVolume=1.0 	   
	_curRate=1.0

voices={
	'Adam':{
		'cb1_mul':1.3,
		'pa6_mul':1.3,
		'fricationAmplitude_mul':0.85,
	},
		'Benjamin':{
		'cf1_mul':1.01,
		'cf2_mul':1.02,
		#'cf3_mul':0.96,
		'cf4':3770,
		'cf5':4100,
		'cf6':5000,
		'cfNP_mul':0.9,
		'cb1_mul':1.3,
		'fricationAmplitude_mul':0.7,
		'pa6_mul':1.3,
	},
	'Caleb ':{
		'aspirationAmplitude':1,
		'voiceAmplitude':0,
	},
	'David':{
		'voicePitch_mul':0.75,
		'endVoicePitch_mul':0.75,
		'cf1_mul':0.75,
		'cf2_mul':0.85,
		'cf3_mul':0.85,
	},
}

def applyVoiceToFrame(frame,voiceName):
	v=voices[voiceName]
	for paramName in (x[0] for x in frame._fields_):
		absVal=v.get(paramName)
		if absVal is not None:
			setattr(frame,paramName,absVal)
		mulVal=v.get('%s_mul'%paramName)
		if mulVal is not None:
			setattr(frame,paramName,getattr(frame,paramName)*mulVal)
*/

typedef struct {
	// voicing and cascaide

  // most are covered from VoiceAmp to parallelbypass below
  // and what of the rest????

  // varying globally depending on voice
	speechPlayer_frameParam_t voicePitch; //  fundermental frequency of voice (phonation) in hz
	speechPlayer_frameParam_t vibratoPitchOffset; // pitch is offset up or down in fraction of a semitone
	speechPlayer_frameParam_t vibratoSpeed; // Speed of vibrato in hz
	speechPlayer_frameParam_t voiceTurbulenceAmplitude; // amplitude of voice breathiness from 0 to 1 
  ////
	speechPlayer_frameParam_t glottalOpenQuotient; // fraction between 0 and 1 of a voice cycle that the glottis is open (allows voice turbulance, alters f1...)
  ////// here on is in phonem params:
	speechPlayer_frameParam_t voiceAmplitude; // amplitude of voice (phonation) source between 0 and 1.
	speechPlayer_frameParam_t aspirationAmplitude; // amplitude of aspiration (voiceless h, whisper) source between 0 and 1.
	speechPlayer_frameParam_t cf1, cf2, cf3, cf4, cf5, cf6, cfN0, cfNP; // frequencies of standard cascaide formants, nasal (anti) 0 and nasal pole in hz
	speechPlayer_frameParam_t cb1, cb2, cb3, cb4, cb5, cb6, cbN0, cbNP; // bandwidths of standard cascaide formants, nasal (anti) 0 and nasal pole in hz
	speechPlayer_frameParam_t caNP; // amplitude from 0 to 1 of cascade nasal pole formant
	// fricatives and parallel
	speechPlayer_frameParam_t fricationAmplitude; // amplitude of frication noise from 0 to 1.
	speechPlayer_frameParam_t pf1, pf2, pf3, pf4, pf5, pf6; // parallel formants in hz
	speechPlayer_frameParam_t pb1, pb2, pb3, pb4, pb5, pb6; // parallel formant bandwidths in hz
	speechPlayer_frameParam_t pa1, pa2, pa3, pa4, pa5, pa6; // amplitude of parallel formants between 0 and 1
	speechPlayer_frameParam_t parallelBypass; // amount of signal which should bypass parallel resonators from 0 to 1 - what is/where?
  /// here
	speechPlayer_frameParam_t preFormantGain; // amplitude from 0 to 1 of all vocal tract sound (voicing, frication) before entering formant resonators. Useful for stopping/starting speech
	speechPlayer_frameParam_t outputGain; // amplitude from 0 to 1 of final output (master volume) 
	speechPlayer_frameParam_t endVoicePitch; //  pitch of voice at the end of the frame length  - see ipa.py
} speechPlayer_frame_t;

const int speechPlayer_frame_numParams=sizeof(speechPlayer_frame_t)/sizeof(speechPlayer_frameParam_t);

speechPlayer_frame_t framer;

const float PITWO=M_PI*2;

float lastValueOne= 0.0;
float lastValueTwo= 0.0;

float getNextNOISE(float lastValue) {
  lastValue=((float)rand()/RAND_MAX)+0.75*lastValue;
  return lastValue;
};

float lastCyclePosOne=0.0;
float lastCyclePosTwo=0.0;

float getNextFREQ(float lastCyclePos, float frequency) {
  float cyclePos=fmodf((frequency/sampleRRate)+lastCyclePos,1);
  lastCyclePos=cyclePos;
  return cyclePos;
};

//FrequencyGenerator pitchGen;
//FrequencyGenerator vibratoGen;
//NoiseGenerator aspirationGen;

bool glottisOpen;
//VoiceGenerator(int sr): pitchGen(sr), vibratoGen(sr), aspirationGen(), glottisOpen(false) {};


float getNextVOICE(const speechPlayer_frame_t* frame) {
  float vibrato=(sinf(getNextFREQ(lastCyclePosOne,frame->vibratoSpeed)*PITWO)*0.06*frame->vibratoPitchOffset)+1; // but we need diff instances of getNExtFREQ - DONE
  float voice=getNextFREQ(lastCyclePosTwo,frame->voicePitch*vibrato);
  float aspiration=getNextNOISE(lastValueOne)*0.2; // again noise instancesDONE
  float turbulence=aspiration*frame->voiceTurbulenceAmplitude;
  glottisOpen=voice>=frame->glottalOpenQuotient;
  if(!glottisOpen) {
    turbulence*=0.01;
  }
  voice=(voice*2)-1;
  voice+=turbulence;
  voice*=frame->voiceAmplitude;
  aspiration*=frame->aspirationAmplitude;
  return aspiration+voice;
};


typedef struct{
float frequency;
float bandwidth;
bool anti;
bool setOnce;
float a, b, c;
 float p1, p2;
}reson;


reson r1,r2,r3,r4,r5,r6,rN0;
  reson rr1,rr2,rr3,rr4,rr5,rr6;


void INITRES(reson *res, bool anti) {
		res->anti=anti;
		res->setOnce=false;
		res->p1=0;
		res->p2=0;
};

void setParamsRES(reson *res, float frequency, float bandwidth) {
		if(!res->setOnce||(frequency!=res->frequency)||(bandwidth!=res->bandwidth)) {
			res->frequency=frequency;
			res->bandwidth=bandwidth;
			float r=exp(-M_PI/sampleRRate*bandwidth);
			res->c=-(r*r);
			res->b=r*cosf(PITWO/sampleRRate*-frequency)*2.0;
			res->a=1.0-res->b-res->c;
			if(res->anti&&frequency!=0) {
				res->a=1.0/res->a;
				res->c*=-res->a;
				res->b*=-res->a;
			}
		}
		res->setOnce=true;
};

float resonateRES(reson *res, float in, float frequency, float bandwidth) {
  setParamsRES(res,frequency,bandwidth);
  float out=res->a*in+res->b*res->p1+res->c*res->p2;
		res->p2=res->p1;
		res->p1=res->anti?in:out;
		return out;
};

//class CascadeFormantGenerator { 

//	public:
//	CascadeFormantGenerator(int sr): sampleRRate(sr), r1(sr), r2(sr), r3(sr), r4(sr), r5(sr), r6(sr), rN0(sr,true), rNP(sr) {};

	float getNextCASC(const speechPlayer_frame_t* frame, bool glottisOpen, float input) {
		input/=2.0;
		float n0Output=resonateRES(&rN0,input,frame->cfN0,frame->cbN0);
		float output;
		//TODO		//		float output=calculateValueAtFadePosition(input,rNP.resonate(n0Output,frame->cfNP,frame->cbNP),frame->caNP);
		    output=resonateRES(&r6,output,frame->cf6,frame->cb6);
		    output=resonateRES(&r5,output,frame->cf5,frame->cb5);
		    output=resonateRES(&r4,output,frame->cf4,frame->cb4);
		    output=resonateRES(&r3,output,frame->cf3,frame->cb3);
		    output=resonateRES(&r2,output,frame->cf2,frame->cb2);
		    output=resonateRES(&r1,output,frame->cf1,frame->cb1);
		return output;
	};





	float getNextPARALLEL(const speechPlayer_frame_t* frame, float input) {
		input/=2.0;
		float output=0;
		output+=(resonateRES(&rr1,input,frame->pf1,frame->pb1)-input)*frame->pa1;
		output+=(resonateRES(&rr2,input,frame->pf2,frame->pb2)-input)*frame->pa2;
		output+=(resonateRES(&rr3,input,frame->pf3,frame->pb3)-input)*frame->pa3;
		output+=(resonateRES(&rr4,input,frame->pf4,frame->pb4)-input)*frame->pa4;
		output+=(resonateRES(&rr5,input,frame->pf5,frame->pb5)-input)*frame->pa5;
		output+=(resonateRES(&rr6,input,frame->pf6,frame->pb6)-input)*frame->pa6;
//		return calculateValueAtFadePosition(output,input,frame->parallelBypass);
		return output;
};

/*
class SpeechWaveGeneratorImpl: public SpeechWaveGenerator {
	private:
	int sampleRRate;
	VoiceGenerator voiceGenerator;
	NoiseGenerator fricGenerator;
	CascadeFormantGenerator cascade;
	ParallelFormantGenerator parallel;
	FrameManager* frameManager;

	public:
	SpeechWaveGeneratorImpl(int sr): sampleRRate(sr), voiceGenerator(sr), fricGenerator(), cascade(sr), parallel(sr), frameManager(NULL) {
	}
*/

// TODO: init all generators res etc, how to handle frames and frame parameters


void handleFrame(const speechPlayer_frame_t* frame){
  // init frame with data - read in one test frame, and how long is frame in terms of sampleCount???? see frameManager?

  // pass on to generateSpeechWave(const speechPlayer_frame_t* frame, const unsigned int sampleCount, u16* sampleBuf);

};


	unsigned int generateSpeechWave(const speechPlayer_frame_t* frame, const unsigned int sampleCount,signed int* sampleBuf) {
	  //		if(!frameManager) return 0; 
		float val=0;
		unsigned int i;
		for(i=0;i<sampleCount;++i) {
		  //		const speechPlayer_frame_t* frame=frameManager->getCurrentFrame();
		  //	if(frame) {


				float voice=getNextVOICE(frame);
				float cascadeOut=getNextCASC(frame,glottisOpen,voice*frame->preFormantGain);
				float fric=getNextNOISE(lastValueTwo)*0.3*frame->fricationAmplitude;
				float parallelOut=getNextPARALLEL(frame,fric*frame->preFormantGain);
				float out=(cascadeOut+parallelOut)*frame->outputGain;
				//				printf("%f\n",out);
				sampleBuf[i]=out*4000;
				
				if (sampleBuf[i]>32767) sampleBuf[i]=32767;
				if (sampleBuf[i]<-32767) sampleBuf[i]=-32767;
				//								printf("%d\n",sampleBuf[i]);
				//	} else {
				//				return i;
				//			}
				//		}
		}
		return sampleCount;
	       
	};


void main(void){
  // set up frame, buffer, fill buffer and write as wav following other example votrax?

  signed int framebuffer[1024]; int i;

  FILE * fo;

  // open file to write
  fo = fopen("testnvp.pcm", "wb");
  speechPlayer_frame_t *framerr=&framer;

  /// globals and sets of voices
  /// also start and end pitch
  framerr->preFormantGain=1.0;
  framerr->vibratoPitchOffset=0.1;
  framerr->vibratoSpeed=5.5;
  framerr->voicePitch=150;

  framerr->outputGain=1.0;
  framerr->endVoicePitch=200;

  /*	speechPlayer_frameParam_t voicePitch; //  fundermental frequency of voice (phonation) in hz
	speechPlayer_frameParam_t vibratoPitchOffset; // pitch is offset up or down in fraction of a semitone
	speechPlayer_frameParam_t vibratoSpeed; // Speed of vibrato in hz
	speechPlayer_frameParam_t voiceTurbulenceAmplitude; // amplitude of voice breathiness from 0 to 1 

	speechPlayer_frameParam_t preFormantGain; // amplitude from 0 to 1 of all vocal tract sound (voicing, frication) before entering formant resonators. Useful for stopping/starting speech
	speechPlayer_frameParam_t outputGain; // amplitude from 0 to 1 of final output (master volume) 
	speechPlayer_frameParam_t endVoicePitch; //  pitch of voice at the end of the frame length  - see ipa.py so not used here?
  */

  // read in from array data[random][x]

  while(1){
  INITRES(&r1,0);
  INITRES(&r2,0);
  INITRES(&r3,0);
  INITRES(&r4,0);
  INITRES(&r5,0);
  INITRES(&r6,0);
  INITRES(&rN0,0);
  INITRES(&rr1,0);
  INITRES(&rr2,0);
  INITRES(&rr3,0);
  INITRES(&rr4,0);
  INITRES(&rr5,0);
  INITRES(&rr6,0);

  unsigned char random=rand()%48; 

framerr->cf1=data[random][0];
framerr->cf2=data[random][1];
framerr->cf3=data[random][2];
framerr->cf4=data[random][3];
framerr->cf5=data[random][4];
framerr->cf6=data[random][5];
framerr->cfN0=data[random][6];
framerr->cfNP=data[random][7];
framerr->cb1=data[random][8];
framerr->cb2=data[random][9];
 framerr->cb3=data[random][10];
 framerr->cb4=data[random][11];
 framerr->cb5=data[random][12];
 framerr->cb6=data[random][13];
 framerr->cbN0=data[random][14];
framerr->cbNP=data[random][15];
framerr->caNP=data[random][16];
framerr->pf1=data[random][17];
framerr->pf2=data[random][18];
framerr->pf3=data[random][19];
framerr->pf4=data[random][20];
framerr->pf5=data[random][21];
framerr->pf6=data[random][22];
framerr->pb1=data[random][23];
framerr->pb2=data[random][24];
framerr->pb3=data[random][25];
framerr->pb4=data[random][26];
framerr->pb5=data[random][27];
framerr->pb6=data[random][28];
framerr->pa1=data[random][29];
framerr->pa2=data[random][30];
framerr->pa3=data[random][31];
framerr->pa4=data[random][32];
framerr->pa5=data[random][33];
framerr->pa6=data[random][34];
framerr->parallelBypass=data[random][35];
framerr->fricationAmplitude=data[random][36];


 framerr->voiceAmplitude=1.0;
  // call generatespeechwave
  generateSpeechWave(framerr,1000,framebuffer);


  for (i=0;i<1000;i++){
    int s16=framebuffer[i];
    //    printf("%d\n",framebuffer[i]);

    unsigned char c = (unsigned)s16 & 255;
    fwrite(&c, 1, 1, fo);
    c = ((unsigned)s16 / 256) & 255;
    fwrite(&c, 1, 1, fo);
  }
}
}
