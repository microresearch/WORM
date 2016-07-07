#ifndef RENDER_H
#define RENDER_H

void Render();
void SetMouthThroat(unsigned char mouth, unsigned char throat);

typedef signed short int16_t;

unsigned char rendersamsample(int16_t* sample, u8* ending);      
int16_t renderframe(void);
void sam_frame_rerun(void);


#endif
