
sz(1)=71;%no of SU
sz(2)=51;%no of PU
rng('shuffle');

%Both SU and PU are represented as dist from BS, node id pair
a = 25;
b = 190;
dist(:,1) = (b-a).*rand(sz(1),1) + a;%sec user dist from BS
dist(:,2)= 1:sz(1);%sec node id
init_dist= transpose(sortrows(dist));

rng('shuffle');
%[A,B,C,D]=tf2ss([0 0 2],[1 0 0]);

%y(t+1)=y(t)+2*u(t) //y=2x
A=eye(sz(1));
C=eye(sz(1));
B =2*C;

D=zeros(sz(1));
sys=ss(A,B,C,D);

%minfo=[1 2 1 0 0 1 0];
imod=ss2mod(A,B,C,D);
pmod = imod;

%Generating constraints
x = randperm(sz(1));

a = 1;
b = 100000;
delx= transpose((b-a).*rand(sz(1),1) + a);



P=3; M=1;%prediction horizon is (H=3)

ywt=[  ]; uwt=[  ]; %default weights for input and output
tend=10; r=[ ];

ulim=[ [-inf*init_dist(1,:)]  [220*(ones(1,71))] [delx] ]; %input constraint setting matrix

ylim=[[ 1*init_dist(1,:)]  [800*(ones(1,71))] ]; %output constraint matrix

[y,u]=scmpc(pmod,imod,ywt,uwt,M,P,tend,r,ulim,ylim); %mpc call

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fileID = fopen('~/Desktop/pair.txt','w');

%fprintf(fileID,'%d %4.4f\n',y(2,:));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%PU has 1-1 reln with channel, licensed
%PU distance from BS

dist1(:,1) = (320 - 60).*rand(sz(2),1) + 60;
dist1(:,2)=1:sz(2);%channel id
pudist = transpose(sortrows( dist1));%PU dist,channel id sorted

count=0;
i=1;
j=1;
 %fprintf(fileID,'Secnode  ChId\n');

while j<=sz(1) && i <=sz(2)
   
if(( y(2,j) >=(pudist(1,i) - 5)) && ( y(2,j) <=(pudist(1,i) + 5)))
    %allocate
    disp('allocated ');
    fprintf('node %d\t Secdist = %d Allocate ChId = %d\n',init_dist(2,j),y(2,j),pudist(2,i));
    fprintf(fileID,'%d\t %d\n',init_dist(2,j),pudist(2,i));%write into file sec node and alloc channel id

    i=i+1; j = j + 1;
    count=count+1;
elseif (y(2,j) > (pudist(1,i) + 5))
    i=i+1;    
else
    j=j+1;
end


end
fclose(fileID);

%fprintf('No of channels allocated = %d\n',count);
%yt = transpose(y);

