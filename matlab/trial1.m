
syms x a b c d e f g h k l
%form hist polynomial

y = a*x^9 + b*x^8 + c*x^7 + d*x^6 + e*x^5 + f*x^4 + g*x^3 + h*x^2  + k*x^1 + l;

%find the diff equation for that
 y1 = diff(y);
 
 
 
 %Transfer function is Y(s)/X(s)
%L[f`(t)] = sF ( s ) - f ( 0) - laplace formula on RHS

%sY(s) - data(10) -> 'l' alone remains in y(0)



data = [1 1 2 2 1 2 3 2 2 2]; %coefficient matrix [a b c d e f g h k l]

% we very well know X(s) is "a.9! + b.8!.s + c.7!.s^2 ..... +k.1!.s^8 +  l.s^9

 % 9      8        7        6         5          4          3           2
%l s  + k s  + 2 h s  + 6 g s  + 24 f s  + 120 e s  + 720 d s  + 5040 c s  + 40320 b s + 362880 a
%------------------------------------------------------------------------------------------------
 %                                               9
  %                                             s
   
  
  
%form coeff vector based on abv transform eqn
for i  =1 : 9
   data(i)=factorial(10-i)*data(i);
    
end
    

num = [ 0 fliplr(data)];

den = [1 zeros(1,10)];

%state space vector notaion
[A,B,C,D]=tf2ss(num,den)
sys=ss(A,B,C,D);


imod=ss2mod(A,B,C,D);
pmod = imod;

P=1; M=1;%prediction horizon is (H=3)
%ywt=[ .2 .121 ]; uwt=[ ]; %default weights for input and output
ywt=[];
uwt = [];
tend=30; r=[ ];

%ulim=[ 1.002 3.01 9 10 2.5 6  ]; %input constraint setting matrix

%ylim=[ 0 0 6.5 7 ]; %output constraint matrix

ulim =[ 0.05 1 1  ];
ylim= [1  inf ];

[w,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call