data =[1 1 1 1 1 0 1 1 1 1 0]; %least recent to most recent

A= [ 0     0     0     0     0     0     0     0     0     0;
     1     0     0     0     0     0     0     0     0     0;
     0     1     0     0     0     0     0     0     0     0;
     0     0     1     0     0     0     0     0     0     0;
     0     0     0     1     0     0     0     0     0     0;
     0     0     0     0     1     0     0     0     0     0;
     0     0     0     0     0     1     0     0     0     0;
     0     0     0     0     0     0     1     0     0     0;
     0     0     0     0     0     0     0     1     0     0;
     0     0     0     0     0     0     0     0     1     0    ];
 
 B= eye(10);

C=   [  0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0;
0     2     4    24    48   480     0     0     0     0];


D= zeros(10);


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

ulim =[ ];
%ulim =[];
%ylimdata = data -1;
%ylim=[  1 1 1 1 1 1 1 1 1 1 inf inf  inf inf inf inf inf inf inf inf];
ylim=[];
[y,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call




