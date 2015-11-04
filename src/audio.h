/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

//#define ARM_MATH_CM4

#ifdef PCSIM
#define int16_t signed short int
#define u16 unsigned short int
#else
#include "stm32f4xx.h"
#include "arm_math.h"
#endif

#define MAX_VILLAGERS 64 // was 64
#define BUFF_LEN 128 // TEST! reduce to 16 
#define AUDIO_BUFSZ 32768 // was 32768

    typedef struct {
      u8 whicheffect,speed,step,last;
      u16 instart,modstart,outstart;
      int32_t inpos,modpos,outpos;// various counters // was int32t???
      u16 inwrap,modwrap,outwrap;
      u8 mirrormod,mirrordel,mirrorspeed,del; 
      //      u8 infected;
      u8 fingered; // what is input here as modifier
      u8  modifier,kmodifier;
    } villager_effect;

    typedef struct {
      u16 length,last;
      u8 setting,inp,del,speed;
    } villager_hardware;

    typedef struct {
      u16 length, last;
      u16 dataoffset;
      u16 knoboffset;
      int32_t samplepos;
      int16_t dirry;
      u8 speed, step,del;
      u8 dir;
    } villager_hardwarehaha;

    typedef struct {
      u16 length,last;
      u16 dataoffset;
      u16 knoboffset;
      int32_t samplepos;
      int16_t dirry;
      u8 speed, step;
      u8 dir;
    } villager_datagenwalker;

/// TODO: mirrors for above!

typedef struct {
  u16 kstart,kcompress,kwrap;
  u16 mstart,mcompress,mwrap;
  u8 mirrormod,mirrordel,mirrorspeed; // how mirror effects mainline start/wrap and samplepos
  u8 koverlay; float index;
  //  u8 infected;
  u8 fingered; // what is input here as modifier
  u16 start,offset,wrap;
  u16 counterr;
  int32_t samplepos;
  u8 del,dirryr;
  int16_t dirry;
  u16 compress;
  u8 speed, step;
  //  float effect,effectinv; // now as value of effect! TODO!
  u8 dir,overlay; // TODO: do as union or whatever for dir//flag for mirror
  u8 running;
    } villagerr;

typedef struct {
  u16 kstart,kwrap,last;
  //  u16 mstart,mwrap;
  u8 mirrormod,mirrordel,mirrorspeed; // how mirror effects mainline start/wrap and samplepos
  //  u8 infected;
  u8 fingered; // what is input here as modifier
  u16 start,wrap;
  int32_t samplepos;
  u8 del; float index;
  int16_t dirry;
  u8 speed, step;
  u8 dir; // TODO: do as union or whatever for dir//flag for mirror
    } villagerw;


/*typedef struct {
  u16 start,wrap,compress;
  u8 mirrormod,mirrorspeed,mirrorstep; // how mirror effects mainline start/wrap and samplepos
  u8 infectedstart,infectedwrap;
  u8 fingered; // what is input here as modifier
  } mirror;*/

    typedef struct {
      u16 start,last;
      u8 CPU; float index;
      u16 wrap;
      int32_t position;
      u8 del,howmany;
      int16_t dirry;
      u8 speed, step; 
      //      u8 dir; // TODO: do as union or whatever for dir//flag for mirror
      //      u8 running;
      signed char m_stack_pos;
      u8 m_stack[16];
      u16 m_reg16bit1;
      u8 m_reg8bit1,m_reg8bit2;
    } villager_generic;//TODO: and  mirror?

void Audio_Init(void);
void I2S_RX_CallBack(int16_t *src, int16_t *dst, int16_t sz);

u8 fingerdir(u8 *speedmod);



void xxrunleakystack(villager_generic *villager);
void xxrunbiota(villager_generic *villager);
void xxrun1(villager_generic *villager);
void xxrunworm(villager_generic *villager);
void xxrunstack(villager_generic *villager);
void xxrunbefunge(villager_generic *villager);
void xxrunlang(villager_generic *villager);
void xxrunbf(villager_generic *villager);
void xxrunturm(villager_generic *villager);
void xxrunca(villager_generic *villager);
void xxrunant(villager_generic *villager);
void xxrunca2(villager_generic *villager);
void xxrunhodge(villager_generic *villager);
void xxrunworm2(villager_generic *villager);
void xxrunleaky(villager_generic *villager);
void xxrunconvy(villager_generic *villager);
void xxrunplague(villager_generic *villager);
void xxrunmicro(villager_generic *villager);
void xxruncw(villager_generic *villager);
void xxrunmasque(villager_generic *villager);
#endif

