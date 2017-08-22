%[A,B,C,D]=tf2ss([0 0 2],[1 0 0]);
%G(s)= 1/s
%H(s)=1/s
%[A,B,C,D]=tf2ss([0 1 0],[1 0 1]);
[A,B,C,D]=tf2ss([0 0.3182  1.04323 -3.0620],[1 0 0 0]);
 

%A=eye(2);
%C=eye(2);

%B =[.1 0; .1 .35];

%D=zeros(2);

sys=ss(A,B,C,D);

%minfo=[1 2 1 0 0 1 0];
imod=ss2mod(A,B,C,D);
pmod = imod;

P=2; M=2;%prediction horizon is (H=3)
%ywt=[ .2 .121 ]; uwt=[ ]; %default weights for input and output
ywt=[];
uwt = [];
tend=30; r=[ ];

%ulim=[ 1.002 3.01 9 10 2.5 6  ]; %input constraint setting matrix

%ylim=[ 0 0 6.5 7 ]; %output constraint matrix

ulim =[ ];
ylim=[ ];

[y,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call
