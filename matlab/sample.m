T=0.5;
A=[1 0 0; 0 1 0;0 0 1];

B = [.5 0 0; .5 .5 0; .5 .5 .5];

C=eye(3);

D=zeros(3);

sys=ss(A,B,C,D);
minfo=[T 3 3 0 0 3 0];
pmod = ss2mod(A,B,C,D,minfo)
