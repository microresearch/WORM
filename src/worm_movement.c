#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "stm32f4xx.h"
#include "audio.h"

typedef struct wormy {
  u8 posx,posy;
  void (*wormfunction)(struct wormy *worms, u8 boundx, u8 boundy); 
  // speed, maxspeed, tail[], counter, directionxY, tailcount, SW, targetXY, accXY, velXY

}wormy;

// >>>>>>>>>>>>>>>>what are the worms?

/*            'basicworm': self.wander,
            'straightworm':self.straight,
            'seeker':self.seek,
            'squiggler':self.squiggler
*/

// >>>>>>>>>>>>>>>helper functions?

/*
def randy(num):
    return random.randrange(0, num, 1)

def randyx(xnum,num):
    x= None
    while x==xnum or x is None:
        x= randy(num)
    return x

def rrr(ranger):
    r= (random.uniform(ranger/-2, ranger/2),random.uniform(ranger/-2, ranger/2)) 
    return r;

def normalize(tup):
    x=tup[0]
    y=tup[1]
    mag=math.sqrt(x*x + y*y)
    if mag!=0:
        x=x/mag
        y=y/mag
    return (x,y)

def limit(tup,limit):
    x=tup[0]
    y=tup[1]
#   (magSq() > max*max) { normalize(); mult(max); 
    if math.sqrt(x*x + y*y) > limit*limit:
        (x,y)=normalize(tup)
        x=x*limit
        y=y*limit
    return (x,y)
*/
  
void followworm(struct wormy *worms, u8 boundx, u8 boundy){
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
