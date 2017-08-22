data =[1 1 1 1 1 0 1 1 1 1 0]; %least recent to most recent

A= [1 0 0 0 0 0 0 0 0 0 ;
    1 1 0 0 0 0 0 0 0 0 ;
    1 1 1 0 0 0 0 0 0 0 ;
    1 1 1 1 0 0 0 0 0 0 ;
    1 1 1 1 1 0 0 0 0 0 ;
    1 1 1 1 1 1 0 0 0 0 ;
    1 1 1 1 1 1 1 0 0 0 ;
    1 1 1 1 1 1 1 1 0 0 ;
    1 1 1 1 1 1 1 1 1 0 ;
    1 1 1 1 1 1 1 1 1 1 ;
    ];


%A= ones(10,10);


B=[data(1) data(11) data(11) data(11) data(11) data(11) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(11) data(11) data(11) data(11) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(11) data(11) data(11) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4) data(11) data(11) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5) data(11) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5)  data(6) data(11) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5)  data(6)  data(7) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5)  data(6)  data(7) data(11) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5)  data(6)  data(7) data(8) data(11) data(11) ;
data(1) data(2)  data(3)  data(4)  data(5)  data(6)  data(7) data(8) data(9) data(10) ];


%C=ones(1,10);
C= eye(10);

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

ulim =[ 0 0 0 0 0 0 0 0 0 0 1000 1000 1000 1000 1000 1000 1000 1000 1000 1000 1 1 1 1 1 1 1 1 1 1 ];
%ulim =[];
%ylimdata = data -1;
%ylim=[  1 1 1 1 1 1 1 1 1 1 inf inf  inf inf inf inf inf inf inf inf];
ylim=[ 0 0 0 0 0 0 0 0 0 0 1 1 1 1 1 1 1 1 1 1];
[y,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call




