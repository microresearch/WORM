#ifndef RENDER_H
#define RENDER_H

void Render();
void SetMouthThroat(unsigned char mouth, unsigned char throat);

typedef signed short int16_t;

unsigned char rendersamsample(int16_t* sample);      
void renderframe(void);

#endif
