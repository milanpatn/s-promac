num=zeros(11,11);
den=num;

for j=1:10
    num(j,:) = [0 zeros(1,10)];
end

for j=1:10
    den(j,:) = [1 zeros(1,10)] ;
end

nums = {num(1,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] ;[zeros(1,11)] num(2,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] 0;[zeros(1,11)] [zeros(1,11)] num(3,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)]  ;[zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(4,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)]  ;[zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(5,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] ;[zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(6,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] ;[zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(7,:) [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] ; [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(8,:) [zeros(1,11)] ; [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(9,:) [zeros(1,11)] ; [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] [zeros(1,11)] num(10,:)};
    