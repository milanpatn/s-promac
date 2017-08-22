%The polynomial equation is formed such that each coefficient is a sum of its
%previous instance and current. Assuming the source of coefficient's as pattern vector.
%Also whenever a 0 is observed in the pattern -1 is multiplied to the sum .


%get last ten states from history file
syms x a b c d e f g h k l

xaxis = [1:10];


%form hist polynomial
y = a*x^9 + b*x^8 + c*x^7 + d*x^6 + e*x^5 + f*x^4 + g*x^3 + h*x^2  + k*x^1 + l;

%find the diff equation for that
 y1 = diff(y);

data = [0 0 1 1 0 0 1 1 0 0 ]; %least recent to most recent

backupdata = data;

for i=2 : 10
      data(i)=data(i) + data(i-1);
end

for i  =1 : 9 
    if(backupdata(i) == 0)
    data(i)=data(i)*(-1);
    end
    
end
    
if(backupdata(10) == 1)
data(10)=data(10)*-1;
end

for i  =1 : 9
    data(i)= factorial(10-i)*data(i)*(1);
end
    
num = [ 0 fliplr(data)  ];
den = [1 zeros(1,10)];

[A,B,C,D]=tf2ss(num,den);
    
sys=ss(A,B,C,D);

imod=ss2mod(A,B,C,D);
pmod = imod;

P=3; M=3;%prediction horizon is (H=3)
%ywt=[ .2 .121 ]; uwt=[ ]; %default weights for input and output
ywt=[];
uwt = [];
tend=30; r=[ ];

ulim =[-0.0000001 100 inf ];
ylim = [-0.0001 1000];


[w,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call

