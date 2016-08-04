#include "audio.h"
#include "worming.h"

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

void straightworm(struct wormy *worms, float addx, float addy, u8 param){
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
  worms->acc=acc;
  worms->vel=vel;
}

// TODO: fill in these but think first on application to parameter lists...

void seekworm(struct wormy *worms, float addx, float addy){

}

void squiggleworm(struct wormy *worms, float addx, float addy){
 
}

void angleworm(struct wormy *worms, float addx, float addy){ 

}

void wanderworm(struct wormy *worms, float addx, float addy, u8 param){
  float x=worms->wloc.x;
  float y=worms->wloc.y;
  xy acc;
  xy vel=worms->vel;

  // do movements and copy back

  acc.x=worms->acc.x+runiform((float)-param,(float)param); // TODO [param here
  acc.y=worms->acc.y+runiform((float)-param,(float)param);

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

wormy* addworm(float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float addx, float addy, u8 param)){
  wormy* worm = malloc(sizeof(wormy));
  worm->boundx=boundx;
  worm->boundy=boundx;
  worm->wloc.x=x;
  worm->wloc.y=y;
  worm->speed=1;
  worm->maxspeed=12.0;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=2;worm->dir.y=4;
  worm->wormfunction=functiony;
  return worm;
}

void addwormsans(wormy* worm, float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float addx, float addy, u8 param)){
  //  wormy* worm = malloc(sizeof(wormy));
  worm->boundx=boundx;
  worm->boundy=boundx;
  worm->wloc.x=x;
  worm->wloc.y=y;
  worm->speed=1;
  worm->maxspeed=2.0;
  worm->acc.x=0;worm->vel.x=0;
  worm->acc.y=0;worm->vel.y=0;
  worm->dir.x=2;worm->dir.y=4;
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

u8 wormvaluedint(wormedparamset* wormset, wormy* wormyy, float speed, u8 offsetx, int16_t offsety, u8 param){ // for uints only
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

void wormunfloat(wormy* wormyy, float speed, u8 param, float *x, float *y){ // for worm as float and no constraints
  //  float x,y;
  wormyy->speed=speed;
  wormyy->wormfunction(wormyy,0.0,0.0, param);
  *x=(wormyy->wloc.x-100.0f)/100.0f;
  *y=(wormyy->wloc.y-100.0f)/100.0f;
//  printf("%f %f\n", x, y); 
}

float wormonefloat(wormy* wormyy, float speed, u8 param, float limit){ // for worm as float and no constraints
  wormyy->speed=speed;
  wormyy->boundy=wormyy->boundx=limit;
  wormyy->wormfunction(wormyy,0.0,0.0, param);
  //  *x=(wormyy->wloc.x-100.0f)/100.0f;
  float y=wormyy->wloc.x;
  return y;
//  printf("%f %f\n", x, y); 
}


/*void main(void){
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
*/
