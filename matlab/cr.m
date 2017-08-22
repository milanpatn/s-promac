 Fs=12000;
 
 
x1=in(:,1);
x2=in(:,2);
x3=in(:,3);
x4=in(:,4);
x5=in(:,5);

y=x1+x2+x3+x4+x5;
Pxx = periodogram(y);
Hpsd = dspdata.psd(Pxx,'Fs',Fs);

plot(Hpsd)
figure
plot(Pxx);

xlabel('Frequency (KHz)')
ylabel('Power/Frequency (dB/KHz)')
title('Power Spectral Density via Periodogram')

check1 = Pxx(53);
check2 = Pxx(105);
check6 = Pxx(150);
check3 = Pxx(187);
check4 = Pxx(223);
check5 = Pxx(239);

if(check1 < 40)
disp('Assigned to channel 1 as user was not present.');
y1 = ammod(x1,1000,Fs);
y = x1 + x2 + x3 + x4 + x5 + 2*y1;
elseif(check2 < 40)
disp('Assigned to channel 2 as user was not present.');
y2 = ammod(x2,2000,Fs);
y = x1 + x2 + x3 + x4 + x5 + 2*y2;
elseif(check3 < 40)
disp('Assigned to channel 4 as user was not present.');
y4 = ammod(x4,4000,Fs);
y= x1 + x2 + x3 + x4 + x5+ 2*y4;
elseif(check4 < 40)
disp('Assigned to channel 5 as user was not present.');
y5 = ammod(x5,5000,Fs);
y = x1 + x2 + x3 + x4 + x5 + 2*y5;
elseif(check5 < 40)
disp('Assigned to channel 6 as user was not present.');
y6 = ammod(x1,6000,Fs);
y = x1 + x2 + x3 + x4 + x5 + 2*y6;
elseif(check6 < 40)
disp('Assigned to channel 3 as user was not present.');
y3 = ammod(x1,3000,12000);
y = x1 + x2 + x3 + x4 + x5 + 2*y3;
else
disp('All user slots in use. Try again later,');
tp=1;
end 
figure
Pxx = periodogram(y);
Hpsd = dspdata.psd(Pxx,'Fs',Fs);
plot(Hpsd);
figure
plot(Pxx);
xlabel('Frequency (KHz)')
ylabel('Power/Frequency (dB/KHz)')
title('Power Spectral Density via Periodogram')

