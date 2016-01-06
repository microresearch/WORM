// generating parameters for docs/klatt

#include <stdio.h>
#include <stdlib.h>

/*
      &frame->F0hz10, &frame->AVdb, 0,1
      &frame->F1hz,   &frame->B1hz, 2,3
      &frame->F2hz,   &frame->B2hz, 4
      &frame->F3hz,   &frame->B3hz, 6
      &frame->F4hz,   &frame->B4hz, 8
      &frame->F5hz,   &frame->B5hz, 10
      &frame->F6hz,   &frame->B6hz, 12
      &frame->FNZhz,  &frame->BNZhz,14
      &frame->FNPhz,  &frame->BNPhz,16
      &frame->ASP,    &frame->Kopen,18,19
      &frame->Aturb,  &frame->TLTdb,20,21
      &frame->AF,     &frame->Kskew,22,23
      &frame->A1,     &frame->B1phz,
      &frame->A2,     &frame->B2phz,
      &frame->A3,     &frame->B3phz,
      &frame->A4,     &frame->B4phz,
      &frame->A5,     &frame->B5phz,
      &frame->A6,     &frame->B6phz,
      &frame->ANP,    &frame->AB,
      &frame->AVpdb,  &frame->Gain0);
*/

typedef struct
{
  int val;
  int min;
  int max;
} framer;

// these are constraints see klatt_params


int val[40]= {1000, 0, 497, 0, 739, 0, 2772, 0, 3364, 0, 4170, 0, 4000, 0, 0, 0, 200, 40, 0, 40, 0, 20, 0, 0, 53, 44, 79, 70, 52, 95, 44, 56, 34, 80, 0, 80, 0, 0, 27, 70};
int mins[40]= {200,  0, 200, 40, 550, 40, 1200, 40, 1200, 40, 1200, 40, 1200, 40, 248, 40, 248, 40, 0, 10, 0, 0, 0, 0, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 0, 0, 0};
int maxs[40]= {4000, 70, 1300, 1000, 3000, 1000, 4999, 1000, 4999, 1000, 4999, 1000, 4999, 2000, 528, 1000, 528, 1000, 70, 65, 80, 24, 80, 40, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 2000, 80, 80, 70, 60};

int dir[40]= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void main(void){
  int x,y;

  framer frame[40];

    for (y=0;y<40;y++){
      frame[y].val=val[y];
	dir[y]=rand()%3;
    }

  for (x=0;x<2000;x++){
    for (y=0;y<40;y++){

      // direction change 0,1-back,2-forwards
      switch(dir[y]){
      case 0:
	// no change
	break;
      case 1:
	// forwards
	val[y]+=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]>maxs[y]) dir[y]=2;
	break;
      case 2:
	// backwards
	val[y]-=rand()%((maxs[y]-mins[y])/10); // later do as table
	if (val[y]<mins[y]) dir[y]=1;
	break;
      }
	printf("%d ",val[y]);
  }
    printf("\n");
  }
}
