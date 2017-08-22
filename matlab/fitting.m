%get last ten states from history file
syms x a b c d e f g h k l

xaxis = [1:53];


%form hist polynomial
y = a*x^9 + b*x^8 + c*x^7 + d*x^6 + e*x^5 + f*x^4 + g*x^3 + h*x^2  + k*x^1 + l;

%find the diff equation for that
 y1 = diff(y);

data =         [0 0 0 1 1 1 0 0 0 1 ]; %least recent to most recent

backupdata = data;


  data =     [0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 1];

for i  =1 : 9  
   
    data(i)= factorial(10-i)*data(i)*(1);
end
    

num = [ 0 fliplr(data)];
den = [1 zeros(1,10)];

[A,B,C,D]=tf2ss(num,den);
    
sys=ss(A,B,C,D);

imod=ss2mod(A,B,C,D);
pmod = imod;

P=1; M=1;%prediction horizon is (H=3)
%ywt=[ .2 .121 ]; uwt=[ ]; %default weights for input and output
ywt=[];
uwt = [];
tend=30; r=[ ];


ulim =[ -.0001 1000 inf];
ylim = [0 100];

[w,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call
