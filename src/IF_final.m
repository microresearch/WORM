%Avrum Hollinger
%MUMT614
%Nov. 18, 2006


%2-mass vocal fold model
%Ishizaka & Flanagan
%Using backward finite difference approximation
%Also uses an approximation to solve the system of equations
%which uses the parameters from the previous time step in order to make the
%simulation more efficient (i.e. only 1.5 iterations per time sample)

%Output is given as u(n), the flow.
%Scaling may be required in order to make it audible
%Clicks and pops may occur with certain parameters sets

clear;
%constants given in avanzini
Fs = 32000; %Hz, sampling rate
N = 32000; %number of iterations
%for n=1:N
%    ps(n)=2000; %input pressure pa?
%end
MINps=30;
MAXps=400;
T1=N/90;
T2=N/80;


%Set pressure waveform envelope
for n=1:N
if n<5
%ps(n)=MAXps;
ps(n)=0;
elseif n<7
ps(n)=0;
%ps(n)=MAXps;
elseif n<T1
ps(n)=ps(n-1)+MAXps/T1;
elseif n<=T2
    ps(n)=ps(n-1);
else
    ps(n)=ps(n-1)+(MINps-MAXps)/(N-T2);
end

end

%set physical constants

p=0;        %relative output pressure, 
rho = 1.14; %kg/m^3 mass density
v = 1.85e-5; %N*s/m^2 greek new: air shear viscosity
lg = 1.63e-2; %m glottal length
twod = 3e-5; %m, glottal width 2d1
d1=twod/2; %1.5000e-005
d2=d1;
m = 4.4e-5/90; %kg, glottal mass
m1=m; %4.8889e-007
m2=m;
k12=0.04; %coupling spring constant
k = 0.09; %N/m, spring constant
k1=k;
k2=k1;
aida=1000000.01; %non-linearity factor
r = 0.0001*sqrt(m*k); %damper constant, N*s/m
r1=r*1;
r2=r1; %2.0976e-008
%Ag0 = 5e-6; %m^2 glottal area at rest
Ag0 = 5e-9; %m^2 glottal area at rest
S = 5e-5; %m^2 output area (vocal tract end)
%S = 5e-4; %m^2 output area (vocal tract end)


%initialize computational constants
T = 1/Fs; %s, sampling period
rhosn=0.69*rho;
hfrho=rho/2;
twvd1lg=12*v*d1*lg*lg;
twvd2lg=12*v*d2*lg*lg;
Ag01=Ag0;
Ag02=Ag0;
Ag012lg=Ag01/2/lg;
Ag022lg=Ag02/2/lg;
lgd1=lg*d1;
lgd2=lg*d2;
m1T=m1/T/T;
m2T=m2/T/T;
r1T=r1/T;
r2T=r2/T;


%equations

%x1 = zeros(1,N);
%x2 = zeros(1,N);

%pm1 = zeros(1,N);
%pm2 = zeros(1,N);
%p = zeros(1,N);
%u = zeros(1,N);

%initialize first two time steps of displacement
%x1(1)=0;
%x1(2)=-Ag0/(2*lg);
%x2(1)=0;
%x2(2)=-Ag0/(2*lg);

%assume some value
%x1(n)=-Ag0/(1.5*lg)
%x2(n)=-Ag0/(1.6*lg)
%x1(n)=0
%x2(n)=0


%initialize first two time steps of displacement
x1(1)=0;
x1(2)=0;
x2(1)=0;
x2(2)=0;
A1(1)=Ag01+lg*x1(1);
A1(2)=Ag01+lg*x1(2);
A2(1)=Ag02+lg*x2(1);
A2(2)=Ag02+lg*x2(2);
pm1(1)=0;
pm1(2)=0;
pm2(1)=0;
pm2(2)=0;
u(1)=0;
u(2)=0;


A1(1)=0;
A1(2)=0;

A2(1)=0;
A2(2)=0;



%start loop here
%start n at three

for n=3:N;

    C11=k1*(1+aida*x1(n-1)^2);
    C12=k2*(1+aida*x2(n-1)^2);
    C21=k1*(1+aida*(x1(n-1)+Ag012lg)^2);
    C22=k2*(1+aida*(x2(n-1)+Ag022lg)^2);
    alpha1=lgd1*pm1(n-1);
    alpha2=lgd2*pm2(n-1);
    beta1=m1T*(x1(n-2)-2*x1(n-1));
    beta2=m2T*(x2(n-2)-2*x2(n-1));
    gamma1=-r1T*x1(n-1);
    gamma2=-r2T*x2(n-1);
    delta1=Ag012lg*C21;
    delta2=Ag022lg*C22;
    lambda1=-k12*x2(n-1);
    lambda2=-k12*x1(n-1);

    if  (x1(n-1)>=-Ag012lg)
        x1(n)=(alpha1-beta1-gamma1-lambda1)/(m1T+r1T+C11+k12);
    else
        x1(n)=(alpha1-beta1-gamma1-lambda1-delta1)/(m1T+r1T+C21+k12);
    end

    if  (x2(n-1)>=-Ag022lg)
        x2(n)=(alpha2-beta2-gamma2-lambda2)/(m2T+r2T+C12+k12);
    else
        x2(n)=(alpha2-beta2-gamma2-lambda2-delta2)/(m2T+r2T+C22+k12);
    end

    A1(n)=Ag01+lg*x1(n);
    A2(n)=Ag02+lg*x2(n);
    
    if (A1(n)<=0)
        A1(n)=0.1e-25;
    end

    if (A2(n)<=0)
        A2(n)=0.1e-25;
    end
    
    
    A1n2=A1(n)*A1(n); 
    A1n3=A1n2*A1(n);

    A2n2=A2(n)*A2(n); 
    A2n3=A2n2*A2(n);


    a= (rhosn/A1n2)+hfrho*(1/A2n2-1/A1n2)+hfrho/A2n2*(2*A2(n)/S*(1-A2(n)/S));
    b= twvd1lg/A1n3+twvd2lg/A2n3;
    c= p-ps(n);

    flow1=(-b+sqrt(b^2-4*a*c))/(2*a);
    flow2=(-b-sqrt(b^2-4*a*c))/(2*a);
    %u(n)=min(abs(flow1),abs(flow2));

    udif1(n)=abs(flow1-u(n-1));
    udif2(n)=abs(flow2-u(n-1));

    if (udif1(n)<udif2(n))
        u(n)=real(flow1);
    else
        u(n)=real(flow2);
    end

    %u(n)=(max(flow1,flow2));
    %um(n)=min(flow1,flow2);

    %solve for mean pressures, pm1, pm2

    g1(n)=rhosn*u(n)^2/A1n2;
    g2(n)=twvd1lg*u(n)/A1n3;
    g4(n)=twvd2lg*u(n)/A2n3;
    g5(n)=hfrho*u(n)^2/A2n2*(2*A2(n)/S*(1-A2(n)/S));

    pm1(n)=ps(n)-g1(n)-g2(n)/2;
    pm2(n)=p+g5(n)+g4(n)/2;




    if (x1(n)>=-Ag012lg)
    
        pm1b(n)=(m1T*(x1(n)-2*x1(n-1)+x1(n-2))+r1T*(x1(n)-x1(n-1))+ k1*x1(n)*(1+aida*x1(n)^2) +k12*(x1(n)-x2(n)))/(lgd1);
        pm1(n)=pm1(n)*0.5+pm1b(n)*(1-0.5);
        %pm1(n)=pm1b(n);
        if (x2(n)>=-Ag022lg)

            pm2b(n)=(m2T*(x2(n)-2*x2(n-1)+x2(n-2))+r2T*(x2(n)-x2(n-1))+ k2*x2(n)*(1+aida*x2(n)^2) -k12*(x1(n)-x2(n)))/(lgd2);
            pm2(n)=pm2(n)/2+pm2b(n)/2;
        else
         %   pm2(n)=ps(n);
         %   pm1(n)=ps(n);
        end 
    else
      %  pm1(n)=ps(n);
      %  pm2(n)=0;
    end

    if pm1(n)>ps(n)
      %  pm1(n)=ps(n);
    end
    if pm2(n)>ps(n)
      %  pm2(n)=ps(n);
    end
    if pm1(n)<0
     %   pm1(n)=abs(pm1(n));
        pm1(n)=0;
    end

    if pm2(n)<0
        pm2(n)=0;
    end
    if u(n)<0
      %  u(n)=0;
    end

end

for n=2:N
	out(n)=(u(n)-u(n-1))*1000.0;
end

plot(out);
pause();

%wavwrite(out, 32000, "test.wav");
%out


