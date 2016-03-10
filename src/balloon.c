/*
 *  balloon1.cpp
 *  balloon1
 *  Implements Ishizaka & Flanagan two mass model of the vocal folds
 *	Uses backward finite difference approximation as well as many others
 *	Output is the flow (u)
 *  Created by Avrum Hollinger on 11/12/06.
 *  Copyright 2006. All rights reserved.
 *
 */
#include <stdio.h>
#include "math.h"

double computeSample(double pressure_in);


double ps;		//subglottal pressure
double r1;		//damping factor
double r2;
double m1;		//mass
double m2;
double k1;		//spring constant
double k2;
double k12;		//coupling spring constant
double d1;		//glottal width
double d2;
double lg;		//glottal length
double aida;		//nonlinearity coefficient
double S;			//subglottal surface area
double Ag01;		//nominal glottal area, with mass at rest position
double Ag02;
double pm1Prev;	//pressure at previous time step
double pm2Prev;
double x1Prev;	//displacement at previous time step
double x1PrevPrev;//displacement at previous time step to the previous one
double x2Prev;
double x2PrevPrev;
double gain;		//after-market gain
double uPrev;		//previous flow value
double Fs;		//calculation sampling rate, not actual audio output sample rate


void init(){
//	r1 =2.0976e-8;
	r1 =2.0976e-8;
//	r2 =2.0976e-8;
	r2 =2.0976e-8;
//	m1 =4.8889e-7;
	m1 =5.8889e-6;
//	m2 =4.8889e-7;
	m2 =5.8889e-6;
	k1 =0.5;
//	k2 =0.09;
	k2 =0.5;
//	k12=0.04;
	k12=0.04;
//	aida =10000000.0;
	aida =0.000001;
//	d1 =1.5e-5;
	d1 =1.5e-5;
	d2 =1.5e-5;
	lg =0.0163;
	gain=10.0;
	S=5e-5;
	Ag01=5e-9;
	Ag02=5e-9;
	x1Prev=0.0;
	x1PrevPrev=0.0;
	x2Prev=0.0;
	x2PrevPrev=0.0;
	pm1Prev=0.0;
	pm2Prev=0.0;
	uPrev=0.0;
	ps=0.0;
	Fs=32000.0;
}

FILE *fo;

int rtick(double *buffer, int bufferSize, double pressureIn) {
	double *samples = (double *) buffer;

/*	bsynth->setM1(5e-8*(double)[(id)dataPointer m1In]);
	bsynth->setM2(5e-8*(double)[(id)dataPointer m2In]);
	bsynth->setR1(5e-9*(double)[(id)dataPointer r1In]);
	bsynth->setR2(5e-9*(double)[(id)dataPointer r2In]);
	bsynth->setK1(1e-3*(double)[(id)dataPointer k1In]);
	bsynth->setK2(1e-3*(double)[(id)dataPointer k2In]);
	bsynth->setD1(1.5e-7*(double)[(id)dataPointer d1In]);
	bsynth->setD2(1.5e-7*(double)[(id)dataPointer d2In]);
	bsynth->setK12(1e-4*(double)[(id)dataPointer k12In]);
	bsynth->setLg(1.3e-4*(double)[(id)dataPointer lgIn]);
	bsynth->setAida(1.1e-4*(double)[(id)dataPointer aidaIn]);
	bsynth->setS(5.5e-7*(double)[(id)dataPointer SIn]);
	bsynth->setAg01(5.1e-10*(double)[(id)dataPointer Ag01In]);
	bsynth->setAg02(5.1e-10*(double)[(id)dataPointer Ag02In]);
	bsynth->setGain((double)[(id)dataPointer gainIn]);
	bsynth->setFs((double)[(id)dataPointer FsIn]);
*/
	int i;
	for (i=0; i<bufferSize; i++ ) {
	  //	  *samples++ = computeSample(pressureIn);
	  //	  printf("%f  ",computeSample(pressureIn));

   signed int s16=(signed int)(computeSample(pressureIn)*32768.0);
   //   printf("%d\n",s16);
   fwrite(&s16,2,1,fo);

		//*samples++ = [(id)dataPointer amp] * bsynth->tick();
	}

	return 0;
};

void main(void){
  fo = fopen("testball.pcm", "wb");

  init();
  double buffer[48000];
  rtick(buffer, 48000, 300.0);

}

double tick(){
  double pressure_in=ps;
  double T=1/Fs;
  double rho = 1.14; 
  double rhosn = rho*0.69;
  double hfrho=rho/2;
  double v = 1.85e-5;
  double twvd1lg=12*v*d1*lg*lg;
  double twvd2lg=12*v*d2*lg*lg;
  double Ag012lg=Ag01/2/lg;
  double Ag022lg=Ag02/2/lg;
  double lgd1=lg*d1;
  double lgd2=lg*d2;
  double m1T=m1/T/T;
  double m2T=m2/T/T;
  double r1T=r1/T;
  double r2T=r2/T;
  double C11=k1*(1+aida*x1Prev*x1Prev);
  double C12=k2*(1+aida*x2Prev*x2Prev);
  double C21=k1*(1+aida*(x1Prev+Ag012lg)*(x1Prev+Ag012lg));
  double C22=k2*(1+aida*(x2Prev+Ag022lg)*(x2Prev+Ag022lg));
  double alpha1=lgd1*pm1Prev;
  double alpha2=lgd2*pm2Prev;
  double beta1=m1T*(x1PrevPrev-2*x1Prev);
  double beta2=m2T*(x2PrevPrev-2*x2Prev);
  double gamma1=-r1T*x1Prev;
  double gamma2=-r2T*x2Prev;
  double delta1=Ag012lg*C21;
  double delta2=Ag022lg*C22;
  double lambda1=-k12*x2Prev;
  double lambda2=-k12*x1Prev;
  double x1=0.0;
  double x2=0.0;
  double pm1=0.0;
  double pm2=0.0;
  double A1=0.0;
  double A2=0.0;
  double A1n2=0.0;
  double A1n3=0.0;
  double A2n2=0.0;
  double A2n3=0.0;
  double a=0.0;
  double b=0.0;
  double c=0.0;
  double det=0.0;
  double flow1=0.0;
  double flow2=0.0;
  double udif1=0.0;
  double udif2=0.0;
  double u=0.0;
  double g1=0.0;
  double g2=0.0;
  double g4=0.0;
  double g5=0.0;
  double pm1b=0.0;
  double pm2b=0.0;

  if  (x1Prev>=-Ag012lg){
    x1=(alpha1-beta1-gamma1-lambda1)/(m1T+r1T+C11+k12);
  }
  else {
    x1=(alpha1-beta1-gamma1-lambda1-delta1)/(m1T+r1T+C21+k12);
  }

  if  (x2Prev>=-Ag022lg){
    x2=(alpha2-beta2-gamma2-lambda2)/(m2T+r2T+C12+k12);
  }
  else{
    x2=(alpha2-beta2-gamma2-lambda2-delta2)/(m2T+r2T+C22+k12);
  }

  A1=Ag01+lg*x1;
  A2=Ag02+lg*x2;
    
  if (A1<=0)
    {A1=0.1e-25;}
    

  if (A2<=0)
    {A2=0.1e-25;}
   
	
	
  A1n2=A1*A1; 
  A1n3=A1n2*A1;

  A2n2=A2*A2; 
  A2n3=A2n2*A2;
	
  a= (rhosn/A1n2)+hfrho*(1/A2n2-1/A1n2)+hfrho/A2n2*(2*A2/S*(1-A2/S));
  b= twvd1lg/A1n3+twvd2lg/A2n3;
  c= -pressure_in;

  det=b*b-4.0*a*c;

  if (det>=0){
    flow1=(-b+sqrt(det))/(2*a);
    flow2=(-b-sqrt(det))/(2*a);
  }
  else{
    flow1=(-b)/(2*a);
    flow2=(-b)/(2*a);
  }

  udif1=fabs(flow1-uPrev);
  udif2=fabs(flow2-uPrev);

  if (udif1<udif2){
    u=flow1;
  }
  else{
    u=flow2;
  }

  //u=max(flow1,flow2);



  g1=rhosn*u*u/A1n2;
  g2=twvd1lg*u/A1n3;
  g4=twvd2lg*u/A2n3;
  g5=hfrho*u*u/A2n2*(2*A2/S*(1-A2/S));

  pm1=pressure_in-g1-g2/2;
  pm2=g5+g4/2;

  if (x1>=-Ag012lg){
    
    pm1b=(m1T*(x1-2*x1Prev+x1PrevPrev)+r1T*(x1-x1Prev)+ k1*x1*(1+aida*x1*x1) +k12*(x1-x2))/(lgd1);
    pm1=pm1/2+pm1b/2;
        
	
    if (x2>=-Ag022lg){

      pm2b=(m2T*(x2-2*x2Prev+x2PrevPrev)+r2T*(x2-x2Prev)+ k2*x2*(1+aida*x2*x2) -k12*(x1-x2))/(lgd2);
      pm2=pm2/2+pm2b/2;
    }
    else{
      //     pm2=pressure_in;
      //   pm1=pressure_in;
    }
  }     
  else{
    //   pm1=pressure_in;
    //     pm2=0.0;
  }

  //   if (pm1>pressure_in)
  //		{pm1=pressure_in;}
    
  //	if (pm2>pressure_in)
  //       {pm2=pressure_in;}
    
  if (pm1<0.0)
    //   pm1(n)=abs(pm1(n));
    {pm1=0.0;}
    

  if (pm2<0.0)
    {pm2=0.0;}
    
  //  if (u<0.0)
  //      {u=0.0;}
		
  pm1Prev=pm1;
  pm2Prev=pm2;
  x1PrevPrev=x1Prev;
  x1Prev=x1;
  x2PrevPrev=x2Prev;
  x2Prev=x2;
  uPrev=u;
  ps=ps+1;

  return gain*u;
}

double computeSample(double pressure_in){
  double T=1/Fs;
  double rho = 1.14; 
  double rhosn = rho*0.69;
  double hfrho=rho/2;
  double v = 1.85e-5;
  double twvd1lg=12*v*d1*lg*lg;
  double twvd2lg=12*v*d2*lg*lg;
  double Ag012lg=Ag01/2/lg;
  double Ag022lg=Ag02/2/lg;
  double lgd1=lg*d1;
  double lgd2=lg*d2;
  double m1T=m1/T/T;
  double m2T=m2/T/T;
  double r1T=r1/T;
  double r2T=r2/T;
  double C11=k1*(1+aida*x1Prev*x1Prev);
  double C12=k2*(1+aida*x2Prev*x2Prev);
  double C21=k1*(1+aida*(x1Prev+Ag012lg)*(x1Prev+Ag012lg));
  double C22=k2*(1+aida*(x2Prev+Ag022lg)*(x2Prev+Ag022lg));
  double alpha1=lgd1*pm1Prev;
  double alpha2=lgd2*pm2Prev;
  double beta1=m1T*(x1PrevPrev-2*x1Prev);
  double beta2=m2T*(x2PrevPrev-2*x2Prev);
  double gamma1=-r1T*x1Prev;
  double gamma2=-r2T*x2Prev;
  double delta1=Ag012lg*C21;
  double delta2=Ag022lg*C22;
  double lambda1=-k12*x2Prev;
  double lambda2=-k12*x1Prev;
  double x1=0.0;
  double x2=0.0;
  double pm1=0.0;
  double pm2=0.0;
  double A1=0.0;
  double A2=0.0;
  double A1n2=0.0;
  double A1n3=0.0;
  double A2n2=0.0;
  double A2n3=0.0;
  double a=0.0;
  double b=0.0;
  double c=0.0;
  double det=0.0;
  double flow1=0.0;
  double flow2=0.0;
  double udif1=0.0;
  double udif2=0.0;
  double u=0.0;
  double g1=0.0;
  double g2=0.0;
  double g4=0.0;
  double g5=0.0;
  double pm1b=0.0;
  double pm2b=0.0;


  if  (x1Prev>=-Ag012lg){
    x1=(alpha1-beta1-gamma1-lambda1)/(m1T+r1T+C11+k12);
  }
  else {
    x1=(alpha1-beta1-gamma1-lambda1-delta1)/(m1T+r1T+C21+k12);
  }

  if  (x2Prev>=-Ag022lg){
    x2=(alpha2-beta2-gamma2-lambda2)/(m2T+r2T+C12+k12);
  }
  else{
    x2=(alpha2-beta2-gamma2-lambda2-delta2)/(m2T+r2T+C22+k12);
  }

  A1=Ag01+lg*x1;
  A2=Ag02+lg*x2;
    
  if (A1<=0)
    {A1=0.1e-25;}
    

  if (A2<=0)
    {A2=0.1e-25;}
   
	
	
  A1n2=A1*A1; 
  A1n3=A1n2*A1;

  A2n2=A2*A2; 
  A2n3=A2n2*A2;
	
  a= (rhosn/A1n2)+hfrho*(1/A2n2-1/A1n2)+hfrho/A2n2*(2*A2/S*(1-A2/S));
  b= twvd1lg/A1n3+twvd2lg/A2n3;
  c= -pressure_in;

  det=b*b-4.0*a*c;

  if (det>=0){
    flow1=(-b+sqrt(det))/(2*a);
    flow2=(-b-sqrt(det))/(2*a);
  }
  else{
    flow1=(-b)/(2*a);
    flow2=(-b)/(2*a);
  }

  udif1=fabs(flow1-uPrev);
  udif2=fabs(flow2-uPrev);

  if (udif1<udif2){
    u=flow1;
  }
  else{
    u=flow2;
  }

  //u=max(flow1,flow2);



  g1=rhosn*u*u/A1n2;
  g2=twvd1lg*u/A1n3;
  g4=twvd2lg*u/A2n3;
  g5=hfrho*u*u/A2n2*(2*A2/S*(1-A2/S));

  pm1=pressure_in-g1-g2/2;
  pm2=g5+g4/2;

  if (x1>=-Ag012lg){
    
    pm1b=(m1T*(x1-2*x1Prev+x1PrevPrev)+r1T*(x1-x1Prev)+ k1*x1*(1+aida*x1*x1) +k12*(x1-x2))/(lgd1);
    pm1=pm1/2+pm1b/2;
        
	
    if (x2>=-Ag022lg){

      pm2b=(m2T*(x2-2*x2Prev+x2PrevPrev)+r2T*(x2-x2Prev)+ k2*x2*(1+aida*x2*x2) -k12*(x1-x2))/(lgd2);
      pm2=pm2/2+pm2b/2;
    }
    else{
      //           pm2=pressure_in;
      //         pm1=pressure_in;
    }
  }     
  else{
    //    pm1=pressure_in;
    //   pm2=0.0;
  }

  /*  if (pm1>pressure_in)
    //	{pm1=pressure_in;}
    
    if (pm2>pressure_in)
      //     {pm2=pressure_in;}
    
      if (pm1<0.0)
	//   pm1(n)=abs(pm1(n));
        {pm1=0.0;}
  */

  if (pm2<0.0)
    {pm2=0.0;}
    
  //  if (u<0.0)
  //    {u=0.0;}
		
    pm1Prev=pm1;
  pm2Prev=pm2;
  x1PrevPrev=x1Prev;
  x1Prev=x1;
  x2PrevPrev=x2Prev;
  x2Prev=x2;
  uPrev=u;
  ps=pressure_in;

  return gain*u;
}

void clearOld()
{
x1Prev=0;
x1PrevPrev=0;
x2Prev=0;
x2PrevPrev=0;
pm1Prev=0;
pm2Prev=0;
uPrev=0;
ps=0;
}

void setR1 (double r1_in){
  r1=r1_in;
}
	
void  setR2 (double r2_in){
  r2=r2_in;
}
void  setK1 (double k1_in){
  k1=k1_in;
}
		
void  setK2 (double k2_in){
  k2=k2_in;
}
	
void setK12 (double k12_in){
  k12 = k12_in;
}
	
void  setM1 (double m1_in){
  m1=m1_in;
}
	
void  setM2 (double m2_in){
  m2=m2_in;
}
	
void  setD1 (double d1_in){
  d1=d1_in;
}
void  setD2 (double d2_in){
  d2=d2_in;
}
	
void  setLg (double lg_in){
  lg=lg_in;
}
	
void  setAida (double aida_in){
  aida=aida_in;
}
	
void  setS (double S_in){
  S=S_in;
}
	
void  setAg01 (double Ag01_in){
  Ag01=Ag01_in;
}
	
void  setAg02 (double Ag02_in){
  Ag02=Ag02_in;
}
	
void  setGain (double gain_in){
  gain=gain_in;
}
	
void  setPs (double pressure_in){
  ps=pressure_in;
}
	
void  setFs (double Fs_in){
  Fs=Fs_in;
}
