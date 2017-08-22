%G(s)= 1/s
%H(s)=1/s
[A,B,C,D]=tf2ss([0 1 0],[1 0 1]);

%y(t+1)=y(t)+2*u(t)  //y=2x
%sz=4
%A=-1*eye(sz(1));
%C=eye(sz(1));
%B =1*C;

%D=zeros(sz(1));
sys=ss(A,B,C,D);

%minfo=[1 2 1 0 0 1 0];
imod=ss2mod(A,B,C,D);
pmod = imod;



P=1; M=1;%prediction horizon is (H=3)

ywt=[  ]; uwt=[  ]; %default weights for input and output
tend=10; r=[ ];

ulim=[ 1 inf 10
    %[ 1 0 1 1] [ 2 2 2 2 ] [ 1 1 1 1]
%[ 1 1 1 1] [ 2 2 2 2 ] [ 1 1 1 1] 

%[ 0 0 1 1] [ 2 2 2 2 ] [ 1 1 1 1] 
]; %input constraint setting matrix

ylim=[];
    %[0 0 0 0] [1 1 1 1] ]; %output constraint matrix

[y,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call
