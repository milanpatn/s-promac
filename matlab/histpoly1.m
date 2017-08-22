%The polynomial equation is formed such that each coefficient is a sum of its
%previous instance and current. Assuming the source of coefficient's as pattern vector.
%for eg at t+5 represents coeff of x^5, where the coefficients represents
%num of 1's found from t-10 till t-5.

%get last ten states from history file
% 1 means available 0 means busy
clc;
clear;
syms x a b c d e f g h k l
sz = 10;
w=0;

%form hist polynomiali    %least recent to most recent
y = a*x^9 + b*x^8 + c*x^7 + d*x^6 + e*x^5 + f*x^4 + g*x^3 + h*x^2  + k*x^1 + l;

%find the diff equation 
y1 = diff(y);

%reading history from file
fileID = fopen('./num2.txt','r+');
comma  = char('');
data = dlmread('./num2.txt');

fileID1 = fopen('./op.txt','w');

sz=size(data);

rows=sz(1);
cols=sz(2);

% data ready

backupdata = data;

for i=1 :rows
    for j=2 : cols
        data(i,j)=data(i,j) + data(i,j-1);
    end
end

%constructing Transform function eqn
for j=1:rows
    for i  =1 : cols - 1
        data(j,i)= factorial(10-i)*data(j,i)*(1);
    end
end
num=zeros(11,11);
den=num;

for j=1:rows
    num(j,:) = [0 fliplr(data(j,:))];
end

for j=1:10
    den(j,:) = [1 zeros(1,10)] ;
end
%op = tf({[num(1,:) 0 0 0 0 0 0 0 0 0 ];[0 num(2,:) 0 0 0 0 0 0 0 0];[0 0 num(3,:) 0 0 0 0 0 0 0];[0 0 0 num(4,:) 0 0 0 0 0 0 0];[0 0 0 0 num(5,:) 0 0 0 0 0 ];[0 0 0 0 0 num(6,:) 0 0 0 0];[0 0 0 0 0 0 num(7,:) 0 0 0];[ 0 0 0 0 0 0 0 0 num(8,:) 0];[0 0 0 0 0 0 0 0 num(9,:) 0];[ 0 0 0 0 0 0 0 0 0 num(10,:)]},{den(1,:);den(2,:);den(3,:);den(4,:);den(5,:);den(6,:);den(7,:);den(8,:);den(9,:);den(10,:)});


Ts=1;
op = tf({num(1,:)    ; num(2,:)  ;  num(3,:)    ;   num(4,:)    ; num(5,:)   ;  num(6,:) ;   num(7,:)   ;   num(8,:) ;  num(9,:) ;    num(10,:)},{den(1,:);den(2,:);den(3,:);den(4,:);den(5,:);den(6,:);den(7,:);den(8,:);den(9,:);den(10,:)});

Q=ss(op);

A = Q.a;
B=Q.b;
C=Q.c;
D=Q.d;

imod=ss2mod(A,B,C,D);
pmod = imod;

P=3; M=3;%prediction horizon is (H=3)
%ywt=[ .2 .121 ]; uwt=[ ]; %default weights for input and output
ywt=[];
uwt = [];
tend=30; r=[ ];
%ulim =[-0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 -0.0000001 100 100 100 100 100 100 100 100 100 100 inf inf inf inf inf inf inf inf inf inf ];
ulim= [ -inf 1000 1];
ylim = [0.0001 0.0001 0.0001 0.0001 0.0001 0.0001 0.0001 0.0001 0.0001 0.0001  1000000 1000000 1000000 1000000 1000000 1000000 1000000 1000000 1000000 1000000 ];
%ylim = [];

 for i=1:10
            fprintf(fileID1,'%d ',1);
 end

fclose(fileID1);
fclose(fileID);

[w,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call

szw = size(w);

if(szw(1) >= 4)
    
    cutoff = mean(w(4,:));
  
fileID1 = fopen('./op.txt','w');  
    for i=1:10
        if(w(4,i) >= cutoff)
            fprintf(fileID1,'%d ',1);
        else
            fprintf(fileID1,'%d ',0);
        end
    end
    
fclose(fileID1);
else
    for i=1:10
            fprintf(fileID1,'%d ',1);
    end
    
fclose(fileID1);
end

%fprintf(fileID,'%d %4.4f\n',y(2,:));