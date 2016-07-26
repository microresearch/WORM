/*#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"
*/

// first lap test and plot

// how to plot: gnuplot -p -e "plot 'testworm' with lines"

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
  worm->maxspeed=2.0;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=2;worm->dir.y=4;
  worm->wormfunction=functiony;
  return worm;
}

/*

worming of params:

say we have worm movements as floats between 0 and 100

we have length of constraint list =x and value MIN and MAX, and pointer to struct of min, max so:

from simpleklatt:

int16_t val[40]= {1000, 0, 497, 0, 739, 0, 2772, 0, 3364, 0, 4170, 0, 4000, 0, 0, 0, 200, 40, 0, 40, 0, 20, 0, 0, 53, 44, 79, 70, 52, 95, 44, 56, 34, 80, 0, 80, 0, 0, 27, 70};
int16_t mins[40]= {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};
int16_t maxs[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

 */

uint16_t constrainedint(float wormm, uint16_t yconmin, uint16_t yconmax){ // less constraining and more scaling
  uint16_t constrained;

  // so 0-100 is scale which needs to be applied to yconmax-yconmin
  float constrainer = (yconmax-yconmin) / 100.0f ;
  float value=(wormm*constrainer)+0.5f;
  constrained=yconmin+(int)(value);
  return constrained;
}

typedef struct{
  u8 length;
  int16_t val[40];
  int16_t mins[40];
  int16_t maxs[40];  
} wormedparamset; // but what of varying lengths

void wormvaluedint(wormedparamset* wormset, wormy* wormyy, float speed, u8 offsetx, u8 offsety){ // for uints only
  // leave speed for now
  wormyy->speed=speed;
  wormyy->wormfunction(wormyy,100.0,100.0);
  //  u8 xconstraint=wormset->length;
  //  u8 xloc=(int)(wormyy->wloc.x+0.5f)%xconstraint; // do we just int or round?
  float xconstraint=wormset->length/ 100.0f;
  u8 xloc=(int)((wormyy->wloc.x * xconstraint)+0.5f); // do we just int or round?

  uint16_t yconstrainmin=wormset->mins[xloc]; 
  uint16_t yconstrainmax=wormset->maxs[xloc]; 

  // now we want to constrain these: 
  wormset->val[xloc]=constrainedint(wormyy->wloc.y, yconstrainmin,yconstrainmax);
  printf("xloc: %d yval: %d wormval %f\n", xloc, wormset->val[xloc], wormyy->wloc.y); 
}

void main(void){
  int x;
  //  wormy* wanderingwormy=addworm(99.2f,99.2f,wanderworm);

  // init simpleklatt parameters

  wormedparamset simpleklatt={40,
    {1000, 0, 497, 0, 739, 0, 2772, 0, 3364, 0, 4170, 0, 4000, 0, 0, 0, 200, 40, 0, 40, 0, 20, 0, 0, 53, 44, 79, 70, 52, 95, 44, 56, 34, 80, 0, 80, 0, 0, 27, 70},
    {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0},
{4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60}
  };

  wormy* this=addworm(10.0f, 10.0f, straightworm); // add start loc
  for (x=0;x<100;x++){
    //        this->wormfunction(this, 100.0, 100.0); // and speed?
	//        printf("%f %f\n", this->wloc.x,this->wloc.y); 
        wormvaluedint(&simpleklatt,this, 4.0, 0, 0);

  }
}
