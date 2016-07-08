#include "stdio.h"
#include "math.h"
#include "stdlib.h"
#include "forlap.h"

typedef struct {
  float samplepos;
  u8 phonem, trigger;
  u16 storeone,storetwo,storethree;
  int16_t lastsample;
  int16_t prevsample;
}genny;

genny* allgen;

int16_t sp0256_get_sample(){
  static int16_t xx=0;
  xx+=1;
  return xx;

}

u16 sp0256(genny* genstruct, int16_t* incoming,  int16_t* outgoing, float samplespeed, u8 size){

  // MODEL GENERATOR: TODO is speed and interpolation options DONE
  static u8 triggered=0;
  u8 xx=0,readpos;
  float remainder;
float samplepos=genstruct->samplepos;
int16_t samplel=genstruct->lastsample;
int16_t lastval=genstruct->prevsample;

  // we need to take account of speed ... also this fractional way here/WITH/interpolation? TODO
  // as is set to 8k samples/sec and we have 32k samplerate

   if (samplespeed<=1){ // slower=UPSAMPLE where we need to interpolate... then low pass afterwards - for what frequency?
     while (xx<size){
       if (samplepos>=1.0f) {
	 lastval=samplel;
	 samplel=sp0256_get_sample();
	 samplepos-=1.0f;
       }
       remainder=samplepos; 
       outgoing[xx]=(lastval*(1-remainder))+(samplel*remainder); // interpol with remainder - to test - 1 sample behind
       printf("outgoing %d xx: %d\n",outgoing[xx],xx);
       //       outgoing[xx]=samplel;
       xx++;
       samplepos+=samplespeed;
     }
   }
   else { // faster=UPSAMPLE? = low pass first for 32000/divisor???
     while (xx<size){
       samplel=sp0256_get_sample();

       // say speed is 2.0 we want every second sample....
       if (samplepos>=samplespeed){
	 outgoing[xx]=samplel;
       printf("outgoing %d xx: %d\n",outgoing[xx],xx);
       samplepos-=samplespeed;
     
       // TEST trigger: 
	 xx++;
       }
    samplepos+=1.0f;
     }
   }

  // refill back counter etc.
 genstruct->samplepos=samplepos;
 genstruct->lastsample=samplel;
 genstruct->prevsample=lastval;
 return size;
};

float speedOfSound(float temperature)
{
    return (331.4 + (0.6 * temperature));
}

void main(){
  float x=0.0f;
  int y=0;
  int xx;
  int m_pc=128;
  int d0;
  unsigned char X=16;
  float samplespeed=128.0f;
  allgen=malloc(sizeof(genny));		
  allgen->samplepos=0.0f;
  int16_t mono_buffer[32];
  int16_t sample_buffer[32];

  y=128;

  //  xx= (0x4000 << 3);
  //  printf("XXXX %d",xx);


  float c = speedOfSound(32);
  int controlPeriod =    rint((c * 10 * 100.0) /(18.0 * 1.0));
  int sampleRate = 4.0 * controlPeriod;

  printf("controlperiod %d\n",controlPeriod);



  /*  while(1){

    //  x=generators[mode](allgen,sample_buffer,mono_buffer,samplespeed,sz/2); 
    xx=sp0256(allgen,sample_buffer,mono_buffer,samplespeed,32);
    }*/

  //  int idx0 = (m_pc    ) >> 3, d0; //???

  //  printf("IDX %d\n", idx0);

  //#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))


  //printf("ELEMENTS: %d\n", NELEMS(crow_coeffs));

  //  printf("SIZE %d",sizeof(sam_vocab));

  // signed short yyy=((X&15)<<12)-32768;

    //int yyy=((X-8)<<12); //1 byte

  // printf("%d\n",yyy);

  /*  for (xx=0;xx<100;xx++){
    x+=11143521654621;
    y=(int)x;
    //    printf("%d %f\n",y,x);

    printf("%d\n",a[y%10]);

    //  if (x==y) printf("OKAY\n");
    }*/
}
