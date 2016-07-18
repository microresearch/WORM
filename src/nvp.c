/// ARM revert

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

#include "stm32f4xx.h"
#include "audio.h"

/// below is from data.py in no particular order output by print.py - no order of phonemes but parameter data is ordered...

// 0 = ʃ 1 = ʍ 2 = a 3 = ɐ 4 = ɒ 5 = ɔ 6 = ɜ 7 = b 8 = d 9 = f 10 = ɪ 11 = t(3 12 = l 13 = n 14 = p 15 = t 16 = v 17 = z 18 = ɾ 19 = j 20 = ʊ 21 = ʌ 22 = ʒ 23 = ɔj 24 = ʔ 25 = d͡ʒ 26 = θ 27 = ɑw 28 = I 29 = ŋ 30 = t͡ʃ 31 = ɑ 32 = ə 33 = ɛ 34 = ɑj 35 = ɡ 36 = e 37 = g 38 = æ 39 = i 40 = k 41 = m 42 = o 43 = ð 44 = s 45 = u 46 = w 47 = ɹ


const float data[48][39]={
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

unsigned int sampleRRate=32000;

typedef unsigned char bool;

#define false 0
#define true 1

inline float calculateValueAtFadePosition(float oldVal, float newVal, float curFadeRatio) {
	return oldVal+((newVal-oldVal)*curFadeRatio);
}

typedef float speechPlayer_frameParam_t;

typedef struct {
  unsigned char index;
  float value;
  bool override;
} voicer;

typedef struct {
  unsigned char howmany;
  voicer voices[10];
} voice;

voice benjie= { 9,{ {0,1.01,0},{1,1.02,0},{2,3770,1},{3,4110,1},{4,5000,1},{7,0.9,0},{8,1.3,0},{36,0.7,0},{34,1.3,0}}};
voice nullie= { 0,{ {0,1.01,0},{1,1.02,0},{2,3770,1},{3,4110,1},{4,5000,1},{7,0.9,0},{8,1.3,0},{36,0.7,0},{34,1.3,0}}};
voice adam= {3,{ {8,1.3,0},{34,1.3,0},{36,0.85,0}}};
voice caleb= {2,{ {38,1,1}, {37,0,1}}};

typedef struct {
  // varying globally depending on voice - but we still need to interpolate 
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

speechPlayer_frame_t framer,oldframer, tempframer;

const float PITWO=M_PI*2;

float lastValueOne= 0.0;
float lastValueTwo= 0.0;

float getNextNOISE(float* lastValue) {
  *lastValue=((float)rand()/RAND_MAX)+0.75* *lastValue;
  return *lastValue;
};

float lastCyclePosOne=0.0;
float lastCyclePosTwo=0.0;

float getNextFREQ(float* lastCyclePos, float frequency) {
  float cyclePos=fmodf((frequency/sampleRRate)+*lastCyclePos,1);
  *lastCyclePos=cyclePos;
  return cyclePos;
};

bool glottisOpen;

float getNextVOICE(const speechPlayer_frame_t* frame) {
  float vibrato=(sinf(getNextFREQ(&lastCyclePosOne,frame->vibratoSpeed)*PITWO)*0.06*frame->vibratoPitchOffset)+1; // but we need diff instances of getNExtFREQ - DONE
  float voice=getNextFREQ(&lastCyclePosTwo,frame->voicePitch*vibrato);
  float aspiration=getNextNOISE(&lastValueOne)*0.2; // again noise instancesDONE
  float turbulence=aspiration*frame->voiceTurbulenceAmplitude;
  glottisOpen=voice>=frame->glottalOpenQuotient;
  if(glottisOpen) {
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


reson r1,r2,r3,r4,r5,r6,rN0,rNP;
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

	float getNextCASC(const speechPlayer_frame_t* frame, float input) {
		input/=2.0;
		float n0Output=resonateRES(&rN0,input,frame->cfN0,frame->cbN0);
		float output;
		output=calculateValueAtFadePosition(input,resonateRES(&rNP,n0Output,frame->cfNP,frame->cbNP),frame->caNP);
		//		output=resonateRES(&rNP,n0Output,frame->cfNP,frame->cbNP);
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
		return calculateValueAtFadePosition(output,input,frame->parallelBypass);
};

speechPlayer_frame_t *framerr,*oldframerr, *tempframe;

unsigned int generateSpeechWave(const speechPlayer_frame_t* frame, u16 sampleCount,u16 interpol, int16_t* sampleBuf) {

  // TODO: samplecounternew as static and then get next frame from array when we hit length (which might change) and write to audio_buffer
  // so length needs to be defined - but not interpolated
		float val=0;
		unsigned int i,j;
		for(i=0;i<sampleCount;i++) {		  

		  if (i<interpol){
		    float curFadeRatio=(float)i/(interpol);
		    for(j=0;j<speechPlayer_frame_numParams;++j) {
		      ((float*)tempframe)[j]=calculateValueAtFadePosition(((float*)oldframerr)[j],((float*)frame)[j],curFadeRatio);  // should not be frame here but guess is same...
			      }
 }

		  // for pitch interpolates:

		  // frameRequest->voicePitchInc=(frame->endVoicePitch-frame->voicePitch)/frameRequest->minNumSamples;
		  // newFrameRequest->frame.voicePitch+=(newFrameRequest->voicePitchInc*newFrameRequest->numFadeSamples);
		  // and: curFrame.voicePitch+=oldFrameRequest->voicePitchInc;
		  // oldFrameRequest->frame.voicePitch=curFrame.voicePitch;

				float voice=getNextVOICE(tempframe);
				float cascadeOut=getNextCASC(tempframe,voice*tempframe->preFormantGain);
				float fric=getNextNOISE(&lastValueTwo)*0.3*tempframe->fricationAmplitude;
				float parallelOut=getNextPARALLEL(tempframe,fric*tempframe->preFormantGain);
				float out=(cascadeOut+parallelOut)*tempframe->outputGain;
				//				printf("%f\n",out);
				sampleBuf[i]=out*4000;
				
				if (sampleBuf[i]>32767) sampleBuf[i]=32767;
				if (sampleBuf[i]<-32767) sampleBuf[i]=-32767;
		}
		return sampleCount;
	};


void change_nvpparams(const speechPlayer_frame_t* frame, float glotty,float prefgain,float vpoffset,float vspeed,float vpitch,float outgain,float envpitch, float voiceamp, float turby);

float *indexy[39];


void init_nvp(void){
  // set up frame, buffer, fill buffer and write as wav following other example votrax?

  INITRES(&r1,0);
  INITRES(&r2,0);
  INITRES(&r3,0);
  INITRES(&r4,0);
  INITRES(&r5,0);
  INITRES(&r6,0);
  INITRES(&rN0,0);
  INITRES(&rNP,0);
  INITRES(&rr1,0);
  INITRES(&rr2,0);
  INITRES(&rr3,0);
  INITRES(&rr4,0);
  INITRES(&rr5,0);
  INITRES(&rr6,0);

  framerr=&framer;
  //change_nvpparams(1.0, 1.0, 0.125, 5.5, 250.0, 2.0, 0, 1.0, 0); // envpitch=endVoicePitch is unused, 
  change_nvpparams(framerr, 1.0, 1.0, 0, 0, 250.0, 1.0, 0, 1.0, 1.0); // envpitch=endVoicePitch is unused, 
  framerr->preFormantGain=0;

  oldframerr=&oldframer;
  tempframe=&tempframer;

   // this shouldn't change
  
   float *indexy[39]={&framerr->cf1, &framerr->cf2, &framerr->cf3, &framerr->cf4, &framerr->cf5, &framerr->cf6, &framerr->cfN0, &framerr->cfNP,  &framerr->cb1, &framerr->cb2, &framerr->cb3, &framerr->cb4, &framerr->cb5, &framerr->cb6, &framerr->cbN0, &framerr->caNP, &framerr->caNP, &framerr->pf1, &framerr->pf2, &framerr->pf3, &framerr->pf4, &framerr->pf5, &framerr->pf6, &framerr->pb1, &framerr->pb2, &framerr->pb3, &framerr->pb4, &framerr->pb5, &framerr->pb6, &framerr->pa1, &framerr->pa2, &framerr->pa3, &framerr->pa4, &framerr->pa5, &framerr->pa6, &framerr->parallelBypass, &framerr->fricationAmplitude, &framerr->voiceAmplitude, &framerr->aspirationAmplitude}; 

   // voice or TEST!
    framerr->preFormantGain=1.0;
    framerr->vibratoPitchOffset=0.1;
    framerr->vibratoSpeed=5.5;
    framerr->voicePitch=150;

    // copy into indexy

  for (u8 i=0;i<39;i++){
    *indexy[i]=data[0][i]; // TESTY!
  }

  memcpy(oldframerr, framerr, sizeof(speechPlayer_frame_t)); // no interpol

}

void change_nvpparams(const speechPlayer_frame_t* frame,float glotty,float prefgain,float vpoffset,float vspeed,float vpitch,float outgain,float envpitch, float voiceamp, float turby){

  /// globals and sets of voices
  /// also start and end pitch
  // constants are for singing:

  /*
frame.preFormantGain=2.0 or 1.0 for not singing
frame.voiceAmplitude=1.0
frame.vibratoPitchOffset=0.125 - change this - maybe singing and voice modes and worm mode TODO!
frame.vibratoSpeed=5.5 - above

pitch is fundamental and endpitch is used to get voicepitchinc as in: 

newFrameRequest->frame.voicePitch+=(newFrameRequest->voicePitchInc*newFrameRequest->numFadeSamples);

frameRequest->voicePitchInc=(frame->endVoicePitch-frame->voicePitch)/frameRequest->minNumSamples;

outputgain: frame.outputGain=2.0 unless we need silence!

  */

  framerr->glottalOpenQuotient=glotty; // fraction between 0 and 1 of a voice cycle that the glottis is open (allows voice turbulance, alters f1...)
  framerr->voiceTurbulenceAmplitude=turby;
  framerr->preFormantGain=prefgain;
  framerr->vibratoPitchOffset=vpoffset;
  framerr->vibratoSpeed=vspeed;
  framerr->voicePitch=vpitch;
  framerr->outputGain=outgain;
  framerr->endVoicePitch=envpitch;
}

void run_nvpframe(void){ // TODO: scheduling so most of this in generate above

  int16_t framebuffer[1024]; 
  int i;
  //  fo = fopen("testnvp.pcm", "wb");
  framerr=&framer;
  oldframerr=&oldframer;
  tempframe=&tempframer;

  // TODO: this is init OR not?
  
  //   float *indexy[39]={&framerr->cf1, &framerr->cf2, &framerr->cf3, &framerr->cf4, &framerr->cf5, &framerr->cf6, &framerr->cfN0, &framerr->cfNP,  &framerr->cb1, &framerr->cb2, &framerr->cb3, &framerr->cb4, &framerr->cb5, &framerr->cb6, &framerr->cbN0, &framerr->caNP, &framerr->caNP, &framerr->pf1, &framerr->pf2, &framerr->pf3, &framerr->pf4, &framerr->pf5, &framerr->pf6, &framerr->pb1, &framerr->pb2, &framerr->pb3, &framerr->pb4, &framerr->pb5, &framerr->pb6, &framerr->pa1, &framerr->pa2, &framerr->pa3, &framerr->pa4, &framerr->pa5, &framerr->pa6, &framerr->parallelBypass, &framerr->fricationAmplitude, &framerr->voiceAmplitude, &framerr->aspirationAmplitude}; 

   // test for one phrase TODO: remove
  int c=0;
  unsigned char sayhan[5]={38,38,13,31,31};

    framerr->preFormantGain=1.0;
    framerr->vibratoPitchOffset=0.1;
    framerr->vibratoSpeed=5.5;
    framerr->voicePitch=150;

    while(c<5){

      unsigned char random=rand()%48; 
    
    random=sayhan[c];
    if (c==0) *indexy[37]=0;
    else *indexy[37]=1;
    if (c==2) framerr->voicePitch=100;
    if (c==3) framerr->voicePitch=90;
    if (c==4) framerr->preFormantGain=0;


  for (i=0;i<39;i++){
    *indexy[i]=data[random][i];
  }


// voice* voicey= &benjie;
 voice* voicey= &nullie;
 //  voice* voicey= &adam;

  /// the howmany is the number of params we change!
 for (i=0;i<voicey->howmany;i++){
      if (voicey->voices[i].override) *indexy[voicey->voices[i].index]=voicey->voices[i].value;
      else *indexy[voicey->voices[i].index]*=voicey->voices[i].value;
 }

 if (c==0)   {
   memcpy(oldframerr, framerr, sizeof(speechPlayer_frame_t)); // no interpol
   framerr->preFormantGain=0;
 }
 else     framerr->preFormantGain=1.0;

    c++;

//still question of length of phoneme/number of frames/samples?

// what are limits for phoneme length?

// in ipa.py def calculatePhonemeTimes(phonemeList,baseSpeed) speed also to vary which is 1;;; or we use phonemetime as variable

    generateSpeechWave(framerr,880,400,framebuffer); // why 880 and 400? is 880 size of frame or?

 memcpy(oldframerr, framerr, sizeof(speechPlayer_frame_t)); 
    }
   
}

static u16 this_frame_length, this_interpol;

void nvp_init(){
  init_nvp();
  this_frame_length=3840; this_interpol=1900;
}

void nvp_newsay(){
    // what do we need to re_init?

}

int16_t nvp_get_sample(){
  float val=0;
  unsigned int i,j;
  static u16 count=0;
  if (count>this_frame_length){
  // is this a new frame?
    memcpy(oldframerr, framerr, sizeof(speechPlayer_frame_t)); // old frame for interpol
    // how do we choose next frame
    unsigned char random=rand()%48;
    // copy in frame
  for (i=0;i<39;i++){
    *indexy[i]=data[random][i];
  }

  }// new frame - we also need to take care of pitch and pitch interpol

  if (i<this_interpol){
    float curFadeRatio=(float)i/(this_interpol);
    for(j=0;j<speechPlayer_frame_numParams;++j) {
      ((float*)tempframe)[j]=calculateValueAtFadePosition(((float*)oldframerr)[j],((float*)framerr)[j],curFadeRatio); 
    }
  }

		  // for pitch interpolates:

		  // frameRequest->voicePitchInc=(frame->endVoicePitch-frame->voicePitch)/frameRequest->minNumSamples;
		  // newFrameRequest->frame.voicePitch+=(newFrameRequest->voicePitchInc*newFrameRequest->numFadeSamples);
		  // and: curFrame.voicePitch+=oldFrameRequest->voicePitchInc;
		  // oldFrameRequest->frame.voicePitch=curFrame.voicePitch;

  float voice=getNextVOICE(tempframe);
  float cascadeOut=getNextCASC(tempframe,voice*tempframe->preFormantGain);
  float fric=getNextNOISE(&lastValueTwo)*0.3*tempframe->fricationAmplitude;
  float parallelOut=getNextPARALLEL(tempframe,fric*tempframe->preFormantGain);
  float out=(cascadeOut+parallelOut)*tempframe->outputGain;
  //				printf("%f\n",out);
  //sampleBuf[i]=out*4000;
  return out*4000.0f; 

  count++;
}
