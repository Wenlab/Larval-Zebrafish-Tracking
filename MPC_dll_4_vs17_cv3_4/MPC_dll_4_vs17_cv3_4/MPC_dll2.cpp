#include "stdafx.h"
#include "MPC_dll.h"

MPC_DLL_API int MPC_dll(int m, int predict_length, int fish_history_length, 
	double gammaX, double gammaY, double max_command, int shift_head2yolk,
	double scale_x, double scale_y, double theta, double scale_x2, double scale_y2, Point *dst_fish_position,
	double *command_history_array, int *position_history_array, int *fish_tr_history_c_array, double *fish_direction_history_c_array, 
	Point2d *c0)
{
	///*to be deleted*/
	////elapsed time
	//LARGE_INTEGER timeStart;    //��ʼʱ��  
 //   LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;      //����ʱ��  
 //   LARGE_INTEGER frequency;    //��ʱ��Ƶ��  
 //   QueryPerformanceFrequency(&frequency);  
 //   double quadpart = (double)frequency.QuadPart;//��ʱ��Ƶ��  
 //   QueryPerformanceCounter(&timeStart);  
	////display input
	//ofstream output;
	//output.open("E:\\display_input.txt", ios::out|ios::app);
	//output << "m: " << m << endl;
	//output << "predict_length: " << predict_length << endl;
	//output << "fish_history_length: " << fish_history_length << endl;
	//output << "gammaX: " << gammaX << endl;
	//output << "gammaY: " << gammaY << endl;
	//output << "max_command: " << max_command << endl;
	//output << "scale_x: " << scale_x << endl;
	//output << "scale_y: " << scale_y << endl;
	//output << "theta: " << theta << endl;
	//output << "scale_x2: " << scale_x2 << endl;
	//output << "scale_y2: " << scale_y2 << endl;
	//output << "dst_fish_position: " << dst_fish_position->x << ", " << dst_fish_position->y << endl;
	//output << "----------------------" << endl;
	//output << "command_history_array: " << endl;
	//for (int i=0;i<=m;i++)
	//{
	//	output << command_history_array[2*i] << ", " << command_history_array[2*i+1] << endl;
	//}
	//output << "----------------------" << endl;
	//output << "position_history_array: " << endl;
	//for (int i=0;i<=fish_history_length;i++)
	//{
	//	output << position_history_array[2*i] << ", " << position_history_array[2*i+1] << endl;
	//}
	//output << "----------------------" << endl;
	//output << "fish_tr_history_c_array: " << endl;
	//for (int i=0;i<=fish_history_length;i++)
	//{
	//	output << fish_tr_history_c_array[2*i] << ", " << fish_tr_history_c_array[2*i+1] << endl;
	//}
	//output << "----------------------" << endl;
	//output << "fish_direction_history_c_array: " << endl;
	//for (int i=0;i<=fish_history_length;i++)
	//{
	//	output << fish_direction_history_c_array[2*i] << ", " << fish_direction_history_c_array[2*i+1] << endl;
	//}
	//output << "----------------------" << endl;
	//display some results
	ofstream output_command, output_stage_position, output_fish_tr, output_fish_direction;
	output_command.open("E:\\output_command.txt", ios::out|ios::app);
	output_stage_position.open("E:\\output_stage_position.txt", ios::out|ios::app);
	output_fish_tr.open("E:\\output_fish_tr.txt", ios::out|ios::app);
	output_fish_direction.open("E:\\output_fish_direction.txt", ios::out|ios::app);
	output_command << command_history_array[2*m] << ", " << command_history_array[2*m+1] << endl;
	output_stage_position << position_history_array[2*fish_history_length] << ", " << position_history_array[2*fish_history_length+1] << endl;
	output_fish_tr << fish_tr_history_c_array[2*fish_history_length] << ", " << fish_tr_history_c_array[2*fish_history_length+1] << endl;
	output_fish_direction << fish_direction_history_c_array[2*fish_history_length] << ", " << fish_direction_history_c_array[2*fish_history_length+1] << endl;
	output_command.close();
	output_stage_position.close();
	output_fish_tr.close();
	output_fish_direction.close();


	//int m = 30;	//length of response kernel
	//int predict_length = 7;
	//int fish_history_length = 6;
	//double gammaX = 0.05;	//coefficient of L2 penalty to the planned future acceleration vector
	//double gammaY = 0.05;
	//double max_command = 10;

	vector<Point2d> H;	//position response kernel (mm)
	/*input H*/
	double Hx_array[300] = {0.000000,0.015800,0.051219,0.053661,0.048107,0.047164,0.047589,0.048001,0.047816,0.047107,0.046906,0.047444,0.047579,0.047285,0.047123,0.047407,0.047690,0.047388,0.047650,0.048048,0.047940,0.047781,0.048194,0.048358,0.048154,0.047950,0.048041,0.048188,0.048278,0.048125,0.048409};
	double Hy_array[300] = {0.000000,-0.005314,-0.032008,-0.048707,-0.052811,-0.052464,-0.051611,-0.050488,-0.050258,-0.049560,-0.049196,-0.048631,-0.048570,-0.048116,-0.048169,-0.048022,-0.048160,-0.048076,-0.048407,-0.048621,-0.048865,-0.048958,-0.049050,-0.048897,-0.048666,-0.048403,-0.048120,-0.048008,-0.047867,-0.047766,-0.047868};
	H.clear();
	for (int i = 0; i <= m; i++)
	{
		H.push_back(Point2d(Hx_array[i], Hy_array[i]));
	}

	vector<Point2d> command_history;	//commands t<0
	/*input command_history*/
	command_history.clear();
	for (int i = 0; i <= m; i++)
	{
		//command_history.push_back(Point2d(0, 0));
		command_history.push_back(Point2d(command_history_array[2 * i], command_history_array[2 * i + 1]));
	}

	vector<Point2d> position_history;	//stage position t<=0
	/*input position history*/
	position_history.clear();
	for (int i = 0; i <= fish_history_length; i++)
	{
		//position_history.push_back(Point2d(20000, 12800));
		position_history.push_back(Point2d(position_history_array[2 * i], position_history_array[2 * i + 1]));
	}
	/*transform stage position unit from count to mm*/
	//double scale_x2 = 1.0 / 10000;	//(mm/count)
	//double scale_y2 = 1.0 / 12800;	//(mm/count)
	vector<Point2d>::iterator it, it2;
	for (it = position_history.begin(); it != position_history.end(); it++)
	{
		it->x = it->x*scale_x2;
		it->y = it->y*scale_y2;
	}

	vector<Point2d> fish_tr_history_c;
	vector<Point2d> fish_direction_history_c;
	/*input fish_tr_history_c and fish_direction_history_c*/
	fish_tr_history_c.clear();
	for (int i = 0; i <= fish_history_length; i++)
	{
		//fish_tr_history_c.push_back(Point2d(10, 10));
		fish_tr_history_c.push_back(Point2d(fish_tr_history_c_array[2 * i], fish_tr_history_c_array[2 * i + 1]));
	}
	fish_direction_history_c.clear();
	for (int i = 0; i <= fish_history_length; i++)
	{
		//fish_direction_history_c.push_back(Point2d(0.707, 0.707));
		fish_direction_history_c.push_back(Point2d(fish_direction_history_c_array[2 * i], fish_direction_history_c_array[2 * i + 1]));
	}

	vector<Point2d> fish_tr_history;
	vector<Point2d> fish_direction_history;
	/*transform fish position and heading direction from camera coordinate to stage coordinate*/
	for (it = fish_tr_history_c.begin(); it != fish_tr_history_c.end(); it++)
	{
		it->x = it->x - dst_fish_position->x;	//calculate the difference between fish position and the destination
		it->y = it->y - dst_fish_position->y;
	}
	//double scale_x, scale_y, theta;
	//scale_x = 20.0 / 750;	//(mm/pixel)
	//scale_y = 20.0 / 750;	//(mm/pixel)
	//theta = atan(0.02);
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
	*c0 = MPC(m, predict_length, H, gammaX, gammaY, shift_head2yolk_f, command_history, position_history, fish_tr_history, fish_direction_history);
	if (c0->x > max_command)
		c0->x = max_command;
	if (c0->x < -max_command)
		c0->x = -max_command;
	if (c0->y > max_command)
		c0->y = max_command;
	if (c0->y < -max_command)
		c0->y = -max_command;
	//cout << "c0: " << c0->x << "\t" << c0->y << endl;

	///*to be deleted*/
	//output << "c0: " << c0->x << ", " << c0->y << endl;
	//QueryPerformanceCounter(&timeEnd);
	//double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
	//output << "elapsed time: " << elapsed << endl;
	//output << "======================" << endl;
	//output.close();

}