#include "MPC_main.h"

Point2d MPC_main(int m, int predict_length, int fish_history_length,
	double gammaX, double gammaY, double max_command, int shift_head2yolk,
	double scale_x, double scale_y, double theta, double scale_x2, double scale_y2, Point *dst_fish_position,
	vector<Point2d> command_history, vector<Point2d> position_history, vector<Point2d> fish_tr_history_c, vector<Point2d> fish_direction_history_c)
{
	vector<Point2d> H;	//position response kernel (mm)
	/*input H*/
	double Hx_array[300] = { 0.000000,0.003575,0.019378,0.030285,0.035119,0.031150,0.029601,0.028469,0.028335,0.029589,0.028783,0.029869,0.029348,0.029555,0.029693,0.029073,0.029514,0.028775,0.029153,0.029132,0.029097,0.029497,0.029155,0.029602,0.029329,0.029450,0.029527,0.029285,0.029493,0.029205,0.029432 };//3ms
	double Hy_array[300] = { 0.000000,-0.000660,-0.008018,-0.019458,-0.026888,-0.031872,-0.033617,-0.033450,-0.033334,-0.033350,-0.032662,-0.032252,-0.032332,-0.032240,-0.031862,-0.031997,-0.031904,-0.031493,-0.031076,-0.030907,-0.030771,-0.030551,-0.030498,-0.030437,-0.030228,-0.029926,-0.029666,-0.029553,-0.029680,-0.029742,-0.029891 };
	H.clear();
	for (int i = 0; i <= m; i++)
	{
		H.push_back(Point2d(Hx_array[i], Hy_array[i]));
	}
	vector<Point2d>::iterator it, it2;
	for (it = position_history.begin(); it != position_history.end(); it++)
	{
		it->x = it->x*scale_x2;
		it->y = it->y*scale_y2;
	}

	vector<Point2d> fish_tr_history;
	vector<Point2d> fish_direction_history;
	/*transform fish position and heading direction from camera coordinate to stage coordinate*/
	for (it = fish_tr_history_c.begin(); it != fish_tr_history_c.end(); it++)
	{
		it->x = it->x - dst_fish_position->x;	//calculate the difference between fish position and the destination
		it->y = it->y - dst_fish_position->y;
	}
	for (it = fish_direction_history_c.begin(); it != fish_direction_history_c.end(); it++)
	{
		fish_direction_history.push_back(trans_c2s(1, 1, theta, *it, Point2d(0, 0)));
	}
	it2 = position_history.begin();
	for (it = fish_tr_history_c.begin(); it != fish_tr_history_c.end(); it++)
	{
		fish_tr_history.push_back(trans_c2s(scale_x, scale_y, theta, *it, *it2));
		it2++;
	}

	double shift_head2yolk_f = shift_head2yolk*scale_x;	//in fact, scale_x should be equal to scale_y now.
	Point2d c0 = MPC(m, predict_length, H, gammaX, gammaY, shift_head2yolk_f, command_history, position_history, fish_tr_history, fish_direction_history);
	if (c0.x > max_command)
		c0.x = max_command;
	if (c0.x < -max_command)
		c0.x = -max_command;
	if (c0.y > max_command)
		c0.y = max_command;
	if (c0.y < -max_command)
		c0.y = -max_command;

	return c0;

}