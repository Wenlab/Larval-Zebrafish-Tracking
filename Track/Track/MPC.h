#include "opencv2/opencv.hpp"
#include <opencv2\highgui\highgui.hpp>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

//t=0 present, t<0 past, t>0 future
//m: length of velocity response kernel
//m+1: length of position response kernel
//predict_length: number of time steps predicted (depends on the fish's behavior, and the timestep)
//H: position response kernel (mm), which is the integral of velocity response kernel h.
//H[t]==0, t<=0
//H[t]==H[m], t>=m
//note: time step of H and commands should be the same.
//gamma: coefficient of L2 penalty to the planned future acceleration vector. 
//gamma should be less than the maximum of H to ensure the speed of catching the fish, but not too large to avoid vibration.
//command_history: commands t<=0
//command_history.size() == m + 1
//position_history: stage position t<=0
//fish_tr_history: fish position t<=0, in stage coordinate (mm)
//fish_direction_history: fish heading direction t<=0, in stage coordinate
//position_history.size() == fish_tr_history.size() == fish_direction_history.size() == fish_history_length + 1 (depends on the fish's behavior, and the timestep)
//return the optimal command at t=1, c1. 
//'optimal' means the sum of square error between the future stage positions and fish positions is least.
Point2d MPC(int m, int predict_length, vector<Point2d> H, double gammaX, double gammaY, double shift_head2yolk,
	vector<Point2d> command_history, vector<Point2d> position_history,
	vector<Point2d> fish_tr_history, vector<Point2d> fish_direction_history);

Point2d calcHt(vector<Point2d> &H, int t);

//predict stage positions (considering only previous commands)
//predicted_stage_positions[0] is the present stage position, the same as 'position'.
void predict_stage_positions(vector<Point2d> &predicted_stage_positions,
	vector<Point2d> &command_history, Point2d position,
	vector<Point2d> &H, int m, int predict_length);

//predict fish positions
//predict_fish_tr[0] is the present fish position, the same as fish_tr_history.back().
void predict_fish_tr(vector<Point2d> &predicted_fish_tr,
	int predict_length, double shift_head2yolk,
	vector<Point2d> fish_tr_history, vector<Point2d> &fish_direction_history);

double inner_prod(Point2d v1, Point2d v2);

//transform fish position and heading direction from camera coordinate to stage coordinate
//p: fish position in camera coordinate
//stage_position: stage position in stage coordinate
Point2d trans_c2s(double scale_x, double scale_y, double theta, Point2d p, Point2d stage_position);
