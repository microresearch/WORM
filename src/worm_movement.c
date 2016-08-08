/*#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"
*/

// first lap test and plot

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


// how to plot: gnuplot -p -e "plot 'testworm' with lines"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "worming.h"

#define PI            3.1415927

///

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

float rr(float ranger){
 float x= (runiform(ranger/-1.0f, ranger/1.0f)); 
return x;
}


void rrr(float ranger, xy* rrrr){
 rrrr->x= (runiform(ranger/-2.0f, ranger/2.0f)); 
 rrrr->y= (runiform(ranger/-2.0f, ranger/2.0f)); 
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

void checkbound(xy* in,float boundx,float boundy, float minx, float miny){

  // x bounds

  if (in->x > boundx) in->x=minx; // was int comparison before
  if (in->x < minx) in->x=boundx;

  // y bounds

  if (in->y > boundy) in->y=miny;
  if (in->y < miny) in->y=boundy;
}

// >>>>>>>>>>>>>>>>what are the worms? straightworm,seekworm,squiggleworm,angleworm.wanderworm

void squiggleworm(struct wormy *worms, float addx, float addy, float param){
    worms->counter++;
  float rot=sinf((float)worms->counter)*param; // this can be multiplied
  float z0 = (worms->acc.x * cosf(rot) - worms->acc.y * sinf(rot));
  float z1 = (worms->acc.x * sinf(rot) + worms->acc.y * cosf(rot));
  xy acc=worms->dir; 
  acc.x+=z0;
  acc.y+=z1;

  xy vel=worms->vel;
  vel.x+=acc.x;
  vel.y+=acc.y;
  limit(&vel,worms->maxspeed);
  worms->wloc.x+=vel.x;
  worms->wloc.y+=vel.y;

  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  worms->acc=acc;
  worms->vel=vel;
}


void spiralworm(struct wormy *worms, float addx, float addy, float param){
  worms->counter+=param;
  //  float rot=sinf((float)worms->counter)*param; // this can be multiplied
  float rot=(float)worms->counter*param; // this can be multiplied
  float z0 = (worms->acc.x * cosf(rot) - worms->acc.y * sinf(rot));
  float z1 = (worms->acc.x * sinf(rot) + worms->acc.y * cosf(rot));
  xy acc=worms->dir; 
  acc.x+=z0;
  acc.y+=z1;

  xy vel=worms->vel;
  vel.x+=acc.x;
  vel.y+=acc.y;
  limit(&vel,worms->maxspeed);
  worms->wloc.x+=vel.x;
  worms->wloc.y+=vel.y;

  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  worms->acc=acc;
  worms->vel=vel;
}

void directworm(struct wormy *worms, float addx, float addy, float param){ // test directionworm
// change angle randomly within param as max deviation
static float angle=1.0;

xy acc;
angle+=rr(param);
acc.x= cosf(angle*(PI/180.0));
acc.y= sinf(angle*(PI/180.0));


  acc.x=acc.x*worms->speed;
  acc.y=acc.y*worms->speed;
  xy vel=worms->vel;
  vel.x+=acc.x;
  vel.y+=acc.y;
  limit(&vel,worms->maxspeed);
  worms->wloc.x+=vel.x;
  worms->wloc.y+=vel.y;

// when hit boundary slowly change angle back or just reverse with larger deviation
if (worms->wloc.x>worms->boundx || worms->wloc.x<addx ) angle=180.0-angle;//-180.0;
if (worms->wloc.y>worms->boundy || worms->wloc.y<addy ) angle=360.0-angle;//-180.0;a
  
//  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  //  worms->acc=acc;
//  worms->vel=vel;

}

void sineworm(struct wormy *worms, float addx, float addy, float param){ // test directionworm

// param as angle in degrees works so...
  xy acc; static float nn=1.0;
float val;
 
 worms->counter+=param*nn;
// if (worms->counter>90.0) nn=-1;
// else if (worms->counter<0.0) nn=1;
 val=worms->counter;
acc.x= param;//*(PI/180.0);//sinf(param* (PI/180.0));
acc.y= sinf(val)*100.0;

  //  acc.x+=worms->dir.x;
  //  acc.y+=worms->dir.y;
  //  normalize(&acc);


  acc.x=acc.x*worms->speed;
  acc.y=acc.y*worms->speed;

  worms->wloc.x+=acc.x;
  worms->wloc.y+=acc.y;

  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  //  worms->acc=acc;
//  worms->vel=vel;
}



void straightworm(struct wormy *worms, float addx, float addy, float param){
  xy acc=worms->dir; xy rrrr;
  rrr(param,&rrrr); // TODO: parameter for deviation! -> was 10
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
  
  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  //  worms->acc=acc;
  worms->vel=vel;
}

// TODO: fill in these but think first on application to parameter lists...

void waveworm(struct wormy *worms, float addx, float addy, float param){ // straight through for wavetable - x and y
  worms->wloc.x+=worms->speed;
  worms->wloc.y+=worms->speed;
  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
 }


void seekworm(struct wormy *worms, float addx, float addy, float param){ // what is it seeking? and in what? so leave for now???
  // only possible would be some kind of buffer mapped to the area...

}

void wanderworm(struct wormy *worms, float addx, float addy, float param){
  float x=worms->wloc.x;
  float y=worms->wloc.y;
  xy acc;
  xy vel=worms->vel;

  // do movements and copy back

  acc.x=worms->acc.x+runiform(-param,param); // TODO [param here
  acc.y=worms->acc.y+runiform(-param,param);

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
  
  checkbound(&worms->wloc,worms->boundx,worms->boundy,addx,addy);
  worms->acc=acc;
  worms->vel=vel;
}

wormy* addworm(float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float addx, float addy, float param)){
  wormy* worm = malloc(sizeof(wormy));
  worm->boundx=boundx;
  worm->boundy=boundx;
  worm->wloc.x=x;
  worm->wloc.y=y;
  worm->speed=0.1;
  worm->maxspeed=2.0;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=1.0;worm->dir.y=1.0; // what is dir?
  worm->wormfunction=functiony;
  return worm;
}

void addwormsans(wormy* worm, float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float addx, float addy, float param)){
  //  wormy* worm = malloc(sizeof(wormy));
  worm->counter=0;
  worm->boundx=boundx;
  worm->boundy=boundx;
  worm->wloc.x=x;
  worm->wloc.y=y;
  worm->speed=0.1;
  worm->maxspeed=2.0;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=1.0;worm->dir.y=1.0;
  worm->wormfunction=functiony;
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

u8 wormvaluedint(wormedparamset* wormset, wormy* wormyy, float speed, u8 offsetx, int16_t offsety, float param){ // for uints only
  // leave speed for now
  wormyy->speed=speed;
  wormyy->wormfunction(wormyy,offsetx,offsety, param);

  // how to add add offsetx and offsety - push them over in wormfunction?

  float xconstraint=wormset->length/ 100.0f;
  u8 xloc=(int)((wormyy->wloc.x * xconstraint)+0.5f); // do we just int or round?
  uint16_t yconstrainmin=wormset->mins[xloc]; 
  uint16_t yconstrainmax=wormset->maxs[xloc]; 

  wormset->val[xloc]=constrainedint(wormyy->wloc.y, yconstrainmin,yconstrainmax);

  //  printf("xloc: %d yval: %d wormval %f\n", xloc, wormset->val[xloc], wormyy->wloc.y); 
  return xloc;
}

void wormunfloat(wormy* wormyy, float speed, float param, float *x, float *y){ // for worm as float and no constraints
  //  float x,y;
  wormyy->speed=speed;
  wormyy->wormfunction(wormyy,0.0,0.0, param);
  *x=(wormyy->wloc.x-100.0f)/100.0f;
  *y=(wormyy->wloc.y-100.0f)/100.0f;
//  printf("%f %f\n", x, y); 
}

float wormonefloat(wormy* wormyy, float speed, float param, float limit){ // for worm as float and no constraints
  wormyy->speed=speed;
  wormyy->boundy=wormyy->boundx=limit;
  wormyy->wormfunction(wormyy,0.0,0.0, param);
  //  *x=(wormyy->wloc.x-100.0f)/100.0f;
  float y=wormyy->wloc.x;
  return y;
//  printf("%f %f\n", x, y); 
}


void main(void){
  int xx;
  // wormy* wanderingwormy=addworm(99.2f,99.2f,wanderworm);
wormy* this =  addworm(10.0f,10.0f,200.0f, 200.0f, directworm);

  // init simpleklatt parameters

  float x,y;

  /*  wormedparamset simpleklatt={40,
    {1000, 0, 497, 0, 739, 0, 2772, 0, 3364, 0, 4170, 0, 4000, 0, 0, 0, 200, 40, 0, 40, 0, 20, 0, 0, 53, 44, 79, 70, 52, 95, 44, 56, 34, 80, 0, 80, 0, 0, 27, 70},
    {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0},
{4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60}
};*/

  //  wormy* this=addworm(100.1f, 100.1f, wanderworm); // add start loc
  for (xx=0;xx<100000;xx++){
    this->wormfunction(this, 0.0,0.0,8.0); // and speed?
    printf("%f %f\n", this->wloc.x,this->wloc.y); 
    //        wormvaluedint(&simpleklatt,this, 4.0, 0, 0);
    //        wormunfloat(wanderingwormy, 12.0, 10,&x,&y);
    //    printf("%f %f\n", x, y); 
  }
}
