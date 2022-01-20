// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FIND_FISH_POSITION_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FIND_FISH_POSITION_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.


#ifdef MPC_DLL_EXPORTS
#define MPC_DLL_API __declspec(dllexport)
#else
#define MPC_DLL_API __declspec(dllimport)
#endif

#include "MPC.h"

MPC_DLL_API int MPC_dll(int m, int predict_length, int fish_history_length, 
	double gammaX, double gammaY, double max_command, int shift_head2yolk,
	double scale_x, double scale_y, double theta, double scale_x2, double scale_y2, Point *dst_fish_position,
	double *command_history_array, int *position_history_array, int *fish_tr_history_c_array, double *fish_direction_history_c_array, 
	Point2d *c0);
