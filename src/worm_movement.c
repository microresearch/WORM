/*#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"
*/

// first lap test and plot

#include "stdio.h"
#include "stdlib.h"
#include "math.h"


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
  xy acc;
  xy vel;
  xy dir;
  float speed; // is it float?
  float maxspeed;
  float parameter;
  void (*wormfunction)(struct wormy *worms, float boundx, float boundy); 
  // speed, maxspeed, tail[], counter, directionxY, tailcount, SW, targetXY, accXY, velXY, how we do angles/up/down?

}wormy;

//............ worm movement is tied to a parameter list

// >>>>>>>>>>>>>>>helper functions?

u8 randy(u8 range){
  return rand()%range;
}

u8 randyx(u8 xnum,u8 num){
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
  return x;

}

void rrr(float ranger, xy* rrrr){
 rrrr->x= (runiform(ranger/-2, ranger/2)); 
 rrrr->y= (runiform(ranger/-2, ranger/2)); 
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
    x=loc->x*limit;
    y=loc->y*limit;
	  }
  loc->x=x;
  loc->y=y;
}


checkbound(xy* in,float boundx,float boundy){

  // x bounds

  if ( (int)in->x > boundx) in->x=0.0f;
  if (in->x < 0) in->x=boundx;

  // y bounds

  if ( (int)in->y > boundy) in->y=0.0f;
  if (in->y < 0) in->y=boundy;
}

// >>>>>>>>>>>>>>>>what are the worms? straightworm,seekworm,squiggleworm,angleworm.wanderworm

void straightworm(struct wormy *worms, float boundx, float boundy){
  xy acc=worms->dir; xy rrrr;
  rrr(10,&rrrr); // TODO: parameter for deviation!
  acc.x+=rrrr.x;
  acc.y+=rrrr.y;
  normalize(&acc);
  acc.x=acc.x*worms->speed;
  acc.y=acc.y*worms->speed;
  xy vel=worms->vel;
  vel.x+=acc.x;
  vel.y+=acc.y;
  limit(&vel,worms->maxspeed);
  worms->wloc.x+=vel.x;
  worms->wloc.y+=vel.y;
  
  checkbound(&worms->wloc,boundx,boundy);
  worms->acc=acc;
  worms->vel=vel;
}

// TODO: fill in these but think first on application to parameter lists...

void seekworm(struct wormy *worms, float boundx, float boundy){

}

void squiggleworm(struct wormy *worms, float boundx, float boundy){
 
}

void angleworm(struct wormy *worms, float boundx, float boundy){ 

}

void wanderworm(struct wormy *worms, float boundx, float boundy){
  float x=worms->wloc.x;
  float y=worms->wloc.y;
  xy acc;
  xy vel=worms->vel;

  // do movements and copy back

  acc.x=worms->acc.x+runiform(-2,2);
  acc.y=worms->acc.y+runiform(-2,2);

  normalize(&acc);

  acc.x=acc.x*worms->speed;
  acc.y=acc.y*worms->speed;
  vel.x+=acc.x;
  vel.y+=acc.y;
  limit(&vel,worms->maxspeed);
  x+=vel.x;
  y+=vel.y;

  worms->wloc.x=x;
  worms->wloc.y=y;
  
  checkbound(&worms->wloc,boundx,boundy);
  worms->acc=acc;
  worms->vel=vel;
}




wormy* addworm(float x, float y, void(*functiony)(struct wormy *worms, float boundx, float boundy)){
  wormy* worm = malloc(sizeof(wormy));
  worm->wloc.x=x;
  worm->wloc.y=y;
  worm->speed=1;
  worm->maxspeed=2;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=2;worm->dir.y=4;

  worm->wormfunction=functiony;
  return worm;
}

void main(void){
  int x;
  //  wormy* wanderingwormy=addworm(99.2f,99.2f,wanderworm);
  wormy* straightwormy=addworm(99.2f,99.2f,straightworm);
  for (x=0;x<1000;x++){
    straightwormy->wormfunction(straightwormy,200,200);
    printf("%f %f\n", straightwormy->wloc.x,straightwormy->wloc.y); 
  }
}
