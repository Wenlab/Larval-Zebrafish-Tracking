%%calculate response kernel
n = 3000;
m = 30;
command_x = load('stage test\with new x axis driver\\test results\3ms white noise 20190125\3\white noise test command_x.lvm');
command_y = load('stage test\with new x axis driver\\test results\3ms white noise 20190125\3\white noise test command_y.lvm');
position=load('stage test\with new x axis driver\\test results\3ms white noise 20190125\3\white noise test position.lvm');
v = position(2:n,1:2)-position(1:(n-1),1:2);
Xx =[];
for i=m:-1:1
	Xx = [Xx,command_x(i:(n-1)-m+i)];
end
Xy =[];
for i=m:-1:1
	Xy = [Xy,command_y(i:(n-1)-m+i)];
end
h_x = inv(Xx'*Xx)*Xx'*v(m:(n-1),1);
h_y = inv(Xy'*Xy)*Xy'*v(m:(n-1),2);
Hx = 0;
Hy = 0;
for i=1:m
	Hx = [Hx;Hx(i,1)+h_x(i,1)];
	Hy = [Hy;Hy(i,1)+h_y(i,1)];
end
t = 0.003:0.003:0.003*n;
figure
plot(t,command_x);
figure
plot(t,command_y);
figure
plot(t,position(:,1)/10000);
figure
plot(t,position(:,2)/12800);
figure
plot(t(1:size(t,2)-1),v(:,1)/10000/0.003);
figure
plot(t(1:size(t,2)-1),v(:,2)/12800/0.003);
t = 0.003:0.003:0.003*m;
figure
plot(t,h_x/10000/0.003);
figure
plot(t,h_y/12800/0.003);
t = [0,t];
figure
plot(t,Hx/10000);
figure
plot(t,Hy/12800);

%%predict position and velocity
position_predict(1,1)=position(1,1);
position_predict(1,2)=position(1,2);
for t=2:n
	position_predict(t,1)=position_predict(t-1,1);
	position_predict(t,2)=position_predict(t-1,2);
	tao_min = 1;
	if t-m>tao_min
		tao_min = t-m;
	end
	for tao=tao_min:t-1
		position_predict(t,1) = position_predict(t,1)+command_x(tao)*(Hx(t-tao+1)-Hx(t-tao));
		position_predict(t,2) = position_predict(t,2)+command_y(tao)*(Hy(t-tao+1)-Hy(t-tao));
	end
end
figure
plot(position_predict(:,1));
figure
plot(position_predict(:,2));
err = position(:,1:2)-position_predict;
figure
plot(err(:,1));
figure
plot(err(:,2));
v_predict = position_predict(2:n,:)-position_predict(1:(n-1),:);
err_v = v-v_predict;
figure
plot(err_v(:,1));
figure
plot(err_v(:,2));
err_v_rel = err_v./v;
figure
plot(err_v_rel(:,1));
figure
plot(err_v_rel(:,2));
figure
hist(err_v_rel(:,1));
figure
hist(err_v_rel(:,2));
figure;
plot(v(:,1));
hold on
plot(v_predict(:,1),'r');
plot(err_v(:,1),'g');
figure;
scatter(v(:,1),v_predict(:,1));



%% 验证position_predict的计算是否正确
for t=1:n-1
	tao_min = 1;
	if t-m+1>tao_min
		tao_min = t-m+1;
	end
	v_predict2(t,1) = 0;
	v_predict2(t,2) = 0;
	for tao=tao_min:t
		v_predict2(t,1) = v_predict2(t,1)+command_x(tao)*h_x(t-tao+1);
		v_predict2(t,2) = v_predict2(t,2)+command_y(tao)*h_y(t-tao+1);
	end
end
err_v2 = v_predict2-v_predict;
figure
plot(err_v2(:,1));
figure
plot(err_v2(:,2));


%% output H
Hx = Hx/10000;
Hy = Hy/12800;
FID = fopen('Hx.txt','w');
fprintf(FID,'%f,',Hx);
fclose(FID);
FID = fopen('Hy.txt','w');
fprintf(FID,'%f,',Hy);
fclose(FID);