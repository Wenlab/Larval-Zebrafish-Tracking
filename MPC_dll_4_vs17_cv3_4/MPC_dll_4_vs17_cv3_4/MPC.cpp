#include "stdafx.h"
#include "MPC.h"

Point2d MPC(int m, int predict_length, vector<Point2d> H, double gammaX, double gammaY, double shift_head2yolk,
	vector<Point2d> command_history, vector<Point2d> position_history,
	vector<Point2d> fish_tr_history, vector<Point2d> fish_direction_history)
{
	 
	vector<Point2d> predicted_fish_tr;	//predicted fish trajectory
	/*predict fish trajectory*/
	predict_fish_tr(predicted_fish_tr,
		predict_length, shift_head2yolk,
		fish_tr_history, fish_direction_history);

	vector<Point2d> predicted_stage_positions;	//predicted stage positions (considering only previous commands)
	/*predict stage positions*/
	predict_stage_positions(predicted_stage_positions,
		command_history, position_history.back(),	//position_history.back() is the present stage position
		H, m, predict_length);


	Mat A1(predict_length - 1, predict_length - 1, CV_64FC1);	//Ax=b (x direction)
	Mat x1(predict_length - 1, 1, CV_64FC1);	//predicted optimal commands
	Mat b1(predict_length - 1, 1, CV_64FC1);
	Mat A2(predict_length - 1, predict_length - 1, CV_64FC1);	//Ax=b (y direction)
	Mat x2(predict_length - 1, 1, CV_64FC1);	//predicted optimal commands
	Mat b2(predict_length - 1, 1, CV_64FC1);

	/*calculate A*/
	double a1, a2;
	for (int i = 2; i <= predict_length; i++)
	{
		for (int j = 2; j <= predict_length; j++)
		{
			a1 = 0;
			a2 = 0;
			for (int k = 1; k <= predict_length; k++)
			{
				a1 += calcHt(H, k - i + 1).x * calcHt(H, k - j + 1).x;
				a2 += calcHt(H, k - i + 1).y * calcHt(H, k - j + 1).y;
			}

			//L2 penalty to the planned future acceleration vector
			if (i == j)
			{
				if (i == predict_length)
				{
					a1 += gammaX;
					a2 += gammaY;
				}
				else
				{
					a1 += gammaX * 2;
					a2 += gammaY * 2;
				}
			}
			else
			{
				if (i == j + 1 || i + 1 == j)
				{
					a1 -= gammaX;
					a2 -= gammaY;
				}
			}

			A1.at<double>(i-2, j-2) = a1;
			A2.at<double>(i-2, j-2) = a2;
		}
	}
	/*calculate b*/
	double bb1, bb2, bbb1, bbb2;
	for (int i = 2; i <= predict_length; i++)
	{
		bb1 = 0;
		bb2 = 0;
		bbb1 = 0;
		bbb2 = 0;
		for (int k = 1; k <= predict_length; k++)
		{
			bb1 -= calcHt(H, k - i + 1).x*(predicted_stage_positions[k].x + predicted_fish_tr[k].x);
			bb2 -= calcHt(H, k - i + 1).y*(predicted_stage_positions[k].y + predicted_fish_tr[k].y);
			bbb1 -= calcHt(H, k - i + 2).x*calcHt(H, k - i + 1).x;
			bbb2 -= calcHt(H, k - i + 2).y*calcHt(H, k - i + 1).y;
		}
		b1.at<double>(i - 2, 0) = bb1 + bbb1*command_history.back().x;
		b2.at<double>(i - 2, 0) = bb2 + bbb1*command_history.back().y;
	}
	//L2 penalty to the planned future acceleration vector
	b1.at<double>(0, 0) += gammaX*command_history.back().x;
	b2.at<double>(0, 0) += gammaY*command_history.back().y;

	/*solve for x*/
	x1 = A1.inv(DECOMP_EIG)*b1;
	x2 = A2.inv(DECOMP_EIG)*b2;


	Point2d c1;
	c1.x = x1.at<double>(0, 0);
	c1.y = x2.at<double>(0, 0);
	
//to be deleted
	//display some results
	//ofstream output_command_position;
	//output_command_position.open("E:\\output_command_position.txt", ios::out|ios::app);
	//output_command_position << 
	//	c1.x << ", " << c1.y <<  
	//	", " << position_history.back().x << ", " << position_history.back().y << endl;
 	//output_command_position.close();
	
	return c1;
}

Point2d calcHt(vector<Point2d> &H, int t)
{
	if (t <= 0)
		return Point2d(0, 0);
	else
	{
		if (t >= H.size())
			return *(H.end() - 1);
		else
			return H[t];
	}
}

void predict_stage_positions(vector<Point2d> &predicted_stage_positions,
	vector<Point2d> &command_history, Point2d position,
	vector<Point2d> &H, int m, int predict_length)
{
	predicted_stage_positions.clear();
	predicted_stage_positions.push_back(position);	//predicted_stage_positions[0] is the present stage position

	for (int t = 1; t <= predict_length; t++)
	{
		Point2d predicted_stage_position_temp = predicted_stage_positions.back();
		for (int tao = t; tao < m; tao++)
		{
			predicted_stage_position_temp.x += command_history[tao].x * (calcHt(H, m + t - tao).x - calcHt(H, m + t -1 - tao).x);
			predicted_stage_position_temp.y += command_history[tao].y * (calcHt(H, m + t - tao).y - calcHt(H, m + t -1 - tao).y);
		}
		predicted_stage_positions.push_back(predicted_stage_position_temp);
	}
}

void predict_fish_tr(vector<Point2d> &predicted_fish_tr,
	int predict_length, double shift_head2yolk,
	vector<Point2d> fish_tr_history, vector<Point2d> &fish_direction_history)
{
	/*shift head to yolk*/
	vector<Point2d>::iterator it,it2;
	it2 = fish_direction_history.begin();
	for (it = fish_tr_history.begin(); it != fish_tr_history.end(); it++)
	{
		it->x -= it2->x*shift_head2yolk;
		it->y -= it2->y*shift_head2yolk;
		it2++;
	}

	predicted_fish_tr.clear();
	predicted_fish_tr.push_back(fish_tr_history.back());	//fish_tr_history.back() and predicted_fish_tr[0] are both the present fish position

	Point2d fish_direction_average(0, 0);	//average fish heading direction
	double fish_direction_average_length;
	for (it = fish_direction_history.begin(); it != fish_direction_history.end(); it++)
	{
		fish_direction_average.x += it->x;
		fish_direction_average.y += it->y;
	}
	fish_direction_average_length = sqrt(inner_prod(fish_direction_average, fish_direction_average));
	if (fish_direction_average_length < 1)	//the fish is taking a U-turn. 可以根据实际的鱼的情况确定该条件
	{
		for (int t = 1; t <= predict_length; t++)	//if the fish is taking a U-turn, we give up predicting the fish trajectory, and just assume that the fish stays static.
		{
			predicted_fish_tr.push_back(fish_tr_history.back());
		}
		
		/*shift yolk to head*/
		it2 = fish_direction_history.end() - 1;
		for (it = predicted_fish_tr.begin(); it != predicted_fish_tr.end(); it++)
		{
			it->x += it2->x*shift_head2yolk;
			it->y += it2->y*shift_head2yolk;
		}
		
		return;
	}
	else
	{
		fish_direction_average.x = fish_direction_average.x / fish_direction_average_length;
		fish_direction_average.y = fish_direction_average.y / fish_direction_average_length;
	}

	Point2d fish_position_average(0, 0);	//average fish position
	for (it = fish_tr_history.begin(); it != fish_tr_history.end(); it++)
	{
		fish_position_average.x += it->x;
		fish_position_average.y += it->y;
	}
	fish_position_average.x = fish_position_average.x / fish_tr_history.size();
	fish_position_average.y = fish_position_average.y / fish_tr_history.size();

	double fish_velocity_average_proj;	//average fish velocity projected to average fish heading direction(assuming that the fish move along the average heading direction)
	Point2d fish_velocity_average(0, 0);	//average fish velocity
	for (it = fish_tr_history.begin(); it != fish_tr_history.end() - 1; it++)
	{
		fish_velocity_average.x += (it + 1)->x - it->x;
		fish_velocity_average.y += (it + 1)->y - it->y;
	}
	fish_velocity_average.x = fish_velocity_average.x / (fish_tr_history.size() - 1);
	fish_velocity_average.y = fish_velocity_average.y / (fish_tr_history.size() - 1);
	fish_velocity_average_proj = inner_prod(fish_velocity_average, fish_direction_average);
	if (fish_velocity_average_proj < 0) fish_velocity_average_proj = 0;

	Point2d fish_position_proj;	//present fish position projected to the line with average fish heading direction and through the average fish position
	fish_position_proj.x = fish_tr_history.back().x - fish_position_average.x;
	fish_position_proj.y = fish_tr_history.back().y - fish_position_average.y;
	double fish_position_proj_length = inner_prod(fish_position_proj, fish_direction_average);
	fish_position_proj.x = fish_direction_average.x*fish_position_proj_length;
	fish_position_proj.y = fish_direction_average.y*fish_position_proj_length;
	fish_position_proj = fish_position_proj + fish_position_average;

	for (int t = 1; t <= predict_length; t++)
	{
		Point2d predicted_fish_pos_t;
		predicted_fish_pos_t.x = fish_position_proj.x + t*fish_velocity_average_proj*fish_direction_average.x;
		predicted_fish_pos_t.y = fish_position_proj.y + t*fish_velocity_average_proj*fish_direction_average.y;
		predicted_fish_tr.push_back(predicted_fish_pos_t);
	}



//to be deleted
	//display some results
	 //ofstream output_fish_v_predicted, output_fish_tr_predicted;
	 //output_fish_v_predicted.open("E:\\output_fish_v_predicted.txt", ios::out|ios::app);
	 //output_fish_tr_predicted.open("E:\\output_fish_tr_predicted.txt", ios::out|ios::app);
	 //output_fish_v_predicted << fish_velocity_average_proj*fish_direction_average.x << ", " << fish_velocity_average_proj*fish_direction_average.y << endl;
	 //output_fish_tr_predicted << 
	 //	predicted_fish_tr[0].x << ", " << predicted_fish_tr[0].y << 
	 //	", " << fish_position_proj.x << ", " << fish_position_proj.y << 
	 //	", " << fish_direction_history.back().x << ", " << fish_direction_history.back().y << 
	 //	", " << fish_direction_average.x << ", " << fish_direction_average.y <<
	 //	", " << fish_position_average.x << ", " << fish_position_average.y <<
	 //	", " << predicted_fish_tr[1].x << ", " << predicted_fish_tr[1].y << ", ";



	/*shift yolk to head*/
	it2 = fish_direction_history.end() - 1;
	for (it = predicted_fish_tr.begin(); it != predicted_fish_tr.end(); it++)
	{
		it->x += it2->x*shift_head2yolk;
		it->y += it2->y*shift_head2yolk;
	}




// //to be deleted
 	//output_fish_tr_predicted << predicted_fish_tr[1].x << ", " << predicted_fish_tr[1].y << endl;
 	//output_fish_v_predicted.close();
 	//output_fish_tr_predicted.close();




}

double inner_prod(Point2d v1, Point2d v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

Point2d trans_c2s(double scale_x, double scale_y, double theta, Point2d p, Point2d stage_position)
{
	double x = -sin(theta)*p.y + cos(theta)*p.x;
	double y =  sin(theta)*p.x + cos(theta)*p.y;
	x = scale_x*x - stage_position.x;
	y = scale_y*y - stage_position.y;
	return Point2d(x, y);
}
