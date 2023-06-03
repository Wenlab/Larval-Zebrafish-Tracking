#pragma once
/*
author: tgd
time: 2022-04-18 packing
info: the hpp of ConsolCoorRead.cpp
*/

#ifndef CONSOL_COOR_READ_
#define CONSOL_COOR_READ_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "F:/Program Files/Advantech/DAQNavi/Examples/ANSI_C/inc/compatibility.h"
#include "F:/Program Files/Advantech/DAQNavi/inc/bdaqctrl.h"
using namespace Automation::BDaq;


class ConsolCoorRead
{
private:
	;
public:
	int32 coordata[2];
	UdCounterCtrl* udCounterCtrl = nullptr;

	// initialize the taskHandle of the console coordinate read task.
	void ConCoorInitialize();

	// read the current console coordinate into coordata.
	void ConCoorRead();

	// stop the taskHandle.
	void ConsolCoorStop();

	// end the taskHandle.
	void ConCoorEnd();
};

#endif /* CONSOL_COOR_READ_ */