/*#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"
*/

// first lap test and plot

#include "stdio.h"

typedef unsigned char UINT8;
typedef unsigned char u8;
typedef signed char INT8;
typedef unsigned short UINT16;
typedef unsigned short u16;
typedef signed short INT16;
typedef unsigned int UINT32;
typedef signed int INT32;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;

///

typedef struct xy {
  float x,y;
} xy;

typedef struct wormy {
  xy wloc;
  void (*wormfunction)(struct wormy *worms, u8 boundx, u8 boundy); 
  // speed, maxspeed, tail[], counter, directionxY, tailcount, SW, targetXY, accXY, velXY, how we do angles/up/down?

}wormy;

//............ worm movement is tied to a parameter list

// >>>>>>>>>>>>>>>helper functions?

u8 randy(u8 range){
  return rand()%range;
}

u8 randyx(u8 xnum,u8 xnum){
  u16 x=666;
  while(x==xnum || x==666){
    x=randy(num);
  }
  return x;
}

float runiform(float a, float b){

  //a + (b-a) * random()

  // smallest random= but no double on ARM
  float x = a + (b-a) * ((float)rand()/(float)(RAND_MAX));
  //

}

float rrr(float ranger){
    float r= (runiform(ranger/-2, ranger/2),runiform(ranger/-2, ranger/2)) 
    return r;
}

void normalize(xy* loc){
  float x=loc->x;  
  float y=loc->y;
  float mag=sqrtf(x*x + y*y);
  if (mag!=0) {
      x=x/mag;
        y=y/mag;
	  }
  loc->x=x;
  loc->y=y;
}

void limit(xy* loc, float limit){
  float x=loc->x;  
  float y=loc->y;
  
  if (sqrtf(x*x + y*y) > limit*limit){
    normalize(loc);
    x=loc->x*limit
      y=loc->y*limit
	  }
  loc->x=x;
  loc->y=y;
}

// check x and y against bounds

// >>>>>>>>>>>>>>>>what are the worms?

/*            'basicworm': self.wander,
            'straightworm':self.straight,
            'seeker':self.seek,
            'squiggler':self.squiggler
*/
  
void wanderworm(struct wormy *worms, u8 boundx, u8 boundy){
  u8 x=worms->posx;
  u8 y=worms->posy;

  // do movements and copy back

}

wormy* addworm(u8 x, u8 y){
  wormy* worm = malloc(sizeof(wormy));
  worm->posx=x;
  worm->posy=y;
  worm->wormfunction=followworm;
  return worm;
}
