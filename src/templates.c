// log pitch

#include "resources.h"

u8 val=_selx*130.0f;
MAXED(val,127);
val=127-val;
logpitch[val];


// or finer with logspeed

int val=_selx*1027.0f;
MAXED(val,1023);
val=1023-val;
logspeed[val];

const int16_t mins[40] __attribute__ ((section (".flash"))) = {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};

const int16_t maxs[40] __attribute__ ((section (".flash"))) = {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

const int16_t range[40] __attribute__ ((section (".flash"))) ={3800, 70, 1100, 960, 2450, 960, 3799, 960, 3799, 960, 3799, 960, 3799, 1960, 280, 960, 280, 960, 70, 55, 80, 24, 80, 40, 80, 960, 80, 960, 80, 960, 80, 960, 80, 960, 80, 1960, 80, 80, 70, 60};

frame[y]=mins[y] + (range[y]*(1.0f-exy[y])); // TODO: floated loggy!


///

extern float _selx, _sely, _selz;
