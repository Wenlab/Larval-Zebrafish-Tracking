%% t: in tracking camera
%% l: in low resolution camera
%% s: stage position
xt = [59,204,343,31,192,350,43,187,337];
yt = [172,174,177,297,299,302,53,56,58];
xl = [81,300,510,44,286,523,54,270,498];
yl = [253,252,250,441,439,438,77,75,73];
xs = [-52512,-3801,42655,-61245,-7377,45206,-58104,-10225,40490];
ys = [33125,33126,33127,86314,86310,86308,-16807,-16807,-16807];

%% for the tracking camera
%% fitline to calculate the rotation of axis
%% the point is moving along one of the motion axises
x = xt(1:3);
y = yt(1:3);
f1 = fit(x',y','poly1')
x = xt(4:6);
y = yt(4:6);
f2 = fit(x',y','poly1')
x = xt(7:9);
y = yt(7:9);
f3 = fit(x',y','poly1')
p1 = -(f1.p1+f2.p1+f3.p1)/3.0
%% calculate the scale(pix/mm)
x_c = [xt(1),xt(3)];
y_c = [yt(1),yt(3)];
x_s = [xs(1),xs(3)];
y_s = [ys(1),ys(3)];
scale1 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
x_c = [xt(4),xt(6)];
y_c = [yt(4),yt(6)];
x_s = [xs(4),xs(6)];
y_s = [ys(4),ys(6)];
scale2 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
x_c = [xt(7),xt(9)];
y_c = [yt(7),yt(9)];
x_s = [xs(7),xs(9)];
y_s = [ys(7),ys(9)];
scale3 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
scale = (scale1+scale2+scale3)/3.0

%% fit the affine transformation from the tracking camera to the low resolution camera
Pt = [xt',yt'];
Pl = [xl',yl'];
tform = fitgeotrans(Pt,Pl,'affine')

%% for the low resolution camera
%% fitline to calculate the rotation of axis
%% the point is moving along one of the motion axises
x = xl(1:3);
y = yl(1:3);
f1 = fit(x',y','poly1')
x = xl(4:6);
y = yl(4:6);
f2 = fit(x',y','poly1')
x = xl(7:9);
y = yl(7:9);
f3 = fit(x',y','poly1')
p1 = -(f1.p1+f2.p1+f3.p1)/3.0
%% calculate the scale(pix/mm)
x_c = [xl(1),xl(3)];
y_c = [yl(1),yl(3)];
x_s = [xs(1),xs(3)];
y_s = [ys(1),ys(3)];
scale1 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
x_c = [xl(4),xl(6)];
y_c = [yl(4),yl(6)];
x_s = [xs(4),xs(6)];
y_s = [ys(4),ys(6)];
scale2 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
x_c = [xl(7),xl(9)];
y_c = [yl(7),yl(9)];
x_s = [xs(7),xs(9)];
y_s = [ys(7),ys(9)];
scale3 = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000])
scale = (scale1+scale2+scale3)/3.0