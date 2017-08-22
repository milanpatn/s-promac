fileID = fopen('./num2.txt','r+');
comma  = char('');
data = dlmread('./num2.txt');

x = 1:1:5;
y = 2*data;

%rng('shuffle');a = 40;b = 320;y= ((b-a).*rand(71,1) + a);

fileID1 = fopen('./op.txt','w');


for i=1:10
fprintf(fileID1,'%d ',y(i,:));
fprintf(fileID1,'%c\n',comma);
end
%fprintf(fileID,'%d %4.4f\n',y(2,:));

fclose(fileID);
fclose(fileID1);
