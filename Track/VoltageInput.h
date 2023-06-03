// author: tgd
// time: 2022-04-18  packing
// info: the hpp of voltage_input.cpp

#pragma once

#ifndef VALTAGE_INPUT_
#define VALTAGE_INPUT_

#include "NIDAQmx.h"


class VoltageInput
{
private:
	;
public:

	// the input voltage number
	float64 _voltage_x[1];
	float64 _voltage_y[1];
	TaskHandle	_taskHandle1 = 0;
	TaskHandle	_taskHandle2 = 0;


	// Initialize the taskHandle.
	void volInitialize();

	// input voltage method, voltage_x is the voltage which will be input to control the x, and voltage_y to control y.
	void volInput(float64 voltage_x, float64 voltage_y);

	// end the taskHandle.
	void volEnd();
};

#endif /* VALTAGE_INPUT_ */