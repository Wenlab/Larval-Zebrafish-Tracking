/*
author: tgd
time: 2022-04-20
function:

There are three steps to use the function, which could read the coordinates of current console location.

using method: when used the ConCoorRead() function, the coordinates of current consol location would be stored in coordata,
which is a array including x and y.

first: use ConCoorInitialize() to initialize the taskHandle.
second: use ConCoorRead() to read the console location.
third: use ConsolCoorStop() to stop the console taskHandle.
fourth: use ConCoorEnd() to end the taskHandle.

note: if you want to control the console for long time, don't end the taskHandle every time,
            just end the console taskHandle in the end is ok!
*/

//#include <stdlib.h>
//#include <stdio.h>
//#include "F:/Program Files/Advantech/DAQNavi/Examples/ANSI_C/inc/compatibility.h"
//#include "F:/Program Files/Advantech/DAQNavi/inc/bdaqctrl.h"
#include "ConsolCoorRead.h"

using namespace Automation::BDaq;

void ConsolCoorRead::ConCoorInitialize()
{
	#define     deviceDescription L"PCI-1784,BID#1"
	const wchar_t* profilePath = L"E:/Program Files/Advantech/DAQNavi/Examples/profile/PCI-1784.xml";
	int32       channelStart = 0;
	int32       channelCount = 2;

    ErrorCode ret = Success;
    // Step 1: Create a 'UdCounterCtrl' for UpDown Counter function.
    udCounterCtrl = UdCounterCtrl::Create();

    do 
    {
        // Step 2: Select a device by device number or device description and specify the access mode.
    // in this example we use ModeWrite mode so that we can fully control the device, including configuring, sampling, etc.
        DeviceInformation devInfo(deviceDescription);
        ret = udCounterCtrl->setSelectedDevice(devInfo);
        CHK_RESULT(ret);
        ret = udCounterCtrl->LoadProfile(profilePath);//Loads a profile to initialize the device.
        CHK_RESULT(ret);

        // Step 3: Set necessary parameters
        ret = udCounterCtrl->setChannelStart(channelStart);
        CHK_RESULT(ret);
        ret = udCounterCtrl->setChannelCount(channelCount);
        CHK_RESULT(ret);

        // Step 4: Set counting type for UpDown Counter
          /******************************************************************************************************************/
          /*In this example, we use the PCIE-1784 and set 'PulseDirection' as the default CountingType.The details see manual.
          /******************************************************************************************************************/
        Array<UdChannel>* udChannel = udCounterCtrl->getChannels();
        for (int i = channelStart; i < channelStart + channelCount; i++)
        {
            ret = udChannel->getItem(i).setCountingType(AbPhaseX4);
            CHK_RESULT(ret);
        }
        CHK_RESULT(ret);

        // Step 5: Start UpDown Counter 
        ret = udCounterCtrl->setEnabled(true);
        CHK_RESULT(ret);
    } while (false);
    
}

void ConsolCoorRead::ConCoorRead()
{
    int32 value = 2;
    udCounterCtrl->Read(value, coordata);
    
}

void ConsolCoorRead::ConsolCoorStop()
{
	ErrorCode ret = Success;
    do
    {
        // Step 7: stop UpDown Counter
        ret = udCounterCtrl->setEnabled(false);
        CHK_RESULT(ret);
    } while (false);
    
}

void ConsolCoorRead::ConCoorEnd()
{
	ErrorCode ret = Success;
    // Step 8: Close device and release any allocated resource.
    udCounterCtrl->Dispose();

    // If something wrong in this execution, print the error code on screen for tracking.
    if (BioFailed(ret))
    {
        wchar_t enumString[256];
        AdxEnumToString(L"ErrorCode", (int32)ret, 256, enumString);
        printf("Some error occurred. And the last error code is 0x%X. [%ls]\n", ret, enumString);
        //waitAnyKey();
    }

}

