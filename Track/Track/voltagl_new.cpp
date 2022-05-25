// author: tgd
// time: 2022-04-18 packing
// function : 
/*
There are three steps to use the function, which could control the console move.

using method: only need to set the voltage_x and voltage_y.

first : use volInterliaze() to interlize the taskHandle.
second : use volTnput(voltage_x, voltage_y) to control the console move.
third: use volEnd() to end the taskHandle.

note: if you want to control the console for long time, don't end the taskHandle every time,
			just end the console taskHandle in the end is ok!
*/

#include <stdio.h>
#include "NIDAQmx.h"
#include "VoltageInput.h"


#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

void VoltageInput::volInitialize()
{
	int			error = 0;
	char		errBuff[2048] = { '\0' };

	/*********************************************/
// DAQmx Configure Code
/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("", &_taskHandle1));
	DAQmxErrChk(DAQmxCreateTask("", &_taskHandle2));

	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandle1, "Dev1/ao0", "", -10.0, 10.0, DAQmx_Val_Volts, ""));
	DAQmxErrChk(DAQmxCreateAOVoltageChan(_taskHandle2, "Dev1/ao1", "", -10.0, 10.0, DAQmx_Val_Volts, ""));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk(DAQmxStartTask(_taskHandle1));
	DAQmxErrChk(DAQmxStartTask(_taskHandle2));

Error:
	if (DAQmxFailed(error))
	{
		printf("Voltage initialize time out!");
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	}

}

void VoltageInput::volInput(float64 voltage_x, float64 voltage_y)
{
	int			error = 0;
	char		errBuff[2048] = { '\0' };
	_voltage_x[0] = voltage_x;
	_voltage_y[0] = voltage_y;
	int32 written_1;
	int32 written_2;

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandle1, 1, 1, 10.0, DAQmx_Val_GroupByChannel, _voltage_x, &written_1, NULL));

	DAQmxErrChk(DAQmxWriteAnalogF64(_taskHandle2, 1, 1, 10.0, DAQmx_Val_GroupByChannel, _voltage_y, &written_2, NULL));


Error:
	if (DAQmxFailed(error))
	{
		printf("Voltage initialize time out!");
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	}

}

void VoltageInput::volEnd()
{
	int			error = 0;
	char		errBuff[2048] = { '\0' };

	if (_taskHandle1 != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(_taskHandle1);
		DAQmxClearTask(_taskHandle1);
	}
	if (_taskHandle2 != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		printf("have worked!\n");
		DAQmxStopTask(_taskHandle2);
		DAQmxClearTask(_taskHandle2);
	}
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit\n");
}