typedef struct{
  u8 length;
  int16_t val[40];
  int16_t mins[40];
  int16_t maxs[40];  
} wormedparamset; // but what of varying lengths

typedef struct xy {
  float x,y;
} xy;

typedef struct wormy {
  xy wloc;
  xy acc;
  xy vel;
  xy dir;
  float boundx,boundy;
  float speed; // is it float?
  float maxspeed;
  float parameter;
  void (*wormfunction)(struct wormy *worms, float addx, float addy, u8 param); 
  // speed, maxspeed, tail[], counter, directionxY, tailcount, SW, targetXY, accXY, velXY, how we do angles/up/down?
}wormy;


// list the worms
void wanderworm(struct wormy *worms, float addx, float addy,u8 param);
void straightworm(struct wormy *worms, float addx, float addy, u8 param);
void wormunfloat(wormy* wormyy, float speed, u8 param, float *x, float *y);
u8 wormvaluedint(wormedparamset* wormset, wormy* wormyy, float speed, u8 offsetx, int16_t offsety, u8 param); //for uints
wormy* addworm(float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float boundx, float boundy, u8 param)); // but we need worm functions here

float wormonefloat(wormy* wormyy, float speed, u8 param, float limit);

void addwormsans(wormy* wormy, float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float boundx, float boundy, u8 param)); // but we need worm functions here
