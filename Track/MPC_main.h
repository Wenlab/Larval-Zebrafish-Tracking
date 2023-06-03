#include "MPC.h"

Point2d MPC_main(int m, int predict_length, int fish_history_length, 
	double gammaX, double gammaY, double max_command, int shift_head2yolk,
	double scale_x, double scale_y, double theta, double scale_x2, double scale_y2, Point *dst_fish_position,
	vector<Point2d> command_history, vector<Point2d> position_history, vector<Point2d> fish_tr_history_c, vector<Point2d> fish_direction_history_c);
