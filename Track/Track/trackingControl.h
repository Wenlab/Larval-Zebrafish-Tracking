#pragma once
#include <string.h>
#include <iostream>
#include<sstream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Double_Window.H>

#include <cstdlib>                   //for exit(0)
#include <string.h>
#include <iostream>
#include<sstream>
#include <functional> // fucntion/bind

#define panel_size 300
#define maxVoltage 2

struct trackingParams
{
	float command_history_length;
	float predict_length;
	float fish_history_length;
	float gammaX;
	float gammaY;
	float max_command;
	float max_shift_head2yolk;
	float scale_x;
	float scale_y;
	float theta;
	float scale_x2;
	float scale_y2;
	float dst_fish_position_x;
	float dst_fish_position_y;
	
	bool trackingOn;
	bool flag_cb;//as a trigger, true if we click one of the buttons: TrackingOff, TrackingOn, Quit
	char action;//which button: P->trackingOff, C->trackingOn, X->Quit

	float voltage_x;
	float voltage_y;

	trackingParams()
	{
		this->command_history_length = 30.0;
		this->predict_length = 6.0;
		this->fish_history_length = 4.0;
		this->gammaX = 0.03;
		this->gammaY = 0.03;
		this->max_command = 10.0;
		this->max_shift_head2yolk = 29.0;
		this->scale_x = 30.0;
		this->scale_y = 30.0;
		this->theta = -0.043;
		this->scale_x2 = 10000.0;
		this->scale_y2 = 12800.0;
		this->dst_fish_position_x = 160;
		this->dst_fish_position_y = 160;

		this->trackingOn = false;
		this->flag_cb = false;
		this->action = 'P';

		this->voltage_x = 0.0;
		this->voltage_y = 0.0;
	}
}; 
//将通过鼠标获取的voltage的值传入params结构体
void getVoltages(trackingParams* params, float voltage_x, float voltage_y);
/*****************************************************************************/
/* This class provides a view to copy the offscreen surface to */
class voltageControlPanel : public Fl_Box
{

public:
	voltageControlPanel(int x, int y, int w, int h);
	~voltageControlPanel();
	void draw();
	int handle(int event);
};

void make_window(trackingParams* params);
void apply_cb(Fl_Widget* o, void *data);  //function prototypes
void ifTracking_cb(Fl_Widget* o, void *data);
void close_cb(Fl_Widget* o, void*data);


