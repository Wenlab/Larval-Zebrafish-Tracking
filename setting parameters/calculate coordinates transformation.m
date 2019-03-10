%% fitline to calculate the rotation of axis
%% the point is moving along one of the motion axises
x = [13,507,68,503,499,86,497,505,38,358];
y = [91,105,92,105,104,93,104,104,91,100];
f = fit(x',y','poly1');
f
%% calculate the scale(pix/mm)
x_c = [];
y_c = [];
x_s = [];
y_s = [];
scale = norm([y_c(2) - y_c(1), x_c(2) - x_c(1)])/norm([(y_s(2) - y_s(1))/12800, (x_s(2) - x_s(1))/10000]);