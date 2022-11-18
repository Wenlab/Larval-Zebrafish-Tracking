
#include"trackingControl.h"

using namespace std;

static  Fl_Double_Window *win = 0;  // the main app window
static  Fl_Offscreen offscreen_buffer = 0;  // the offscreen surface
bool g_manualControl = true;

float g_voltage_x = 0;
float g_voltage_y = 0;

trackingParams* g_params;   //global

/* Constructor */
voltageControlPanel::voltageControlPanel(int x, int y, int w, int h) : Fl_Box(x, y, w, h)
{
	cout << "create control panel successful" << endl;
} // Constructor
voltageControlPanel::~voltageControlPanel()
{
}
/*****************************************************************************/
void voltageControlPanel::draw()
{
	if (offscreen_buffer)
	{ // offscreen exists
		 //blit the required view from the offscreen onto the box
		fl_copy_offscreen(x(), y(), w(), h(), offscreen_buffer, 0, 0);
	}
	else
	{ // create the offscreen
		win->make_current(); //ensures suitable graphic context
		offscreen_buffer = fl_create_offscreen(w(), h());
		if (!offscreen_buffer)
		{
			fprintf(stderr, "Failed buffer creation");
			exit(1);
		}
		fl_begin_offscreen(offscreen_buffer); /* Open the offscreen context */
		fl_color(FL_WHITE);
		fl_rectf(0, 0, w(), h());
		fl_end_offscreen(); /* close the offscreen context */
		/* init screen with offscreen buffer */
		fl_copy_offscreen(x(), y(), w(), h(), offscreen_buffer, 0, 0);
	}
} // draw method
/*****************************************************************************/
int voltageControlPanel::handle(int event)
{
	static char labeltext[100];
	int button, x, y;
	int retvalue = 0;
	static int x_old, y_old;
	static int push1st = 0;
	if (!offscreen_buffer) return 1;
	retvalue = Fl_Box::handle(event);


	//float voltage_x = 0.0;
	//float voltage_y = 0.0;
	float x_bias = 400;
	float y_bias = 50;
	if (1)  //手动控制（改为->无论哪种模式，都可以手动控制）
	{
		switch (event)
		{
		case FL_PUSH:
		case FL_DRAG:
			button = Fl::event_button();
			x = Fl::event_x();
			y = Fl::event_y();
			g_voltage_x = (float(x) - x_bias - panel_size / 2) / panel_size * maxVoltage * 2;
			g_voltage_y = (float(y) - y_bias - panel_size / 2) / panel_size * maxVoltage * 2;
			//cout << g_voltage_y << "   " << g_voltage_x << endl;
			g_params->voltage_x = -g_voltage_x;  //将X轴反过来
			g_params->voltage_y = g_voltage_y;

			break;

			//cout << g_params->command_history_length << endl;
		default:
			g_voltage_x = 0;
			g_voltage_y = 0;
			//cout << g_voltage_y << "   " << g_voltage_x << endl;
			g_params->voltage_x = g_voltage_x;
			g_params->voltage_y = g_voltage_y;
		
		};
		switch (button)
		{
		case 1: // Left button
			retvalue = 1;
			break;
		case 3: // Right button
			retvalue = 1;
			break;
		}
		//switch (event)
		//{
		//case FL_PUSH:
		//	if (push1st == 0)
		//	{
		//		push1st = 1;
		//		break;
		//	}
		//	else
		//	{
		//		push1st = 0;
		//	}
		//case FL_DRAG:
		//{
		//	push1st = 0; //clear if dragging
		//}
		//break;
		//default:
		//	//redraw();
		//	break;
		//}
	}
	return retvalue;
} // handle
/*****************************************************************************/

void getVoltages(trackingParams* params, float voltage_x, float voltage_y)
{
	params->voltage_x = voltage_x;
	params->voltage_y = voltage_y;

	return;
}

void make_window(trackingParams* params)
{
	g_params = params;
	//fl_register_images();
	win = new Fl_Double_Window(800, 500, "Tracking Control");
	//Fl_Window* win = new Fl_Double_Window(800, 500, "Tracking Control");
	win->begin();
	//Fl_Button* initialize = new Fl_Button(10, 450, 70, 30, "&I&nitialize"); //child 0    : 1st widget
	Fl_Button*  apply = new Fl_Button(40, 460, 70, 30, "A&pply"); //child 0   : 2nd widget
	Fl_Button* ifTracking = new Fl_Button(130, 460, 100, 30, "&TrackingOn"); //child 1    : 3rd widget


	Fl_Input* command_history_length = new Fl_Input(200, 10, 120, 20, "command_history_length");  //child 2
	command_history_length->value(to_string(params->command_history_length).c_str());	//基本不变

	Fl_Input* predict_length = new Fl_Input(200, 40, 120, 20, "predict_length");  //child 3
	predict_length->value(to_string(params->predict_length).c_str());	//10以内

	Fl_Input* fish_history_length = new Fl_Input(200, 70, 120, 20, "fish_history_length");  //child 4
	fish_history_length->value(to_string(params->fish_history_length).c_str());	//10以内

	Fl_Input* gammaX = new Fl_Input(200, 100, 120, 20, "gammaX");   //child 5
	gammaX->value(to_string(params->gammaX).c_str());	//0.015~0.05, step 0.005或0.001
	Fl_Input* gammaY = new Fl_Input(200, 130, 120, 20, "gammaY");  //child 6
	gammaY->value(to_string(params->gammaY).c_str());	//0.015~0.05, step 0.005或0.001

	Fl_Input* max_command = new Fl_Input(200, 160, 120, 20, "max_command");  //child 7
	max_command->value(to_string(params->max_command).c_str()); 	// 基本不变

	Fl_Input* max_shift_head2yolk = new Fl_Input(200, 190, 120, 20, "max_shift_head2yolk");  //child 8
	max_shift_head2yolk->value(to_string(params->max_shift_head2yolk).c_str());  //50以内

	Fl_Input* scale_x = new Fl_Input(200, 220, 120, 20, "scale_x");  //child 9
	scale_x->value(to_string(params->scale_x).c_str());  //分母大概在一个数量级内变化
	Fl_Input* scale_y = new Fl_Input(200, 250, 120, 20, "scale_y");   //child 10
	scale_y->value(to_string(params->scale_y).c_str());  //分母大概在一个数量级内变化

	Fl_Input* theta = new Fl_Input(200, 280, 120, 20, "theta");     //child 11
	theta->value(to_string(params->theta).c_str());   //+-0.5，两位有效数字

	Fl_Input* scale_x2 = new Fl_Input(200, 310, 120, 20, "scale_x2");   //child 12
	scale_x2->value(to_string(params->scale_x2).c_str());   //基本不变
	Fl_Input* scale_y2 = new Fl_Input(200, 340, 120, 20, "scale_y2");   //child 13
	scale_y2->value(to_string(params->scale_y2).c_str());   //基本不变

	Fl_Input* dst_fish_position_x = new Fl_Input(200, 370, 120, 20, "dst_fish_position_x");  //child 14
	dst_fish_position_x->value(to_string(params->dst_fish_position_x).c_str());  //0~360
	Fl_Input* dst_fish_position_y = new Fl_Input(200, 400, 120, 20, "dst_fish_position_y");  //child 15
	dst_fish_position_y->value(to_string(params->dst_fish_position_y).c_str());  //0~360

	Fl_Button* close = new Fl_Button(260, 460, 70, 30, "&Quit"); //child 16   

	Fl_Input* shift_head = new Fl_Input(200, 430, 120, 20, "shift_head");  //child 17
	shift_head->value(to_string(params->shift_head).c_str());  //

	static voltageControlPanel *os_box = new voltageControlPanel(400, 50, panel_size, panel_size);


	win->end();
	win->resizable(os_box);
	apply->callback(apply_cb,params);
	ifTracking->callback(ifTracking_cb,params);
	close->callback(close_cb, params);
	win->show();

	return;
}


void apply_cb(Fl_Widget* o, void *data)
{
	trackingParams* params = (trackingParams*)data;
	g_params = params;
	params->flag_cb = true;

	Fl_Button* b = (Fl_Button*)o;

	Fl_Input* command_history_length = (Fl_Input*)b->parent()->child(2);
	params->command_history_length = atof(command_history_length->value());

	Fl_Input* predict_length = (Fl_Input*)b->parent()->child(3);
	params->predict_length = atof(predict_length->value());

	Fl_Input* fish_history_length = (Fl_Input*)b->parent()->child(4);
	params->fish_history_length = atof(fish_history_length->value());

	Fl_Input* gammaX = (Fl_Input*)b->parent()->child(5);
	params->gammaX = atof(gammaX->value());

	Fl_Input* gammaY = (Fl_Input*)b->parent()->child(6);
	params->gammaY = atof(gammaY->value());

	Fl_Input* max_command = (Fl_Input*)b->parent()->child(7);
	params->max_command = atof(max_command->value());

	Fl_Input* max_shift_head2yolk = (Fl_Input*)b->parent()->child(8);
	params->max_shift_head2yolk = atof(max_shift_head2yolk->value());

	Fl_Input* scale_x = (Fl_Input*)b->parent()->child(9);
	params->scale_x = atof(scale_x->value());

	Fl_Input* scale_y = (Fl_Input*)b->parent()->child(10);
	params->scale_y = atof(scale_y->value());

	Fl_Input* theta = (Fl_Input*)b->parent()->child(11);
	params->theta = atof(theta->value());

	Fl_Input* scale_x2 = (Fl_Input*)b->parent()->child(12);
	params->scale_x2 = atof(scale_x2->value());

	Fl_Input* scale_y2 = (Fl_Input*)b->parent()->child(13);
	params->scale_y2 = atof(scale_y2->value());

	Fl_Input* dst_fish_position_x = (Fl_Input*)b->parent()->child(14);
	params->dst_fish_position_x = atof(dst_fish_position_x->value());

	Fl_Input* dst_fish_position_y = (Fl_Input*)b->parent()->child(15);
	params->dst_fish_position_y = atof(dst_fish_position_y->value());

	Fl_Input* shift_head = (Fl_Input*)b->parent()->child(17);
	params->shift_head = atof(shift_head->value());

	//std::cout << params->voltage_x << std::endl;
	//std::cout << params->command_history_length << std::endl;
	//std::cout << params->predict_length << std::endl;
	//std::cout << params->fish_history_length << std::endl;
	//std::cout << params->gammaX<< std::endl;
	//std::cout << params->gammaY << std::endl;
	//std::cout << params->max_command << std::endl;
	//std::cout << params->max_shift_head2yolk << std::endl;
	//std::cout << params->scale_x << std::endl;
	//std::cout << params->scale_y << std::endl;
	//std::cout << params->theta << std::endl;
	//std::cout << params->scale_x2 << std::endl;
	//std::cout << params->scale_y2 << std::endl;
	//std::cout << params->dst_fish_position_x << std::endl;
	//std::cout << params->dst_fish_position_y << std::endl;

	//cout << "apply_cb g_params voltage: " << g_params->voltage_x << ", " << g_params->voltage_y << endl;
	//cout << "apply_cb params voltage: " << params->voltage_x << ", " << params->voltage_y << endl;
}


void ifTracking_cb(Fl_Widget* o, void *data)
{
	trackingParams* params = (trackingParams*)data;
	g_params = params;
	Fl_Button* b = (Fl_Button*)o;
	if (false == params->trackingOn)
	{
		char newcap[] = "TrackingOff";
		b->copy_label(newcap);
		params->flag_cb = true;
		params->action = 'C';//To ensure that action and flag_cb are updated simultaneously, maybe we should use some sychronization.
		params->trackingOn = true;
		g_manualControl = false;
	}
	else if (true == params->trackingOn)
	{
		char newcap[] = "TrackingOn";
		b->copy_label(newcap);
		params->flag_cb = true;
		params->action = 'P';//To ensure that action and flag_cb are updated simultaneously, maybe we should use some sychronization.
		params->trackingOn = false;
		g_manualControl = true;
	}
}




void close_cb(Fl_Widget* o, void*data)
{
	trackingParams* params = (trackingParams*)data;
	g_params = params;
	params->flag_cb = true;
	params->action = 'X';//To ensure that action and flag_cb are updated simultaneously, maybe we should use some sychronization.
	/*exit(0);*/
}
