/***************************************************************************
*
* CirClassExample.cpp
*
* This application uses the CircularInterface class to acquire images
* into a circular buffer. This application is equivalent to the Circular.c
* application, the difference being the Circular.c application
* uses the Bi API and this application uses the CircularInterface class.
*
* There are several advantages to using the CircularInterface class
* that are demonstrated in this application.
* The advantages being:
*	1. Fewer method/function calls to accomplish sequence capture.
*	2. No need to call cleanup methods/functions.
*	3. Fewer method/function parameters.
*	4. Improved error handling by throwing exceptions.
*
* We encouraged you to compare this application to the Circular.c
* application to see the advantages for yourself.
*
* This example demonstrates the use of CircularInterface class.
*
* Copyright (C) 2004 by BitFlow, Inc.  All Rights Reserved.
*
***************************************************************************/



// note that: don't minmize the cmd window, ortherwise the process will be error!!!

#include "stdafx.h"
#include "CircClassExample.h"
#include "CircularInterface.h"
#include <conio.h>
#include "CiApi.h"
#include <math.h>
#include <windows.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
 
//tensorRT
#include"TRTruntime.h"

//opencv
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<opencv2/imgcodecs/imgcodecs.hpp>
#include<opencv2/imgproc.hpp>

//Consol Coordinate Read
#include "ConsolCoorRead.h"

//Voltage Input
#include "VoltageInput.h"

//MPC
#include "MPC_main.h"

using namespace cv;
using namespace std;

#define DEBUG_FLAG 0


// Threads
UINT WaitForBufferDone(LPVOID lpdwParam);
UINT CirErrorThread(LPVOID lpdwParam);
UINT TRTImageProcessThread(LPVOID lpdwParam);

MSG		Msg;
BFBOOL	endTest = FALSE;
int		hDspSrf = -1;	// handle to display surface 
BFBOOL display = FALSE; // the default of display is TRUE

CWinApp theApp;

int TRTflag=0;

using namespace std;
using namespace BufferAcquisition;

struct bitflowTRTStru 
{
	BFU32 frameNum;
	void* imageDataBuffer;
	TRTruntime trt;
	bitflowTRTStru()
	{
	}
};

//tgd
TRTruntime trt(1, 320, 320, 2);

BFU32 	rv;
BiCirHandle	cirHandle;
BFTickRec T0, T1;

//ConsolCoorRead
ConsolCoorRead consoleread;

//VoltageInput
VoltageInput voltage;
float64 voltage_x = 1.0;
float64 voltage_y = 1.0;

//Historical data
vector<Point2d> command_history;
vector<Point2d> position_history;
vector<Point2d> fish_tr_history_c;
vector<Point2d> fish_direction_history_c;

// parameters for MPC
int command_history_length = 30;
int predict_length = 6;
int fish_history_length = 4;
double gammaX = 0.03;
double gammaY = 0.03;
double max_command = 10.0;
int max_shift_head2yolk = 29;
double scale_x = 1.0 / 30.0;
double scale_y = 1.0 / 30.0;
double theta = atan(-0.043);
double scale_x2 = 1.0 / 10000.0;
double scale_y2 = 1.0 / 12800.0;
Point dst_fish_position = Point(160, 160);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(BFNULL), BFNULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: MFC initialization failed\n"));
		nRetCode = 1;
	}
	else
	{
		BFU32	boardType, boardNum, init, serNum;
		char	ch;
		BFU32	numBuffers = 50;
		BFU32	cirSetupOptions = 0;
		BFU32	errorMode = CirErStop;
		CWinThread* pErrorThread = NULL;
		CWinThread* pFrameDoneThread = NULL;
		CWinThread* pFrameIMGThread = NULL;

		// track class
		bitflowTRTStru bt;

		if (DoBrdOpenDialog(BOD_BRD_NUM_NON_FAMILY | BOD_HIDEJUSTOPEN, FF_BITFLOW_MODERN, &boardType, &boardNum,
			&init, &serNum))
		{
			return -1;
		}



		try
		{
			//初始化board
			cout << "Creating instance of circular interface." << endl;
			CircularInterface board(boardNum, numBuffers, errorMode,
				cirSetupOptions);


			// CHECK THE BIT DEPTH OF THE CAMERA
			cout << "bitType: " << board.getBrdInfo(BiCamInqBitsPerPix) << endl;

			//tensorRT
			cout << "load model....." << endl;
			TRTruntime trt(1, 320, 320, 2);
			trt.DeserializeModel("trackKeyPointModel_0421_unet_320.trt");
			trt.createInferenceContext();
			cout << "Deserialize TRT model Done." << endl;
			bt.trt = trt;
			bt.frameNum = -1;
			bt.imageDataBuffer = NULL;

			// Initialize the console coordinate reading task.
			consoleread.ConCoorInitialize();

			// Initialize the voltage input task.
			voltage.volInitialize();

			// clear the history
			command_history.clear();
			position_history.clear();
			fish_tr_history_c.clear();
			fish_direction_history_c.clear();

			// Initialize command_history with 0.
			for (int i = 0; i <= command_history_length; i++)
				command_history.push_back(Point2d(0, 0));

			// Create display surface to view sequence.	
			if (display)
			{
				if (!DispSurfCreate((PBFS32)&hDspSrf, board.getBrdInfo(BiCamInqXSize),
					board.getBrdInfo(BiCamInqYSize0),
					board.getBrdInfo(BiCamInqBitsPerPixDisplay), BFNULL))
				{
					cout << "Couldn't create display surface" << endl;
					return 1;
				}
			}




			pErrorThread = AfxBeginThread(CirErrorThread, &board, THREAD_PRIORITY_HIGHEST);
			if (pErrorThread == BFNULL)
				return 1;

			pFrameDoneThread = AfxBeginThread(WaitForBufferDone, &board, THREAD_PRIORITY_HIGHEST);
			if (pFrameDoneThread == BFNULL)
				return 1;

			pFrameIMGThread = AfxBeginThread(TRTImageProcessThread, &bt, THREAD_PRIORITY_HIGHEST);
			if (pFrameDoneThread == BFNULL)
				return 1;
			

			printf("\nPress G (as in Go) to start Acquisition ");
			printf("	Press S to Stop Acquisition \n");
			printf("Press P to Pause				Press C to Continue\n");
			printf("Press A to Abort\n");
			printf("Press X to exit test\n\n");

			while (!endTest)
			{
				// Wait here for a keyboard stroke
				while (!BFkbhit() && !endTest)
				{
					if (PeekMessage(&Msg, BFNULL, 0, 0, PM_REMOVE))
						DispatchMessage(&Msg);
					
					if (TRTflag == 1 && bt.frameNum != cirHandle.FrameCount)
					{
						bt.frameNum = cirHandle.FrameCount;
						bt.imageDataBuffer = cirHandle.pBufData;
						if (cirHandle.FrameCount % 10000 == 0)
							cout << "current num:" << bt.frameNum << endl;
					}
					
				}
				if (!endTest)
					ch = BFgetch();
				else
				{
					ch = 'X';
					TRTflag = 2;
				}

				cout << endl;

				switch (toupper(ch))
				{
				case 'G': // Start acquisition
					TRTflag = 1;
					board.cirControl(BISTART, BiAsync);
					cout << "Circular Acquisition Started." << endl;
					break;

				case 'P':// Pause acquisition
					TRTflag = 0;
					board.cirControl(BIPAUSE, BiAsync);
					cout << "Circular Acquisition Paused." << endl;
					break;

				case 'C':// Resume acquisition
					TRTflag = 1;
					board.cirControl(BIRESUME, BiAsync);
					cout << "Circular Acquisition Resumed." << endl;
					break;

				case 'S':// Stop acquisition
					TRTflag = 2;
					board.cirControl(BISTOP, BiAsync);
					break;

				case 'A':// Abort acquisition
					TRTflag = 2;
					board.cirControl(BIABORT, BiAsync);
					break;

				case 'X':// Exit application
					TRTflag = 2;
					if (board.getStartAcqFlag())
						board.cirControl(BIABORT, BiAsync);
					endTest = TRUE;
					break;

				default:
					cout << "Key not Recognized, Try Again" << endl;
					break;
				}

			}
			CString Str;
			Str.Format("\nCaptured %d Frames\nMissed %d Frames\n",
				board.getNumFramesCaptured(), board.getNumFramesMissed());
			cout << Str << endl;

			BFRC error = board.getCirError();
			while (error != BI_OK)
			{
				board.showError(error);
				error = board.getCirError();
			}

			cout << "\nPress Any Key to Continue." << endl;
			while (!BFkbhit())
			{
				/* needed for display window */
				if (PeekMessage(&Msg, BFNULL, 0, 0, PM_REMOVE))
					DispatchMessage(&Msg);
				else
					Sleep(0);
			}

			// absorb key stroke 
			if (BFkbhit()) BFgetch();

			// Close Display window
			if (display)
				DispSurfClose(hDspSrf);

			// No need to clean up resources, the destructor of the
			// CircularInterface class will handle clean up.

			// clean up
			consoleread.ConsolCoorStop();
			consoleread.ConCoorEnd();
			voltage.volInput(0, 0);
			voltage.volEnd();
		}
		catch (BFException e)
		{
			e.showErrorMsg();
			nRetCode = 1;
		}

		if (display)
		{
			if (DispSurfIsOpen(hDspSrf))
			{
				// Close Display window
				DispSurfClose(hDspSrf);
			}
		}

		// Wait for threads to end
		DWORD exitCode;
		while (GetExitCodeThread(pErrorThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

		while (GetExitCodeThread(pFrameDoneThread->m_hThread, &exitCode) &&
			exitCode == STILL_ACTIVE)
		{
			Sleep(10);
		}

	}

	return nRetCode;
}


UINT WaitForBufferDone(LPVOID lpdwParam)
{
	
	CircularInterface* board = (CircularInterface*)lpdwParam;
	try
	{
		// loop until cleanup is called
		rv = board->waitDoneFrame(INFINITE, &cirHandle);
		

		if (display)
			BFTick(&T0);

		while (rv != BI_CANCEL_CIR_FRAME_DONE)
		{
			// print buffer info if a valid buffer has been acquired
			if (rv == BI_CIR_STOPPED)
				cout << "Circular Acquisition Stopped." << endl;
			else if (rv == BI_CIR_ABORTED)
				cout << "Circular Acquisition Aborted." << endl;
			else if (rv == BI_ERROR_CIR_WAIT_TIMEOUT)
				cout << "BiSeqWaitDone has timed out." << endl;
			else if (rv == BI_ERROR_CIR_WAIT_FAILED)
				cout << "The wait in BiSeqWaitDone Failed." << endl;
			else if (rv == BI_ERROR_QEMPTY)
				cout << "The queue was empty." << endl;
			else
			{
				if (display)
				{
					// only update display every 30 msec
					if (BFTickDelta(&T0, BFTick(&T1)) > 50)
					{
						// This new function will format the image data for display,
						// then display the image. This reduces a lot of code. (See
						// Circular.c)
						if (!DispSurfFormatBlit(hDspSrf, cirHandle.pBufData,    //image data pointer
							board->getBrdInfo(BiCamInqBitsPerPix), BFDISP_FORMAT_NORMAL))
						{
							cout << "Could not update the display surface" << endl;
							return 1;
						}
						BFTick(&T0);

					}
				}

				/*
				BFU32 pnextnum = -1;
				PBFQNode currentNote = cirHandle.pNode;
				BFU32 pnum = currentNote->BufferNumber;
				cout << "start num: " << pnum << endl;
				cout << endl;
				int a = 0;
				while (a!=5)
				{
					BFU32 currentnum = currentNote->BufferNumber;
					cout << "current BufferNumber: " << currentnum << endl;
					cout << "status: " << currentNote->Status << endl;
					currentNote = currentNote->Next;
					pnextnum = currentNote->BufferNumber;
					cout << "next BufferNumber: " << pnextnum << endl;
					cout << endl;
					a++;
				}
				cout << "-----------------------------------" << endl;

				*/
				
				/*
				if (board->getNumFramesCaptured()%1000==0)
				{
					cout << "caputured image num: " << board->getNumFramesCaptured() << endl;
				}
				*/

				// single thread:

				/*
				//TOdO: add track code
				vector<cv::Point> outputVec;
				
				void* image = cirHandle.pBufData;

				trt.launchInference(image, outputVec);

				cout << "frame num:" << cirHandle.FrameCount << endl;
				*/


				// Mark the buffer as AVAILABLE after processing
				board->setBufferStatus(cirHandle, BIAVAILABLE);

				// output some stats
				//cout << setfill('0');
				
				//cout << "Buffer: " << setw(8) << cirHandle.BufferNumber << " ";
				//cout << "captured num: " << board->getNumFramesCaptured() << endl;
			}

			rv = board->waitDoneFrame(INFINITE, &cirHandle);
			//cout << "grab thread" << endl;
		}
	}
	catch (BFException e)
	{
		e.showErrorMsg();

		// End application if an error occurs
		endTest = TRUE;

		return 1;
	}

	return 0;
}

UINT CirErrorThread(LPVOID lpdwParam)
{
	CircularInterface* board = (CircularInterface*)lpdwParam;

	while (board->cirErrorWait() != BI_CIR_CLEANUP)
	{
		cout << "ErrorThread - Acquisition Error!!" << endl;
	}

	return 0;
}



UINT TRTImageProcessThread(LPVOID lpdwParam)
{
	bitflowTRTStru* bt = (bitflowTRTStru*)lpdwParam;;
	//CircularInterface* board = (CircularInterface*)lpdwParam;

	cout << "start TRT model process..." << endl;
	BFU32 frameCount = -2;
	BFU32 frameCount_processed = 0;

	Mat test(cv::Size(320, 320), CV_8UC1);
	Mat test2(cv::Size(320, 320), CV_32FC1);

	try
	{
		while (1)
		{
			 
			//if (bt->frameNum % 1000 == 0 && bt->frameNum!=0)
			if(bt->frameNum==1)
				cout << "frameCount: " << frameCount << endl;

			 //cout << TRTflag << endl;

			if  (TRTflag == 0) 
				continue;
			else if (TRTflag  == 1)
			{
				//
				// cout << "1:" << bt->frameNum << endl;
				if (frameCount != bt->frameNum && bt->imageDataBuffer != NULL)
				{
					// elapsed time
					LARGE_INTEGER timeStart;    //开始时间  
					LARGE_INTEGER timeEnd,timeEnd1,timeEnd2,timeEnd3,timeEnd4,timeEnd5,timeEnd6,timeEnd7;      //结束时间  
					LARGE_INTEGER frequency;    //计时器频率  
					QueryPerformanceFrequency(&frequency);  
					double quadpart = (double)frequency.QuadPart;//计时器频率  
					QueryPerformanceCounter(&timeStart);  

					// read the coordinates of the current console location.
					//cout << "read a pair of console coordinates ..." << endl;
					consoleread.ConCoorRead();
					//printf("\n channel %u Current UpDown count: %u\n", 0, consoleread.coordata[0]);
					//printf("\n channel %u Current UpDown count: %u\n", 1, consoleread.coordata[1]);
					if (position_history.empty())// If position_history is empty（这里应该改为长度不等于fish_history_length，并先clear，这样在长度变化时可以调节！！！）, initialize it with the data of the first frame.
						for (int i = 0; i <= fish_history_length; i++)
							position_history.push_back(Point2d(consoleread.coordata[0], -consoleread.coordata[1]));// Note the sign!!!
					else// Push back the present data, and pop the data at the beginning
					{
						position_history.push_back(Point2d(consoleread.coordata[0], -consoleread.coordata[1]));// Note the sign!!!
						vector<Point2d>::iterator it = position_history.begin();
						position_history.erase(it);
					}

					QueryPerformanceCounter(&timeEnd1);
					//cout << frameCount << "    " << bt->frameNum << endl;
					test.data = (uchar*) bt->imageDataBuffer;
					vector<cv::Point> outputVec;
					
					test.convertTo(test2, CV_32FC1);
					test2 = test2 / 255;
					QueryPerformanceCounter(&timeEnd2);
					bt->trt.launchInference(test2, outputVec);
					QueryPerformanceCounter(&timeEnd3);

					double elapsed_ConCoorRead = (timeEnd1.QuadPart - timeStart.QuadPart) / quadpart;
					double elapsed_convertion = (timeEnd2.QuadPart - timeEnd1.QuadPart) / quadpart;
					double elapsed_process = (timeEnd3.QuadPart - timeEnd2.QuadPart) / quadpart;

					//imwrite("D:/tgd/Track-TEST/testImg/"+cv::format("%.4d", frameCount) + ".jpg", test2);
					

					frameCount = bt->frameNum;
					//if (frameCount % 10 == 0)
					//	cout << "processed frame count: " << bt->frameNum <<endl;

					
					//save image to test img process
					if (DEBUG_FLAG)
					{
						Mat test3 = test.clone();
						for (int i = 0; i < outputVec.size(); i++)
						{
							cv::circle(test3, outputVec[i], 3, 128, 2);
						}
						imwrite("H:/track_in_c/testImg/" + cv::format("%.6d", frameCount) + ".jpg", test3);
					}

					Point2d fish_direction = Point2d(outputVec[0].x - outputVec[1].x, outputVec[0].y - outputVec[1].y);
					double shift_head2yolk = sqrt(fish_direction.x*fish_direction.x + fish_direction.y*fish_direction.y);
					if (shift_head2yolk > max_shift_head2yolk || shift_head2yolk < 3 || (outputVec[1].x == 0 && outputVec[1].y == 0 && outputVec[0].x == 0 && outputVec[0].y == 0))//fish detection error!!!
					{
						cout << "fish detection error!" << endl;
						cout << "head: " << outputVec[0] << endl;
						cout << "yolk: " << outputVec[1] << endl << endl;
						voltage.volInput(0, 0);//Stop the stage while fish detection error.
						continue;//Do nothing else while fish detection error.
					}
					fish_direction = Point2d(fish_direction.x / shift_head2yolk, fish_direction.y / shift_head2yolk);//normalization

					if (fish_tr_history_c.empty())// If fish_tr_history_c is empty（这里应该改为长度不等于fish_history_length，并先clear，这样在长度变化时可以调节！！！）, initialize it with the data of the first frame.
						for (int i = 0; i <= fish_history_length; i++)
							fish_tr_history_c.push_back(Point2d(outputVec[0].x, outputVec[0].y));
					else// Push back the present data, and pop the data at the beginning
					{
						fish_tr_history_c.push_back(Point2d(outputVec[0].x, outputVec[0].y));
						vector<Point2d>::iterator it = fish_tr_history_c.begin();
						fish_tr_history_c.erase(it);
					}

					if (fish_direction_history_c.empty())// If fish_direction_history_c is empty（这里应该改为长度不等于fish_history_length，并先clear，这样在长度变化时可以调节！！！）, initialize it with the data of the first frame.
						for (int i = 0; i <= fish_history_length; i++)
							fish_direction_history_c.push_back(fish_direction);
					else// Push back the present data, and pop the data at the beginning
					{
						fish_direction_history_c.push_back(fish_direction);
						vector<Point2d>::iterator it = fish_direction_history_c.begin();
						fish_direction_history_c.erase(it);
					}

					QueryPerformanceCounter(&timeEnd4);
					double elapsed_history = (timeEnd4.QuadPart - timeEnd3.QuadPart) / quadpart;

					//MPC
					Point2d c0 = MPC_main(command_history_length, predict_length, fish_history_length,
						gammaX, gammaY, max_command, shift_head2yolk,
						scale_x, scale_y, theta, scale_x2, scale_y2, &dst_fish_position,
						command_history, position_history, fish_tr_history_c, fish_direction_history_c);
					voltage_x = c0.x;
					voltage_y = -c0.y;// Note the sign!!!

					QueryPerformanceCounter(&timeEnd5);
					double elapsed_MPC = (timeEnd5.QuadPart - timeEnd4.QuadPart) / quadpart;

					// Output the voltage
					voltage.volInput(voltage_x, voltage_y);
					//如果长度不等于command_history_length，应先clear，再重新初始化，这样在长度变化时可以调节！！！
					// Push back the present data, and pop the data at the beginning
					command_history.push_back(Point2d(voltage_x, voltage_y));
					vector<Point2d>::iterator it = command_history.begin();
					command_history.erase(it);

					// elapsed time
					QueryPerformanceCounter(&timeEnd);
					double elapsed = (timeEnd.QuadPart - timeStart.QuadPart) / quadpart;
					double elapsed_vector = (timeEnd.QuadPart - timeEnd5.QuadPart) / quadpart;

					// frameCount_processed
					frameCount_processed++;

					// output
					if (DEBUG_FLAG)
					{
						cout << "console coordinate: " << consoleread.coordata[0] << ", " << -consoleread.coordata[1] << endl;// Note the sign!!!
						cout << "head: " << outputVec[0] << endl;
						cout << "yolk: " << outputVec[1] << endl;
						cout << "voltage output: " << voltage_x << "," << voltage_y << endl;
						cout << "ConCoorRead elapsed time: " << elapsed_ConCoorRead << endl;
						cout << "Image convertion elapsed time: " << elapsed_convertion << endl;
						cout << "Image process elapsed time: " << elapsed_process << endl;
						cout << "History preparation elapsed time: " << elapsed_history << endl;
						cout << "MPC elapsed time: " << elapsed_MPC << endl;
						cout << "vector operation elapsed time: " << elapsed_vector << endl;
						cout << "elapsed time: " << elapsed << endl;
						cout << "frameCount processed: " << frameCount_processed << endl << endl;
					}
				}
			}
			else if (TRTflag == 2)
			{
				cout << "frameCount processed: " << frameCount_processed << endl << endl;
				cout << "break" << endl;
				break;
			}
		}
	}
	catch (BFException e)
	{
		e.showErrorMsg();

		// End application if an error occurs
		endTest = TRUE;

		return 1;
	}

	return 0;
}
