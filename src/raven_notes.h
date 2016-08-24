// settings for IF model:

/* NOTES:

Lengths and fixed variables are fixed per size/human/bird.

open/critical is:

m1/m2 - mass in kg (ok but why is /90?)
k12 coupling constant
k1/k2 - spring constants N/m?
r is damping as 0.0001*sqrt(m*k)

what is conversion from g/ms2 to N/m

say k(dove/zacarelli)= 0.02g/ms2 = 0.00002kg/ms2=0.000196133 sqrt=0.014?
r=0.00000980665
coupling=0.00004903325

 */

/* IF_final.m

-> raven.c mostly accords with this

p=0;        %relative output pressure, 
rho = 1.14; %kg/m^3 mass density - air density CHECK=X
v = 1.85e-5; %N*s/m^2 greek new: air shear viscosity X

lg = 1.63e-2; %m glottal length - female= 0.7cm = 0.007m male=13mm = 0.013 X close enough
twod = 3e-5; %m, glottal width 2d1 = male= 0.8mm = 0.0008 - not close?
d1=twod/2; %1.5000e-005 - so say 4e-4
d2=d1;

///////

m = 4.4e-5/90; %kg, glottal mass WHY IS divided by 90? Avanzini has 4.4e-5
m1=m; %4.8889e-007
m2=m;

k12=0.04; %coupling spring constant ???
k = 0.09; %N/m, spring constant Avanzini has 20 N/m ?????
k1=k;
k2=k1;

aida=1000000.01; %non-linearity factor

r = 0.0001*sqrt(m*k); %damper constant, N*s/m Avanzini has 0.1 * sqrt(m*k)  m0.000044 * 20 - 0.002966
r1=r*1;
r2=r1; %2.0976e-008

%Ag0 = 5e-6; %m^2 glottal area at rest - this is what Avanzini has
Ag0 = 5e-9; %m^2 glottal area at rest

S = 5e-5; %m^2 output area (vocal tract end)
%S = 5e-4; %m^2 output area (vocal tract end) - this looks right according to Praat data eg: http://www.fon.hum.uva.nl/rob/VocalTractExamples/

*/

// test with this data raven.c

   /* raven details (see kahrs.pdf and zacarelli)

for ring dove mass is 0.001g which is 0.000001 kg 1e-6

k as spring constant 2*PI*F0=sqrt(k/m)

so for F0 of 200Hz say 1200=sqrt(k/1e-6); = k=1.44? and r=0.0012

 kahrs:

 

 from zacarelli (which one?) we have:

 stiffness (g ms−2)	k1, k2	22.0×10−3
 damping constant (g ms−1)	r1, r2	1.2×10−3
 coupling constant (g ms−2)	kc	6.0×10−3

 but not sure how to convert between????

 m1/m2=glottal mass - 3.848451000647498e-6 - 0.00000384 /90

 k1/k2-spring constant N/m - 3.11 ???
 k12=coupling spring constant ???

 d1/d2=glottal width 2dl /2??? diameter is 7mm  say 2mm now or is this *thickness?* 1e-4 - from fletcher is 100 micrometer
 r1/r2 = 0.0001*sqrt(m*k); %damper constant, N*s/m - 1.386e-7 // but depends on K spring constant can vary 0.0001

 Ag0 = 5e-9; %m^2 glottal area at rest - 2mm say at rest= 3.14mm 3.14e-6
 S = 5e-5; %m^2 output area (vocal tract end) - 20mm diameter BEAK 314mm = 0.000314

 lg= 1.63e-2; %m glottal length - say 7mm=7e-3

ring dove zaccarelli - mv05_047.pdf

length glottis = 0.3cm = 0.003
rest area = 0.002 cm2 in m2 = 
m1=0.0015g
m2=0.0003g

coupling = 0.0025 g/ms2
damping r = 0.002 g/ms2

k1 = 0.08 g/ms2
k2=0.008 g/ms2

    */
