typedef struct {
  u16 generated;
  u16 writepos;
} pair;


pair klatt_phoneme(u16 writepos, u8 phoneme);
pair demandVOSIM_SC(u16 writepos,float freq,float cycles,float decay);
u16 runVOSIM_SC(u16 count);

