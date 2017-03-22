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
#include "resources.h"

//#include "forlap.h"

/// below is from data.py in no particular order output by print.py - no order of phonemes but parameter data is ordered...

// 0 = ʃ 1 = ʍ 2 = a 3 = ɐ 4 = ɒ 5 = ɔ 6 = ɜ 7 = b 8 = d 9 = f 10 = ɪ 11 = t(3 12 = l 13 = n 14 = p 15 = t 16 = v 17 = z 18 = ɾ 19 = j 20 = ʊ 21 = ʌ 22 = ʒ 23 = ɔj 24 = ʔ 25 = d͡ʒ 26 = θ 27 = ɑw 28 = I 29 = ŋ 30 = t͡ʃ 31 = ɑ 32 = ə 33 = ɛ 34 = ɑj 35 = ɡ 36 = e 37 = g 38 = æ 39 = i 40 = k 41 = m 42 = o 43 = ð 44 = s 45 = u 46 = w 47 = ɹ

/*

test data structure for vocab... need end phoneme = 255

print str(looky.get(phon))+",",
print str(phoneme['voicePitch'])+",",
print str(phoneme['endVoicePitch'])+",",
print str(frameDuration)+",",
print str(fadeDuration)+" },"
 */

typedef struct {
  unsigned char phon;
  float voicePitch;
  float endVoicePitch;
  float frameDuration;
  float fadeDuration;
} nvp_vocab_;

static const nvp_vocab_ nvp_v_worm[] =  { { 46, 110.190511588, 101.54464594, 42.0, 28.0}, { 6, 101.54464594, 83.3883280795, 88.2, 35.0}, { 41, 83.3883280795, 74.7424624317, 42.0, 14.0}, {255, 0.0f, 0.0f, 0.0f, 0.0f}};

const float data[48][39]  __attribute__ ((section (".flash"))) ={
{ 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 75.0f , 225.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 100.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.466666666667f, 0.4f, 0.4f, 0.383333333333 , 0.0f , 1.0f , 0.0f, 0.0f },
{ 290.0f, 610.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 55.0f , 60.0f , 45.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 290.0f, 610.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 50.0f, 80.0f, 60.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 0.0f, 0.75 },
{ 650.0f , 1430.0f , 2500.0f , 3300.0f , 3750.0f , 4900.0f , 250.0f , 200.0f , 116.6f, 76.5f , 178.0f , 250.0f , 200.0f , 1000.0f , 100.0f , 100.0f , 0.0f, 700.0f, 1220.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 130.0f, 70.0f, 160.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 530.0f, 1310.0f, 2400.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 37.5f, 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 620.0f, 1220.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 50.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 620.0f , 1100.0f , 2520.0f , 3300.0f , 3750.0f , 4900.0f , 250.0f , 200.0f , 115.5f, 52.5f, 86.25f , 250.0f , 200.0f , 1000.0f , 100.0f , 100.0f , 0.0f, 700.0f, 1220.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 130.0f, 70.0f, 160.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 450.0f, 870.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 99.0f , 75.0f , 60.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 600.0f, 990.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 90.0f, 100.0f, 80.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 500.0f, 1400.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 110.0f , 45.0f , 82.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 500.0f, 1400.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 100.0f, 60.0f, 110.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 200.0f, 1100.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 75.0f , 97.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 200.0f, 1100.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 100.0f, 130.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 1.0f , 1.0f , 1.0f , 0.0f },
{ 200.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 75.0f , 127.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 200.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 100.0f, 170.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.333333333333 , 0.333333333333 , 0.0f , 0.0f , 0.833333333333 , 0.0f , 1.0f , 1.0f , 0.0f },
{ 340.0f, 1100.0f, 2080.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 90.0f , 112.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 340.0f, 1100.0f, 2080.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 120.0f, 150.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.95f , 1.0f , 0.0f, 0.0f },
{ 360.0f, 1800.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 55.0f , 75.0f , 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 400.0f, 1800.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 50.0f, 100.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 350.0f, 1800.0f, 2820.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 67.5f , 225.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 350.0f, 1800.0f, 2820.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 90.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.366666666667f, 0.5f , 0.433333333333f, 0.433333333333f, 0.0f , 1.0f , 0.0f, 0.0f },
{ 310.0f, 1050.0f, 2880.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 55.0f , 75.0f , 210.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 310.0f, 1050.0f, 2880.0f, 3300.0f, 3750.0f, 4900.0f, 50.0f, 100.0f, 280.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 280.0f, 1700.0f, 2740.0f, 3300.0f, 3750.0f, 4900.0f, 450.0f, 216.0f , 44.0f , 225.0f , 225.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.5f , 480.0f, 1340.0f, 2470.0f, 3300.0f, 3750.0f, 4900.0f, 40.0f, 300.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 400.0f, 1100.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 330.0f , 112.5f , 165.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 400.0f, 1100.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 300.0f, 150.0f, 220.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 1.1f, 1.0f , 0.0f, 0.0f },
{ 400.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 330.0f , 90.0f , 187.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 400.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 300.0f, 120.0f, 250.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.416666666667f, 0.416666666667f, 0.0f , 0.0f , 1.0f , 0.0f , 1.0f , 0.0f, 0.0f },
{ 220.0f, 1100.0f, 2080.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 67.5f, 90.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 220.0f, 1100.0f, 2080.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 90.0f, 120.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.95f, 1.0f , 1.0f , 0.0f },
{ 240.0f, 1390.0f, 2530.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 45.0f , 135.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 240.0f, 1390.0f, 2530.0f, 3300.0f, 3750.0f, 4900.0f, 70.0f, 60.0f, 180.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.866666666667f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 300.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 176.0f , 82.5f, 157.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 300.0f, 1600.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 160.0f, 110.0f, 210.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.316666666667f, 0.433333333333f, 0.5f, 0.516666666667f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 260.0f, 2070.0f, 3020.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 44.0f , 187.5f, 375.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 260.0f, 2070.0f, 3020.0f, 3300.0f, 3750.0f, 4900.0f, 40.0f, 250.0f, 500.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 405.0f, 900.0f, 2420.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 75.0f , 60.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 450.0f, 1100.0f, 2350.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 100.0f, 80.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 620.0f, 1220.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 37.5f, 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 620.0f, 1220.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 50.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 45.0f , 210.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 70.0f, 60.0f, 280.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.466666666667f, 0.4f, 0.4f, 0.383333333333f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 550.0f, 960.0f, 2400.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 37.5f, 97.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 550.0f, 960.0f, 2400.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 50.0f, 130.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 100.0f, 150.0f, 200.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 67.5f, 90.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 100.0f, 150.0f, 200.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 90.0f, 120.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 0.0f, 0.75 },
{ 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 45.0f , 210.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 100.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.466666666667f, 0.4f, 0.4f, 0.383333333333f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 320.0f, 1290.0f, 2540.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 67.5f, 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 320.0f, 1290.0f, 2540.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 90.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.466666666667f, 0.633333333333f, 1.0f , 0.0f, 0.0f },
{ 640.0f, 1230.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 52.5f, 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 640.0f, 1230.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 70.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 400.0f, 1800.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 55.0f , 75.0f , 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 400.0f, 1800.0f, 2570.0f, 3300.0f, 3750.0f, 4900.0f, 50.0f, 100.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 480.0f, 2000.0f, 2900.0f, 3300.0f, 3750.0f, 4900.0f, 450.0f, 216.0f , 44.0f , 225.0f , 225.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.5f, 480.0f, 2000.0f, 2900.0f, 3300.0f, 3750.0f, 4900.0f, 40.0f, 300.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 75.0f , 225.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f , 300.0f, 1840.0f, 2750.0f, 3300.0f, 3750.0f, 4900.0f, 200.0f, 100.0f, 300.0f, 250.0f, 200.0f, 1000.0f, 0.0f , 0.0f , 0.466f, 0.4f, 0.4f, 0.383, 0.0f , 1.0f , 0.0f , 0.0f },
{ 700.0f, 1220.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 143.0f , 52.5f, 120.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 700.0f, 1220.0f, 2600.0f, 3300.0f, 3750.0f, 4900.0f, 130.0f, 70.0f, 160.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 500.0f, 1400.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 110.0f , 45.0f , 82.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 500.0f, 1400.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 100.0f, 60.0f, 110.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 530.0f, 1680.0f, 2500.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 67.5f, 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 530.0f, 1680.0f, 2500.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 90.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 660.0f, 1200.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 110.0f , 52.5f, 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 660.0f, 1200.0f, 2550.0f, 3300.0f, 3750.0f, 4900.0f, 100.0f, 70.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 200.0f, 1990.0f, 2850.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 112.5f, 210.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 200.0f, 1990.0f, 2850.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 150.0f, 280.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.5f, 0.45f, 0.366666666667f, 0.383333333333f, 0.383333333333f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 480.0f, 1720.0f, 2520.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 75.0f , 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 480.0f, 1720.0f, 2520.0f, 3300.0f, 3750.0f, 4900.0f, 70.0f, 100.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 200.0f, 1990.0f, 2850.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 112.5f, 210.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 200.0f, 1990.0f, 2650.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 150.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.8 , 0.65f, 0.366666666667f, 0.383333333333f, 0.383333333333f, 0.0f , 1.0f , 1.0f , 0.0f },
{ 620.0f, 1660.0f, 2430.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 112.5f, 240.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 620.0f, 1660.0f, 2430.0f, 3300.0f, 3750.0f, 4900.0f, 70.0f, 150.0f, 320.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 310.0f, 2020.0f, 2960.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 49.5f, 150.0f , 300.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 310.0f, 2020.0f, 2960.0f, 3300.0f, 3750.0f, 4900.0f, 45.0f, 200.0f, 400.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },
{ 300.0f, 1990.0f, 2850.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 275.0f , 120.0f , 247.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 300.0f, 1990.0f, 2650.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 130.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.8f , 0.633333333333f, 0.366666666667f, 0.383333333333f, 0.383333333333f, 0.0f , 1.0f , 0.0f, 0.0f },

{ 472.0f , 1100.0f, 2130.0f, 3300.0f, 3750.0f, 4900.0f, 450.0f, 216.0f , 44.0f , 150.0f , 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.f, 480.0f, 1270.0f, 2130.0f, 3300.0f, 3750.0f, 4900.0f, 40.0f, 200.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },//41

{ 540.0f, 1100.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 88.0f , 52.5f, 52.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 540.0f, 1100.0f, 2300.0f, 3300.0f, 3750.0f, 4900.0f, 80.0f, 70.0f, 70.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },//42
{ 270.0f, 1290.0f, 2540.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 66.0f , 60.0f , 127.5f, 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 270.0f, 1290.0f, 2540.0f, 3300.0f, 3750.0f, 4900.0f, 60.0f, 80.0f, 170.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.466666666667f, 0.633333333333f, 1.0f , 1.0f , 0.0f },//43
{ 320.0f, 1390.0f, 2530.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 220.0f , 60.0f , 150.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 320.0f, 1390.0f, 2530.0f, 3300.0f, 3750.0f, 5250.0f, 200.0f, 80.0f, 200.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.866666666667f, 0.0f , 1.0f , 0.0f, 0.0f },//44
{ 290.0f, 1350.0f, 2280.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 71.5f, 82.5f, 105.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 350.0f, 1250.0f, 2200.0f, 3300.0f, 3750.0f, 4900.0f, 65.0f , 110.0f, 140.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },//45
{ 290.0f, 610.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 55.0f , 60.0f , 45.0f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 290.0f, 610.0f, 2150.0f, 3300.0f, 3750.0f, 4900.0f, 50.0f, 80.0f, 60.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f },//46
{ 310.0f, 1050.0f, 1350.0f, 3300.0f, 3750.0f, 4900.0f, 250.0f, 200.0f , 77.0f , 75.0f , 112.5f , 250.0f, 200.0f, 1000.0f, 100.0f, 100.0f, 0.0f, 310.0f, 1050.0f, 2050.0f, 3300.0f, 3750.0f, 4900.0f, 70.0f, 100.0f, 150.0f, 250.0f, 200.0f, 1000.0f, 0.0f, 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f , 0.0f, 1.0f , 0.0f }//47
};

unsigned int sampleRRate=32000;

typedef unsigned char bool;

#define false 0
#define true 1

typedef struct {
  unsigned char index;
  float value;
  bool override;
} voicer;

typedef struct {
  unsigned char howmany;
  voicer voices[10];
} voice;

voice benjie= { 9,{ {0,1.01f,0},{1,1.02,0},{2,3770.0f,1},{3,4110.0f,1},{4,5000.0f,1},{7,0.9f,0},{8,1.3f,0},{36,0.7f,0},{34,1.3f,0}}};
voice nullie= { 0,{ {0,1.01f,0},{1,1.02,0},{2,3770.0f,1},{3,4110.0f,1},{4,5000.0f,1},{7,0.9f,0},{8,1.3f,0},{36,0.7f,0},{34,1.3f,0}}};
voice adam= {3,{ {8,1.3f,0},{34,1.3f,0},{36,0.85f,0}}};
voice caleb= {2,{ {38,1.0f,1}, {37,0.0f,1}}};

// add singing voice but this is via setnvpparams
/*
frame.preFormantGain=2.0 // 
frame.voiceAmplitude=1.0 // 37
frame.vibratoPitchOffset=0.125 
frame.vibratoSpeed=5.5
*/

extern float _selx, _sely, _selz;

inline float calculateValueAtFadePosition(float oldVal, float newVal, float curFadeRatio) {
	return oldVal+((newVal-oldVal)*curFadeRatio);
}

typedef float speechPlayer_frameParam_t;

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

speechPlayer_frame_t framer,oldframer, tempframe;

const float PITWO=M_PI*2.0f;

float lastValueOne= 0.0f;
float lastValueTwo= 0.0f;

float getNextNOISE(float* lastValue) {
  *lastValue=((float)rand()/RAND_MAX)+0.75f* *lastValue;
  return *lastValue;
};

float lastCyclePosOne=0.0f;
float lastCyclePosTwo=0.0f;

float getNextFREQ(float* lastCyclePos, float frequency) {
  float cyclePos=fmodf((frequency/sampleRRate)+*lastCyclePos,1);
  *lastCyclePos=cyclePos;
  return cyclePos;
};

bool glottisOpen;

float getNextVOICE(const speechPlayer_frame_t* frame) {
  float vibrato=(sinf(getNextFREQ(&lastCyclePosOne,frame->vibratoSpeed)*PITWO)*0.06*frame->vibratoPitchOffset)+1.0f; // but we need diff instances of getNExtFREQ - DONE
  float voice=getNextFREQ(&lastCyclePosTwo,frame->voicePitch*vibrato);
  float aspiration=getNextNOISE(&lastValueOne)*0.2f; // again noise instancesDONE
  float turbulence=aspiration*frame->voiceTurbulenceAmplitude;
  glottisOpen=voice>=frame->glottalOpenQuotient;
  if(glottisOpen) {
    turbulence*=0.01f;
  }
  voice=(voice*2.0f)-1.0f;
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
		res->p1=0.0f;
		res->p2=0.0f;
};

void setParamsRES(reson *res, float frequency, float bandwidth) {
		if(!res->setOnce||(frequency!=res->frequency)||(bandwidth!=res->bandwidth)) {
			res->frequency=frequency;
			res->bandwidth=bandwidth;
			float r=expf(-M_PI/sampleRRate*bandwidth);
			res->c=-(r*r);
			res->b=r*cosf(PITWO/sampleRRate*-frequency)*2.0f;
			res->a=1.0f-res->b-res->c;
			if(res->anti&&frequency!=0) {
				res->a=1.0f/res->a;
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
		input/=2.0f;
		float n0Output=resonateRES(&rN0,input,frame->cfN0,frame->cbN0);
		float output;
		output=calculateValueAtFadePosition(input,resonateRES(&rNP,n0Output,frame->cfNP,frame->cbNP),frame->caNP);
		output=resonateRES(&r6,output,frame->cf6,frame->cb6);
		output=resonateRES(&r5,output,frame->cf5,frame->cb5);
		output=resonateRES(&r4,output,frame->cf4,frame->cb4);
		output=resonateRES(&r3,output,frame->cf3,frame->cb3);
		output=resonateRES(&r2,output,frame->cf2,frame->cb2);
		output=resonateRES(&r1,output,frame->cf1,frame->cb1);
		return output;
	};

	float getNextPARALLEL(const speechPlayer_frame_t* frame, float input) {
		input/=2.0f;
		float output=0.0f;
		output+=(resonateRES(&rr1,input,frame->pf1,frame->pb1)-input)*frame->pa1;
		output+=(resonateRES(&rr2,input,frame->pf2,frame->pb2)-input)*frame->pa2;
		output+=(resonateRES(&rr3,input,frame->pf3,frame->pb3)-input)*frame->pa3;
		output+=(resonateRES(&rr4,input,frame->pf4,frame->pb4)-input)*frame->pa4;
		output+=(resonateRES(&rr5,input,frame->pf5,frame->pb5)-input)*frame->pa5;
		output+=(resonateRES(&rr6,input,frame->pf6,frame->pb6)-input)*frame->pa6;
		return calculateValueAtFadePosition(output,input,frame->parallelBypass);
};


void change_nvpparams(const speechPlayer_frame_t* frame, float glotty,float prefgain,float vpoffset,float vspeed,float vpitch,float outgain,float envpitch, float voiceamp, float turby);

float* indexy[39];

void init_nvp(void){
  // set up frame, buffer, fill buffer and write as wav following other example votrax?
     float *indexyy[39]={&framer.cf1, &framer.cf2, &framer.cf3, &framer.cf4, &framer.cf5, &framer.cf6, &framer.cfN0, &framer.cfNP,  &framer.cb1, &framer.cb2, &framer.cb3, &framer.cb4, &framer.cb5, &framer.cb6, &framer.cbN0, &framer.cbNP, &framer.caNP, &framer.pf1, &framer.pf2, &framer.pf3, &framer.pf4, &framer.pf5, &framer.pf6, &framer.pb1, &framer.pb2, &framer.pb3, &framer.pb4, &framer.pb5, &framer.pb6, &framer.pa1, &framer.pa2, &framer.pa3, &framer.pa4, &framer.pa5, &framer.pa6, &framer.parallelBypass, &framer.fricationAmplitude, &framer.voiceAmplitude, &framer.aspirationAmplitude}; 

     for (u8 x=0;x<39;x++) indexy[x]=indexyy[x];

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

  //change_nvpparams(1.0, 1.0, 0.125, 5.5, 250.0, 2.0, 0, 1.0, 0); // envpitch=endVoicePitch is unused, 
  change_nvpparams(&framer, 1.0f, 1.0f, 0.0f, 0.0f, 250.0f, 1.0f, 0.0f, 1.0f, 1.0f); // envpitch=endVoicePitch is unused, 
  framer.preFormantGain=0;

   // voice or TEST!
  framer.preFormantGain=0.6f; // keep low-ish
  framer.vibratoPitchOffset=0.1f;
  framer.vibratoSpeed=5.5f;
    //    framer.voicePitch=150;

    // copy into indexy

  for (u8 i=0;i<39;i++){
    *indexy[i]=data[0][i]; // TESTY!
  }

  memcpy(&oldframer, &framer, sizeof(speechPlayer_frame_t)); // no interpol

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

  framer.glottalOpenQuotient=glotty; // fraction between 0 and 1 of a voice cycle that the glottis is open (allows voice turbulance, alters f1...)
  framer.voiceTurbulenceAmplitude=turby;
  framer.preFormantGain=prefgain;
  framer.vibratoPitchOffset=vpoffset;
  framer.vibratoSpeed=vspeed;
  framer.voicePitch=vpitch;
  framer.outputGain=outgain;
  framer.endVoicePitch=envpitch;
}


static u16 this_frame_length, this_interpol;
static float this_pitch, this_pitch_inc;

void nvp_newvoice(voice* voiced){

 voice* voicey= voiced;
 //  voice* voicey= &adam;

 for (u8 i=0;i<voicey->howmany;i++){
      if (voicey->voices[i].override) *indexy[voicey->voices[i].index]=voicey->voices[i].value;
      else *indexy[voicey->voices[i].index]*=voicey->voices[i].value; // TODO apply during samples
 }

}

void nvp_init(){
  init_nvp();
  this_frame_length=3840; this_interpol=1900;
  nvp_newvoice(&benjie); // caleb, adam, nullie, benjie
}

u8 changed=0;

u8 nvp_newsay(){
  static u8 oldframe=0, nextframe=0;
  oldframe=nextframe;
  nextframe=_selz*50.0f;
  MAXED(nextframe,48);
  nextframe=48-nextframe;
  if (oldframe!=nextframe) changed=1;
  else changed=0;

  //  nextframe=42; // fixing that rougue frame
    if (nextframe==0) *indexy[37]=0; // NASALS=29,13,39 and give wierd resonance
  else {
  for (u8 i=0;i<39;i++){
    *indexy[i]=data[nextframe-1][i];
  }
  }
  // frame length, interpol sets to half that and pitch = SELY, SELZ,
    this_frame_length=(int)(4096.0f*_sely)<<2; // sely
  this_interpol=this_frame_length/2;
  // pitch
  return nextframe;
}

u8 nvp_newsay_vocab(){
  static u8 counter=0, nextframe=0;
  // nextframe is next in struct
  nextframe=nvp_v_worm[counter].phon;
  if (nextframe==255) {
    counter=0;
    nextframe=nvp_v_worm[0].phon;    
  }
    if (nextframe==0) *indexy[37]=0; // NASALS=29,13,39 and give wierd resonance
  else {
  for (u8 i=0;i<39;i++){
    *indexy[i]=data[nextframe-1][i];
  }
  }
  // frame length, interpol sets to half that and pitch = SELY, SELZ,
    //    this_frame_length=(int)(4096.0f*_sely)<<2; // sely
      //    this_interpol=this_frame_length/2;
    this_frame_length=nvp_v_worm[counter].frameDuration*(256.0f*_sely);
    this_interpol=nvp_v_worm[counter].fadeDuration;
    this_pitch=nvp_v_worm[counter].voicePitch;
    this_pitch_inc=(nvp_v_worm[counter].endVoicePitch-nvp_v_worm[counter].voicePitch)/this_frame_length;
    counter++;
    changed=1;
    return nextframe;
}


int16_t nvp_get_sample(){
  float val=0;
  unsigned int j;
  static u16 count=0;
  if (count>this_frame_length){
  // is this a new frame?
    memcpy(&oldframer, &framer, sizeof(speechPlayer_frame_t)); // old frame for interpol
    nvp_newsay();
    count=0;
    //    memcpy(&tempframe, &framer, sizeof(speechPlayer_frame_t)); // old frame for interpol TESTING no interpol
  }// new frame - we also need to take care of pitch and pitch interpol

    if (count<this_interpol && changed){
    float curFadeRatio=(float)count/(this_interpol);
    for(j=0;j<speechPlayer_frame_numParams;++j) {
      ((float*)&tempframe)[j]=calculateValueAtFadePosition(((float*)&oldframer)[j],((float*)&framer)[j],curFadeRatio); 
    }
    }

  //  &tempframe=&framer;
  // for pitch interpolates: but we just use our pitch here

    // frameRequest->voicePitchInc=(frame->endVoicePitch-frame->voicePitch)/frameRequest->minNumSamples;
    // newFrameRequest->frame.voicePitch+=(newFrameRequest->voicePitchInc*newFrameRequest->numFadeSamples);
    // and: curFrame.voicePitch+=oldFrameRequest->voicePitchInc;
    // oldFrameRequest->frame.voicePitch=curFrame.voicePitch;
    int16_t vale=1024.0f*(1.0f-_selx);
  tempframe.voicePitch=256.0f*logspeed[vale]; 

  float voice=getNextVOICE(&tempframe);
  float cascadeOut=getNextCASC(&tempframe,voice*tempframe.preFormantGain);
  float fric=getNextNOISE(&lastValueTwo)*0.3f*tempframe.fricationAmplitude;
  float parallelOut=getNextPARALLEL(&tempframe,fric*tempframe.preFormantGain);
  float out=(cascadeOut+parallelOut)*tempframe.outputGain;

  count++;
  out=out*4000.0f;
  if (out>32000.0f) out=32000.0f;
  else if (out<-32000.0f) out=-32000.0f;
   return (int)out;
}

int16_t nvp_get_sample_vocab(){//TODO_ length and pitch rise and fall!
  float val=0;
  unsigned int j;
  static u16 count=0;
  if (count>this_frame_length){
  // is this a new frame?
    memcpy(&oldframer, &framer, sizeof(speechPlayer_frame_t)); // old frame for interpol
    nvp_newsay_vocab();
    count=0;
    //    memcpy(&tempframe, &framer, sizeof(speechPlayer_frame_t)); // old frame for interpol TESTING no interpol
  }// new frame - we also need to take care of pitch and pitch interpol

    if (count<this_interpol && changed){
    float curFadeRatio=(float)count/(this_interpol);
    for(j=0;j<speechPlayer_frame_numParams;++j) {
      ((float*)&tempframe)[j]=calculateValueAtFadePosition(((float*)&oldframer)[j],((float*)&framer)[j],curFadeRatio); 
    }
    }

  //  &tempframe=&framer;
  // for pitch interpolates: but we just use our pitch here
    

		  // frameRequest->voicePitchInc=(frame->endVoicePitch-frame->voicePitch)/frameRequest->minNumSamples;
		  // newFrameRequest->frame.voicePitch+=(newFrameRequest->voicePitchInc*newFrameRequest->numFadeSamples);
		  // and: curFrame.voicePitch+=oldFrameRequest->voicePitchInc;
		  // oldFrameRequest->frame.voicePitch=curFrame.voicePitch;
    int16_t vale=1024.0f*(1.0f-_selx);
    //  tempframe.voicePitch=256.0f*logspeed[vale]; 
    // TEST pitchinc...
    tempframe.voicePitch=this_pitch*logspeed[vale];
    this_pitch+=this_pitch_inc;

  float voice=getNextVOICE(&tempframe);
  float cascadeOut=getNextCASC(&tempframe,voice*tempframe.preFormantGain);
  float fric=getNextNOISE(&lastValueTwo)*0.03f*tempframe.fricationAmplitude; // was 0.3
  float parallelOut=getNextPARALLEL(&tempframe,fric*tempframe.preFormantGain);
  float out=(cascadeOut+parallelOut)*tempframe.outputGain;

  count++;
  out=out*4000.0f;
  if (out>32000.0f) out=32000.0f;
  else if (out<-32000.0f) out=-32000.0f;
   return (int)out;
}

