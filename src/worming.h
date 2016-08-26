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
  float counter;
  float boundx,boundy;
  float speed; // is it float?
  float maxspeed;
  float parameter;
  void (*wormfunction)(struct wormy *worms, float addx, float addy, float param); 
  // speed, maxspeed, tail[], counter, directionxY, tailcount, SW, targetXY, accXY, velXY, how we do angles/up/down?
}wormy;


// list the worms
void wanderworm(struct wormy *worms, float addx, float addy,float param);
void spiralworm(struct wormy *worms, float addx, float addy,float param);
void directworm(struct wormy *worms, float addx, float addy,float param);
void sineworm(struct wormy *worms, float addx, float addy,float param);
void straightworm(struct wormy *worms, float addx, float addy, float param);

void wormunfloat(wormy* wormyy, float speed, float param, float *x, float *y);
u8 wormvaluedint(wormedparamset* wormset, wormy* wormyy, float speed, u8 offsetx, int16_t offsety, float param); //for uints
wormy* addworm(float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float boundx, float boundy, float param)); // but we need worm functions here

float wormonefloat(wormy* wormyy, float speed, float param, float limit);

void addwormsans(wormy* wormy, float x, float y, float boundx, float boundy, void(*functiony)(struct wormy *worms, float boundx, float boundy, float param)); // but we need worm functions here
